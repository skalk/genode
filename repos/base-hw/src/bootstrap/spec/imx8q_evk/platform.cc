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


/**
 * Leave out the first page (being 0x0) from bootstraps RAM allocator,
 * some code does not feel happy with addresses being zero
 */
Bootstrap::Platform::Board::Board()
: early_ram_regions(Memory_region { ::Board::RAM_BASE, ::Board::RAM_SIZE }),
  late_ram_regions(Memory_region { }),
  core_mmio(Memory_region { ::Board::UART_BASE, ::Board::UART_SIZE },
            Memory_region { ::Board::Cpu_mmio::IRQ_CONTROLLER_DISTR_BASE,
                            ::Board::Cpu_mmio::IRQ_CONTROLLER_DISTR_SIZE },
            Memory_region { ::Board::Cpu_mmio::IRQ_CONTROLLER_REDIST_BASE,
                            ::Board::Cpu_mmio::IRQ_CONTROLLER_REDIST_SIZE })
{
	::Board::Pic pic {};

	static volatile unsigned long iomux_values[][2] {
		// IOMUXC
		{ 0x30330064, 0x6        },
		{ 0x30330140, 0x0        },
		{ 0x30330144, 0x0        },
		{ 0x30330148, 0x0        },
		{ 0x3033014C, 0x0        },
		{ 0x30330150, 0x0        },
		{ 0x30330154, 0x0        },
		{ 0x30330158, 0x0        },
		{ 0x30330180, 0x2        },
		{ 0x30330184, 0x0        },
		{ 0x30330188, 0x0        },
		{ 0x3033018C, 0x0        },
		{ 0x30330190, 0x0        },
		{ 0x30330194, 0x0        },
		{ 0x30330198, 0x0        },
		{ 0x3033019C, 0x0        },
		{ 0x303301A0, 0x0        },
		{ 0x303301A4, 0x0        },
		{ 0x303301A8, 0x0        },
		{ 0x303301AC, 0x0        },
		{ 0x303301BC, 0x0        },
		{ 0x303301C0, 0x0        },
		{ 0x303301C4, 0x0        },
		{ 0x303301C8, 0x0        },
		{ 0x303301E8, 0x0        },
		{ 0x303301EC, 0x0        },
		{ 0x303301FC, 0x1        },
		{ 0x30330200, 0x1        },
		{ 0x3033021C, 0x10       },
		{ 0x30330220, 0x10       },
		{ 0x30330224, 0x10       },
		{ 0x30330228, 0x10       },
		{ 0x3033022C, 0x12       },
		{ 0x30330230, 0x12       },
		{ 0x30330244, 0x0        },
		{ 0x30330248, 0x0        },
		{ 0x3033029C, 0x19       },
		{ 0x303302A4, 0x19       },
		{ 0x303302A8, 0x19       },
		{ 0x303302B0, 0xD6       },
		{ 0x303302C0, 0x4F       },
		{ 0x303302C4, 0x16       },
		{ 0x303302CC, 0x59       },
		{ 0x3033033C, 0x9F       },
		{ 0x30330340, 0xDF       },
		{ 0x30330344, 0xDF       },
		{ 0x30330348, 0xDF       },
		{ 0x3033034C, 0xDF       },
		{ 0x30330350, 0xDF       },
		{ 0x30330368, 0x59       },
		{ 0x30330370, 0x19       },
		{ 0x3033039C, 0x19       },
		{ 0x303303A0, 0x19       },
		{ 0x303303A4, 0x19       },
		{ 0x303303A8, 0xD6       },
		{ 0x303303AC, 0xD6       },
		{ 0x303303B0, 0xD6       },
		{ 0x303303B4, 0xD6       },
		{ 0x303303B8, 0xD6       },
		{ 0x303303BC, 0xD6       },
		{ 0x303303C0, 0xD6       },
		{ 0x303303E8, 0xD6       },
		{ 0x303303EC, 0xD6       },
		{ 0x303303F0, 0xD6       },
		{ 0x303303F4, 0xD6       },
		{ 0x303303F8, 0xD6       },
		{ 0x303303FC, 0xD6       },
		{ 0x30330400, 0xD6       },
		{ 0x30330404, 0xD6       },
		{ 0x30330408, 0xD6       },
		{ 0x3033040C, 0xD6       },
		{ 0x30330410, 0xD6       },
		{ 0x30330414, 0xD6       },
		{ 0x30330424, 0xD6       },
		{ 0x30330428, 0xD6       },
		{ 0x3033042C, 0xD6       },
		{ 0x30330430, 0xD6       },
		{ 0x30330450, 0xD6       },
		{ 0x30330454, 0xD6       },
		{ 0x30330464, 0x49       },
		{ 0x30330468, 0x49       },
		{ 0x3033046C, 0x16       },
		{ 0x30330484, 0x67       },
		{ 0x30330488, 0x67       },
		{ 0x3033048C, 0x67       },
		{ 0x30330490, 0x67       },
		{ 0x30330494, 0x76       },
		{ 0x30330498, 0x76       },
		{ 0x303304AC, 0x49       },
		{ 0x303304B0, 0x49       },
		{ 0x303304C8, 0x1        },
		{ 0x303304CC, 0x4        },
		{ 0x30330500, 0x1        },
		{ 0x30330504, 0x2        },
		{ 0x30340038, 0x49409600 },
		{ 0x30340040, 0x49409200 }
	};

	unsigned num_values = sizeof(iomux_values) / (2*sizeof(unsigned long));
	for (unsigned i = 0; i < num_values; i++)
		*((volatile Genode::uint32_t*)iomux_values[i][0]) = (Genode::uint32_t)iomux_values[i][1];

	enum Mmio_base_addresses { GPIO_1_MMIO_BASE = 0x30200000 };

	struct Gpio : Genode::Mmio
	{
		Gpio() : Genode::Mmio(GPIO_1_MMIO_BASE) { }

		struct Data       : Register<0x0,  32> {};
		struct Dir        : Register<0x4,  32> {};
		struct Int_conf_0 : Register<0xc,  32> {};
		struct Int_conf_1 : Register<0x10, 32> {};
		struct Int_mask   : Register<0x14, 32> {};
		struct Int_stat   : Register<0x18, 32> {};
	} regulator;

	/* configure GPIO PIN 13 of GPIO 1 for high voltage */
	regulator.write<Gpio::Int_conf_0>(0);
	regulator.write<Gpio::Int_conf_1>(0);
	regulator.write<Gpio::Int_mask>(0x1000);
	regulator.write<Gpio::Int_stat>(0xffffffff);
	regulator.write<Gpio::Dir>(0x2328);
	regulator.write<Gpio::Data>(0x9f40);

	::Board::Cpu::set_frequency();
}
