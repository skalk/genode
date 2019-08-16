/*
 * \brief  Test server
 * \author Alexander Boettcher
 * \date   2019-08-06
 */

/*
 * Copyright (C) 2019 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#include <base/rpc_server.h>
#include <base/rpc_client.h>

namespace Test
{
	using namespace Genode;

	class Session;
	class Client;
	class Component;
	class Connection;

	typedef Genode::Capability<Session> Capability;
};

struct Test::Session : Genode::Session
{
	static const char *service_name() { return "PINGPONG"; }
	enum { CAP_QUOTA = 2 };

	GENODE_RPC(Rpc_pingpong, void, pingpong);
	GENODE_RPC_INTERFACE(Rpc_pingpong);
};

struct Test::Client : Genode::Rpc_client<Test::Session>
{
	Client(Genode::Capability<Test::Session> cap)
	: Rpc_client<Test::Session>(cap) { }

	void pingpong() { call<Test::Session::Rpc_pingpong>(); }
};

struct Test::Component : Genode::Rpc_object<Test::Session, Test::Component>
{
	void pingpong()
	{
//		Genode::log("receivid ping - respond");
	}
};
