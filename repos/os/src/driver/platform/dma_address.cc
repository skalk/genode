/*
 * \brief  Very simple DMA address range allocator
 * \author Stefan Kalkowski
 * \date   2026-01-14
 *
 * WARNING: DO NOT COPY IT!!!
 * This simple kind of range allocator is inefficient, and open to
 * fragmentation. It is for the limited use-case in the platform driver
 * useful only, and get replaced whenever a general useful alternative
 * is available.
 */

/*
 * Copyright (C) 2026 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#include <dma_address.h>

using Driver::Dma_address;
using Driver::Dma_address_list;
using Driver::Dma_address_allocator;
using Driver::Dma_reservation;

Dma_address::~Dma_address()
{
	_alloc_list.remove(&_alloc_le);
	_list.remove(&_list_le);
}


Dma_reservation::Dma_reservation(Dma_address_list &list, Range range)
:
	Dma_address(list, list, range)
{
	list._insert(_list_le);
}


bool Dma_address_list::_insert(Le &le)
{
	auto *prev = (Le*) nullptr;

	for (auto *cur = first(); cur; cur = cur->next()) {
		if (le.object()->end < cur->object()->start)
			break;

		if (cur->object()->conflicts(*le.object()))
			return false;

		prev = cur;
	}

	insert(&le, prev);
	return true;
}


void Dma_address_allocator::reserve(Range r,
                                    Constructible<Dma_reservation> &res)
{
	res.construct(_addr_list, Range(r.start, r.end));

	if (!_addr_list._insert(res->_alloc_le)) {
		res.destruct();
		return;
	}
}


void Dma_address_allocator::alloc(size_t size,
                                  Align align,
                                  Constructible<Dma_address> &addr,
                                  Dma_address_list &list)
{
	using Le = List_element<Dma_address>;

	addr_t start = _start;

	Le *prev = nullptr;
	for (Le *cur = _addr_list.first(); cur; cur = cur->next()) {
		if (start+size-1 < cur->object()->start)
			break;
		start = align_addr(cur->object()->end, align);
		prev = cur;
	}

	/* enough space till end of address space ? */
	if ((_end-start+1) < size)
		return;

	addr.construct(_addr_list, list, Range(start, start+size-1));
	_addr_list.insert(&addr->_alloc_le, prev);
	list._insert(addr->_list_le);
}
