/*
 * \brief   Platform implementations specific for base-hw and i.MX8Q EVK
 * \author  Stefan Kalkowski
 * \date    2019-06-12
 */

/*
 * Copyright (C) 2019 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#include <platform.h>


void Board::Cpu::wake_up_all_cpus(void * ip)
{
	enum Function_id { CPU_ON = 0xC4000003 };

	unsigned long result = 0;
	for (unsigned i = 1; i < NR_OF_CPUS; i++) {
		asm volatile("mov x0, %1  \n"
		             "mov x1, %2  \n"
		             "mov x2, %3  \n"
		             "mov x3, %2  \n"
		             "smc #0      \n"
		             "mov %0, x0  \n"
		             : "=r" (result) : "r" (CPU_ON), "r" (i), "r" (ip)
		                      : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
		                        "x8", "x9", "x10", "x11", "x12", "x13", "x14");
	}
}


void Board::Cpu::set_frequency()
{
	enum Mmio_base_addresses {
		CCM_ANALOG_MMIO_BASE = 0x30360000,
		CCM_MMIO_BASE        = 0x30380000,
	};

	struct Ccm : Genode::Mmio
	{
		Ccm() : Genode::Mmio(CCM_MMIO_BASE) { }

		struct Target_root_0 : Register<0x8000, 32>
		{
			struct Mux : Bitfield<24, 3>
			{
				enum { ARM_PLL = 1, SYS_PLL1 = 4 };
			};

			struct Enable : Bitfield<28, 1> {};
		};
	} ccm;

	struct Ccm_analog : Genode::Mmio
	{
		Ccm_analog() : Genode::Mmio(CCM_ANALOG_MMIO_BASE) { }

		struct Pll_arm_0 : Register<0x28, 32>
		{
			struct Output_div_value : Bitfield<0, 5> {};
			struct Newdiv_ack       : Bitfield<11,1> {};
			struct Newdiv_val       : Bitfield<12,1> {};
		};
		struct Pll_arm_1 : Register<0x2c, 32>
		{
			struct Int_div_ctl      : Bitfield<0,  7> {};
			struct Frac_div_ctl     : Bitfield<7, 24> {};
		};
	} pll;

	/* set Cortex A53 reference clock to System PLL1 800 MHz */
	ccm.write<Ccm::Target_root_0::Mux>(Ccm::Target_root_0::Mux::SYS_PLL1);

	/* set new divider value for Pll_arm */
	pll.write<Ccm_analog::Pll_arm_1>(Ccm_analog::Pll_arm_1::Int_div_ctl::bits(0x4a));
	pll.write<Ccm_analog::Pll_arm_0::Output_div_value>(0);
	pll.write<Ccm_analog::Pll_arm_0::Newdiv_val>(1);
	while (!pll.read<Ccm_analog::Pll_arm_0::Newdiv_ack>()) { ; }
	pll.write<Ccm_analog::Pll_arm_0::Newdiv_val>(0);

	/* set Cortex A53 reference clock to ARM_PLL 1500 MHz */
	ccm.write<Ccm::Target_root_0::Mux>(Ccm::Target_root_0::Mux::ARM_PLL);
}
