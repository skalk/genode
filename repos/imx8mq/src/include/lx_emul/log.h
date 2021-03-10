/**
 * \brief  Lx_emul support to log messages from the kernel code
 * \author Stefan Kalkowski
 * \date   2021-03-24
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

void lx_emul_vprintf(char const *, va_list);

#ifdef __cplusplus
}
#endif


