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

#include <browsable_dictionary.h>

namespace Driver {
	using namespace Genode;

	struct Dma_buffer;

	struct Cname {
		Ram_dataspace_capability const cap;

		bool operator > (Cname const &other) const {
			return cap.local_name() > other.cap.local_name(); }

		bool operator == (Cname const &other) const {
			return cap.local_name() == other.cap.local_name(); }

		void print(Output &out) const {
			Genode::print(out, cap.local_name()); }
	};
}


struct Driver::Dma_buffer : Cname, Dictionary<Dma_buffer, Cname>::Element
{
	struct Range {
		addr_t const start;
		size_t const size;
	} const phys_range;

	addr_t const dma_addr;

	Dma_buffer(Dictionary<Dma_buffer, Cname> &dict,
	           Ram_dataspace_capability const cap,
	           Range phys_range, addr_t dma_addr)
	:
		Cname(cap),
		Dictionary<Dma_buffer, Cname>::Element(dict, *this),
		phys_range(phys_range), dma_addr(dma_addr)
	{ }
};

#endif /* _SRC__DRIVER__PLATFORM__DMA_BUFFER_H_ */

