/*
 * \brief  Scheduler for executing Task objects
 * \author Sebastian Sumpf
 * \author Josef Soentgen
 * \author Norman Feske
 * \author Stefan Kalkowski
 * \date   2014-10-10
 */

/*
 * Copyright (C) 2014-2021 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#include <util/list.h>

namespace Lx_kit {
	class Scheduler;
	class Task;
}


class Lx_kit::Scheduler
{
	public:

		Task & current();

		bool active() const;

		void add(Task & task);
		void remove(Task & task);

		void schedule();
		void unblock_irq_handler();
		void unblock_time_handler();

		Task & task(void * t);

	private:

		Genode::List<Task> _present_list { };
		Task             * _current      { nullptr };
};
