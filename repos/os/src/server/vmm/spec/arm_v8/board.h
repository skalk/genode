/*
 * \brief  VMM address space utility
 * \author Stefan Kalkowski
 * \date   2019-11-13
 */

/*
 * Copyright (C) 2019 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _SRC__SERVER__VMM__BOARD_H_
#define _SRC__SERVER__VMM__BOARD_H_

namespace Vmm {

	enum {
		GIC_VERSION      = 3,
		GICD_MMIO_START  = 0x8000000,
		GICD_MMIO_SIZE   = 0x10000,
		GICC_MMIO_START  = 0x8010000,
		GICR_MMIO_START  = 0x80a0000,
		GICR_MMIO_SIZE   = 0xf60000,

		PL011_MMIO_START = 0x9000000,
		PL011_MMIO_SIZE  = 0x1000,
		PL011_IRQ        = 33,

		RAM_START        = 0x40000000,
		RAM_SIZE         = 128 * 1024 *1024,

		VTIMER_IRQ       = 27,

		MAX_CPUS         = 1,
	};
}

#endif /* _SRC__SERVER__VMM__BOARD_H_ */
