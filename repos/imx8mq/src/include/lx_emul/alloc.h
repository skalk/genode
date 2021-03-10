/**
 * \brief  Lx_emul support to allocate memory
 * \author Stefan Kalkowski
 * \date   2021-03-25
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void * lx_emul_alloc(unsigned long size);
void * lx_emul_alloc_aligned(unsigned long size, unsigned long align);
void * lx_emul_alloc_dma(unsigned long size, void ** dma_addr);
void   lx_emul_free(const void * ptr);

#ifdef __cplusplus
}
#endif
