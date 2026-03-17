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

#include <dataspace/client.h>

#include <device.h>
#include <pci.h>
#include <session_component.h>

using Driver::Session_component;


Genode::Capability<Platform::Device_interface>
Session_component::_acquire(Device &device)
{
	Device_component * dc = new (heap())
		Device_component(_device_registry, _env, *this, _devices, device);

	device.acquire(*this);
	update_devices_rom();

	return _env.ep().rpc_ep().manage(dc);
};


void Session_component::_release_device(Device_component &dc)
{
	_env.ep().rpc_ep().dissolve(&dc);

	/* release device (calls disable_device()) before destroying device component */
	Device::Name name = dc.device();
	_devices.for_each([&] (Device &dev) {
		if (name == dev.name()) dev.release(*this); });

	/* destroy device component */
	destroy(heap(), &dc);
	update_devices_rom();
}


void Session_component::_free_dma_buffer(Dma_buffer &buf)
{
	Ram_dataspace_capability cap = buf.cap;

	_pd.with_io_mmu_domain([&] (auto &domain) {
		domain.remove_range({ buf.dma_addr, buf.phys_range.size });
		_pd.for_each_io_mmu([&] (auto &io_mmu) {
			io_mmu.iotlb_flush(_domain); });
	});

	destroy(heap(), &buf);
	_env_ram.free(cap);
}


void Session_component::update_policy()
{
	enum Device_state { AWAY, CHANGED, UNCHANGED };

	_device_registry.for_each([&] (Device_component &dc) {
		Device_state state = AWAY;
		_devices.for_each([&] (Device const &dev) {
			if (dev.name() != dc.device())
				return;
			state = (dev.owner(*this) && _pd.matches(dev)) ? UNCHANGED : CHANGED;
		});

		if (state == UNCHANGED)
			return;

		if (state == CHANGED)
			warning("Device ", dc.device(),
			        " has changed, will close device session");
		else
			warning("Device ", dc.device(),
			        " unavailable, will close device session");
		_release_device(dc);
	});

	update_devices_rom();
};


void Session_component::generate(Generator &g)
{
	if (_pd._version.valid())
		g.attribute("version", _pd._version);

	_devices.for_each([&] (Device const &dev) {
		if (_pd.matches(dev)) dev.generate(g, _pd._info); });
}


Genode::Heap & Session_component::heap() { return _md_alloc; }


void Session_component::update_devices_rom()
{
	_rom_session.trigger_update();
}


void Session_component::enable_device(Device const &device)
{
	_devices.with_io_mmu(device, [&] (auto &io_mmu) {
		with_io_mmu_domain([&] (auto &domain) {
			io_mmu.enregister(device, domain); });
	});
	pci_enable(_env, device);
}


void Session_component::disable_device(Device const &device)
{
	pci_disable(_env, device);
	_devices.with_io_mmu(device, [&] (auto &io_mmu) {
		with_io_mmu_domain([&] (auto &domain) {
			io_mmu.deregister(device, domain); });
	});
}


Genode::Rom_session_capability Session_component::devices_rom() {
	return _rom_session.cap(); }


Genode::Capability<Platform::Device_interface>
Session_component::acquire_device(Platform::Session::Device_name const &name)
{
	Capability<Platform::Device_interface> cap;

	_devices.for_each([&] (Device &dev)
	{
		if (dev.name() != name || !_pd.matches(dev))
			return;
		if (dev.owned())
			warning("Cannot aquire device ", name, " already in use");
		else
			cap = _acquire(dev);
	});

	return cap;
}


Genode::Capability<Platform::Device_interface>
Session_component::acquire_single_device()
{
	Capability<Platform::Device_interface> cap;

	_devices.for_each([&] (Device &dev) {
		if (!cap.valid() && _pd.matches(dev) && !dev.owned())
			cap = _acquire(dev); });

	return cap;
}


void Session_component::release_device(Capability<Platform::Device_interface> device_cap)
{
	if (!device_cap.valid())
		return;

	_device_registry.for_each([&] (Device_component &dc) {
		if (device_cap.local_name() == dc.cap().local_name())
			_release_device(dc); });
}


