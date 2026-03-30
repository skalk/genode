/*
 * \brief  Platform driver - session component
 * \author Stefan Kalkowski
 * \date   2020-04-13
 */

/*
 * Copyright (C) 2020 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _SRC__DRIVER__PLATFORM__SESSION_COMPONENT_H_
#define _SRC__DRIVER__PLATFORM__SESSION_COMPONENT_H_

#include <base/attached_rom_dataspace.h>
#include <base/env.h>
#include <base/heap.h>
#include <base/quota_guard.h>
#include <base/registry.h>
#include <base/session_object.h>
#include <os/dynamic_rom_session.h>
#include <os/session_policy.h>
#include <platform_session/platform_session.h>
#include <util/dictionary.h>

#include <dma_buffer.h>
#include <device_component.h>
#include <device_owner.h>
#include <io_mmu.h>
#include <irq_controller.h>
#include <pd.h>

namespace Driver {
	class Session_component;
	class Root;
}


class Driver::Session_component
:
	public  Session_object<Platform::Session>,
	public  Device_owner,
	private Registry<Driver::Session_component>::Element,
	private Dynamic_rom_session::Producer
{
	public:

		using Session_registry = Registry<Session_component>;
		using Policy_version   = String<64>;

		Session_component(Env &env, Pd &pd, Device_model &devices,
		                  Resources const &resources);

		~Session_component();

		Heap &heap();

		Ram_quota_guard & ram_quota_guard() { return _ram_quota_guard(); }
		Cap_quota_guard & cap_quota_guard() { return _cap_quota_guard(); }

		void with_io_mmu_domain(auto const &fn) {
			_pd.with_io_mmu_domain(fn); }

		void for_each_io_mmu(auto const &fn) {
			_pd.for_each_io_mmu(fn); }

		Attempt<Ok, Alloc_error> update_iommu_costs();

		void update_policy();
		void update_devices_rom();

		/**************************
		 ** Device Owner methods **
		 **************************/

		void enable_device(Device const &) override;
		void disable_device(Device const &) override;

		/**************************
		 ** Platform Session API **
		 **************************/

		using Device_capability = Capability<Platform::Device_interface>;
		using Device_name       = Platform::Session::Device_name;

		Rom_session_capability devices_rom() override;
		Device_capability acquire_device(Device_name const &) override;
		Device_capability acquire_single_device() override;
		void release_device(Device_capability) override;
		Ram_dataspace_capability alloc_dma_buffer(size_t, Cache) override;
		void free_dma_buffer(Ram_dataspace_capability ram_cap) override;
		addr_t dma_addr(Ram_dataspace_capability) override;

	private:

		friend class Root;

		Env          &_env;
		Pd           &_pd;
		Device_model &_devices;

		Accounted_ram_allocator _env_ram { _env.ram(), _ram_quota_guard(),
		                                   _cap_quota_guard()  };

		Heap _md_alloc { _env_ram, _env.rm() };

		Registry<Device_component> _device_registry { };

		Memory::Constrained_obj_allocator<Dma_buffer>
			_dma_buffer_alloc { _md_alloc };

		Dma_address_list _dma_address_list { };

		Dictionary<Dma_buffer, Dma_buffer_name> _dma_buffers {};

		Cost _costs { 0, 0 };

		Dynamic_rom_session _rom_session { _env.ep(), _env.ram(), _env.rm(),
		                                   *this };

		Device_capability _acquire(Device &device);
		void              _release_device(Device_component &dc);
		void              _free_dma_buffer(Dma_buffer &buf);

		/*
		 * Noncopyable
		 */
		Session_component(Session_component const &);
		Session_component &operator = (Session_component const &);

		/***************************************
		 ** Dynamic_rom_session::Producer API **
		 ***************************************/

		void generate(Generator &) override;
};

#endif /* _SRC__DRIVER__PLATFORM__SESSION_COMPONENT_H_ */
