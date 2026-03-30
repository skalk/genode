/*
 * \brief  Expanding page table allocator
 * \author Johannes Schlatow
 * \author Stefan Kalkowski
 * \date   2023-10-18
 */

/*
 * Copyright (C) 2023 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _SRC__DRIVERS__PLATFORM__PAGE_TABLE_ALLOCATOR_H_
#define _SRC__DRIVERS__PLATFORM__PAGE_TABLE_ALLOCATOR_H_

/* Genode includes */
#include <base/attached_ram_dataspace.h>
#include <base/heap.h>
#include <base/tslab.h>
#include <cpu/page_table.h>
#include <pd_session/pd_session.h>
#include <util/dictionary.h>
#include <util/misc_math.h>

#include <types.h>

namespace Driver {
	using namespace Genode;

	class Page_table_allocator;
}


class Driver::Page_table_allocator
{
	public:

		static constexpr size_t PAGE_SIZE  = 1 << SIZE_LOG2_4KB;
		static constexpr size_t TABLE_SIZE = PAGE_SIZE;

		using Error  = Page_table_error;
		using Result = Attempt<Ok, Error>;

	private:

		struct Entry;
		struct Key;

		using Dictionary = Genode::Dictionary<Key, addr_t>;

		struct Key : Dictionary::Element
		{
			Entry &entry;

			Key(Dictionary &dict, addr_t addr, Entry &entry)
				: Dictionary::Element(dict, addr), entry(entry) {}
		};

		struct Entry;
		using List_element = Genode::List_element<Entry>;
		using List         = Genode::List<List_element>;

		struct Entry
		{
			Attached_ram_dataspace dataspace;

			addr_t const virt;
			addr_t const phys;

			Key v;
			Key p;

			List_element elem { this };

			Entry(Ram_allocator &ram,
			      Env::Local_rm &rm,
			      Pd_session    &pd,
			      Dictionary    &virt_dict,
			      Dictionary    &phys_dict)
			:
				dataspace(ram, rm, TABLE_SIZE, Genode::CACHED),
				virt((addr_t)dataspace.local_addr<void>()),
				phys(pd.dma_addr(dataspace.cap())),
				v(virt_dict, virt, *this),
				p(phys_dict, phys, *this) {}
		};

		Ram_allocator &_ram;
		Env::Local_rm &_rm;
		Pd_session    &_pd;

		static constexpr size_t SLAB_BLOCK_SIZE =
			PAGE_SIZE - Sliced_heap::meta_data_size();
		uint8_t _initial_sb_tables[SLAB_BLOCK_SIZE];
		Tslab<Entry, SLAB_BLOCK_SIZE> _alloc_tables;

		List _empty_list {};

		Dictionary _virt_dict {};
		Dictionary _phys_dict {};

		Result _alloc(auto const &fn)
		{
			List_element *le = _empty_list.first();

			if (le) {
				_empty_list.remove(le);
				Entry &entry = *le->object();
				fn(entry.virt, entry.phys);
				return Ok();
			}

			return _alloc_tables.try_alloc(sizeof(Entry)).convert<Result>(
				[&] (auto &res) -> Result {
					try {
						Entry &entry =
							*construct_at<Entry>(res.ptr, _ram, _rm, _pd,
							                     _virt_dict, _phys_dict);
						res.deallocate = false;
						fn(entry.virt, entry.phys);
						return Ok();
					} catch(Out_of_ram) {
						return Error::OUT_OF_RAM;
					} catch(Out_of_caps) {
						return Error::OUT_OF_CAPS;
					} catch(...) {
						return Error::DENIED;
					}
				},
				[] (Alloc_error e) {
					switch (e) {
					case Alloc_error::OUT_OF_CAPS: return Error::OUT_OF_CAPS;
					case Alloc_error::OUT_OF_RAM:  return Error::OUT_OF_RAM;
					case Alloc_error::DENIED:      break;
					};
					return Error::DENIED;
				});
		}

		static constexpr addr_t INVALID_ADDR = ~0UL;

	public:

		Page_table_allocator(Ram_allocator &ram, Env::Local_rm &rm,
		                     Pd_session &pd, Allocator &md_alloc)
		:
			_ram(ram), _rm(rm), _pd(pd),
			_alloc_tables(md_alloc, _initial_sb_tables) {}

		~Page_table_allocator()
		{
			while (_empty_list.first()) {
				List_element *le = _empty_list.first();
				_empty_list.remove(le);
				le->object()->~Entry();
				_alloc_tables.free(le->object(), sizeof(Entry));
			}
		}

		template <typename TABLE, typename FN1, typename FN2>
		void with_table(addr_t phys_addr, FN1 && match_fn, FN2 no_match_fn) const
		{
			if (phys_addr == INVALID_ADDR)
				return;

			 _phys_dict.with_element(phys_addr,
					[&] (Key const &k) { match_fn(*(TABLE*)k.entry.virt); },
					[&] () { no_match_fn(); });
		}

		template <typename TABLE> addr_t construct()
		{
			static_assert((sizeof(TABLE) == TABLE_SIZE), "unexpected size");

			addr_t phys_addr = INVALID_ADDR;

			auto result = _alloc([&] (addr_t, addr_t phys) {
				phys_addr = phys; });
			if (result.failed())
				error("Allocating new page-table failed!");

			return phys_addr;
		}

		template <typename TABLE> void destruct(addr_t phys_addr)
		{
			_phys_dict.with_element(phys_addr,
				[&] (Key &k) {
					((TABLE*)k.entry.virt)->~TABLE();
					_empty_list.insert(&k.entry.elem);
				},
				[] () { });
		}

		template <typename TABLE, typename ENTRY>
		Result create(typename ENTRY::access_t &descriptor)
		{
			return _alloc([&] (addr_t virt, addr_t phys) {
				construct_at<TABLE>((void*)virt);
				descriptor = ENTRY::create(phys);
			});
		}

		template <typename TABLE>
		void destroy(TABLE &table)
		{
			_virt_dict.with_element((addr_t)&table,
				[&] (Key &k) {
					table.~TABLE();
					_empty_list.insert(&k.entry.elem);
				},
				[] () { });
		}

		template <typename TABLE>
		Result lookup(addr_t phys_addr, auto const fn)
		{
			Result result = Error::INVALID_RANGE;
			with_table<TABLE>(phys_addr,
				[&] (TABLE &t) { result = fn(t); },
				[] () {});
			return result;
		}

		static Cost costs(size_t table_count)
		{
			/*
			 * measured overheads when allocating, attaching, and touching a lot of
			 * pages in a loop, taking the maximum across different kernels and
			 * slightly increased it to stay safe
			 */
			static constexpr size_t page_ram_overhead        = 1024;
			static constexpr size_t ten_pages_cap_overhead   = 12;
			static constexpr size_t single_page_cap_overhead = 2;

			size_t ram  = align_addr(table_count*(page_ram_overhead+TABLE_SIZE),
			                         { SIZE_LOG2_4KB });
			size_t caps = table_count / 10 * ten_pages_cap_overhead +
			              (table_count % 10) * single_page_cap_overhead;
			return { ram, caps };
		}
};

#endif /* _SRC__DRIVERS__PLATFORM__PAGE_TABLE_ALLOCATOR_H_ */
