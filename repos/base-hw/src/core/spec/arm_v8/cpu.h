/*
 * \brief  CPU driver for core
 * \author Stefan Kalkowski
 * \date   2019-05-10
 */

/*
 * Copyright (C) 2019 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _CORE__SPEC__ARM_V8__CPU_H_
#define _CORE__SPEC__ARM_V8__CPU_H_

/* Genode includes */
#include <util/register.h>
#include <cpu/cpu_state.h>
#include <base/internal/align_at.h>
#include <hw/spec/arm_64/cpu.h>

namespace Kernel { struct Thread_fault; }

namespace Genode {
	struct Cpu;
	using sizet_arithm_t = __uint128_t;
	using uint128_t      = __uint128_t;
}

struct Genode::Cpu : Hw::Arm_64_cpu
{
	struct alignas(16) Fpu_state
	{
		Genode::uint128_t q[32];
	};

	struct alignas(8) Context : Cpu_state
	{
		Genode::uint64_t pstate { };
		Fpu_state        fpu_state { };

		Context(bool privileged);
	};

	struct Mmu_context
	{
		Ttbr::access_t ttbr;

		Mmu_context(addr_t page_table_base);
		~Mmu_context();

		Genode::uint16_t id() {
			return Ttbr::Asid::get(ttbr); }
	};

	void switch_to(Context&, Mmu_context &);

	static void mmu_fault(Context &, Kernel::Thread_fault &);

	/**
	 * Return kernel name of the executing CPU
	 */
	static unsigned executing_id() { return 0; }


	static void clean_data_cache_by_virt_region(addr_t, size_t);
	static void invalidate_instr_cache_by_virt_region(addr_t, size_t);
};

#endif /* _CORE__SPEC__ARM_V8__CPU_H_ */
