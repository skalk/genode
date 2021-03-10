#include <base/log.h>
#include <lx_kit/env.h>
#include <lx_emul/clock.h>

extern "C" int of_device_is_compatible(const struct device_node * node,
                                       const char * compat);

struct clk * lx_emul_clock_get(const struct device_node * node,
                               const char * name)
{
	using namespace Lx_kit;

	struct clk * ret = nullptr;

	env().devices.for_each([&] (Device & d) {
		if (!of_device_is_compatible(node, d.compatible())) { return; }
		ret = name ? d.clock(name) : d.clock(0U);
		if (!ret) { warning("No clock ", name, " found for device ", d.name()); }
	});

	return ret;
}


unsigned long lx_emul_clock_get_rate(struct clk * clk)
{
	if (!clk) { return 0; }

	return clk->rate;
}
