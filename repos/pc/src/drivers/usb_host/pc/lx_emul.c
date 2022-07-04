/*
 * \brief  Linux emulation environment specific to this driver
 * \author Stefan Kalkowski
 * \date   2021-08-31
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#include <lx_emul.h>

#include <asm-generic/delay.h>
#include <linux/delay.h>

void __const_udelay(unsigned long xloops)
{
       unsigned long usecs = xloops / 0x10C7UL;
       if (usecs < 100)
               lx_emul_time_udelay(usecs);
       else
               usleep_range(usecs, usecs * 10);
}


void __udelay(unsigned long usecs)
{
	lx_emul_time_udelay(usecs);
}
