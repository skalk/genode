/*
 * \brief  DMA address range management
 * \author Stefan Kalkowski
 * \date   2026-01-14
 */

/*
 * Copyright (C) 2026 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _SRC__DRIVER__DMA_ADDRESS_H_
#define _SRC__DRIVER__DMA_ADDRESS_H_

/* Genode includes */
#include <base/allocator.h>
#include <util/list.h>

namespace Driver {
	using namespace Genode;

	class Dma_address;
	class Dma_reservation;
	class Dma_address_list;
	class Dma_address_allocator;
}


class Driver::Dma_address : public Range_allocator::Range, Noncopyable
{
	protected:

		friend class Dma_address_allocator;

		List_element<Dma_address> _alloc_le { this };
		List_element<Dma_address> _list_le  { this };

		Dma_address_list &_alloc_list;
		Dma_address_list &_list;

	public:

		Dma_address(Dma_address_list &alloc_list,
		            Dma_address_list &list,
		            Range range)
		:
			Range_allocator::Range(range),
			_alloc_list(alloc_list),
			_list(list) { }

		~Dma_address();

		bool conflicts(Range_allocator::Range &r) const {
			return !(r.start > end || r.end < start); }

		void print(Output &output) const
		{
			Genode::print(output, "Dma_address(", Hex(start),
			              ", ", Hex(end), ")");
		}
};


struct Driver::Dma_reservation : Dma_address
{
	Dma_reservation(Dma_address_list &list, Range range);
};


class Driver::Dma_address_list : List<List_element<Dma_address>>
{
	private:

		friend class Dma_address;
		friend class Dma_reservation;
		friend class Dma_address_allocator;

		using Le = List_element<Dma_address>;

		bool _insert(Le &le);

	public:

		void for_each(auto const &fn) const
		{
			for (auto *cur = first(); cur; cur = cur->next())
				fn(*cur->object());
		}
};


/**
 * This simple address range allocator doesn't keep additional meta-data,
 * like free blocks or similar apart from the actual allocations the caller
 * is responsible for. Therefore, we can keep address ranges of different
 * stakeholders in one structure, and every party can account its allocations
 * appropriatedly.
 */
class Driver::Dma_address_allocator : Genode::Noncopyable
{
	public:

		using Range = Range_allocator::Range;

	private:

		Dma_address_list _addr_list {};

		addr_t _start { 0x1000 };

		/*
		 * We limit the address range to 4GB by now,
		 * some devices only support 32bit
		 */
		addr_t _end { 0xffffffff };

		/*
		 * Interrupt address range is special handled and in general not
		 * usable for normal DMA translations, see chapter 3.15
		 * of "Intel Virtualization Technology for Directed I/O"
		 * (March 2023, Revision 4.1)
		 */
		Dma_reservation _irq_reservation { _addr_list,
		                                   { 0xfee00000, 0xfeefffff } };

	public:

		void reserve(Range r, Constructible<Dma_reservation> &);

		void alloc(size_t size, Align align, Constructible<Dma_address> &addr,
		           Dma_address_list &list);
};

#endif /* _SRC__DRIVER__DMA_ADDRESS_H_ */
