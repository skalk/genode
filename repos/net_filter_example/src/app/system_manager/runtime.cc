/**
 * \brief  System manager component - runtime config generation
 * \author Stefan Kalkowski
 * \date   2021-03-02
 */


#include <log_session/log_session.h>
#include <rm_session/rm_session.h>
#include <rom_session/rom_session.h>
#include <timer_session/timer_session.h>
#include <nic_session/nic_session.h>
#include <rtc_session/rtc_session.h>

#include <runtime.h>
#include <state.h>

/** include private header from sculpt manager to simplify xml generation **/
#include <xml.h>

using namespace Manager;
using namespace Genode;
using namespace Sculpt;
using Label = String<64>;


/**
 * Create the parent-provides rules of the runtime configuration
 */
static inline void gen_parent_provides(Xml_generator & xml)
{
	xml.node("parent-provides", [&] () {
			gen_parent_service<Rom_session>(xml);
			gen_parent_service<Cpu_session>(xml);
			gen_parent_service<Pd_session>(xml);
			gen_parent_service<Log_session>(xml);
			gen_parent_service<Timer::Session>(xml);
			gen_parent_service<Nic::Session>(xml);
			gen_parent_service<Report::Session>(xml);
			gen_parent_service<Rtc::Session>(xml);
	});
}


/**
 * Create a vfs component start node
 *
 * \param name    name of the new node
 * \param writer  label of the service route that gets write access to the vfs
 */
static inline void gen_vfs_node(Xml_generator & xml, Label name, Label writer)
{
	xml.node("start", [&] () {
		gen_common_start_content(xml, name.string(),
		                         Cap_quota{200},
		                         Ram_quota{16*1024*1024});
		gen_named_node(xml, "binary", "vfs");
		gen_provides<File_system::Session>(xml);
		xml.node("config", [&] () {
			xml.node("vfs", [&] () { xml.node("ram", [&] () { }); });
			xml.node("policy", [&] () {
				xml.attribute("label", writer.string());
				xml.attribute("root",      "/");
				xml.attribute("writeable", true);
			});
			xml.node("default-policy", [&] () {
				xml.attribute("root",      "/");
				xml.attribute("writeable", false);
			});
		});
		xml.node("route", [&] () {
			gen_parent_route<Rom_session> (xml);
			gen_parent_route<Cpu_session> (xml);
			gen_parent_route<Pd_session>  (xml);
			gen_parent_route<Log_session> (xml);
		});
	});
}


/**
 * Create the common libc configuration for a libc-dependent component
 */
static inline void gen_libc(Xml_generator & xml, bool nic)
{
	xml.node("libc", [&] () {
		xml.attribute("stdin",  "/dev/null");
		xml.attribute("stdout", "/dev/log");
		xml.attribute("stderr", "/dev/log");
		xml.attribute("rtc",    "/dev/rtc");
		xml.attribute("rng",    "/dev/random");
		if (nic) { xml.attribute("socket", "/socket"); }
	});
}


/**
 * Create the common libc vfs configuration for a libc-dependent component
 *
 * \param nic  true is network is needed
 * \param fn   functor to create additional content in the vfs config node
 */
template <typename FN>
static inline void gen_libc_vfs(Xml_generator & xml, bool nic, FN const & fn)
{
	xml.node("vfs", [&] () {

		gen_named_node(xml, "dir", "dev", [&] () {
			xml.node("log", [&] () {});
			xml.node("null", [&] () {});
			xml.node("rtc", [&] () {});
			gen_named_node(xml, "jitterentropy", "random");
			gen_named_node(xml, "jitterentropy", "urandom");
		});

		if (nic) {
			gen_named_node(xml, "dir", "socket", [&] () {
				xml.node("lxip", [&] () { xml.attribute("dhcp", true); });
			});
		}

		fn();
	});
}


/**
 * Create the 'publisher' meaning the MQTT placeholder for the green network
 */
