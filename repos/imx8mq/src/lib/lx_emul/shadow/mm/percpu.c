/**
 * \brief  Replaces mm/percpu.c
 * \author Stefan Kalkowski
 * \date   2021-03-16
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#include <linux/percpu.h>
#include <lx_emul/alloc.h>

void __percpu * __alloc_percpu(size_t size, size_t align)
{
	return lx_emul_alloc_aligned(size, align);
}
