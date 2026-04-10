/*
 * \brief  Platform driver - protection domain
 * \author Stefan Kalkowski
 * \date   2026-03-16
 */

/*
 * Copyright (C) 2026 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#include <device.h>
#include <pci.h>
#include <pd.h>
#include <root.h>

using Driver::Pd;

Driver::Io_mmu::Domain & Pd::_create_domain()
{
	/* Use non-functional domain if no IOMMU is in use */
	static Io_mmu::Domain dummy { };

	Io_mmu::Domain *domain = &dummy;

	_devices.for_each([&] (Device const &dev) {
		if (!matches(dev) || domain != &dummy)
			return;

		_devices.with_io_mmu(dev, [&] (auto &io_mmu) {
			domain = &io_mmu.create_domain(); });
	});

	return *domain;
}


bool Pd::_dma_remapable() const
{
	/* iterate IOMMU devices and determine address translation mode */
	bool mpu_present   { false };
	bool iommu_present { false };

	_devices.for_each([&] (Device const &dev) {
		if (!matches(dev)) return;

		_devices.with_io_mmu(dev, [&] (auto &io_mmu) {
			if (io_mmu.mpu()) mpu_present   = true;
			else              iommu_present = true;
		});
	});

	return iommu_present && !mpu_present;
}


bool Pd::matches(Device const &dev) const
{
	return with_matching_policy(label(), _config.node(),
		[&] (Node const &policy) {

			/* check PCI devices */
			if (pci_device_matches(policy, dev))
				return true;

			/* check for dedicated device name */
			bool ret = false;
			policy.for_each_sub_node("device", [&] (Node const &node) {
				if (dev.name() == node.attribute_value("name", Device::Name()))
					ret = true;
			});
			return ret;

		}, [] { return false; });
};


void Pd::update_policy(bool info, Policy_version version)
{
	_info = info;
	_version = version;
	_sessions.for_each([&] (auto &session) {
		session.update_policy(); });
}


void Pd::close_all_sessions(Root &root)
{
	_sessions.for_each([&] (auto &session) {
		root.close(session.cap()); });
}


Pd::Pd(Env                          &env,
       Attached_rom_dataspace const &config,
       Device_model                 &devices,
       Dictionary                   &dictionary,
       Session::Label const         &label,
       bool                          info,
       Policy_version                version)
:
	Dictionary::Element(dictionary, label),
	_env(env),
	_config(config),
	_devices(devices),
	_label(label),
	_info(info),
	_version(version),
	_domain(_create_domain())
{
	/*
	 * Iterate matching devices and reserve reserved memory regions at DMA
	 * allocator.
	 */
	_devices.for_each([&] (Device &dev) {
		if (!matches(dev)) return;

		dev.for_each_reserved_memory([&] (unsigned, auto range,
		                                  auto &reservation) {
			_dma_address_alloc.reserve({ range.start, range.start+range.size-1},
			                           reservation); });
	});
}


Pd::~Pd()
{
	_sessions.for_each([&] (auto &) {
		error("Session for label ", _label, " still existent!"); });
}
