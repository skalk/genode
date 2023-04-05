/*
 * \brief  Kernel backend for execution contexts in userland
 * \author Adrian-Ken Rueegsegger
 * \author Reto Buerki
 * \author Stefan Kalkowski
 * \date   2015-02-09
 */

/*
 * Copyright (C) 2015-2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* core includes */
#include <kernel/cpu.h>
#include <kernel/thread.h>

using namespace Kernel;


void Thread::exception(Cpu & cpu)
{
	using Genode::Cpu_state;

	switch (regs->trapno) {

	case Cpu_state::PAGE_FAULT:
		_mmu_exception();
		return;

	case Cpu_state::UNDEFINED_INSTRUCTION:
		Genode::raw(*this, ": undefined instruction at ip=", (void*)regs->ip);
		_die();
		return;

	case Cpu_state::SUPERVISOR_CALL:
		_call();
		return;
	}

	if (regs->trapno >= Cpu_state::INTERRUPTS_START &&
	    regs->trapno <= Cpu_state::INTERRUPTS_END) {
		_interrupt(_user_irq_pool, cpu.id());
		return;
	}

	Genode::raw(*this, ": triggered unknown exception ", regs->trapno,
	            " with error code ", regs->errcode, " at ip=", (void*)regs->ip, " sp=", (void*)regs->sp);

	_die();
}


void Thread::panic()
{
	using namespace Genode;
	using Genode::Cpu_state;

	log("");
	switch (regs->trapno) {

	case Cpu_state::PAGE_FAULT:
		log("Exception reason: page-fault (address=", Hex(Cpu::Cr2::read()), ")");
		break;

	case Cpu_state::UNDEFINED_INSTRUCTION:
		log("Exception reason: undefined instruction ");
		break;

	case Cpu_state::SUPERVISOR_CALL:
		log("Exception reason: syscall (number=", regs->rax, ")");
		break;
	}

	if (regs->trapno >= Cpu_state::INTERRUPTS_START &&
	    regs->trapno <= Cpu_state::INTERRUPTS_END)
		log("Exception reason: interrupt");

	log("");
	log("Register dump");
	log("-------------");
	log("ip     = ", Hex(regs->ip));
	log("sp     = ", Hex(regs->sp));
	log("cs     = ", Hex(regs->cs));
	log("ss     = ", Hex(regs->ss));
	log("eflags = ", Hex(regs->eflags));
	log("rax    = ", Hex(regs->rax));
	log("rbx    = ", Hex(regs->rbx));
	log("rcx    = ", Hex(regs->rcx));
	log("rdx    = ", Hex(regs->rdx));
	log("rdi    = ", Hex(regs->rdi));
	log("rsi    = ", Hex(regs->rsi));
	log("rbp    = ", Hex(regs->rbp));
}
