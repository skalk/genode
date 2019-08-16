/*
 * \brief  Slave attempt - UNFINISHED
 * \author Alexander Boettcher
 * \date   2019-08-06
 */

/*
 * Copyright (C) 2019 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#pragma once

#include <os/slave.h>
#include <os/static_parent_services.h>
#include <root/component.h>

namespace Test
{
	using namespace Genode;

	class Root;
	class Connection;

	struct Policy;
	struct Inter_pd_master;
	struct Inter_pd_slave;
};

struct Test::Policy
:
	private Static_parent_services<Cpu_session, Rom_session,
	                               Pd_session, Log_session>,
	public Slave::Policy
{
	Genode::Signal_context_capability _do_later;

	Policy(Env &env, Name const &name,
	       Genode::Signal_context_capability do_later)
	:
		Static_parent_services(env),
		Slave::Policy(env, name, name, *this, env.ep().rpc_ep(),
		              Cap_quota{100}, Ram_quota{1024*1024}),
		_do_later(do_later)
	{ }

	void announce_service(Service::Name const &service_name) override
	{
		Genode::error("ohoh ", service_name);
		if (service_name == "PINGPONG") {
			Genode::error("yes");
			Genode::Signal_transmitter(_do_later).submit();
		}
	}
};

struct Test::Inter_pd_master
{
	Genode::Env   &_env;

	Genode::Signal_handler<Inter_pd_master> _wake_up { _env.ep(), *this, &Inter_pd_master::xxx };

	Test::Policy   _policy { _env, "test-pingpong", _wake_up };
	Genode::Child  _child  { _env.rm(), _env.ep().rpc_ep(), _policy };

	Inter_pd_master(Genode::Env &env)
	:
		_env(env)
	{
		unsigned _cnt = 0; 
		Genode::String<256> const config("<config><counter>", _cnt, "</counter></config>");
		_policy.configure(config.string());
	}

	void xxx()
	{
/*
		Test::Policy::Child_policy::Route route = _policy.resolve_session_request("PINGPONG", "");
		Genode::Service &service = route.service;
*/
/*
		Genode::Session_label label;
		Genode::Parent::Client::Id id { 34 };
		Genode::Session_state::Args args;
		Genode::Affinity affinity;

		service.create_session(_child.session_factory(), _env.id_space(), id, label, args, affinity);
*/
		Genode::Parent::Client::Id id { 34 };
		Genode::Parent::Session_args args { "ram_quota=16K " };
		Genode::Affinity affinity;
		_child.session(id, "PINGPONG", args, affinity);
	}
};

class Test::Root : public Root_component<Test::Component>
{
	public:

		Root(Entrypoint &ep, Allocator &md_alloc)
		:
			Root_component(ep, md_alloc)
		{ }

		Test::Component *_create_session(const char *, Affinity const &) override
		{
			return new (md_alloc()) Test::Component();
		}
};

struct Test::Inter_pd_slave
{
	Genode::Env         &_env;
	Genode::Sliced_heap  _sliced_heap { _env.ram(), _env.rm() };
	Test::Root           _root        { _env.ep(), _sliced_heap };

	Inter_pd_slave(Genode::Env &env) : _env(env)
	{
		_env.parent().announce(_env.ep().manage(_root));
	}
};
