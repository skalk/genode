/*
 * \brief  Board driver for core
 * \author Stefan Kalkowski
 * \date   2017-04-27
 */

/*
 * Copyright (C) 2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _CORE__SPEC__ARNDALE__BOARD_H_
#define _CORE__SPEC__ARNDALE__BOARD_H_

/* base includes */
#include <drivers/defs/arndale.h>
#include <drivers/uart/exynos.h>

#include <hw/spec/arm/cortex_a15.h>

#include <kernel/irq.h>

namespace Board {
	using namespace Arndale;
	using Cpu_mmio = Hw::Cortex_a15_mmio<IRQ_CONTROLLER_BASE>;

	struct Serial : Genode::Exynos_uart, Kernel::Irq
	{
		Serial(Genode::addr_t mmio, unsigned c, unsigned b);

		void occurred();
	};

	enum {
		UART_BASE  = UART_2_MMIO_BASE,
		UART_CLOCK = UART_2_CLOCK,
	};

	static constexpr bool SMP = true;
}

#endif /* _CORE__SPEC__ARNDALE__BOARD_H_ */
