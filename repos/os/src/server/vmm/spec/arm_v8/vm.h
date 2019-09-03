/*
 * \brief  VMM example for ARMv8 virtualization
 * \author Stefan Kalkowski
 * \date   2019-07-18
 */

/*
 * Copyright (C) 2019 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _SRC__SERVER__VMM__VM_H_
#define _SRC__SERVER__VMM__VM_H_

#include <ram.h>
#include <exception.h>
#include <cpu.h>
#include <gic.h>
#include <pl011.h>

#include <base/attached_ram_dataspace.h>
#include <base/attached_rom_dataspace.h>
#include <vm_session/connection.h>

namespace Vmm { class Vm; }

class Vmm::Vm
{
	private:

		enum {
			RAM_ADDRESS   = 0x40000000,
			RAM_SIZE      = 128 * 1024 *1024,
			KERNEL_OFFSET = 0x80000,
			INITRD_OFFSET = 32 * 1024 * 1024,
			DTB_OFFSET    = 64 * 1024 * 1024,
		};

		Genode::Vm_connection          _vm;
		Genode::Attached_rom_dataspace _kernel_rom;
		Genode::Attached_rom_dataspace _dtb_rom;
		Genode::Attached_rom_dataspace _initrd_rom;
		Genode::Attached_ram_dataspace _vm_ram;
		Ram                            _ram;
		Genode::Heap                   _heap;
		Cpu::Signal_handler<Vm>        _vm_handler;
		Mmio_bus                       _bus;
		Gic                            _gic;
		Cpu                            _cpu;
		Pl011                          _uart;

		void _load_kernel();
		void _load_dtb();
		void _load_initrd();
		void _handle() {} /* dummy handler */

	public:

		Vm(Genode::Env & env);

		void  handle_data_abort();
		void  handle_hyper_call();
		Cpu & cpu() { return _cpu; }
};

#endif /* _SRC__SERVER__VMM__VM_H_ */
