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
#include <cpu.h>
#include <vm.h>
#include <psci.h>

using Vmm::Cpu_base;
using Vmm::Cpu;
using Vmm::Gic;

Genode::uint64_t Cpu_base::State::reg(unsigned idx) const
{
	if (idx > 15) return 0;

	Genode::uint32_t * r = (Genode::uint32_t*)this;
	r += idx;
	return *r;
}


void Cpu_base::State::reg(unsigned idx, Genode::uint64_t v)
{
	if (idx > 15) return;

	Genode::uint32_t * r = (Genode::uint32_t*)this;
	r += idx;
	*r = v;
}


Cpu_base::System_register::Iss::access_t
Cpu_base::System_register::Iss::value(unsigned op0, unsigned crn, unsigned op1,
                                      unsigned crm, unsigned op2)
{
	access_t v = 0;
	Crn::set(v, crn);
	Crm::set(v, crm);
	Opcode1::set(v, op1);
	Opcode2::set(v, op2);
	return v;
};


Cpu_base::System_register::Iss::access_t
Cpu_base::System_register::Iss::mask_encoding(access_t v)
{
	return Crm::masked(v) |
	       Crn::masked(v) |
	       Opcode1::masked(v) |
	       Opcode2::masked(v);
}


void Cpu_base::_handle_brk()
{
	Genode::error(__func__, " not implemented yet");
}


void Cpu_base::handle_exception()
{
	Genode::log("VM EXC ", _state.cpu_exception, " ", (void*)_state.ip, " ", _state.esr_el2);
	/* check exception reason */
	switch (_state.cpu_exception) {
	case Cpu::NO_EXCEPTION:                 break;
	case Cpu::FIQ:          [[fallthrough]];
	case Cpu::IRQ:          _handle_irq();  break;
	case Cpu::TRAP:         _handle_sync(); break;
	default:
		throw Exception("Curious exception ",
		                _state.cpu_exception, " occured");
	}
	_state.cpu_exception = Cpu::NO_EXCEPTION;
}


void Cpu_base::dump()
{
	using namespace Genode;

	auto lambda = [] (unsigned i) {
		switch (i) {
		case 0:   return "und";
		case 1:   return "svc";
		case 2:   return "abt";
		case 3:   return "irq";
		case 4:   return "fiq";
		default:  return "unknown";
		};
	};

	log("VM state (", _active ? "active" : "inactive", ") :");
	for (unsigned i = 0; i < 13; i++) {
		log("  r", i, "         = ",
		    Hex(_state.reg(i), Hex::PREFIX, Hex::PAD));
	}
	log("  sp         = ", Hex(_state.sp,      Hex::PREFIX, Hex::PAD));
	log("  lr         = ", Hex(_state.lr,      Hex::PREFIX, Hex::PAD));
	log("  ip         = ", Hex(_state.ip,      Hex::PREFIX, Hex::PAD));
	log("  cpsr       = ", Hex(_state.cpsr,    Hex::PREFIX, Hex::PAD));
	for (unsigned i = 0; i < State::Mode_state::MAX; i++) {
		log("  sp_", lambda(i), "     = ",
			Hex(_state.mode[i].sp, Hex::PREFIX, Hex::PAD));
		log("  lr_", lambda(i), "     = ",
			Hex(_state.mode[i].lr, Hex::PREFIX, Hex::PAD));
		log("  spsr_", lambda(i), "   = ",
			Hex(_state.mode[i].spsr, Hex::PREFIX, Hex::PAD));
	}
	log("  exception  = ", _state.cpu_exception);
	log("  esr_el2    = ", Hex(_state.esr_el2,   Hex::PREFIX, Hex::PAD));
	log("  hpfar_el2  = ", Hex(_state.hpfar_el2, Hex::PREFIX, Hex::PAD));
	log("  far_el2    = ", Hex(_state.far_el2,   Hex::PREFIX, Hex::PAD));
	log("  hifar      = ", Hex(_state.hifar,     Hex::PREFIX, Hex::PAD));
	log("  dfsr       = ", Hex(_state.dfsr,      Hex::PREFIX, Hex::PAD));
	log("  ifsr       = ", Hex(_state.ifsr,      Hex::PREFIX, Hex::PAD));
	log("  sctrl      = ", Hex(_state.sctrl,     Hex::PREFIX, Hex::PAD));
	_timer.dump();
}


void Cpu_base::initialize_boot(Genode::addr_t ip, Genode::addr_t dtb)
{
	state().reg(1, 0xffffffff); /* invalid machine type */
	state().reg(2, dtb);
	state().ip = ip;
}


Genode::addr_t Cpu::Ccsidr::read() const
{
	struct Clidr : Genode::Register<32>
	{
		enum Cache_entry {
			NO_CACHE,
			INSTRUCTION_CACHE_ONLY,
			DATA_CACHE_ONLY,
			SEPARATE_CACHE,
			UNIFIED_CACHE
		};

		static unsigned level(unsigned l, access_t reg) {
			return (reg >> l*3) & 0b111; }
	};

	struct Csselr : Genode::Register<32>
	{
		struct Instr : Bitfield<0, 1> {};
		struct Level : Bitfield<1, 4> {};
	};

	enum { INVALID = 0xffffffff };

	unsigned level = Csselr::Level::get(csselr.read());
	bool     instr = Csselr::Instr::get(csselr.read());

	if (level > 6) {
		Genode::warning("Invalid Csselr value!");
		return INVALID;
	}

	// Kernel: v = 0xa200023 CLIDR
	// Kernel: 0 = 0x700fe01a (data)  0x203fe009 (instr)
	// Kernel: 1 = 0x707fe03a (data)


//	unsigned ce = Clidr::level(level, state.clidr_el1);
//
//	if (ce == Clidr::NO_CACHE ||
//	    (ce == Clidr::DATA_CACHE_ONLY && instr)) {
//		Genode::warning("Invalid Csselr value!");
//		return INVALID;
//	}
//
//	if (ce == Clidr::INSTRUCTION_CACHE_ONLY ||
//	    (ce == Clidr::SEPARATE_CACHE && instr)) {
//		Genode::log("Return Ccsidr instr value ", state.ccsidr_inst_el1[level]);
//		return state.ccsidr_inst_el1[level];
//	}
//
//	Genode::log("Return Ccsidr value ", state.ccsidr_data_el1[level]);
//	return state.ccsidr_data_el1[level];
	return 0;
}


Cpu::Cpu(Vm                      & vm,
         Genode::Vm_connection   & vm_session,
         Mmio_bus                & bus,
         Gic                     & gic,
         Genode::Env             & env,
         Genode::Heap            & heap,
         Genode::Entrypoint      & ep)
: Cpu_base(vm, vm_session, bus, gic, env, heap, ep),
  _sr_midr   (0, 0, 0, 0, "MIDR",   false, 0x410f0000,     _reg_tree),
  _sr_mpidr  (0, 0, 0, 5, "MPIDR",  false, 1<<31|cpu_id(), _reg_tree),
  _sr_csselr (0, 2, 0, 0, "CSSELR", true, 0,               _reg_tree),
  _sr_ccsidr (_sr_csselr, _reg_tree)
{
	_state.cpsr  = 0x93; /* el1 mode and IRQs disabled */
	_state.sctrl = 0xc50078;
}
