/*
 * \brief   Kernel backend for execution contexts in userland
 * \author  Stefan Kalkowski
 * \date    2019-05-11
 */

/*
 * Copyright (C) 2019 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#include <kernel/cpu.h>
#include <kernel/kernel.h>
#include <kernel/pd.h>
#include <kernel/thread.h>

extern "C" void kernel_to_user_context_switch(void *, void *);

using namespace Kernel;

void Thread::exception(Cpu &)
{
	static bool initialized = false;
	if (!initialized) {
		initialized = true;
		return;
	}

	Cpu::Esr::access_t esr = Cpu::Esr_el1::read();
	switch (Cpu::Esr::Ec::get(esr)) {
	case Cpu::Esr::Ec::SVC:
		_call();
		return;
	case Cpu::Esr::Ec::INST_ABORT_SAME_LEVEL: [[fallthrough]];
	case Cpu::Esr::Ec::DATA_ABORT_SAME_LEVEL:
		Genode::raw("Fault in kernel/core ESR=", Genode::Hex(esr));
		[[fallthrough]];
	case Cpu::Esr::Ec::INST_ABORT_LOW_LEVEL:  [[fallthrough]];
	case Cpu::Esr::Ec::DATA_ABORT_LOW_LEVEL:
		_mmu_exception();
		return;
	default:
		Genode::log("Unknown cpu exception EC=", Cpu::Esr::Ec::get(esr),
		            " ISS=", Cpu::Esr::Iss::get(esr),
		            " ip=", (void*)regs->ip);
	};
	while (1) { ; }
}


/**
 * on ARM with multiprocessing extensions, maintainance operations on TLB,
 * and caches typically work coherently across CPUs when using the correct
 * coprocessor registers (there might be ARM SoCs where this is not valid,
 * with several shareability domains, but until now we do not support them)
 */
void Kernel::Thread::Pd_update::execute() { };


bool Kernel::Pd::update(Cpu &)
{
	Cpu::tlb_invalidate_by_asid(0);
	return false;
}


void Kernel::Thread::_call_update_data_region()
{
	Genode::raw(__func__, " not implemented yet!");
}


void Kernel::Thread::_call_update_instr_region()
{
	addr_t const base = (addr_t)user_arg_1();
	size_t const size = (size_t)user_arg_2();
	Cpu::clean_data_cache_by_virt_region(base, size);
	Cpu::invalidate_instr_cache_by_virt_region(base, size);
}


void Thread::proceed(Cpu & cpu)
{
	cpu.switch_to(*regs, pd().mmu_regs);
	kernel_to_user_context_switch((static_cast<Cpu::Context*>(&*regs)),
	                              (void*)cpu.stack_start());
}


void Thread::user_ret_time(Kernel::time_t const t)  { regs->r[0] = t;   }
void Thread::user_arg_0(Kernel::Call_arg const arg) { regs->r[0] = arg; }
void Thread::user_arg_1(Kernel::Call_arg const arg) { regs->r[1] = arg; }
void Thread::user_arg_2(Kernel::Call_arg const arg) { regs->r[2] = arg; }
void Thread::user_arg_3(Kernel::Call_arg const arg) { regs->r[3] = arg; }
void Thread::user_arg_4(Kernel::Call_arg const arg) { regs->r[4] = arg; }

Kernel::Call_arg Thread::user_arg_0() const { return regs->r[0]; }
Kernel::Call_arg Thread::user_arg_1() const { return regs->r[1]; }
Kernel::Call_arg Thread::user_arg_2() const { return regs->r[2]; }
Kernel::Call_arg Thread::user_arg_3() const { return regs->r[3]; }
Kernel::Call_arg Thread::user_arg_4() const { return regs->r[4]; }