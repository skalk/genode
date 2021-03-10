/*
 * \brief  i.MX8 framebuffer driver Linux port
 * \author Stefan Kalkowski
 * \date   2021-03-08
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#include <base/attached_rom_dataspace.h>
#include <base/component.h>
#include <timer_session/connection.h>
#include <capture_session/connection.h>
#include <os/pixel_rgb888.h>
#include <util/reconstructible.h>
#include <lx_emul/fb.h>
#include <lx_emul/init.h>
#include <lx_kit/env.h>
#include <lx_kit/init.h>

namespace Framebuffer {
	using namespace Genode;
	struct Driver;
}


struct Framebuffer::Driver
{
	Env                  & env;
	Timer::Connection      timer   { env };
	Attached_rom_dataspace dtb_rom { env, "dtb" };

	class Fb
	{
		private:

			Capture::Connection         _capture;
			Capture::Area const         _size;
			Capture::Connection::Screen _captured_screen;
			void                      * _base;

			/*
			 * Non_copyable
			 */
			Fb(const Fb&);
			Fb & operator=(const Fb&);

		public:

			void paint()
			{
				using Pixel = Capture::Pixel;
				Surface<Pixel> surface((Pixel*)_base, _size);
				_captured_screen.apply_to_surface(surface);
			}

			Fb(Env & env, void * base, unsigned xres, unsigned yres)
			:
				_capture(env),
				_size{xres, yres},
				_captured_screen(_capture, env.rm(), _size),
				_base(base) {}
	};

	Constructible<Fb> fb {};

	void handle_timer()
	{
		if (fb.constructed()) { fb->paint(); }
	}

	Signal_handler<Driver> timer_handler { env.ep(), *this,
	                                       &Driver::handle_timer };

	Driver(Env & env) : env(env)
	{
		Lx_kit::initialize(env);
	}

	void start()
	{
		log("--- i.MX 8MQ framebuffer driver started ---");

		lx_emul_start_kernel(dtb_rom.local_addr<void>());

		timer.sigh(timer_handler);
		timer.trigger_periodic(20*1000);
	}
};


static Framebuffer::Driver & driver(Genode::Env & env)
{
	static Framebuffer::Driver driver(env);
	return driver;
}


extern "C" void lx_emul_framebuffer_ready(void * base, unsigned long,
                                          unsigned xres, unsigned yres)
{
	Genode::Env & env = Lx_kit::env().env;
	driver(env).fb.construct(env, base, xres, yres);

	Genode::log("--- i.MX 8MQ framebuffer driver initialized ---");
}


void Component::construct(Genode::Env &env)
{
	driver(env).start();
}
