/*
 * \brief   Platform implementations specific for base-hw and i.MX8Q EVK
 * \author  Stefan Kalkowski
 * \date    2019-06-12
 */

/*
 * Copyright (C) 2019 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#include <platform.h>


void Board::Cpu::wake_up_all_cpus(void * ip)
{
	enum Function_id { CPU_ON = 0xC4000003 };

	unsigned long result = 0;
	for (unsigned i = 1; i < NR_OF_CPUS; i++) {
		asm volatile("mov x0, %1  \n"
		             "mov x1, %2  \n"
		             "mov x2, %3  \n"
		             "mov x3, %2  \n"
		             "smc #0      \n"
		             "mov %0, x0  \n"
		             : "=r" (result) : "r" (CPU_ON), "r" (i), "r" (ip)
		                      : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
		                        "x8", "x9", "x10", "x11", "x12", "x13", "x14");
	}
}