static inline void gen_publisher(Xml_generator & xml)
{
	xml.node("start", [&] () {
		gen_common_start_content(xml, "publisher",
		                         Cap_quota{300},
		                         Ram_quota{128*1024*1024});
		gen_named_node(xml, "binary", "lighttpd");

		xml.node("config", [&] () {

			gen_libc_vfs(xml, true, [&] () {
				gen_named_node(xml, "rom", "lighttpd.conf");
				gen_named_node(xml, "dir", "content", [&] () {
					xml.node("fs", [&] () { });
				});
			});

			gen_libc(xml, true);

			xml.node("arg", [&] () { xml.attribute("value", "lighttpd"); });
			xml.node("arg", [&] () { xml.attribute("value", "-f"); });
			xml.node("arg", [&] () { xml.attribute("value",
			                                       "/lighttpd.conf"); });
			xml.node("arg", [&] () { xml.attribute("value", "-D"); });
		});

		xml.node("route", [&] () {
			gen_parent_route<Rom_session>(xml);
			gen_parent_route<Cpu_session>(xml);
			gen_parent_route<Pd_session> (xml);
			gen_parent_route<Log_session>(xml);
			gen_parent_route<Timer::Session>(xml);
			gen_parent_route<Rtc::Session>(xml);
			gen_named_node(xml, "service",
			               Nic::Session::service_name(), [&] () {
				xml.node("parent", [&] () {
					xml.attribute("label", "green");
				});
			});
			gen_named_node(xml, "service",
			               File_system::Session::service_name(), [&] () {
				gen_named_node(xml, "child", "green_fs");
			});
		});
	});
}


/**
 * Create the 'subscriber' meaning the MQTT placeholder for the blue network
 */
static inline void gen_subscriber(Xml_generator & xml, System_state & state)
{
	xml.node("start", [&] () {
		gen_common_start_content(xml, "subscriber",
		                         Cap_quota{300},
		                         Ram_quota{128*1024*1024});
		gen_named_node(xml, "binary", "fetchurl");

		xml.node("config", [&] () {

			gen_libc_vfs(xml, true, [&] () {
				gen_named_node(xml, "dir", "content", [&] () {
					xml.node("fs", [&] () { });
				});
			});

			gen_libc(xml, true);

			state.for_each_resource([&] (Resource const & resource) {
				xml.node("fetch", [&] () {
					Resource::Label path { "/content/", resource.name };
					xml.attribute("url",  resource.url);
					xml.attribute("path", path.string());
					xml.attribute("retry", 3);
				});
			});
			xml.node("report", [&] () { xml.attribute("progress", true); });
		});

		xml.node("route", [&] () {
			gen_parent_route<Rom_session>(xml);
			gen_parent_route<Cpu_session>(xml);
			gen_parent_route<Pd_session> (xml);
			gen_parent_route<Log_session>(xml);
			gen_parent_route<Timer::Session>(xml);
			gen_parent_route<Rtc::Session>(xml);
			gen_parent_route<Report::Session>(xml);
			gen_named_node(xml, "service",
			               Nic::Session::service_name(), [&] () {
				xml.node("parent", [&] () {
					xml.attribute("label", "blue");
				});
			});
			gen_named_node(xml, "service",
			               File_system::Session::service_name(), [&] () {
				gen_named_node(xml, "child", "blue_fs");
			});
		});
	});
}


/**
 * Create the VFS used by the filter component
 */
