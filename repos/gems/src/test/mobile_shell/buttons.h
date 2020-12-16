/*
 * \brief  Mobile shell - button abstractions
 * \author Stefan Kalkowski
 * \date   2020-12-08
 */

/*
 * Copyright (C) 2020 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#include <base/allocator.h>
#include <base/attached_rom_dataspace.h>
#include <util/list_model.h>

namespace Shell {
	class Button;
	class Button_list;
	using namespace Genode;
};


class Shell::Button : public List_model<Button>::Element
{
	public:

		using Label = String<64>;

		Button(Label label, bool hold);

		void press(bool const hold);
		bool hold() const;
		bool has_label(Label const label) const;
		Label label() const;

	private:

		Label const _label;
		bool        _hold;
};


struct Shell::Button_list
{
	public:

		struct Change_handler : Interface
		{
			virtual void button_list_changed() = 0;
		};

		Button_list(Env            & env,
		            Allocator      & alloc,
		            Change_handler & handler);

		void handle_rom();

	private:

		Env                       & _env;
		Allocator                 & _alloc;
		Change_handler            & _change_handler;
		List_model<Button>          _list { };
		Attached_rom_dataspace      _rom { _env, "button_list" };
		Signal_handler<Button_list> _rom_handler {
			_env.ep(), *this, &Button_list::handle_rom };

		struct Update_policy : List_model<Button>::Update_policy
		{
			Button_list & button_list;

			Update_policy(Button_list & list)
			: button_list(list) {}

			Button & create_element(Xml_node node)
			{
				using Label = Button::Label;
				Label const label = node.attribute_value("label", Label());
				bool  const hold  = node.attribute_value("hold", false);
				return *new (button_list._alloc) Button(label, hold);
			}

			void update_element(Button & elem, Xml_node node) {
				elem.press(node.attribute_value("hold", false)); }

			static bool element_matches_xml_node(Button const & elem,
			                                     Xml_node node)
			{
				return elem.has_label(node.attribute_value("label",
				                                           Button::Label()));
			}

			static bool node_is_element(Genode::Xml_node node) {
				return node.has_type("button"); }

			void destroy_element(Button & elem) {
				destroy(button_list._alloc, &elem); }
		};

	public:

		template <typename FN>
		void for_each_button(FN const &fn) { _list.for_each(fn); }
};
