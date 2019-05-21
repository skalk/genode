/*
 * \brief   Startup code for bootstrap
 * \author  Stefan Kalkowski
 * \date    2019-05-11
 */

/*
 * Copyright (C) 2019 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

.section ".text.crt0"

	.global _start
	_start:

	/****************
	 ** Enable FPU **
	 ****************/

	mov x0, #0b11
	lsl x0, x0, #20
	msr cpacr_el1, x0


	/***********************
	 ** Detect CPU number **
	 ***********************/

	mrs x0, mpidr_el1
	and x0, x0, #0b11111111
	cmp x0, #0
	bne __endless_sleep


	/***************************
	 ** Zero-fill BSS segment **
	 ***************************/

	ldr x0, =_bss_local_start
	ldr x1, =_bss_local_end
	1:
	cmp x1, x0
	ble 2f
	str xzr, [x0]
	add x0, x0, #8
	b   1b
	2:


	/************************************
	 ** Jump to high-level entry point **
	 ************************************/

	ldr x0, =_start_stack        /* load stack address  into x0 */
	msr vbar_el1, x0
	mov sp, x0
	bl init

	__endless_sleep:
		wfe
		1: b 1b

	_bss_local_start:
	.quad _bss_start

	_bss_local_end:
	.quad _bss_end

	.p2align 4
	.space 0x4000
	_start_stack:

	.rept 512
	nop
	.endr
	mrs x24, far_el1
	mrs x25, ttbr0_el1
	mrs x26, ttbr1_el1
	mrs x27, elr_el1
	mrs x28, esr_el1
	1: b 1b

