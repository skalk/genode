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

#include <vm.h>

using Vmm::Vm;

void Vm::_load_kernel()
{
	Genode::memcpy((void*)(_ram.local() + KERNEL_OFFSET),
	               _kernel_rom.local_addr<void>(),
	               _kernel_rom.size());
}

void Vm::_load_dtb()
{
	Genode::memcpy((void*)(_ram.local() + DTB_OFFSET),
	               _dtb_rom.local_addr<void>(),
	               _dtb_rom.size());
}


void Vm::_load_initrd()
{
	Genode::memcpy((void*)(_ram.local() + INITRD_OFFSET),
	               _initrd_rom.local_addr<void>(),
	               _initrd_rom.size());
}


void Vm::handle_hyper_call()
{
	Genode::warning("unknown hypercall!");
	_cpu.dump();
}


void Vm::handle_data_abort()
{
	_bus.handle_memory_access(_cpu);
	_cpu.state().ip += sizeof(Genode::uint32_t);
}


Vm::Vm(Genode::Env & env)
: _vm(env),
  _kernel_rom(env, "linux"),
  _dtb_rom(env, "dtb"),
  _initrd_rom(env, "initrd"),
  _vm_ram(env.ram(), env.rm(), RAM_SIZE, Genode::UNCACHED),
  _ram(RAM_ADDRESS, RAM_SIZE, (Genode::addr_t)_vm_ram.local_addr<void>()),
  _heap(env.ram(), env.rm()),
  _vm_handler(_cpu, env.ep(), *this, &Vm::_handle),
  _gic("Gicv2", 0x8000000, 0x1000, env),
  _cpu(*this, _vm, _gic, env, _heap, _vm_handler,
       _ram.base() + KERNEL_OFFSET,
       _ram.base() + DTB_OFFSET),
  _uart("Pl011", 0x9000000, 0x1000, 33, _cpu, env)
{
	_bus.add(_gic);
	_bus.add(_uart);
	_vm.attach(_vm_ram.cap(), RAM_ADDRESS);
	_vm.attach_pic(0x8010000);
	_load_kernel();
	_load_dtb();
	_load_initrd();

	Genode::log("Start virtual machine ...");

	_cpu.run();
};
