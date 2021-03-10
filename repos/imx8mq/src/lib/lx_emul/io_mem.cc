#include <lx_kit/env.h>
#include <lx_emul/io_mem.h>

void * lx_emul_io_mem_map(unsigned long phys_addr,
                          unsigned long size)
{
	using namespace Lx_kit;

	void * ret = nullptr;
	env().devices.for_each([&] (Device & d) {
		if (d.io_mem(phys_addr, size)) {
			ret = d.io_mem_local_addr(phys_addr, size);
		}
	});
	return ret;
}
