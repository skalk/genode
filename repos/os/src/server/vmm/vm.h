/*
 * \brief  VMM example for ARMv8 virtualization
 * \author Stefan Kalkowski
 * \date   2019-07-18
 */

/*
 * Copyright (C) 2019 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _SRC__SERVER__VMM__VM_H_
#define _SRC__SERVER__VMM__VM_H_

#include <board.h>
#include <ram.h>
#include <exception.h>
#include <cpu.h>
#include <gic.h>
#include <hw_device.h>
#include <pl011.h>
#include <virtio_console.h>
#include <virtio_net.h>

#include <base/attached_io_mem__dataspace.h>
#include <base/attached_rom_dataspace.h>
#include <vm_session/connection.h>

namespace Vmm { class Vm; }

class Vmm::Vm
{
	private:

		using Ep = Genode::Entrypoint;

		enum { STACK_SIZE = sizeof(unsigned long) * 2048, };

		Genode::Env                  & _env;
		Genode::Vm_connection          _vm         { _env           };
		Genode::Attached_rom_dataspace _kernel_rom { _env, "linux"  };
		Genode::Attached_rom_dataspace _dtb_rom    { _env, "dtb"    };
		Genode::Attached_rom_dataspace _initrd_rom { _env, "initrd" };
		Genode::Attached_io_mem_dataspace _vm_ram  { _env, RAM_START, RAM_SIZE };
		Ram                            _ram        { RAM_START, RAM_SIZE,
		                                             (Genode::addr_t)_vm_ram.local_addr<void>()};
		Genode::Heap                   _heap       { _env.ram(), _env.rm() };
		Mmio_bus                       _bus;
		Gic                            _gic;
		Genode::Constructible<Ep>      _eps[MAX_CPUS];
		Genode::Constructible<Cpu>     _cpus[MAX_CPUS];
		Pl011                          _uart;
		Virtio_console                 _virtio_console;

		void _load_kernel();
		void _load_dtb();
		void _load_initrd();

		typedef Hw_device<2,4> Direct_device;
		typedef void (*Resources)(Direct_device &);

		enum { NUMBER_HW_DESCR = 63 };

		/* pass-through devices in order of appearance */
		Resources const _hw_descriptors[NUMBER_HW_DESCR] {
			[](Direct_device &device) { device.mmio(0x30360000, 0x10000); device.irqs(81); }, /* anatop */
			[](Direct_device &device) { device.mmio(0x30380000, 0x10000); device.irqs(117, 118); }, /* ccm */
			[](Direct_device &device) { device.mmio(0x306a0000, 0x30000); device.irqs(79, 80); }, /* system counter */
			[](Direct_device &device) { device.mmio(0x30350000, 0x10000);                      }, /* ocotp-ctrl */
			[](Direct_device &device) { device.mmio(0x30330000, 0x10000);                      }, /* iomuxc */
			[](Direct_device &device) { device.mmio(0x30200000, 0x10000); device.irqs(96, 97);   }, /* gpio */
			[](Direct_device &device) { device.mmio(0x30210000, 0x10000); device.irqs(98, 99);   }, /* gpio */
			[](Direct_device &device) { device.mmio(0x30220000, 0x10000); device.irqs(100, 101); }, /* gpio */
			[](Direct_device &device) { device.mmio(0x30230000, 0x10000); device.irqs(102, 103); }, /* gpio */
			[](Direct_device &device) { device.mmio(0x30240000, 0x10000); device.irqs(104, 105); }, /* gpio */
			[](Direct_device &device) { device.mmio(0x30a20000, 0x10000); device.irqs(67); }, /* i2c */
			[](Direct_device &device) { device.mmio(0x30a30000, 0x10000); device.irqs(68); }, /* i2c */
			[](Direct_device &device) { device.mmio(0x30a40000, 0x10000); device.irqs(69); }, /* i2c */
			[](Direct_device &device) { device.mmio(0x30a50000, 0x10000); device.irqs(70); }, /* i2c */
			[](Direct_device &device) { device.mmio(0x30bd0000, 0x10000); device.irqs(34); }, /* sdma */
			[](Direct_device &device) { device.mmio(0x302c0000, 0x10000); device.irqs(135); }, /* sdma */
			[](Direct_device &device) { device.mmio(0x33000000, 0x2000);  device.irqs(44); }, /* dma-apbh */
			[](Direct_device &device) { device.mmio(0x303a0000, 0x10000); device.irqs(119); }, /* gpc (GENERAL POWER CONTROLLER!) */
			[](Direct_device &device) { device.mmio(0x30390000, 0x10000); device.irqs(121); }, /* src */
			[](Direct_device &device) {                                   device.irqs(39); }, /* pmu */
			[](Direct_device &device) { device.mmio(0x30b40000, 0x10000); device.irqs(54); }, /* usdhc */
			[](Direct_device &device) { device.mmio(0x30b50000, 0x10000); device.irqs(55); }, /* usdhc */
			[](Direct_device &device) { device.mmio(0x302d0000, 0x10000); device.irqs(87); }, /* gpt */
			[](Direct_device &device) { device.mmio(0x33002000, 0x6000);  device.irqs(46); }, /* gpmi-nand */
			[](Direct_device &device) { device.mmio(0x38100000, 0x10000); device.irqs(72); }, /* usb */
			[](Direct_device &device) { device.mmio(0x381f0000, 0x1000);                   }, /* usb-phy */
			[](Direct_device &device) { device.mmio(0x38200000, 0x10000); device.irqs(73); }, /* usb */
			[](Direct_device &device) { device.mmio(0x382f0000, 0x1000);                   }, /* usb-phy */
			[](Direct_device &device) {                                   device.irqs(134, 141, 142, 143); }, /* busfreq */
			[](Direct_device &device) { device.mmio(0x30370000, 0x10000); device.irqs(51, 52, 148); }, /* snvs */
			[](Direct_device &device) { device.mmio(0x30820000, 0x10000); device.irqs(63); }, /* ecspi */
			[](Direct_device &device) { device.mmio(0x30830000, 0x10000); device.irqs(64); }, /* ecspi */
			[](Direct_device &device) { device.mmio(0x30840000, 0x10000); device.irqs(65); }, /* ecspi */
			[](Direct_device &device) { device.mmio(0x30be0000, 0x10000); device.irqs(150, 151, 152); }, /* network */
			[](Direct_device &device) { device.mmio(0x3d800000, 0x400000); device.irqs(130); }, /* ddr_pmu */
			[](Direct_device &device) { device.mmio(0x30260000, 0x10000); device.irqs(81); }, /* tmu */
			[](Direct_device &device) { device.mmio(0x32e2d000, 0x1000);  device.irqs(50); }, /* irqsteer */
			[](Direct_device &device) { device.mmio(0x30280000, 0x10000);  device.irqs(110); }, /* wdog */
			[](Direct_device &device) { device.mmio(0x30290000, 0x10000);  device.irqs(111); }, /* wdog */
			[](Direct_device &device) { device.mmio(0x302a0000, 0x10000);  device.irqs(42); }, /* wdog */
			[](Direct_device &device) { device.mmio(0x38300000, 0x200000);  device.irqs(39, 40); }, /* vpu */
			[](Direct_device &device) { device.mmio(0x38000000, 0x40000);  device.irqs(35); }, /* gpu */
			[](Direct_device &device) { device.mmio(0x30010000, 0x10000);  device.irqs(127); }, /* sai */
			[](Direct_device &device) { device.mmio(0x30030000, 0x20000);  device.irqs(122); }, /* sai */
			[](Direct_device &device) { device.mmio(0x30050000, 0x10000);  device.irqs(132); }, /* sai */
			[](Direct_device &device) { device.mmio(0x308b0000, 0x10000);  device.irqs(128); }, /* sai */
			[](Direct_device &device) { device.mmio(0x308c0000, 0x10000);  device.irqs(82); }, /* sai */
			[](Direct_device &device) { device.mmio(0x30810000, 0x10000);  device.irqs(38); }, /* spdif */
			[](Direct_device &device) { device.mmio(0x308a0000, 0x10000);  device.irqs(45); }, /* spdif */
			[](Direct_device &device) { device.mmio(0x30900000, 0x40000);  device.irqs(123); }, /* caam */
			[](Direct_device &device) { device.mmio(0x30aa0000, 0x10000);  }, /* mu */
			[](Direct_device &device) { device.mmio(0x100000, 0x8000); }, /* caam-sm */
			[](Direct_device &device) { device.mmio(0x30660000, 0x10000);  device.irqs(113); }, /* pwm */
			[](Direct_device &device) { device.mmio(0x30670000, 0x10000);  device.irqs(114); }, /* pwm */
			[](Direct_device &device) { device.mmio(0x30680000, 0x10000);  device.irqs(115); }, /* pwm */
			[](Direct_device &device) { device.mmio(0x30690000, 0x10000);  device.irqs(116); }, /* pwm */
			[](Direct_device &device) { device.mmio(0x30a90000, 0x10000);  device.irqs(74); }, /* csi1_bridge */
			[](Direct_device &device) { device.mmio(0x30b80000, 0x10000);  device.irqs(75); }, /* csi2_bridge */
			[](Direct_device &device) { device.mmio(0x30a70000, 0x1000);  device.irqs(76); }, /* mipi_csi1 */
			[](Direct_device &device) { device.mmio(0x30b60000, 0x1000);  device.irqs(77); }, /* mipi_csi1 */
			[](Direct_device &device) { device.mmio(0x30320000, 0x10000);  device.irqs(37); }, /* lcdif */
			[](Direct_device &device) { device.mmio(0x30a00000, 0x1000); device.irqs(66); }, /* mipi_dsi_bridge, dsi_phy */
			[](Direct_device &device) { device.mmio(0x30340000, 0x10000); }, /* iomux-gpr */

			//[](Direct_device &device) { device.mmio(0x33800000, 0x400000);  device.irqs(154, 159); }, /* pci */
			//[](Direct_device &device) { device.mmio(0x33c00000, 0x400000);  device.irqs(106, 112); }, /* pci */
			//[](Direct_device &device) { device.mmio(0x30bb0000, 0x10000, 0x8000000, 0x10000000); device.irqs(139); }, /* qspi */
		};

		Genode::Constructible<Direct_device> _hw_devices[NUMBER_HW_DESCR];
		Hw_device<1,8>                       _dcss { _env, _vm, boot_cpu() };
		Hw_device<3, 2>                      _hdmi { _env, _vm, boot_cpu() };

		void _construct_hw()
		{
			Genode::warning("Creating devices");
			for (unsigned index = 0; index < NUMBER_HW_DESCR; index++) {
				_hw_devices[index].construct(_env, _vm, boot_cpu());
				_hw_descriptors[index](*_hw_devices[index]);
			}

			/* dcss */
			_dcss.mmio(0x32e00000, 0x2d000);
			_dcss.irqs(35, 36, 37, 38, 40, 41, 48, 49); /* CAUTION 41 is EDGE */

			/* hdmi */
			_hdmi.mmio(0x32c00000, 0x100000, 0x32e40000, 0x40000, 0x32e2f000, 0x1000);
			_hdmi.irqs(57);
		}

	public:

		Vm(Genode::Env & env);

		Mmio_bus & bus() { return _bus; }
		Cpu      & boot_cpu();

		template <typename F>
		void cpu(unsigned cpu, F func)
		{
			if (cpu >= MAX_CPUS) Genode::error("Cpu number out of bounds ");
			else                 func(*_cpus[cpu]);
		}

		static unsigned last_cpu() { return MAX_CPUS - 1; }
};

#endif /* _SRC__SERVER__VMM__VM_H_ */
