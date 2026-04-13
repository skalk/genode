/*
 * \brief  Platform driver root component
 * \author Stefan Kalkowski
 * \date   2020-04-13
 */

/*
 * Copyright (C) 2020-2026 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _SRC__DRIVER__PLATFORM__ROOT_H_
#define _SRC__DRIVER__PLATFORM__ROOT_H_

#include <base/attached_rom_dataspace.h>
#include <base/registry.h>
#include <root/component.h>

#include <device.h>
#include <pd.h>
#include <session_component.h>

namespace Driver { class Root; }

class Driver::Root : public Root_component<Session_component>
{
	private:

		Env &_env;

		Attached_rom_dataspace const &_config;

		Device_model &_devices;

		Memory::Constrained_obj_allocator<Pd> _pd_alloc;

		Browsable_dictionary<Pd, Session::Label> _pds {};

		bool _warn_once { true };

		Create_result _create_session(const char * args) override;

		void _upgrade_session(Session_component &, const char *) override;

		void _destroy_session(Session_component &) override;

	public:

		Root(Env                          &env,
		     Sliced_heap                  &sliced_heap,
		     Heap                         &heap,
		     Attached_rom_dataspace const &config,
		     Device_model                 &devices);

		void update_policy();
};

#endif /* _SRC__DRIVER__PLATFORM__ROOT_H_ */
