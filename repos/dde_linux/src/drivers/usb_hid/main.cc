/*
 * \brief  C++ initialization, session, and client handling
 * \author Sebastian Sumpf
 * \author Stefan Kalkowski
 * \date   2023-06-29
 */

/*
 * Copyright (C) 2023 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#include <base/attached_rom_dataspace.h>
#include <base/component.h>
#include <base/env.h>
#include <base/registry.h>

#include <lx_emul/init.h>
#include <lx_emul/task.h>
#include <lx_emul/input_leds.h>
#include <lx_user/io.h>
#include <lx_kit/env.h>

#include <genode_c_api/event.h>

/* C-interface */
#include <usb_hid.h>

#include <led_state.h>

using namespace Genode;

struct Main
{
	Env &env;

	Attached_rom_dataspace config_rom { env, "config" };

	Usb::Led_state capslock { env, "capslock" },
	               numlock  { env, "numlock"  },
	               scrlock  { env, "scrlock"  };

	Signal_handler<Main> signal_handler { env.ep(), *this, &Main::handle_signal };
	Signal_handler<Main> config_handler { env.ep(), *this, &Main::handle_config };

	Main(Env &env)
	:
		env(env)
	{
		Lx_kit::initialize(env, signal_handler);
		env.exec_static_constructors();

		Genode_c_api::initialize_usb_client(env, Lx_kit::env().heap,
		                                    signal_handler);

		genode_event_init(genode_env_ptr(env),
		                  genode_allocator_ptr(Lx_kit::env().heap));

		config_rom.sigh(config_handler);
		handle_config();

		lx_emul_start_kernel(nullptr);
	}

	void handle_signal()
	{
		lx_user_handle_io();
		Lx_kit::env().scheduler.execute();
	}

	void handle_config()
	{
		config_rom.update();
		Genode::Xml_node config = config_rom.xml();
		capslock.update(config, config_handler);
		numlock .update(config, config_handler);
		scrlock .update(config, config_handler);
		handle_signal();
	}
};


static Constructible<Main> &singleton()
{
	static Constructible<Main> main {};
	return main;
}


void Component::construct(Env & env) { singleton().construct(env); }


void lx_emul_led_state_update()
{
	if (singleton().constructed())
		lx_emul_input_leds_update(singleton()->capslock.enabled(),
		                          singleton()->numlock.enabled(),
		                          singleton()->scrlock.enabled());
}
