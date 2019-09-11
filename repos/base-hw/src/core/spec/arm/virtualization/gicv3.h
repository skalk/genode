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

#ifndef _CORE__SPEC__ARM__VIRTUALIZATION__GICV3_H_
#define _CORE__SPEC__ARM__VIRTUALIZATION__GICV3_H_

#include <hw/spec/arm/gicv3.h>

namespace Board { class Pic; };

class Board::Pic : public Hw::Pic
{
	public:

		struct Virtual_context {};

		void load(Virtual_context &) {}
		void save(Virtual_context &) {}

		void ack_virtual_irq() {}
		void insert_virtual_irq(Virtual_context &, unsigned) {}
		void disable_virtualization() {}
};

#endif /* _CORE__SPEC__ARM__VIRTUALIZATION__GICV3_H_ */

