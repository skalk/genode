/*
 * \brief  Platform driver - handling of IOMMUs controlled by the kernel
 * \author Alexander Boettcher
 * \author Johannes Schlatow
 * \date   2015-11-05
 */

/*
 * Copyright (C) 2015-2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _SRC__DRIVER__PLATFORM__KERNEL_IO_MMU_H_
#define _SRC__DRIVER__PLATFORM__KERNEL_IO_MMU_H_

/* base */
#include <base/allocator_avl.h>
#include <base/env.h>
#include <base/quota_guard.h>
#include <region_map/client.h>
#include <pci/types.h>
#include <pd_session/connection.h>
#include <io_mem_session/capability.h>

/* local inludes */
#include <io_mmu.h>

namespace Driver {
	using namespace Genode;

	class Kernel_io_mmu;
}


class Driver::Kernel_io_mmu : public Io_mmu
{
	private:

		class Device_pd : public Io_mmu::Domain
		{
			private:
		
				friend class Kernel_io_mmu;
		
				Env &_env;

				Pd_connection _pd { _env, Pd_connection::Device_pd() };

				[[nodiscard]] bool _upgrade_ram();
				[[nodiscard]] bool _upgrade_caps();
		
			public:
		
				Device_pd(Env &env);
		
				Result add_range(Io_mmu::Range const &, addr_t const,
				               Dataspace_capability const) override;
				void remove_range(Io_mmu::Range const &) override;
		};


		Env &_env;
		Tslab<Device_pd, sizeof(Device_pd)*32> _domain_alloc;

	public:

		Kernel_io_mmu(Env               &env,
		              Allocator         &md_alloc,
		              Io_mmu_devices    &io_mmu_devices,
		              Device_name const &name);
		~Kernel_io_mmu();


		/*********************
		 ** Iommu interface **
		 *********************/

		Driver::Io_mmu::Domain & create_domain() override;

		void destroy_domain(Driver::Io_mmu::Domain &) override;

		void enregister(Device const &, Domain &) override;
		void deregister(Device const &, Domain &) override;
};


#endif /* _SRC__DRIVER__PLATFORM__KERNEL_IO_MMU_H_ */
