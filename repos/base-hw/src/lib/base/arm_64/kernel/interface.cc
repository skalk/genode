/*
 * \brief  Interface between kernel and userland
 * \author Stefan Kalkowski
 * \date   2019-05-09
 */

/*
 * Copyright (C) 2019 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Genode includes */
#include <kernel/interface.h>

using namespace Kernel;


/************************************
 ** Helper macros for kernel calls **
 ************************************/

#define CALL_1_FILL_ARG_REGS \
	register Call_arg arg_0_reg asm("x0") = arg_0;

#define CALL_2_FILL_ARG_REGS \
	CALL_1_FILL_ARG_REGS \
	register Call_arg arg_1_reg asm("x1") = arg_1;

#define CALL_3_FILL_ARG_REGS \
	CALL_2_FILL_ARG_REGS \
	register Call_arg arg_2_reg asm("x2") = arg_2;

#define CALL_4_FILL_ARG_REGS \
	CALL_3_FILL_ARG_REGS \
	register Call_arg arg_3_reg asm("x3") = arg_3;

#define CALL_5_FILL_ARG_REGS \
	CALL_4_FILL_ARG_REGS \
	register Call_arg arg_4_reg asm("x4") = arg_4;

#define CALL_6_FILL_ARG_REGS \
	CALL_5_FILL_ARG_REGS \
	register Call_arg arg_5_reg asm("x5") = arg_5;

#define CALL_1_SWI "svc 0\n" : "+r" (arg_0_reg)
#define CALL_2_SWI CALL_1_SWI:  "r" (arg_1_reg)
#define CALL_3_SWI CALL_2_SWI,  "r" (arg_2_reg)
#define CALL_4_SWI CALL_3_SWI,  "r" (arg_3_reg)
#define CALL_5_SWI CALL_4_SWI,  "r" (arg_4_reg)
#define CALL_6_SWI CALL_5_SWI,  "r" (arg_5_reg)

bool hw_debug_kernel_trace_syscalls = true;

#if defined(__aarch64__)
#include <base/log.h>
static inline bool running_in_kernel()
{
	int var = 0;
	addr_t stack = (addr_t)&var;

	/* check for user stack */
	if (stack < 0xffffffc000000000UL)
		return false;

	/* check for core thread stack */
	if (stack >= 0xffffffe000000000UL &&
	    stack <  0xffffffe010000000UL)
		return false;

	return true;
}

static inline void sanity_check()
{
	if (hw_debug_kernel_trace_syscalls && running_in_kernel()) {
		Genode::log("=== Kernel backtrace start ===");
		addr_t * fp;

		asm volatile ("mov %0, x29" : "=r"(fp) ::);

		while (fp) {
			addr_t ip = fp[1];
			fp = (addr_t*) fp[0];
			Genode::log(Genode::Hex(ip));
		}
		Genode::log("=== Kernel backtrace end ===");
	}
}
#else
#define sanity_check()
#endif


/******************
 ** Kernel calls **
 ******************/

Call_ret_64 Kernel::call64(Call_arg arg_0)
{
	sanity_check();
	return Kernel::call(arg_0);
}


Call_ret Kernel::call(Call_arg arg_0)
{
	sanity_check();
	CALL_1_FILL_ARG_REGS
	asm volatile(CALL_1_SWI);
	return arg_0_reg;
}


Call_ret Kernel::call(Call_arg arg_0,
                      Call_arg arg_1)
{
	sanity_check();
	CALL_2_FILL_ARG_REGS
	asm volatile(CALL_2_SWI);
	return arg_0_reg;
}


Call_ret Kernel::call(Call_arg arg_0,
                      Call_arg arg_1,
                      Call_arg arg_2)
{
	sanity_check();
	CALL_3_FILL_ARG_REGS
	asm volatile(CALL_3_SWI);
	return arg_0_reg;
}


Call_ret Kernel::call(Call_arg arg_0,
                      Call_arg arg_1,
                      Call_arg arg_2,
                      Call_arg arg_3)
{
	sanity_check();
	CALL_4_FILL_ARG_REGS
	asm volatile(CALL_4_SWI);
	return arg_0_reg;
}


Call_ret Kernel::call(Call_arg arg_0,
                      Call_arg arg_1,
                      Call_arg arg_2,
                      Call_arg arg_3,
                      Call_arg arg_4)
{
	sanity_check();
	CALL_5_FILL_ARG_REGS
	asm volatile(CALL_5_SWI);
	return arg_0_reg;
}


Call_ret Kernel::call(Call_arg arg_0,
                      Call_arg arg_1,
                      Call_arg arg_2,
                      Call_arg arg_3,
                      Call_arg arg_4,
                      Call_arg arg_5)
{
	sanity_check();
	CALL_6_FILL_ARG_REGS
	asm volatile(CALL_6_SWI);
	return arg_0_reg;
}
