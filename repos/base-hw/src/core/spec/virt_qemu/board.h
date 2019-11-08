/*
 * \brief  Board driver for core
 * \author Alexander Boettcher
 * \date   2019-05-10
 */

/*
 * Copyright (C) 2019 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _CORE__SPEC__VIRT_QEMU__BOARD_H_
#define _CORE__SPEC__VIRT_QEMU__BOARD_H_

#include <hw/spec/arm_64/virt_qemu_board.h>
#include <spec/arm/virtualization/gicv3.h>
#include <spec/arm/generic_timer.h>

namespace Board {
	using namespace Hw::Virt_qemu_board;

	enum {
		TIMER_IRQ           = 14 + 16,
		VT_TIMER_IRQ        = 11 + 16,
		VT_MAINTAINANCE_IRQ = 9  + 16
	};
};

#endif /* _CORE__SPEC__VIRT_QEMU__BOARD_H_ */
