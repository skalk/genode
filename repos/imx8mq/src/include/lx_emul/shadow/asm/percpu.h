#pragma once

#include_next <asm/percpu.h>

static inline unsigned long __dummy_cpu_offset(void)
{
	return 0;
}

#undef  __my_cpu_offset
#define __my_cpu_offset __dummy_cpu_offset()
