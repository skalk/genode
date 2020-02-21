/*
 * \brief   Parts of platform that are specific to Virt QEMU
 * \author  Stefan Kalkowski
 * \date    2018-11-07
 */

/*
 * Copyright (C) 2018 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#include <platform.h>
#include <spec/arm/cpu.h>

extern "C" void *    _start_setup_stack;   /* entrypoint for non-boot CPUs */
static unsigned char hyp_mode_stack[1024]; /* hypervisor mode's kernel stack */

Bootstrap::Platform::Board::Board()
: early_ram_regions(Memory_region { ::Board::RAM_BASE, ::Board::RAM_SIZE }),
  core_mmio(Memory_region { ::Board::Cpu_mmio::IRQ_CONTROLLER_DISTR_BASE,
                            ::Board::Cpu_mmio::IRQ_CONTROLLER_DISTR_SIZE },
            Memory_region { ::Board::Cpu_mmio::IRQ_CONTROLLER_CPU_BASE,
                            ::Board::Cpu_mmio::IRQ_CONTROLLER_CPU_SIZE },
            Memory_region { ::Board::Cpu_mmio::IRQ_CONTROLLER_VT_CTRL_BASE,
                            ::Board::Cpu_mmio::IRQ_CONTROLLER_VT_CTRL_SIZE },
            Memory_region { ::Board::UART_BASE, ::Board::UART_SIZE }) { }


static inline void prepare_hypervisor(Genode::addr_t table)
{
	using Cpu = Hw::Arm_cpu;

	/* set hypervisor exception vector */
	Cpu::Hvbar::write(Hw::Mm::hypervisor_exception_vector().base);

	/* set hypervisor's translation table */
	Cpu::Httbr_64bit::write(table);

	Cpu::Ttbcr::access_t ttbcr = 0;
	Cpu::Ttbcr::Irgn0::set(ttbcr, 1);
	Cpu::Ttbcr::Orgn0::set(ttbcr, 1);
	Cpu::Ttbcr::Sh0::set(ttbcr, 2);
	Cpu::Ttbcr::Eae::set(ttbcr, 1);

	/* prepare MMU usage by hypervisor code */
	Cpu::Htcr::write(ttbcr);

	/* don't trap on cporocessor 10 + 11, but all others */
	Cpu::Hcptr::access_t hcptr = 0;
	Cpu::Hcptr::Tcp<0>::set(hcptr, 1);
	Cpu::Hcptr::Tcp<1>::set(hcptr, 1);
	Cpu::Hcptr::Tcp<2>::set(hcptr, 1);
	Cpu::Hcptr::Tcp<3>::set(hcptr, 1);
	Cpu::Hcptr::Tcp<4>::set(hcptr, 1);
	Cpu::Hcptr::Tcp<5>::set(hcptr, 1);
	Cpu::Hcptr::Tcp<6>::set(hcptr, 1);
	Cpu::Hcptr::Tcp<7>::set(hcptr, 1);
	Cpu::Hcptr::Tcp<8>::set(hcptr, 1);
	Cpu::Hcptr::Tcp<9>::set(hcptr, 1);
	Cpu::Hcptr::Tcp<12>::set(hcptr, 1);
	Cpu::Hcptr::Tcp<13>::set(hcptr, 1);
	Cpu::Hcptr::Tta::set(hcptr, 1);
	Cpu::Hcptr::Tcpac::set(hcptr, 1);
	Cpu::Hcptr::write(hcptr);

	enum Memory_attributes {
		DEVICE_MEMORY          = 0x04,
		NORMAL_MEMORY_UNCACHED = 0x44,
		NORMAL_MEMORY_CACHED   = 0xff,
	};

	Cpu::Mair0::access_t mair0 = 0;
	Cpu::Mair0::Attr0::set(mair0, NORMAL_MEMORY_UNCACHED);
	Cpu::Mair0::Attr1::set(mair0, DEVICE_MEMORY);
	Cpu::Mair0::Attr2::set(mair0, NORMAL_MEMORY_CACHED);
	Cpu::Mair0::Attr3::set(mair0, DEVICE_MEMORY);
	Cpu::Hmair0::write(mair0);

	Cpu::Vtcr::access_t vtcr = ttbcr;
	Cpu::Vtcr::Sl0::set(vtcr, 1); /* set to starting level 1 */
	Cpu::Vtcr::write(vtcr);

	Cpu::Sctlr::access_t sctlr = Cpu::Sctlr::read();
	Cpu::Sctlr::C::set(sctlr, 1);
	Cpu::Sctlr::I::set(sctlr, 1);
	Cpu::Sctlr::V::set(sctlr, 1);
	Cpu::Sctlr::M::set(sctlr, 1);
	Cpu::Sctlr::Z::set(sctlr, 1);
	Cpu::Hsctlr::write(sctlr);
}


static inline void switch_to_supervisor_mode()
{
	using Cpsr = Hw::Arm_cpu::Psr;

	Cpsr::access_t cpsr = 0;
	Cpsr::M::set(cpsr, Cpsr::M::SVC);
	Cpsr::F::set(cpsr, 1);
	Cpsr::I::set(cpsr, 1);

	asm volatile (
		"msr sp_svc, sp         \n" /* copy current mode's sp           */
		"msr lr_svc, lr         \n" /* copy current mode's lr           */
		"adr lr, 1f             \n" /* load exception return address    */
		"msr elr_hyp, lr        \n" /* copy current mode's lr to hyp lr */
		"mov sp, %[stack]       \n" /* copy to hyp stack pointer        */
		"msr spsr_cxfs, %[cpsr] \n" /* set psr for supervisor mode      */
		"eret                   \n" /* exception return                 */
		"1:":: [cpsr] "r" (cpsr), [stack] "r" (&hyp_mode_stack));
}


unsigned Bootstrap::Platform::enable_mmu()
{
	using Cpu = ::Board::Cpu;

	/* locally initialize interrupt controller */
	::Board::Pic pic { };

	prepare_hypervisor((addr_t)core_pd->table_base);
	switch_to_supervisor_mode();

	Cpu::Sctlr::init();
	Cpu::Cpsr::init();

	Cpu::invalidate_data_cache();
	Cpu::enable_mmu_and_caches((Genode::addr_t)core_pd->table_base);

	return Cpu::Mpidr::Aff_0::get(Cpu::Mpidr::read());
}
