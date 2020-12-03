/*
 * \brief  Event file system
 * \author Christian Prochaska
 * \date   2020-11-26
 */

/*
 * Copyright (C) 2020 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Genode includes */
#include <util/xml_generator.h>
#include <vfs/dir_file_system.h>
#include <vfs/file_system_factory.h>

/* local includes */
#include <event_text_file_system.h>


namespace Vfs_event {
	using namespace Vfs;
	using namespace Genode;

	class Local_factory;
	class File_system;
}


struct Vfs_event::Local_factory : File_system_factory
{
	Vfs::Env &_vfs_env;

	Event_text_file_system _event_text_fs { _vfs_env.env() };

	Local_factory(Vfs::Env &vfs_env) : _vfs_env(vfs_env) { }

	Vfs::File_system *create(Vfs::Env&, Xml_node node) override
	{
		if (node.has_type(Event_text_file_system::name()))
			return &_event_text_fs;

		return nullptr;
	}
};


class Vfs_event::File_system : private Local_factory,
                               public  Vfs::Dir_file_system
{
	private:

		typedef String<200> Config;
		static Config _config(Xml_node node)
		{
			char buf[Config::capacity()] { };

			Xml_generator xml(buf, sizeof(buf), "dir", [&] () {
				typedef String<64> Name;
				xml.attribute("name", node.attribute_value("name", Name("event")));
				xml.node("text", [&] () { });
			});

			return Config(Cstring(buf));
		}

	public:

		File_system(Vfs::Env &vfs_env, Genode::Xml_node node)
		:
			Local_factory(vfs_env),
			Vfs::Dir_file_system(vfs_env,
			                     Xml_node(_config(node).string()),
			                     *this)
		{ }

		char const *type() override { return "event"; }
};


struct Event_factory : Vfs::File_system_factory
{
	Vfs::File_system *create(Vfs::Env &vfs_env,
	                         Genode::Xml_node node) override
	{
		return new (vfs_env.alloc())
		           Vfs_event::File_system(vfs_env, node);
	}
};


extern "C" Vfs::File_system_factory *vfs_file_system_factory(void)
{
	static Event_factory factory;
	return &factory;
}
