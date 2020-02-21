/*
 * \brief   Imx7 Sabrelite specific board definitions
 * \author  Stefan Kalkowski
 * \date    2019-06-01
 */

/*
 * Copyright (C) 2019 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _SRC__BOOTSTRAP__SPEC__VIRT_QEMU__BOARD_H_
#define _SRC__BOOTSTRAP__SPEC__VIRT_QEMU__BOARD_H_

#include <hw/spec/arm/virt_qemu_board.h>
#include <hw/spec/arm/lpae.h>
#include <spec/arm/cpu.h>
#include <hw/spec/arm/gicv2.h>

namespace Board {
	using namespace Hw::Virt_qemu_board;
	using Pic = Hw::Gicv2;
	static constexpr bool NON_SECURE = true;
}

#endif /* _SRC__BOOTSTRAP__SPEC__IMX&_SABRELITE__BOARD_H_ */
