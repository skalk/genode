/*
 * \brief   ARMv8 cpu context initialization
 * \author  Stefan Kalkowski
 * \date    2017-04-12
 */

/*
 * Copyright (C) 2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* base includes */
#include <util/bit_allocator.h>
#include <cpu/memory_barrier.h>

/* base-hw core includes */
#include <board.h>
#include <kernel/thread.h>

using namespace Core;


Cpu::Context::Context(bool privileged)
{
	Spsr::El::set(pstate, privileged ? 1 : 0);
}


bool Cpu::active(Mmu_context & mmu_context)
{
	return (mmu_context.id() == Ttbr::Asid::get(Ttbr0_el1::read()));
}


void Cpu::switch_to(Mmu_context & mmu_context)
{
	Ttbr0_el1::write(mmu_context.ttbr);
}


void Cpu::mmu_fault(Cpu::Context &, Kernel::Thread_fault & fault)
{
	Esr::access_t esr = Esr_el1::read();

	fault.addr = Far_el1::read();

	switch (Esr::Iss::Abort::Fsc::get(Esr::Iss::get(esr))) {
	case Esr::Iss::Abort::Fsc::TRANSLATION:
		fault.type = Kernel::Thread_fault::PAGE_MISSING;
		return;
	case Esr::Iss::Abort::Fsc::PERMISSION:
		fault.type = Esr::Iss::Abort::Write::get(Esr::Iss::get(esr))
			? Kernel::Thread_fault::WRITE : Kernel::Thread_fault::EXEC;
		return;
	default:
		raw("MMU-fault not handled ESR=", Hex(esr));
		fault.type = Kernel::Thread_fault::UNKNOWN;
	};
}


Cpu::Mmu_context::Mmu_context(addr_t table,
                              Board::Address_space_id_allocator &id_alloc)
:
	_addr_space_id_alloc(id_alloc),
	ttbr(Ttbr::Baddr::masked(table))
{
	Ttbr::Asid::set(ttbr, (uint16_t)_addr_space_id_alloc.alloc());
}


Cpu::Mmu_context::~Mmu_context()
{
	_addr_space_id_alloc.free(id());
}


size_t Cpu::cache_line_size()
{
	static size_t cache_line_size = 0;

	if (!cache_line_size) {
		size_t i = 1 << Ctr_el0::I_min_line::get(Ctr_el0::read());
		size_t d = 1 << Ctr_el0::D_min_line::get(Ctr_el0::read());
		cache_line_size = min(i,d) * 4; /* word size is fixed in ARM */
	}
	return cache_line_size;
}


template <typename FUNC>
static inline void cache_maintainance(addr_t const base,
                                      size_t const size,
                                      FUNC & func)
{
	/* align the start address to catch all related cache lines */
	addr_t start     = (addr_t)base & ~(Cpu::cache_line_size()-1UL);
	addr_t const end = base + size;
	for (; start < end; start += Cpu::cache_line_size()) func(start);
}


void Cpu::cache_coherent_region(addr_t const base, size_t const size)
{
	memory_barrier();

	auto lambda = [] (addr_t const base) {
		asm volatile("dc cvau, %0" :: "r" (base));
		asm volatile("dsb ish");
		asm volatile("ic ivau, %0" :: "r" (base));
		asm volatile("dsb ish");
		asm volatile("isb");
	};

	cache_maintainance(base, size, lambda);
}


void Cpu::cache_clean_invalidate_data_region(addr_t const base,
                                             size_t const size)
{
	memory_barrier();

	auto lambda = [] (addr_t const base) {
		asm volatile("dc civac, %0" :: "r" (base)); };

	cache_maintainance(base, size, lambda);

	asm volatile("dsb sy");
	asm volatile("isb");
}


void Cpu::cache_invalidate_data_region(addr_t const base, size_t const size)
{
	memory_barrier();

	auto lambda = [] (addr_t const base) {
		asm volatile("dc ivac, %0" :: "r" (base)); };

	cache_maintainance(base, size, lambda);

	asm volatile("dsb sy");
	asm volatile("isb");
}


void Cpu::clear_memory_region(addr_t const addr, size_t const size,
                              bool changed_cache_properties)
{
	memory_barrier();

	/* normal memory is cleared by D-cache zeroing */
	auto normal = [] (addr_t const base) {
		asm volatile("dc zva,  %0" :: "r" (base));
		asm volatile("ic ivau, %0" :: "r" (base));
	};

	/* DMA memory gets additionally evicted from the D-cache */
	auto dma = [] (addr_t const base) {
		asm volatile("dc zva,   %0" :: "r" (base));
		asm volatile("dc civac, %0" :: "r" (base));
		asm volatile("ic ivau,  %0" :: "r" (base));
	};

	if (changed_cache_properties) {
		cache_maintainance(addr, size, dma);
	} else {
		cache_maintainance(addr, size, normal);
	}

	asm volatile("dsb ish");
	asm volatile("isb");
}


void Cpu::dump(Genode::Cpu_state & state)
{
	using namespace Genode;

	log("");
	log("Dump of CPU state:");
	log("");
	for (unsigned i = 0; i < 31; i++)
		log("  X", i, ": ", Hex(state.r[i]));
	log("  SP: ", Hex(state.sp));
	log("  IP: ", Hex(state.ip));
	log("  ESR_EL1: ", Hex(Esr_el1::read()));
	log("  FAR_EL1: ", Hex(Far_el1::read()));
	log("  ACTLR_EL1: ", Hex(Actlr_el1::read()));
	log("  MAIR_EL1: ", Hex(Mair_el1::read()));
	log("  SCTLR_EL1: ", Hex(Sctlr_el1::read()));
	log("  TCR_EL1: ", Hex(Tcr_el1::read()));
	log("  TTBR0_EL1: ", Hex(Ttbr0_el1::read()));
	log("  TTBR1_EL1: ", Hex(Ttbr1_el1::read()));
}