static inline void gen_filter_vfs(Xml_generator & xml)
{
	xml.node("start", [&] () {
		gen_common_start_content(xml, "vfs",
		                         Cap_quota{300},
		                         Ram_quota{10*1024*1024});
		gen_provides<File_system::Session>(xml);
		xml.node("config", [&] () {
			gen_libc_vfs(xml, false, [&] () {
				gen_named_node(xml, "tar", "coreutils-minimal.tar");
				gen_named_node(xml, "dir", "src", [&] () {
					xml.node("fs", [&] () {
						xml.attribute("label", "blue");
					});
				});
				gen_named_node(xml, "dir", "dst", [&] () {
					xml.node("fs", [&] () {
						xml.attribute("label", "green");
					});
				});
			});
			xml.node("default-policy", [&] () {
				xml.attribute("root",      "/");
				xml.attribute("writeable", true);
			});
		});

		xml.node("route", [&] () {
			gen_parent_route<Rom_session>(xml);
			gen_parent_route<Cpu_session>(xml);
			gen_parent_route<Pd_session> (xml);
			gen_parent_route<Log_session>(xml);
			gen_parent_route<Rtc::Session>(xml);
			gen_parent_route<Timer::Session>(xml);
			gen_named_node(xml, "service",
			               File_system::Session::service_name(), [&] () {
				xml.attribute("label", "blue");
				gen_named_node(xml, "child", "blue_fs");
			});
			gen_named_node(xml, "service",
			               File_system::Session::service_name(), [&] () {
				xml.attribute("label", "green");
				gen_named_node(xml, "child", "green_fs");
			});
		});
	});
}


/**
 * Create a fs_rom component for the filter component, which translates files
 * from the filter's VFS into ROMs, needed for the ELF binary loading
 */
static inline void gen_filter_vfs_rom(Xml_generator & xml)
{
	xml.node("start", [&] () {
		gen_common_start_content(xml, "fs_rom",
		                         Cap_quota{100},
		                         Ram_quota{10*1024*1024});

		gen_provides<Rom_session>(xml);
		xml.node("config", [&] () { });

		xml.node("route", [&] () {
			gen_parent_route<Rom_session>(xml);
			gen_parent_route<Cpu_session>(xml);
			gen_parent_route<Pd_session> (xml);
			gen_parent_route<Log_session>(xml);
			gen_named_node(xml, "service",
			               File_system::Session::service_name(), [&] () {
				gen_named_node(xml, "child", "vfs");
			});
		});
	});
};


/**
 * Create the filter component start node
 */
static inline void gen_filter(Xml_generator & xml, System_state & state)
{
	xml.node("report",  [&] () {
		xml.attribute("state", true);
	});

	gen_filter_vfs(xml);
	gen_filter_vfs_rom(xml);

	xml.node("start", [&] () {
		gen_common_start_content(xml, "filter",
		                         Cap_quota{100},
		                         Ram_quota{10*1024*1024});
		gen_named_node(xml, "binary", "/bin/cp");
		xml.node("config", [&] () {
			gen_libc(xml, false);
			xml.node("vfs", [&] () {
				xml.node("fs", [&] () {});
			});

			xml.node("arg", [&] () { xml.attribute("value", "cp"); });

			state.for_each_resource([&] (Resource const & resource) {
				Resource::Label path { "/src/", resource.name };
				xml.node("arg", [&] () {
					xml.attribute("value", path.string()); });
			});

			xml.node("arg", [&] () { xml.attribute("value", "/dst/"); });
		});

		xml.node("route", [&] () {
			gen_named_node(xml, "service",
			               File_system::Session::service_name(), [&] () {
				gen_named_node(xml, "child", "vfs");
			});
			gen_named_node(xml, "service",
			               Rom_session::service_name(), [&] () {
				xml.attribute("label_last", "/bin/cp");
				gen_named_node(xml, "child", "fs_rom");
			});
			gen_parent_route<Rom_session>(xml);
			gen_parent_route<Cpu_session>(xml);
			gen_parent_route<Pd_session> (xml);
			gen_parent_route<Log_session>(xml);
			gen_parent_route<Timer::Session>(xml);
		});
	});
};


void Manager::generate_runtime_config(Xml_generator & xml, System_state & state)
{
	xml.attribute("verbose", state.verbose());

	gen_parent_provides(xml);
	gen_vfs_node(xml, "blue_fs",  "subscriber -> ");
	gen_vfs_node(xml, "green_fs", "vfs -> green");
	gen_publisher(xml);

	switch (state.state()) {
	case System_state::WAIT: break;
	case System_state::COPY: gen_filter(xml, state); break;
	case System_state::POLL: gen_subscriber(xml, state);
	};
}
