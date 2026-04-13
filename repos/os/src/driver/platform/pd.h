/*
 * \brief  Platform driver - protection domain
 * \author Stefan Kalkowski
 * \date   2025-12-18
 */

/*
 * Copyright (C) 2025 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _SRC__DRIVER__PLATFORM__PD_H_
#define _SRC__DRIVER__PLATFORM__PD_H_

#include <base/attached_rom_dataspace.h>

#include <dma_address.h>
#include <browsable_dictionary.h>
#include <device_owner.h>
#include <io_mmu.h>
#include <irq_controller.h>

namespace Driver {
	using namespace Genode;

	class Pd;
	class Root;
	class Session_component;
}


class Driver::Pd
:
	private Browsable_dictionary<Pd, Session::Label>::Element
{
	public:

		using Policy_version = String<64>;
		using Dictionary = Browsable_dictionary<Pd, Session::Label>;

	private:

		friend class Genode::Avl_tree<Pd>;
		friend class Genode::Avl_node<Pd>;
		friend class Genode::Dictionary<Pd, Session::Label>;
		friend class Browsable_dictionary<Pd, Session::Label>;
		friend class Session_component;

		Env &_env;

		Attached_rom_dataspace const &_config;

		Device_model &_devices;

		Session::Label _label;

		bool _info;

		Policy_version _version;

		Dma_address_allocator _dma_address_alloc {};

		Io_mmu::Domain &_domain;

		Registry<Session_component> _sessions {};

		Io_mmu::Domain & _create_domain();

		bool _dma_remapable() const;

	public:

		/*
		 * Noncopyable
		 */
		Pd(Pd const &) = delete;
		Pd &operator = (Pd const &) = delete;

		Pd(Env                          &env,
		   Attached_rom_dataspace const &config,
		   Device_model                 &devices,
		   Dictionary                   &dictionary,
		   Session::Label const         &label,
		   bool                          info,
		   Policy_version                version);

		~Pd();

		Session::Label label() const { return _label; }

		bool matches(Device const &) const;

		void update_policy(bool info, Policy_version version);

		void with_io_mmu_domain(auto const &fn) { fn(_domain); }

		void for_each_io_mmu(auto const &fn)
		{
			_devices.for_each_io_mmu([&] (auto &io_mmu) {
				bool match = false;
				_devices.for_each([&] (Device const &dev) {
					if (matches(dev))
						dev.with_io_mmu([&] (auto &dev_io_mmu) {
							if (dev_io_mmu.name == io_mmu.name())
								match = true; });
				});

				if (match) fn(io_mmu);
			});
		}


		void close_all_sessions(Root &);

		bool empty()
		{
			bool empty = true;
			_sessions.for_each([&] (auto const &) {
				empty = false; });
			return empty;
		}
};

#endif /* _SRC__DRIVER__PLATFORM__PD_H_ */
