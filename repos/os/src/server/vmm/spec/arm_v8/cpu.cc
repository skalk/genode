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
#include <vmm.h>

Cpu::System_register::Iss::access_t
Cpu::System_register::Iss::value(unsigned op0, unsigned crn, unsigned op1,
                                 unsigned crm, unsigned op2)
{
	access_t v = 0;
	Crn::set(v, crn);
	Crm::set(v, crm);
	Opcode0::set(v, op0);
	Opcode1::set(v, op1);
	Opcode2::set(v, op2);
	return v;
};


Cpu::System_register::Iss::access_t
Cpu::System_register::Iss::mask_encoding(access_t v)
{
	return Crm::masked(v) |
	       Crn::masked(v) |
	       Opcode1::masked(v) |
	       Opcode2::masked(v) |
	       Opcode0::masked(v);
}


Cpu::System_register::System_register(unsigned         op0,
                                      unsigned         crn,
                                      unsigned         op1,
                                      unsigned         crm,
                                      unsigned         op2,
                                      const char     * name,
                                      bool             writeable,
                                      Genode::addr_t   v,
                                      Genode::Avl_tree<System_register> & tree)
: _encoding(Iss::value(op0, crn, op1, crm, op2)),
  _name(name),
  _writeable(writeable),
  _value(v)
{
	tree.insert(this);
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

	unsigned ce = Clidr::level(level, state.clidr_el1);

	if (ce == Clidr::NO_CACHE ||
	    (ce == Clidr::DATA_CACHE_ONLY && instr)) {
		Genode::warning("Invalid Csselr value!");
		return INVALID;
	}

	if (ce == Clidr::INSTRUCTION_CACHE_ONLY ||
	    (ce == Clidr::SEPARATE_CACHE && instr)) {
		Genode::log("Return Ccsidr instr value ", state.ccsidr_inst_el1[level]);
		return state.ccsidr_inst_el1[level];
	}

	Genode::log("Return Ccsidr value ", state.ccsidr_data_el1[level]);
	return state.ccsidr_data_el1[level];
}


Genode::addr_t Cpu::Ctr_el0::read() const
{
	Genode::addr_t ret;
	asm volatile("mrs %0, ctr_el0" : "=r" (ret));
	return ret;
}


bool Cpu::_handle_sys_reg()
{
	using Iss = System_register::Iss;

	Iss::access_t v = _state.esr_el2;
	System_register * reg = _reg_tree.first();
	if (reg) reg = reg->find_by_encoding(Iss::mask_encoding(v));

	if (!reg) {
		Genode::error("unknown system register access @ ip=", (void*)_state.ip, ":");
		Genode::error(Iss::Direction::get(v) ? "read" : "write",
		              ": "
		              "op0=",  Iss::Opcode0::get(v), " "
		              "op1=",  Iss::Opcode1::get(v), " "
		              "r",    Iss::Register::get(v),   " "
		              "crn=",  Iss::Crn::get(v),        " "
		              "crm=",  Iss::Crm::get(v), " ",
		              "op2=",  Iss::Opcode2::get(v));
		return false;
	}

	if (Iss::Direction::get(v)) { /* read access  */
		_state.r[Iss::Register::get(v)] = reg->read();
	} else {                      /* write access */
		if (!reg->writeable()) {
			Genode::error("writing to system register ",
			              reg->name(), " not allowed!");
			return false;
		}
		reg->write(_state.r[Iss::Register::get(v)]);
	}
	_state.ip += sizeof(Genode::uint32_t);
	return true;
}


void Cpu::_handle_wfi()
{
	if (_state.esr_el2 & 1)
		throw Exception("WFE not implemented yet");

	_active = false;
	_timer.schedule_timeout();
	_state.ip += sizeof(Genode::uint32_t);
}


void Cpu::_handle_sync()
{
	/* check device number*/
	switch (Esr::Ec::get(_state.esr_el2)) {
	case Esr::Ec::HVC:
		_vmm.handle_hyper_call();
		break;
	case Esr::Ec::MRS_MSR:
		_handle_sys_reg();
		break;
	case Esr::Ec::DA:
		_vmm.handle_data_abort((Genode::uint64_t)_state.hpfar_el2 << 8);
		break;
	case Esr::Ec::WFI:
		_handle_wfi();
		return;
	default:
		throw Exception("Unknown trap: %x",
		                Esr::Ec::get(_state.esr_el2));
	};
}


void Cpu::_handle_irq()
{
	enum { /* FIXME */ VT_TIMER_IRQ = 27 };
	switch (_state.irqs.last_irq) {
	case VT_TIMER_IRQ:
		_timer.handle_irq();
		break;
	default:
		_gic.handle_irq();
	};
}


void Cpu::_update_state()
{
	if (!_gic.pending_irq()) return;
	_active = true;
	_timer.cancel_timeout();
}


