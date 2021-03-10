/**
 * \brief  Lx_kit timeout backend
 * \author Stefan Kalkowski
 * \date   2021-05-05
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#pragma once

#include <timer_session/connection.h>

namespace Lx_kit {
	class Scheduler;
	class Timeout;
}


class Lx_kit::Timeout
{
	public:

		void start(unsigned long us);
		void stop();

		Timeout(Timer::Connection & timer,
		        Scheduler & scheduler);

	private:

		void _handle(Genode::Duration);

		using One_shot = Timer::One_shot_timeout<Timeout>;

		Scheduler         & _scheduler;
		One_shot            _timeout;
};

