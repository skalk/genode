/*
 * \brief   Platform implementations specific for base-hw and Raspberry Pi3
 * \author  Stefan Kalkowski
 * \date    2019-05-11
 */

/*
 * Copyright (C) 2019 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#include <util/mmio.h>
#include <platform.h>

/**
 * Leave out the first page (being 0x0) from bootstraps RAM allocator,
 * some code does not feel happy with addresses being zero
 */
Bootstrap::Platform::Board::Board()
: early_ram_regions(Memory_region { ::Board::RAM_BASE + 0x1000,
                                    ::Board::RAM_SIZE - 0x1000 }),
  late_ram_regions(Memory_region { ::Board::RAM_BASE, 0x1000 }),
  core_mmio(Memory_region { ::Board::UART_BASE, ::Board::UART_SIZE },
            Memory_region { ::Board::LOCAL_IRQ_CONTROLLER_BASE,
                            ::Board::LOCAL_IRQ_CONTROLLER_SIZE },
            Memory_region { ::Board::IRQ_CONTROLLER_BASE,
                            ::Board::IRQ_CONTROLLER_SIZE }) {}


extern unsigned int _crt0_qemu_start_secondary_cpus;

struct Pic : Genode::Mmio
{
	template <unsigned CPU_NUM>
		struct Core_mailbox_set : Register<0x8c+CPU_NUM*0x10, 32> {};

	Pic(void * ip) : Genode::Mmio(Board::LOCAL_IRQ_CONTROLLER_BASE)
	{
		write<Core_mailbox_set<1>>((unsigned long)ip);
		write<Core_mailbox_set<2>>((unsigned long)ip);
		write<Core_mailbox_set<3>>((unsigned long)ip);
	}
};

void Board::Cpu::wake_up_all_cpus(void * ip)
{
	_crt0_qemu_start_secondary_cpus = 1;

	::Pic pic(ip);
	asm volatile("dsb #15; sev");
}
