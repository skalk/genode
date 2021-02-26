/**
 * \brief  System manager component - runtime configuration
 * \author Stefan kalkowski
 * \date   2021-03-02
 */

#include <util/xml_generator.h>

namespace Manager {
	using namespace Genode;

	class System_state;

	/**
	 * This helper function generates the Init configuration
	 * of the managed runtime accordingly to the given current state
	 */
	void generate_runtime_config(Xml_generator & xml, System_state & state);
}
