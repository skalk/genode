/*
 * \brief  Board definitions for Virtual Qemu platform
 * \author Stefan Kalkowski
 * \date   2019-05-10
 */

/*
 * Copyright (C) 2019 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _SRC__INCLUDE__HW__SPEC__ARM_64__VIRT_QEMU__BOARD_H_
#define _SRC__INCLUDE__HW__SPEC__ARM_64__VIRT_QEMU__BOARD_H_

#include <drivers/uart/pl011.h>
#include <hw/spec/arm/boot_info.h>

/* see qemu-3.1.0/hw/arm/virt.c */

namespace Hw::Virt_qemu_board {
	using Serial   = Genode::Pl011_uart;

	enum {
		RAM_BASE   = 0x40000000,
		RAM_SIZE   = 0x20000000, /* XXX qemu -m size ? */

		UART_BASE  = 0x9000000,
		UART_SIZE  = 0x1000,
		UART_CLOCK = 24000000,

		CACHE_LINE_SIZE_LOG2 = 6,
	};

	namespace Cpu_mmio {
		enum {
			IRQ_CONTROLLER_DISTR_BASE = 0x8000000,
			IRQ_CONTROLLER_DISTR_SIZE = 0x10000,

			IRQ_CONTROLLER_CPU_BASE = 0x8010000,
			IRQ_CONTROLLER_CPU_SIZE = 0x10000,

			IRQ_CONTROLLER_VT_CPU_BASE = 0x8040000,
			IRQ_CONTROLLER_VT_CPU_SIZE = 0x1000,

			IRQ_CONTROLLER_VT_CTRL_BASE = 0x8030000,
			IRQ_CONTROLLER_VT_CTRL_SIZE = 0x1000,

			IRQ_CONTROLLER_REDIST_BASE = 0x80a0000,
			IRQ_CONTROLLER_REDIST_SIZE = 0xF60000,

		};
	};
};

#endif /* _SRC__INCLUDE__HW__SPEC__ARM_64__VIRT_QEMU__BOARD_H_ */
