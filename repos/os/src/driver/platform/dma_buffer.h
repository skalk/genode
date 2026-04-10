/*
 * \brief  Platform driver - DMA buffer
 * \author Stefan Kalkowski
 * \date   2026-03-16
 */

/*
 * Copyright (C) 2026 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _SRC__DRIVER__PLATFORM__DMA_BUFFER_H_
#define _SRC__DRIVER__PLATFORM__DMA_BUFFER_H_

/* Genode includes */
#include <base/allocator_avl.h>
#include <base/ram.h>

#include <dma_address.h>
#include <browsable_dictionary.h>
namespace Driver {
	using namespace Genode;

	struct Dma_buffer_base;
	struct Dma_buffer_name;
	struct Dma_buffer;
}

struct Driver::Dma_buffer_base
{
	Ram::Constrained_allocator::Result allocation;

	Dma_buffer_base(Ram::Constrained_allocator &ram_alloc,
	                size_t size, Cache cache)
	: allocation(ram_alloc.try_alloc(size, cache)) {}

	bool ok() const { return allocation.ok(); }

	Ram_dataspace_capability _cap() const {
		return allocation.convert<Ram_dataspace_capability>(
			[] (auto &a) { return a.cap; },
			[] (auto) { return Ram_dataspace_capability(); }); }
};


struct Driver::Dma_buffer_name
{
	Ram_dataspace_capability const cap;

	bool operator > (Dma_buffer_name const &other) const {
		return cap.local_name() > other.cap.local_name(); }

	bool operator == (Dma_buffer_name const &other) const {
		return cap.local_name() == other.cap.local_name(); }

	void print(Output &out) const {
		Genode::print(out, cap.local_name()); }
};


struct Driver::Dma_buffer
:
	Dma_buffer_base,
	Dma_buffer_name,
	Dictionary<Dma_buffer, Dma_buffer_name>::Element
{

	struct Range {
		addr_t const start;
		size_t const size;
	} const phys_range;

	Constructible<Dma_address> address {};

	bool const remapable;

	addr_t dma_addr() const
	{
		return !remapable ? phys_range.start
		                  : (address.constructed() ? address->start : ~0UL);
	}

	using Constructed = Genode::Attempt<Ok, Alloc_error>;
	Constructed constructed() const
	{
		if (ok()) {
			if (!remapable || address.constructed())
				return Ok();
			else
				return Alloc_error::DENIED;
		}

		return allocation.convert<Alloc_error>(
			[] (auto&) { return Alloc_error::DENIED; /* impossible */ },
			[] (auto e) { return e; });
	};

	Dma_buffer(Dictionary<Dma_buffer, Dma_buffer_name> &dict,
	           Ram::Constrained_allocator &ram_alloc,
	           size_t size, Cache cache,
	           Pd_session &pd,
	           Dma_address_allocator &addr_alloc,
	           Dma_address_list &addr_list,
	           bool remapable)
	:
		Dma_buffer_base(ram_alloc, size, cache),
		Dma_buffer_name(_cap()),
		Dictionary<Dma_buffer, Dma_buffer_name>::Element(dict, *this),
		phys_range(ok() ? pd.dma_addr(cap) : 0,
		           ok() ? pd.ram_size(cap) : 0),
		remapable(remapable)
	{
		if (ok()) addr_alloc.alloc(phys_range.size, {12}, address, addr_list);
	}
};

#endif /* _SRC__DRIVER__PLATFORM__DMA_BUFFER_H_ */
