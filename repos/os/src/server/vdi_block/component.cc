/*
 * \brief  VDI file as a Block session
 * \author Josef Soentgen
 * \author Alexander Boettcher
 * \date   2018-11-01
 */

/*
 * Copyright (C) 2018-2020 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode includes */
#include <base/attached_ram_dataspace.h>
#include <base/attached_rom_dataspace.h>
#include <base/component.h>
#include <base/log.h>
#include <timer_session/connection.h>
#include <util/xml_node.h>

#include "vdi_file.h"

namespace Vdi {
	struct Main;
	struct Block_session_handler;
	struct Block_session_component;
	using namespace Genode;
}

struct Vdi::Block_session_handler : Interface
{
	Genode::Env             &env;

	Signal_handler<Block_session_handler> request_handler
	  { env.ep(), *this, &Block_session_handler::handle};

	Block_session_handler(Env &env) : env(env) { }

	~Block_session_handler() { }

	virtual void handle_requests()= 0;

	void handle()
	{
		handle_requests();
	}
};

struct Vdi::Block_session_component : Rpc_object<::Block::Session>,
                                      Block_session_handler,
                                      ::Block::Request_stream
{
	Vdi::File &vdi;

	Block_session_component(Env &env, Ram_dataspace_capability ram_cap,
	                        Vdi::File &file)
	:
	  Block_session_handler(env),
	  Request_stream(env.rm(), ram_cap, env.ep(), request_handler, file.info()),
	  vdi(file)
	{
		env.ep().manage(*this);

		vdi.set_notify_cap(request_handler);
	}

	~Block_session_component()
	{
		vdi.set_notify_cap(Genode::Signal_context_capability());

		env.ep().dissolve(*this);
	}

	Info info() const override { return Request_stream::info(); }

	Capability<Tx> tx_cap() override { return Request_stream::tx_cap(); }

	void handle_requests() override
	{
		while (true) {

			bool progress = false;

			with_requests([&] (::Block::Request request) {

				Response response = Response::RETRY;

				with_payload([&] (Request_stream::Payload const &payload) {
					response = vdi.handle(request, payload);

					if (response == Response::ACCEPTED) {

						progress = true;
						request.success = true;

						bool done = false;
						try_acknowledge([&](Ack &ack) {

							if (done) return;

							ack.submit(request);
							done = true;
						});

						if (!done)
							Genode::error("ack missing ... stall ahead");

#if 0
						Genode::log("request ", (int)request.operation.type,
						            request.operation.type == ::Block::Operation::Type::WRITE ? " write" : "",
						            request.operation.type == ::Block::Operation::Type::READ  ? " read" : "",
						            " request offset=",request.offset,
						            " block=", request.operation.block_number,
						            " count=", request.operation.count,
						            " done");
#endif
					} else {
						if (response == Response::RETRY)
							return;

						Genode::error("unknown state - request ", (int)request.operation.type,
						              request.operation.type == ::Block::Operation::Type::WRITE ? " write" : "",
						              request.operation.type == ::Block::Operation::Type::READ  ? " read" : "",
						              " request offset=",request.offset,
						              " block=", request.operation.block_number,
						              " count=", request.operation.count);
						Genode::Mutex mutex;
						while (true) { mutex.acquire(); }
					}
				});

				return response;
			});

			if (progress == false) break;
		}

		/* poke */
		wakeup_client_if_needed();
	}
};

struct Vdi::Main : Rpc_object<Typed_root<::Block::Session>>
{
	Env                                    &env;
	Attached_rom_dataspace                  config { env, "config" };
	Constructible<Attached_ram_dataspace>   block_ds { };
	Constructible<Vdi::File>                vdi_file { };
	Constructible<Block_session_component>  client { };
	Signal_handler<Main>                    notify { env.ep(), *this,
	                                                 &Main::init};

	void init()
	{
		bool init_done = vdi_file->init(notify);
		if (init_done)
			env.parent().announce(env.ep().manage(*this));
	}

	Main(Env &env)
	: env(env)
	{
		log("--- Starting VDI driver ---");

		vdi_file.construct(env, config.xml());
		vdi_file->init(notify);
	}

	Session_capability session(Root::Session_args const &args,
	                           Affinity const &) override
	{
		if (client.constructed())
			throw Service_denied();
		if (block_ds.constructed())
			throw Service_denied();

		Session_label  const label = label_from_args(args.string());

		Ram_quota const ram_quota = ram_quota_from_args(args.string());
		size_t const tx_buf_size =
			Arg_string::find_arg(args.string(), "tx_buf_size").ulong_value(0);

		if (!tx_buf_size)
			throw Service_denied();

		if (tx_buf_size > ram_quota.value) {
			error("insufficient 'ram_quota' from '", label, "',"
			      " got ", ram_quota, ", need ", tx_buf_size);
			throw Insufficient_ram_quota();
		}

		try {
			block_ds.construct(env.ram(), env.rm(), tx_buf_size);
			client.construct(env, block_ds->cap(), *vdi_file);
			return client->cap();
		} catch (...) {
			error("rejecting session request '", label, "'");
		}

		throw Service_denied();
	}

	void upgrade(Session_capability, Root::Upgrade_args const&) override {
		Genode::warning(__func__, " not implemented"); }

	void close(Session_capability cap) override {
		Genode::warning(__func__, " not implemented ", cap);
		if (block_ds.constructed())
			block_ds.destruct();
		if (client.constructed())
			client.destruct();
	}
};

void Component::construct(Genode::Env &env) { static Vdi::Main server(env); }
