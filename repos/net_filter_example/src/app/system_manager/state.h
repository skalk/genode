/**
 * \brief  System manager component - system state
 * \author Stefan Kalkowski
 * \date   2021-03-02
 */

#include <base/attached_rom_dataspace.h>
#include <base/env.h>
#include <base/heap.h>
#include <os/reporter.h>

#include <runtime.h>

namespace Manager {
	struct Resource;
	class System_state;
}


/**
 * A Resource holds the data needed to collect, and copy over a single item
 * from the blue network to the green network
 */
struct Manager::Resource : Genode::List<Manager::Resource>::Element
{
	using Label = String<256>;

	Label const name; /* file name of the resource         */
	Label const url;  /* URL where to collect the resource */

	/**
	 * Constructs a resource from the corresponding resource
	 * Xml_node, and inserts it into the given resource list
	 */
	Resource(Xml_node xml, List<Resource> & list)
	:
		name(xml.attribute_value("name", Label())),
		url(xml.attribute_value("url",   Label()))
	{
		list.insert(this);
	}
};


/**
 * The System_state holds configuration data, like the resources to collect,
 * keeps the whole system state, and verifies state transitions
 */
class Manager::System_state
{
	public:

		enum State { POLL, COPY, WAIT };

	private:

		Env                  & _env;
		Heap                   _heap           { _env.ram(), _env.rm() };
		Attached_rom_dataspace _config         { _env, "config"   };
		Expanding_reporter     _runtime_config { _env, "config",
		                                         "runtime_config" };

		State      _state   { WAIT };
		bool const _verbose { _config.xml().attribute_value("verbose", false) };

		unsigned const _interval_ms {
			_config.xml().attribute_value("interval_ms", 5000U) };

		List<Resource> _resources {};

	public:

		System_state(Env & env)
		: _env(env)
		{
			_config.xml().for_each_sub_node("resource", [&] (Xml_node node) {
				new (_heap) Resource(node, _resources); });
		}

		void state(State state)
		{
			if ((state == WAIT && _state != COPY) ||
			    (state == POLL && _state != WAIT) ||
			    (state == COPY && _state != POLL)) { return; }

			_state = state;
			_runtime_config.generate([&] (Xml_generator & xml) {
				generate_runtime_config(xml, *this); });
		}

		State state()   const { return _state;   }
		bool  verbose() const { return _verbose; }

		unsigned poll_interval_us() const { return _interval_ms * 1000; }

		template <typename FN>
		void for_each_resource(FN const & fn) const
		{
			for (Resource const * r = _resources.first(); r; r = r->next()) {
				fn(*r); }
		}
};
