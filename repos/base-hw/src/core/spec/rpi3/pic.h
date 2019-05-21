/*
 * \brief  Programmable interrupt controller for core
 * \author Stefan Kalkowski
 * \date   2019-05-27
 */

/*
 * Copyright (C) 2019 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _CORE__SPEC__RPI3__PIC_H_
#define _CORE__SPEC__RPI3__PIC_H_

namespace Genode { class Pic; }
namespace Kernel { using Pic = Genode::Pic; }

class Genode::Pic
{
	public:

		enum { IPI = 1, NR_OF_IRQ = 1 };

		Pic() {}

		void send_ipi(unsigned const) { }
		void send_ipi() { }
		static constexpr bool fast_interrupts() { return false; }
		bool take_request(unsigned &) { return false; }
		void finish_request() { }
		void unmask(unsigned const, unsigned const) { }
		void mask(unsigned const) { }
};

#endif /* _CORE__SPEC__RPI3__PIC_H_ */
