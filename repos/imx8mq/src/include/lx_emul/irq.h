/**
 * \brief  Lx_emul support for device clocks
 * \author Stefan Kalkowski
 * \date   2021-04-14
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

struct irq_desc;

void lx_emul_irq_unmask(unsigned int irq);

void lx_emul_irq_mask(unsigned int irq);

void lx_emul_irq_eoi(unsigned int irq);

int lx_emul_irq_task_function(void * data);

extern void * lx_emul_irq_task_struct;

unsigned int lx_emul_irq_last(void);

#ifdef __cplusplus
}
#endif

