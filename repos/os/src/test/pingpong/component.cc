/*
 * \brief  Pingpong
 * \author Alexander Boettcher
 * \date   2019-08-06
 */

/*
 * Copyright (C) 2019 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#include <base/component.h>
#include <base/attached_rom_dataspace.h>

#include "test.h"
#include "trace.h"
#include "inter.h"

namespace Test
{
	using namespace Genode;

	struct Intra_pd;
};

struct Intra_pd : Test::Tracing
{
	Genode::Env     &_env;

	Genode::Rpc_entrypoint _rpc_server { &_env.pd(), 4 * 4096, "call_me" };
	Test::Component        _comp       { };
	Test::Capability       _rpc_cap    { _rpc_server.manage(&_comp) };
	Test::Client           _client     { _rpc_cap };

	/*
	 * Ping pong
	 */
	Intra_pd(Genode::Env &env, bool use_tracing) : Tracing(env), _env(env)
	{
		unsigned const rounds = 100;

		Genode::Trace::Timestamp times[rounds];

		Genode::log("test: warm up");

		if (use_tracing)
			start_tracing();

		/* make sure all code is fetched by one prepare call to server */
		_client.pingpong();

		Genode::log("test: start measurement");

		Genode::Trace::Execution_time const idle_start = idle_time();

		/* start measurement */
		asm volatile (""::"r"(&idle_start):"memory");
		Genode::Trace::Timestamp const start = Genode::Trace::timestamp();
		asm volatile (""::"r"(&start):"memory");

		for (unsigned i = 0; i < rounds; i++) {
			_client.pingpong();
			if (use_tracing)
				times[i] = Genode::Trace::timestamp();
		}

		Genode::Trace::Timestamp const end = use_tracing ? times[rounds - 1] : Genode::Trace::timestamp();

		asm volatile (""::"r"(&end):"memory");
		/* end measurement */

		Genode::Trace::Execution_time const idle_end = idle_time();
		asm volatile (""::"r"(&idle_end):"memory");

		if (use_tracing)
			stop_tracing();

		Genode::log("test: stop measurement\n");

		Genode::log("result: rounds ", rounds, ", cycles ", end - start);
		Genode::log("result: average cycles/round: ", ((end - start) / rounds));
		Genode::log("result: idle time - thread     context: ", idle_end.thread_context - idle_start.thread_context, "(", idle_end.thread_context, ",", idle_start.thread_context,")");
		Genode::log("result: idle time - scheduling context: ", idle_end.scheduling_context - idle_start.scheduling_context, "(", idle_end.scheduling_context, ",", idle_start.scheduling_context,")");

		if (use_tracing) {
			Genode::log("\nresult: tracing evaluation:");
			evaluate_tracing(1 /* skip first measurements of test call */, times, rounds, start);
		}
	}

};

struct Test::Connection : Genode::Connection<Test::Session>, Test::Client
{
	Connection(Env &env)
	:
		Genode::Connection<Test::Session>(env, session(env.parent(),
		                                     " cap_quota=4, ")),
		Client(cap())
	{ }
};

#if 0
static bool master(Genode::Attached_rom_dataspace &config)
{
	try {
		config.xml().sub_node("parent-provides");
	} catch (...) {
		return false;
	}

	return true;
}
#endif

void Component::construct(Genode::Env &env)
{
	Genode::Attached_rom_dataspace config { env, "config" };

	bool const use_tracing = config.xml().attribute_value("trace", false);

	static Intra_pd intra_pd(env, use_tracing);

#if 0
	if (master(config)) {

//		static Test::Inter_pd_master inter_pd(env);

//		inter_pd.xxx();
	//	Test::Connection k(env);
	} else {
//		static Test::Inter_pd_slave  inter_pd(env);
	}
#endif
}
