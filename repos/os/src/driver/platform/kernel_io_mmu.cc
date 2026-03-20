/*
 * \brief  Platform driver - handling of IOMMUs controlled by the kernel
 * \author Alexander Boettcher
 * \author Stefan Kalkowski
 * \author Johannes Schlatow
 * \date   2013-02-10
 */

/*
 * Copyright (C) 2013-2023 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Genode includes */
#include <base/attached_io_mem_dataspace.h>
#include <base/log.h>
#include <dataspace/client.h>
#include <region_map/client.h>
#include <pd_session/client.h>
#include <util/retry.h>

/* local includes */
#include <device.h>
#include <kernel_io_mmu.h>

using namespace Driver;


bool Kernel_io_mmu::Device_pd::_upgrade_ram()
{
	Ram_quota const ram { 4096 };

	return _env.pd().transfer_quota(_pd.rpc_cap(), ram)
	       == Pd_session::Transfer_result::OK;
}


bool Kernel_io_mmu::Device_pd::_upgrade_caps()
{
	Cap_quota const caps { 2 };

	return _env.pd().transfer_quota(_pd.rpc_cap(), caps)
	       == Pd_session::Transfer_result::OK;
}


Kernel_io_mmu::Device_pd::Result
Kernel_io_mmu::Device_pd::add_range(Io_mmu::Range        const &range,
                                    addr_t               const,
                                    Dataspace_capability const cap)
{
	using namespace Genode;

	if (range.start == 0) return Error::INVALID_RANGE;

	for (;;) {
		Result const result = _pd.attach_dma(cap, range.start).convert<Result>(
			[&] (Ok) -> Result {
				/* trigger eager mapping of memory */
				switch (_pd.map(Pd_session::Virt_range { range.start,
				                                         range.size })) {
				case Pd_session::Map_result::OUT_OF_RAM:  return Error::OUT_OF_RAM;
				case Pd_session::Map_result::OUT_OF_CAPS: return Error::OUT_OF_CAPS;
				case Pd_session::Map_result::OK: ;
				};
				return Ok();
			},
			[&] (Pd_session::Attach_dma_error e) {
				switch (e) {
				case Pd_session::Attach_dma_error::OUT_OF_RAM:
				case Pd_session::Attach_dma_error::OUT_OF_CAPS:
				case Pd_session::Attach_dma_error::DENIED:
					error("Device PD: attach_dma denied!");
				}
				return Error::DENIED;
			}
		);

		if (result == Error::OUT_OF_RAM && _upgrade_ram())
			continue;

		if (result == Error::OUT_OF_CAPS && _upgrade_caps())
			continue;

		return result;
	}
}


void Kernel_io_mmu::Device_pd::remove_range(Io_mmu::Range const &range)
{
	_env.rm().detach(range.start);
}


Kernel_io_mmu::Device_pd::Device_pd(Env &env)
:
	_env(env)
{
	_pd.ref_account(env.pd_session_cap());
}


void Kernel_io_mmu::enregister(Device const &device, Domain &domain)
{
	Device_pd &dpd = static_cast<Device_pd&>(domain);

	device.with_pci_config([&] (Device::Pci_config const &cfg) {
		Attached_io_mem_dataspace io_mem { _env, cfg.addr, 0x1000 };
		Pci::Bdf bdf {cfg.bus_num, cfg.dev_num, cfg.func_num};

		dpd._env.rm().attach(io_mem.cap(), {
			.size       = 0x1000,  .offset    = { },
			.use_at     = { },     .at        = { },
			.executable = { },     .writeable = true
		}).with_result(
			[&] (auto &a) {

				/* trigger eager mapping of memory */
				dpd._pd.map(Pd_session::Virt_range { (addr_t)a.ptr, a.num_bytes });

				/* try to assign pci device to this protection domain */
				if (!dpd._pd.assign_pci((addr_t)a.ptr, Pci::Bdf::rid(bdf)))
					log("Assignment of PCI device ", bdf, " to device PD failed, no IOMMU?!");

				/* after assignment, we don't need the mapping anymore */
				a.deallocate = true;
			},
			[&] (Region_map::Attach_error) {
				error("failed to attach PCI device to device PD"); }
		);
	});
}


void Kernel_io_mmu::deregister(Device const &, Domain &)
{
	warning("Cannot unassign PCI device from device PD (not implemented by kernel).");
}



Io_mmu::Domain &
Kernel_io_mmu::create_domain()
{
	return *new (_domain_alloc) Device_pd(_env);
}


void Kernel_io_mmu::destroy_domain(Driver::Io_mmu::Domain &)
{
	error(__func__,
	      " for kernel controlled IOMMU domain should never be called!");
}


Kernel_io_mmu::Kernel_io_mmu(Env               &env,
                             Allocator         &md_alloc,
                             Io_mmu_devices    &io_mmu_devices,
                             Device_name const &name)
:
	Io_mmu(io_mmu_devices, name),
	_env(env),
	_domain_alloc(md_alloc)
{ };


Kernel_io_mmu::~Kernel_io_mmu() { }
