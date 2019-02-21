/*
 * \brief  Some platform tests for the base-hw
 * \author Stefan Kalkowski
 * \date   2019-02-06
 *
 */

/*
 * Copyright (C) 2019 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#include <base/log.h>
#include <base/component.h>
#include <base/heap.h>
#include <base/signal.h>
#include <base/rpc_server.h>
#include <util/reconstructible.h>
#include <kernel/interface.h>

using namespace Genode;

enum Timeouts_us {
	MS_1   = 1000,
	MS_MAX = 10000000,
};

struct Timer_thread : Thread
{
	unsigned cnt { 0 };
	unsigned timeo;
	Signal_receiver sig_recv { };
	Genode::Signal_context ctx { };
	Signal_context_capability cap { sig_recv.manage(&ctx) };

	void entry()
	{
		for (;;) {
			Kernel::timeout(timeo, (addr_t)cap.data());
			sig_recv.wait_for_signal();
			cnt++;

		}
	}

	Timer_thread(Env & env, unsigned t)
	: Thread(env, "timer", 2048*sizeof(long)), timeo(t) {
		start(); };
};


struct Main
{
	enum { STEP = 10 * MS_1, THREADS = 10 };

	Genode::Env  &env;
	Genode::Heap  heap { env.ram(), env.rm() };
	Constructible<Timer_thread> threads[THREADS];
	Signal_receiver sig_recv { };
	Genode::Signal_context ctx { };
	Signal_context_capability cap { sig_recv.manage(&ctx) };

	Main(Env &env);

	Kernel::time_t wait(unsigned ms)
	{
		Kernel::time_t time = Kernel::time();
		Kernel::timeout(ms*1000, (addr_t)cap.data());
		sig_recv.wait_for_signal();
		return Kernel::time() - time;
	}
};


Main::Main(Env &env) : env(env)
{
	log("testing base-hw platform");

	if (Kernel::timeout_max_us() < STEP * THREADS + MS_1) {
		Genode::error("maximum timeout less than 1 second");
		return;
	}

#if 0
	for (unsigned i = 10; i < 20000; i*=2) {
		log("wait ", i, " ms");
		Kernel::time_t us = wait(i);
		if ((us / 1000) != i) error("waited ", us, " us instead of ", i*1000);
	}

	log("sleep times succeeded");
#endif

	for (unsigned i = 0; i < THREADS; i++)
		threads[i].construct(env, MS_1 + STEP*i);

	log("will wait a bit");
	//wait(1000);
	wait(MS_MAX/1000);
	log("lets look");

	for (unsigned i = 0; i < THREADS; i++)
		log(threads[i]->cnt, " ", MS_MAX / threads[i]->timeo);

	log("testing base-hw platform done");
}


void Component::construct(Genode::Env &env) { static Main main(env); }