Genode::Ram_dataspace_capability
Session_component::alloc_dma_buffer(size_t const, Cache)
{
#if 0
Session_component::alloc_dma_buffer(size_t const size, Cache cache)
{
	struct Guard {

		Accounted_ram_allocator &_env_ram;
		Heap                    &_heap;
		Io_mmu::Domain          &_domain;
		bool                     _cleanup { true };

		Ram_dataspace_capability ram_cap { };

		struct {
			Dma_buffer * buf { nullptr };
		};

		void disarm() { _cleanup = false; }

		Guard(Accounted_ram_allocator &env_ram,
		      Heap                    &heap,
		      Io_mmu::Domain          &domain)
		:
			_env_ram(env_ram), _heap(heap), _domain(domain)
		{ }

		~Guard()
		{
			if (_cleanup && buf) {
				/* make sure to remove buffer range from domain */
				_domain.remove_range({ buf->dma_addr, buf->size });
				destroy(_heap, buf);
			}

			if (_cleanup && ram_cap.valid())
				_env_ram.free(ram_cap);
		}
	} guard { _env_ram, heap(), _domain };

	/*
	 * Check available quota beforehand and reflect the state back
	 * to the client because the 'Expanding_pd_session_client' will
	 * ask its parent otherwise.
	 */
	enum { WATERMARK_CAP_QUOTA = 8, };
	if (_env.pd().avail_caps().value < WATERMARK_CAP_QUOTA)
		throw Out_of_caps();

	enum { WATERMARK_RAM_QUOTA = 4096, };
	if (_env.pd().avail_ram().value < WATERMARK_RAM_QUOTA)
		throw Out_of_ram();

	try {
		guard.ram_cap = _env_ram.alloc(size, cache);
	} catch (Ram_allocator::Denied) { }

	if (!guard.ram_cap.valid()) return guard.ram_cap;


	try {
		Dma_buffer &buf = _dma_allocator.alloc_buffer(guard.ram_cap,
		                                              _env.pd().dma_addr(guard.ram_cap),
		                                              _env.pd().ram_size(guard.ram_cap),
		                                              _dma_remapable());
		guard.buf = &buf;

		_domain.add_range({ buf.dma_addr, buf.size }, buf.phys_addr, buf.cap).with_error(
			[] (auto err) {
				if (err == decltype(err)::OUT_OF_RAM)
					throw Out_of_ram();
				if (err == decltype(err)::OUT_OF_CAPS)
					throw Out_of_caps();
		});

	} catch (Dma_allocator::Out_of_virtual_memory) { }

	guard.disarm();
	return guard.ram_cap;
#endif
	return Ram_dataspace_capability();
}


void Session_component::free_dma_buffer(Ram_dataspace_capability ram_cap)
{
	if (!ram_cap.valid()) { return; }

	_dma_buffers.with_element({ram_cap},
		[&] (Dma_buffer &buf) { _free_dma_buffer(buf); },
		[] () { /* ignore wrong capability argument */ });
}


Genode::addr_t Session_component::dma_addr(Ram_dataspace_capability ram_cap)
{
	addr_t ret = 0;

	if (!ram_cap.valid())
		return ret;

	_dma_buffers.with_element({ram_cap},
		[&] (Dma_buffer &buf) { ret = buf.dma_addr; },
		[] () { /* ignore wrong capability argument */ });

	return ret;
}


Session_component::Session_component(Env &env, Pd &pd, Device_model &devices,
                                     Resources const &resources)
:
	Session_object<Platform::Session>(env.ep(), resources, pd.label()),
	Session_registry::Element(pd._sessions, *this),
	Dynamic_rom_session::Producer("devices"),
	_env(env), _pd(pd), _devices(devices)
{
	/*
	 * FIXME: As the ROM session does not propagate Out_of_*
	 *        exceptions resp. does not account costs for the ROM
	 *        dataspace to the client for the moment, we cannot do
	 *        so in the dynamic rom session, and cannot use the
	 *        accounted ram allocator within it. Therefore,
	 *        we account the costs here until the ROM session interface
	 *        changes.
	 */
	if (!_cap_quota_guard().try_withdraw(Cap_quota{Rom_session::CAP_QUOTA}))
		throw Out_of_caps();
	if (!_ram_quota_guard().try_withdraw(Ram_quota{5*1024}))
		throw Out_of_ram();
}


Session_component::~Session_component()
{
	_device_registry.for_each([&] (Device_component &dc) {
		_release_device(dc); });

	/* free up dma buffers */
	while (_dma_buffers.with_any_element([&] (Dma_buffer &buf) {
		_free_dma_buffer(buf); })) ;

	/* replenish quota for rom sessions, see constructor for explanation */
	_cap_quota_guard().replenish(Cap_quota{Rom_session::CAP_QUOTA});
	_ram_quota_guard().replenish(Ram_quota{5*1024});
}
