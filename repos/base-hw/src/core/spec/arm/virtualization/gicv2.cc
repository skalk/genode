/*
 * \brief  Gicv2 with virtualization extensions
 * \author Stefan Kalkowski
 * \date   2019-09-02
 */

/*
 * Copyright (C) 2019 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#include <util/mmio.h>

#include <platform.h>
#include <spec/arm/virtualization/gicv2.h>

using Board::Pic;

Pic::Gich::Gich()
: Genode::Mmio(Genode::Platform::mmio_to_virt(Board::Cpu_mmio::IRQ_CONTROLLER_VT_CTRL_BASE)) { }


void Pic::load(Pic::Virtual_context & context)
{
	_gich.write<Gich::Gich_misr  >(context.misr);
	_gich.write<Gich::Gich_vmcr  >(context.vmcr);
	_gich.write<Gich::Gich_apr   >(context.apr);
	_gich.write<Gich::Gich_elrsr0>(context.elrsr);
	_gich.write<Gich::Gich_lr0   >(context.lr);
	_gich.write<Gich::Gich_hcr>(0b1);
}


void Pic::save(Pic::Virtual_context & context)
{
	context.misr   = _gich.read<Gich::Gich_misr  >();
	context.vmcr   = _gich.read<Gich::Gich_vmcr  >();
	context.apr    = _gich.read<Gich::Gich_apr   >();
	context.eisr   = _gich.read<Gich::Gich_eisr0 >();
	context.elrsr  = _gich.read<Gich::Gich_elrsr0>();
	context.lr     = _gich.read<Gich::Gich_lr0   >();
}


void Pic::ack_virtual_irq()
{
	_gich.write<Gich::Gich_lr0>(0);
	_gich.write<Gich::Gich_elrsr0>(_gich.read<Gich::Gich_elrsr0>() | 1);
	_gich.write<Gich::Gich_misr>(0);
}


void Pic::insert_virtual_irq(Pic::Virtual_context & c, unsigned irq)
{
	enum { SPURIOUS = 1023 };

	if (irq != SPURIOUS && !c.lr) {
		c.elrsr &= 0x7ffffffe;
		c.lr     = irq | 1 << 28 | 1 << 19;
	}
}


void Pic::disable_virtualization()
{
	_gich.write<Gich::Gich_hcr>(0);
}
