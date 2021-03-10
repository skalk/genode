#pragma once

#include_next <asm/bug.h>

#undef  __WARN_FLAGS
#define __WARN_FLAGS(flags) 
