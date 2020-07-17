#pragma once

#include <base/env.h>

struct Gpc
{
	enum Pu {
		MIPI      = 0,
		PCIE_1    = 1,
		USB_OTG_1 = 2,
		USB_OTG_2 = 3,
		GPU       = 4,
		VPU       = 5,
		HDMI      = 6,
		DISP      = 7,
		CSI_1     = 8,
		CSI_2     = 9,
		PCIE_2    = 10
	};

	enum {
		SIP_SERVICE_FUNC = 0xc2000000,
		GPC_PM_DOMAIN    = 0x3,
		ON               = 1,
		OFF              = 0
	};

	Gpc(Genode::Env & env)
	{
		for (unsigned domain = MIPI; domain <= PCIE_2; domain++) {
			Genode::Pd_session::Managing_system_state state;
			state.r[0] = SIP_SERVICE_FUNC;
			state.r[1] = GPC_PM_DOMAIN;
			state.r[2] = domain;
			state.r[3] = OFF;
			env.pd().managing_system(state);
		}
	};
};
