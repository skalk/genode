/*
 * \brief  VMM cpu object
 * \author Stefan Kalkowski
 * \date   2019-07-18
 */

/*
 * Copyright (C) 2019 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _SRC__SERVER__VMM__CPU_H_
#define _SRC__SERVER__VMM__CPU_H_

#include <cpu_base.h>

namespace Vmm { class Cpu; }

class Vmm::Cpu : public Vmm::Cpu_base
{
	public:

		Cpu(Vm                      & vm,
		    Genode::Vm_connection   & vm_session,
		    Mmio_bus                & bus,
		    Gic                     & gic,
		    Genode::Env             & env,
		    Genode::Heap            & heap,
		    Genode::Entrypoint      & ep);

		enum Exception_type {
			NO_EXCEPTION,
			RESET,
			UNDEFINED,
			HVC,
			PF_ABORT,
			DATA_ABORT,
			IRQ,
			FIQ,
			TRAP
		};

	private:

		/******************************
		 ** Identification registers **
		 ******************************/

		System_register _sr_midr;
		System_register _sr_mpidr;
};

#endif /* _SRC__SERVER__VMM__CPU_H_ */