void Cpu::dump()
{
	using namespace Genode;

	auto lambda = [] (addr_t exc) {
		switch (exc) {
		case AARCH64_SYNC:   return "aarch64 sync";
		case AARCH64_IRQ:    return "aarch64 irq";
		case AARCH64_FIQ:    return "aarch64 fiq";
		case AARCH64_SERROR: return "aarch64 serr";
		case AARCH32_SYNC:   return "aarch32 sync";
		case AARCH32_IRQ:    return "aarch32 irq";
		case AARCH32_FIQ:    return "aarch32 fiq";
		case AARCH32_SERROR: return "aarch32 serr";
		default:             return "unknown";
		};
	};

	log("VM state (", _active ? "active" : "inactive", ") :");
	for (unsigned i = 0; i < 31; i++) {
		log("  r", i, "         = ",
		    Hex(_state.r[i], Hex::PREFIX, Hex::PAD));
	}
	log("  sp         = ", Hex(_state.sp,      Hex::PREFIX, Hex::PAD));
	log("  ip         = ", Hex(_state.ip,      Hex::PREFIX, Hex::PAD));
	log("  sp_el1     = ", Hex(_state.sp_el1,  Hex::PREFIX, Hex::PAD));
	log("  elr_el1    = ", Hex(_state.elr_el1, Hex::PREFIX, Hex::PAD));
	log("  pstate     = ", Hex(_state.pstate,  Hex::PREFIX, Hex::PAD));
	log("  exception  = ", _state.exception_type, " (",
	                       lambda(_state.exception_type), ")");
	log("  esr_el2    = ", Hex(_state.esr_el2, Hex::PREFIX, Hex::PAD));
	_timer.dump();
}


Cpu::Cpu(Vmm                     & vmm,
         Genode::Vm_connection   & vm,
         Gic                     & gic,
         Genode::Env             & env,
         Genode::Heap            & heap,
         Genode::Vm_handler_base & handler,
         Genode::addr_t            ip,
         Genode::addr_t            dtb)
: _vmm(vmm),
  _vm(vm),
  _heap(heap),
  _vcpu_id(_vm.create_vcpu(heap, env, handler)),
  _state(*((State*)env.rm().attach(_vm.cpu_state(_vcpu_id)))),
	//                op0, crn, op1, crm, op2, writeable, reset value
  _sr_id_aa64afr0_el1 (3, 0, 0, 5, 4, "ID_AA64AFR0_EL1",  false, 0x0,                     _reg_tree),
  _sr_id_aa64afr1_el1 (3, 0, 0, 5, 5, "ID_AA64AFR1_EL1",  false, 0x0,                     _reg_tree),
  _sr_id_aa64dfr0_el1 (3, 0, 0, 5, 0, "ID_AA64DFR0_EL1",  false, 0xf0f0f006,              _reg_tree),
  _sr_id_aa64dfr1_el1 (3, 0, 0, 5, 1, "ID_AA64DFR1_EL1",  false, 0x0,                     _reg_tree),
  _sr_id_aa64isar0_el1(3, 0, 0, 6, 0, "ID_AA64ISAR0_EL1", false, _state.id_aa64isar0_el1, _reg_tree),
  _sr_id_aa64isar1_el1(3, 0, 0, 6, 1, "ID_AA64ISAR1_EL1", false, _state.id_aa64isar1_el1, _reg_tree),
  _sr_id_aa64mmfr0_el1(3, 0, 0, 7, 0, "ID_AA64MMFR0_EL1", false, _state.id_aa64mmfr0_el1, _reg_tree),
  _sr_id_aa64mmfr1_el1(3, 0, 0, 7, 1, "ID_AA64MMFR1_EL1", false, _state.id_aa64mmfr1_el1, _reg_tree),
  _sr_id_aa64mmfr2_el1(3, 0, 0, 7, 2, "ID_AA64MMFR2_EL1", false, _state.id_aa64mmfr2_el1, _reg_tree),
  _sr_id_aa64pfr0_el1 (_state.id_aa64pfr0_el1, _reg_tree),
  _sr_id_aa64pfr1_el1 (3, 0, 0, 4, 1, "ID_AA64PFR1_EL1",  false, 0x0,                     _reg_tree),
  _sr_id_aa64zfr0_el1 (3, 0, 0, 4, 4, "ID_AA64ZFR0_EL1",  false, 0x0,                     _reg_tree),
  _sr_aidr_el1        (3, 0, 1, 0, 7, "AIDR_EL1",         false, 0x0,                     _reg_tree),
  _sr_revidr_el1      (3, 0, 0, 0, 6, "REVIDR_EL1",       false, 0x0,                     _reg_tree),

  _sr_clidr_el1       (3, 0, 1, 0, 2, "CLIDR_EL1",        false, _state.clidr_el1,        _reg_tree),
  _sr_csselr_el1      (3, 0, 2, 0, 0, "CSSELR_EL1",       true,  0x0,                     _reg_tree),
  _sr_ctr_el0         (_reg_tree),
  _sr_ccsidr_el1      (_sr_csselr_el1, _state, _reg_tree),

  //_sr_pmccfiltr_el0   (3, 14, 3, 15, 7, "PMCCFILTR_EL0",  true,  0x0,                     _reg_tree),
  _sr_pmuserenr_el0   (3, 9, 3, 14, 0,  "PMUSEREN_EL0",   true,  0x0,                     _reg_tree),

  _sr_mdscr           (2, 0, 0, 2, 2, "MDSCR_EL1",        true,  0x0,                     _reg_tree),
  _sr_osdlr           (2, 1, 0, 3, 4, "OSDLR_EL1",        true,  0x0,                     _reg_tree),
  _sr_oslar           (2, 1, 0, 0, 4, "OSLAR_EL1",        true,  0x0,                     _reg_tree),

  _gic(*this, gic),
  _timer(env, *_gic.ppi[11], *this)
{
	/*
	 * Dummy debug register only used on QEMU,
	 * because of incomplete trap behaviour
	 */
	for (unsigned i = 0; i < 16; i++) {
		_sr_dbgbvr[i].construct(i, _reg_tree);
		_sr_dbgbcr[i].construct(i, _reg_tree);
		_sr_dbgwcr[i].construct(i, _reg_tree);
		_sr_dbgwvr[i].construct(i, _reg_tree);
	}

	_state.r[0]   = dtb;
	_state.ip     = ip;
	_state.pstate = 0b1111000101; /* el1 mode and IRQs disabled */
}
