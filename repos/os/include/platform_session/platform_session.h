/*
 * \brief  Platform session interface
 * \author Stefan Kalkowski
 * \date   2020-04-28
 */

/*
 * Copyright (C) 2020 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _INCLUDE__PLATFORM_SESSION__PLATFORM_SESSION_H_
#define _INCLUDE__PLATFORM_SESSION__PLATFORM_SESSION_H_

#include <base/quota_guard.h>
#include <base/ram_allocator.h>
#include <base/rpc_args.h>
#include <rom_session/capability.h>
#include <irq_session/capability.h>
#include <io_mem_session/capability.h>
#include <io_port_session/capability.h>
#include <session/session.h>

namespace Platform {

	using namespace Genode;

	struct Device_interface;
	struct Session;
}


struct Platform::Device_interface : Interface
{
	/**
	 * Byte-offset range of memory-mapped I/O registers within dataspace
	 */
	struct Range { addr_t start; size_t size; };

	/**
	 * Handle to identify an allocated MSI(-x)
	 */
	struct Msi_handle { unsigned value; };

	using Alloc_msi_result = Attempt<Msi_handle, Alloc_error>;

	GENODE_RPC(Rpc_irq, Irq_session_capability, irq, unsigned);
	GENODE_RPC(Rpc_io_mem, Io_mem_session_capability, io_mem,
	           unsigned, Range &);
	GENODE_RPC(Rpc_io_port_range, Io_port_session_capability, io_port_range,
	           unsigned);
	GENODE_RPC(Rpc_msi_alloc, Alloc_msi_result, alloc_msi,
	           Signal_context_capability, bool);
	GENODE_RPC(Rpc_msi_free, void, free_msi, Msi_handle);

	GENODE_RPC_INTERFACE(Rpc_irq, Rpc_io_mem, Rpc_io_port_range,
	                     Rpc_msi_alloc, Rpc_msi_free);
};


struct Platform::Session : Genode::Session
{
	/**
	 * \noapi
	 */
	static const char *service_name() { return "Platform"; }

	static constexpr unsigned CAP_QUOTA = 18;

	virtual ~Session() { }

	using Device_name = String<64>;

	/**
	 * Request ROM session containing the information about available devices.
	 *
	 * \return  capability to ROM dataspace
	 */
	virtual Rom_session_capability devices_rom() = 0;

	/**
	 * Acquire device known by unique 'name'
	 */
	virtual Capability<Device_interface> acquire_device(Device_name const &name) = 0;

	/**
	 * Acquire the first resp. single device of this session
	 */
	virtual Capability<Device_interface> acquire_single_device() = 0;

	/**
	 * Free server-internal data structures representing the device
	 *
	 * Use this method to relax the resource-allocation of the Platform session.
	 */
	virtual void release_device(Capability<Device_interface> device) = 0;


	/*********************
	 ** RPC declaration **
	 *********************/

	GENODE_RPC(Rpc_devices_rom, Rom_session_capability, devices_rom);
	GENODE_RPC_THROW(Rpc_acquire_device, Capability<Device_interface>, acquire_device,
	                 GENODE_TYPE_LIST(Out_of_ram, Out_of_caps),
	                 Device_name const &);
	GENODE_RPC_THROW(Rpc_acquire_single_device, Capability<Device_interface>,
	                 acquire_single_device, GENODE_TYPE_LIST(Out_of_ram, Out_of_caps));
	GENODE_RPC(Rpc_release_device, void, release_device, Capability<Device_interface>);

	GENODE_RPC_INTERFACE(Rpc_devices_rom, Rpc_acquire_device,
	                     Rpc_acquire_single_device, Rpc_release_device);
};

#endif /* _INCLUDE__PLATFORM_SESSION__PLATFORM_SESSION_H_ */
