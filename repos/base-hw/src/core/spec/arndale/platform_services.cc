/*
 * \brief   Platform specific services for Arndale
 * \author  Stefan Kalkowski
 * \date    2014-07-08
 */

/*
 * Copyright (C) 2014-2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Genode includes */
#include <base/service.h>
#include <base/heap.h>

/* core includes */
#include <platform.h>
#include <platform_services.h>
#include <core_service.h>
#include <map_local.h>
#include <vm_root.h>
#include <platform.h>

#include <kernel/cpu.h>

extern Genode::addr_t hypervisor_exception_vector;

/*
 * Add ARM virtualization specific vm service
 */
void Genode::platform_add_local_services(Rpc_entrypoint *ep,
                                         Sliced_heap *sh,
                                         Registry<Service> *services)
{
	using namespace Genode;

	map_local(Platform::core_phys_addr((addr_t)&hypervisor_exception_vector),
	          Hw::Mm::hypervisor_exception_vector().base, 1,
	          Hw::PAGE_FLAGS_KERN_TEXT);

	static Vm_root vm_root(ep, sh);
	static Core_service<Vm_session_component> vm_service(*services, vm_root);
}


Board::Serial::Serial(Genode::addr_t mmio, unsigned c, unsigned b)
: Genode::Exynos_uart(mmio, c, b),
  Kernel::Irq(Board::UART_2_IRQ, *Kernel::cpu_pool()->executing_cpu())
{
	Genode::Exynos_uart::_rx_enable();
	Kernel::Irq::enable();
}


void Board::Serial::occurred()
{
	if (!_rx_avail()) return;
	Genode::raw("pressed: ", _rx_char());

	Genode::raw("Dump all thread states");

	for (Kernel::Thread * thread = Kernel::thread_list().first(); thread;
		 thread = thread->Genode::List<Kernel::Thread>::Element::next())
		Genode::raw(*thread);

	Genode::raw("Dump finished");
}
