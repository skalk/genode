/**
 * \brief  System manager component - controls application gateway scenario
 * \author Stefan Kalkowski
 * \date   2021-02-22
 */

#include <base/attached_rom_dataspace.h>
#include <base/component.h>
#include <base/env.h>
#include <base/log.h>
#include <os/reporter.h>
#include <timer_session/connection.h>

#include <runtime.h>
#include <state.h>

namespace Manager { struct Main; }


struct Manager::Main
{
	Env                  & env;
	Attached_rom_dataspace subscriber_state   { env, "subscriber_state"       };
	Attached_rom_dataspace runtime_state      { env, "runtime_state"          };
	Timer::Connection      timer              { env                           };

	Signal_handler<Main> subscriber_handler { env.ep(), *this,
	                                          &Main::handle_subscriber_report };
	Signal_handler<Main> runtime_handler    { env.ep(), *this,
	                                          &Main::handle_runtime_report    };
	Signal_handler<Main> timer_handler      { env.ep(), *this,
	                                          &Main::handle_timer_signal      };
	System_state         state              { env                             };

	void handle_subscriber_report()
	{
		subscriber_state.update();
		if (!subscriber_state.valid()) { return; }

		bool all_fetched = true;
		subscriber_state.xml().for_each_sub_node("fetch", [&] (Xml_node node) {
			double total  = node.attribute_value("total", (double)-1);
			double now    = node.attribute_value("now",   (double) 0);
			if (!total || (total != now)) { all_fetched = false; }
		});
		if (all_fetched) { state.state(System_state::COPY); }
	}

	void handle_runtime_report()
	{
		runtime_state.update();
		if (!runtime_state.valid()) { return; }

		runtime_state.xml().for_each_sub_node("child", [&] (Xml_node & node) {
			String<64> name = node.attribute_value("name", String<64>());
			bool exited = node.has_attribute("exited");
			if (name == "filter" && exited) {
				state.state(System_state::WAIT);
			}
		});
	}

	void handle_timer_signal()
	{
		state.state(System_state::POLL);
	}

	Main(Env & env) : env(env)
	{
		subscriber_state.sigh(subscriber_handler);
		runtime_state.sigh(runtime_handler);
		timer.sigh(timer_handler);
		timer.trigger_periodic(state.poll_interval_us());

		log("--- system manager started ---");

		if (state.verbose()) {
			log("Periodic interval configured is ", state.poll_interval_us(),
			    " microseconds"); }

		if (state.verbose()) { log("Create new runtime configuration"); }
		state.state(System_state::POLL);
	}
};


void Component::construct(Genode::Env &env) {
	static Manager::Main main(env); }
