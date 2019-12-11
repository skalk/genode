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

/**
 * Leave out the first page (being 0x0) from bootstraps RAM allocator,
 * some code does not feel happy with addresses being zero
 */
Bootstrap::Platform::Board::Board()
: early_ram_regions(Memory_region { ::Board::RAM_BASE, ::Board::RAM_SIZE }),
  late_ram_regions(Memory_region { }),
  core_mmio(Memory_region { ::Board::UART_BASE, ::Board::UART_SIZE },
            Memory_region { ::Board::Cpu_mmio::IRQ_CONTROLLER_DISTR_BASE,
                            ::Board::Cpu_mmio::IRQ_CONTROLLER_DISTR_SIZE },
            Memory_region { ::Board::Cpu_mmio::IRQ_CONTROLLER_REDIST_BASE,
                            ::Board::Cpu_mmio::IRQ_CONTROLLER_REDIST_SIZE })
{
	::Board::Pic pic {};

#if 0
	static volatile unsigned long initial_values[][2] {
		// (IOMUX Controller)
		{ 0x30340038, 0x49409600 },
		{ 0x30340040, 0x49409200 },
		{ 0x30330064, 0x6 },
		{ 0x30330140, 0x0 },
		{ 0x30330144, 0x0 },
		{ 0x30330148, 0x0 },
		{ 0x3033014c, 0x0 },
		{ 0x30330150, 0x0 },
		{ 0x30330154, 0x0 },
		{ 0x30330158, 0x0 },
		{ 0x30330180, 0x2 },
		{ 0x30330184, 0x0 },
		{ 0x30330188, 0x0 },
		{ 0x3033018c, 0x0 },
		{ 0x30330190, 0x0 },
		{ 0x30330194, 0x0 },
		{ 0x30330198, 0x0 },
		{ 0x3033019c, 0x0 },
		{ 0x303301a0, 0x0 },
		{ 0x303301a4, 0x0 },
		{ 0x303301a8, 0x0 },
		{ 0x303301ac, 0x0 },
		{ 0x303301bc, 0x0 },
		{ 0x303301c0, 0x0 },
		{ 0x303301c4, 0x0 },
		{ 0x303301c8, 0x0 },
		{ 0x303301e8, 0x0 },
		{ 0x303301ec, 0x0 },
		{ 0x303301fc, 0x1 },
		{ 0x30330200, 0x1 },
		{ 0x3033021c, 0x10 },
		{ 0x30330220, 0x10 },
		{ 0x30330224, 0x10 },
		{ 0x30330228, 0x10 },
		{ 0x3033022c, 0x12 },
		{ 0x30330230, 0x12 },
		{ 0x30330244, 0x0 },
		{ 0x30330248, 0x0 },
		{ 0x3033029c, 0x19 },
		{ 0x303302a4, 0x19 },
		{ 0x303302a8, 0x19 },
		{ 0x303302b0, 0xd6 },
		{ 0x303302c0, 0x4f },
		{ 0x303302cc, 0x59 },
		{ 0x30330308, 0x9f },
		{ 0x3033030c, 0xdf },
		{ 0x30330310, 0xdf },
		{ 0x30330314, 0xdf },
		{ 0x30330318, 0xdf },
		{ 0x3033031c, 0xdf },
		{ 0x30330320, 0xdf },
		{ 0x30330324, 0xdf },
		{ 0x30330328, 0xdf },
		{ 0x3033032c, 0xdf },
		{ 0x30330334, 0x9f },
		{ 0x3033033c, 0x9f },
		{ 0x30330340, 0xdf },
		{ 0x30330344, 0xdf },
		{ 0x30330348, 0xdf },
		{ 0x3033034c, 0xdf },
		{ 0x30330350, 0xdf },
		{ 0x30330368, 0x59 },
		{ 0x30330370, 0x19 },
		{ 0x3033039c, 0x19 },
		{ 0x303303a0, 0x19 },
		{ 0x303303a4, 0x19 },
		{ 0x303303a8, 0xd6 },
		{ 0x303303ac, 0xd6 },
		{ 0x303303b0, 0xd6 },
		{ 0x303303b4, 0xd6 },
		{ 0x303303b8, 0xd6 },
		{ 0x303303bc, 0xd6 },
		{ 0x303303c0, 0xd6 },
		{ 0x303303e8, 0xd6 },
		{ 0x303303ec, 0xd6 },
		{ 0x303303f0, 0xd6 },
		{ 0x303303f4, 0xd6 },
		{ 0x303303f8, 0xd6 },
		{ 0x303303fc, 0xd6 },
		{ 0x30330400, 0xd6 },
		{ 0x30330404, 0xd6 },
		{ 0x30330408, 0xd6 },
		{ 0x3033040c, 0xd6 },
		{ 0x30330410, 0xd6 },
		{ 0x30330414, 0xd6 },
		{ 0x30330424, 0xd6 },
		{ 0x30330428, 0xd6 },
		{ 0x3033042c, 0xd6 },
		{ 0x30330430, 0xd6 },
		{ 0x30330450, 0xd6 },
		{ 0x30330454, 0xd6 },
		{ 0x30330464, 0x49 },
		{ 0x30330468, 0x49 },
		{ 0x3033046c, 0x16 },
		{ 0x30330484, 0x67 },
		{ 0x30330488, 0x67 },
		{ 0x3033048c, 0x67 },
		{ 0x30330490, 0x67 },
		{ 0x30330494, 0x76 },
		{ 0x30330498, 0x76 },
		{ 0x3033049c, 0x49 },
		{ 0x303304a0, 0x49 },
		{ 0x303304ac, 0x49 },
		{ 0x303304b0, 0x49 },
		{ 0x303304c8, 0x1 },
		{ 0x303304cc, 0x4 },
		{ 0x30330500, 0x1 },
		{ 0x30330504, 0x2 },
		// (Global Power Controller)
		{ 0x303a0030, 0xeb22de22 },
		{ 0x303a0034, 0xfffff1c7 },
		{ 0x303a0038, 0x7bffbc00 },
		{ 0x303a003c, 0xfa3bf12a },
		{ 0x303a004c, 0xffffdfff },
		{ 0x303a01b4, 0x3980 },
		{ 0x303a01c4, 0xffff7fff },
		{ 0x303a01cc, 0xffffbfff },
		{ 0x303a01d4, 0xffff7fff },
		{ 0x303a01dc, 0xffff7fff },
		{ 0x303a01fc, 0x10fff9f },
		// Oscillator settings
		{ 0x30270000, 0x18020F0 },
		{ 0x30270004, 0x0 },
		{ 0x30278000, 0x18020F0 },
		{ 0x30278004, 0x0 },
		{ 0x30278000, 0x18020F0 },
		{ 0x30278004, 0x0 },
		// (Clock Controller Module)
		{ 0x30384170, 0x0 },
		{ 0x30384220, 0x0 },
		{ 0x30384250, 0x3 },
		{ 0x303842f0, 0x0 },
		{ 0x303843a0, 0x3 },
		{ 0x30384510, 0x0 },
		{ 0x30384520, 0x0 },
		{ 0x30384540, 0x0 },
		{ 0x30384550, 0x0 },
		{ 0x30384560, 0x0 },
		{ 0x30388030, 0x11000400 },
		{ 0x303880a0, 0x10000000 },
		{ 0x303880b0, 0x1100 },
		{ 0x30388100, 0x1000000 },
		{ 0x30388180, 0x1000000 },
		{ 0x303881b0, 0x1000100 },
		{ 0x30388200, 0x1000000 },
		{ 0x30388230, 0x1000100 },
		{ 0x30360000, 0x88080 },
		{ 0x30360004, 0x292a2fa6 },
		{ 0x30360008, 0x88080 },
		{ 0x3036000c, 0x10385ba3 },
		{ 0x30360010, 0x98080 },
		{ 0x30360014, 0x3fffff1a },
		{ 0x30360018, 0x88081 },
		{ 0x30360054, 0x2b9 }
	};

	unsigned num_values = sizeof(initial_values) / (2*sizeof(unsigned long));
	for (unsigned i = 0; i < num_values; i++)
		*((volatile Genode::uint32_t*)initial_values[i][0]) = (Genode::uint32_t)initial_values[i][1];

	enum Function_id { CPU_FREQ = 0xC2000001, CPU_FREQ_SET = 0 };

	unsigned long result = 0;
	asm volatile("mov x0, %1  \n"
	             "mov x1, %2  \n"
	             "mov x2, %3  \n"
	             "mov x3, %4  \n"
	             "smc #0      \n"
	             "mov %0, x0  \n"
	             : "=r" (result) : "r" (CPU_FREQ), "r" (CPU_FREQ_SET), "r" (0), "r" (1500000000)
	                      : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
	                        "x8", "x9", "x10", "x11", "x12", "x13", "x14");
	Genode::log("cpu frequency set returned ", result);
#endif
}


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
