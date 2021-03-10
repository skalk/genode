/**
 * \brief  Lx_kit memory allocation backend
 * \author Stefan Kalkowski
 * \date   2021-03-25
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

/* Genode includes */
#include <base/log.h>
#include <os/backtrace.h>
#include <platform_session/connection.h>

/* local includes */
#include <lx_kit/memory.h>


Lx_kit::Mem_allocator::Mem_allocator(Genode::Env          & env,
                                     Heap                 & heap,
                                     Platform::Connection & platform)
: _env(env), _heap(heap), _platform(platform), _mem(&heap) {}


void * Lx_kit::Mem_allocator::alloc(size_t size, size_t align)
{
	if (!size) { return nullptr; }

	void * out_addr = nullptr;
	if (_mem.alloc_aligned(size, &out_addr, log2(align)).error()) {
		size_t                   ds_size = align_addr(size, 12);
		Ram_dataspace_capability ds_cap  = _env.ram().alloc(ds_size);
		try {
			void * addr = _env.rm().attach(ds_cap);
			_mem.add_range((addr_t)addr, ds_size);
		} catch (Out_of_caps) {
			_env.ram().free(ds_cap);
			throw;
		}
		_mem.alloc_aligned(size, &out_addr, log2(align));
	}

	if (!out_addr) {
		error("memory allocation failed for ", size, " align ", align);
		backtrace();
	}
	else { memset(out_addr, 0, size); }
	return out_addr;
}


void * Lx_kit::Mem_allocator::alloc_dma(size_t  size,
                                        void ** dma_addr)
{
	if (!size) { return nullptr; }

	Ram_dataspace_capability cap = _platform.alloc_dma_buffer(size, UNCACHED);
	Dma_buffer & db = *new (_heap)
		Dma_buffer{{}, _env.rm().attach(cap), size, cap, _platform.dma_addr(cap)};
	_dma_buffers.insert(&db);
	*dma_addr = (void*)db.dma_addr;
	return (void*) db.addr;
}


void Lx_kit::Mem_allocator::free(const void * ptr)
{
	if (_mem.valid_addr((addr_t)ptr)) {
		_mem.free(const_cast<void*>(ptr));
		return;
	}

	addr_t addr = (addr_t) ptr;
	for (Dma_buffer * b = _dma_buffers.first(); b; b = b->next()) {

		if (!(b->addr <= addr && (b->addr+b->size) >= addr)) { continue; }

		_env.rm().detach(b->addr);
		_platform.free_dma_buffer(b->cap);
		return;
	}
}
