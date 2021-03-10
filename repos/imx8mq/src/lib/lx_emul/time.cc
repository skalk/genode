#include <base/log.h>
#include <lx_kit/env.h>
#include <lx_emul/debug.h>
#include <lx_emul/time.h>


extern "C" void lx_emul_time_event(unsigned long evt)
{
	Lx_kit::env().timeout.start(evt);
}


extern "C" void lx_emul_time_stop()
{
	Lx_kit::env().timeout.stop();
}


extern "C" unsigned long lx_emul_time_counter()
{
	unsigned long ret = Lx_kit::env().timer.curr_time().trunc_to_plain_us().value;
	return ret;
}
