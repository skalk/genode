/**
 * \brief  Lx_emul backend for memory allocation
 * \author Stefan Kalkowski
 * \date   2021-03-22
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#include <lx_emul/alloc.h>
#include <lx_kit/env.h>

extern "C" void * lx_emul_alloc_aligned(unsigned long size, unsigned long align)
{
	return Lx_kit::env().memory.alloc(size, align);
};


extern "C" void * lx_emul_alloc(unsigned long size)
{
	/* always align memory objects to 32 bytes, like malloc, heap etc. */
	return Lx_kit::env().memory.alloc(size, 32);
};


extern "C" void * lx_emul_alloc_dma(unsigned long size,
                                    void       ** dma_addr)
{
	return Lx_kit::env().memory.alloc_dma(size, dma_addr);
};


extern "C" void lx_emul_free(const void * ptr)
{
	Lx_kit::env().memory.free(ptr);
};
