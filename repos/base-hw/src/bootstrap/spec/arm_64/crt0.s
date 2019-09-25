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

	/***********************
	 ** Detect CPU number **
	 ***********************/

	mrs x0, mpidr_el1
	and x0, x0, #0b11111111
	cbz x0, _crt0_fill_bss_zero
	wfe
	b _start


	/***************************
	 ** Zero-fill BSS segment **
	 ***************************/

	_crt0_fill_bss_zero:
	ldr x0, =_bss_start
	ldr x1, =_bss_end
	1:
	cmp x1, x0
	b.eq _crt0_enable_fpu
	str xzr, [x0], #8
	b 1b


	/****************
	 ** Enable FPU **
	 ****************/

	_crt0_enable_fpu:
	mov x0, #0b11
	lsl x0, x0, #20
	msr cpacr_el1, x0


	/***********************************
	 ** Invalidate data/unified cache **
	 ***********************************/

	MRS X0, CLIDR_EL1
	AND W3, W0, #0x07000000  // Get 2 x Level of Coherence
	LSR W3, W3, #23
	CBZ W3, Finished
	MOV W10, #0              // W10 = 2 x cache level
	MOV W8, #1               // W8 = constant 0b1
	Loop1: ADD W2, W10, W10, LSR #1 // Calculate 3 x cache level
	LSR W1, W0, W2           // extract 3-bit cache type for this level
	AND W1, W1, #0x7
	CMP W1, #2
	B.LT Skip                // No data or unified cache at this level
	MSR CSSELR_EL1, X10      // Select this cache level
	ISB                      // Synchronize change of CSSELR
	MRS X1, CCSIDR_EL1       // Read CCSIDR
	AND W2, W1, #7           // W2 = log2(linelen)-4
	ADD W2, W2, #4           // W2 = log2(linelen)
	UBFX W4, W1, #3, #10     // W4 = max way number, right aligned
	CLZ W5, W4               /* W5 = 32-log2(ways), bit position of way in DC 
								 operand */
	LSL W9, W4, W5           /* W9 = max way number, aligned to position in DC
								 operand */
	LSL W16, W8, W5          // W16 = amount to decrement way number per iteration
	Loop2: UBFX W7, W1, #13, #15    // W7 = max set number, right aligned
	LSL W7, W7, W2           /* W7 = max set number, aligned to position in DC
								    operand    */
	LSL W17, W8, W2          // W17 = amount to decrement set number per iteration
	Loop3: ORR W11, W10, W9         // W11 = combine way number and cache number...
	ORR W11, W11, W7         // ... and set number for DC operand
	DC CSW, X11              // Do data cache clean by set and way
	SUBS W7, W7, W17         // Decrement set number
	B.GE Loop3
	SUBS X9, X9, X16         // Decrement way number
	B.GE Loop2
	Skip:  ADD W10, W10, #2         // Increment 2 x cache level
	CMP W3, W10
	DSB #0                   /* Ensure completion of previous cache maintenance  operation */
	B.GT Loop1
	Finished:


	/**********************
	 ** Initialize stack **
	 **********************/

	ldr x0, =_crt0_start_stack
	mov sp, x0
	bl init

	.p2align 4
	.space 0x4000
	_crt0_start_stack:
