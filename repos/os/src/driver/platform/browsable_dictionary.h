/*
 * \brief  Platform driver - browsable dictionary
 * \author Stefan Kalkowski
 * \date   2026-03-16
 */

/*
 * Copyright (C) 2026 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _SRC__DRIVER__PLATFORM__BROWSABLE_DICTIONARY_H_
#define _SRC__DRIVER__PLATFORM__BROWSABLE_DICTIONARY_H_

#include <base/registry.h>
#include <util/dictionary.h>

namespace Driver {
	template <typename T, typename NAME> class Browsable_dictionary;
}


template <typename T, typename NAME>
class Driver::Browsable_dictionary : public Genode::Dictionary<T, NAME>
{
	public:

		struct Element;

	private:

		Genode::Registry<Element> _elems {};

		friend class Element;

	public:

		class Element : Genode::Registry<Element>::Element,
		                public Genode::Dictionary<T, NAME>::Element
		{
			private:

				friend class Genode::Avl_tree<T>;
				friend class Genode::Avl_node<T>;
				friend class Genode::Dictionary<T, NAME>;

			public:

				Element(Browsable_dictionary &dict, NAME const &name)
				:
					Genode::Registry<Element>::Element(dict._elems, *this),
					Genode::Dictionary<T, NAME>::Element(dict, name)
				{}
		};

		void for_each(auto const &fn)
		{
			_elems.for_each([&] (auto &elem) {
				fn(static_cast<T&>(elem)); });
		}
};

#endif /* _SRC__DRIVER__PLATFORM__BROWSABLE_DICTIONARY_H_ */
