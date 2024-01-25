/*
 * \brief   Kernel lock
 * \author  Martin Stein
 * \author  Stefan Kalkowski
 * \date    2012-11-30
 */

/*
 * Copyright (C) 2012-2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _CORE__SPEC__SMP__KERNEL__LOCK_H_
#define _CORE__SPEC__SMP__KERNEL__LOCK_H_

namespace Kernel {
	class Cpu_pool;
	class Lock;
}


class Kernel::Lock
{
	private:

		enum { INVALID = ~0U };

		enum State { UNLOCKED, LOCKED };

		Cpu_pool         &_pool;
		int volatile      _locked      { UNLOCKED };
		unsigned volatile _current_cpu { INVALID  };

	public:

		Lock(Cpu_pool & pool) : _pool(pool) {}

		void lock();
		void unlock();

		struct Guard
		{
			Lock &_lock;

			explicit Guard(Lock &lock) : _lock(lock) { _lock.lock(); }

			~Guard() { _lock.unlock(); }
		};
};

#endif /* _CORE__SPEC__SMP__KERNEL__LOCK_H_ */
