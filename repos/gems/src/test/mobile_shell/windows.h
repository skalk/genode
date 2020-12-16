/*
 * \brief  Mobile shell - window abstractions
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
#include <decorator/types.h>
#include <util/xml_generator.h>

namespace Shell {
	class Window;
	class Window_list;
	using namespace Genode;
};


class Shell::Window : public List_model<Window>::Element
{
	public:

		using Label = String<64>;

		Window(unsigned id, Label label);

		void generate(Xml_generator &xml) const;
		void resize(Xml_generator &xml);

		bool has_id(unsigned id) const;
		bool has_label(Label label) const;
		bool resize_needed() const;

		void position(int x, int y, unsigned w, unsigned h);
		void focus(bool focus);

	private:

		unsigned const  _id;
		Label    const  _label;
		Decorator::Rect _geometry { };
		bool            _focused  { false };
		bool            _resized  { false };
};


struct Shell::Window_list
{
	public:

		struct Change_handler : Interface
		{
			virtual void window_list_changed() = 0;
		};

		Window_list(Env            & env,
		            Allocator      & alloc,
		            Change_handler & handler);

		void handle_rom();

	private:

		Env                       & _env;
		Allocator                 & _alloc;
		Change_handler            & _change_handler;
		List_model<Window>          _list { };
		Attached_rom_dataspace      _rom { _env, "window_list" };
		Signal_handler<Window_list> _rom_handler {
			_env.ep(), *this, &Window_list::handle_rom };

		struct Update_policy : List_model<Window>::Update_policy
		{
			Window_list & window_list;

			Update_policy(Window_list & list)
			: window_list(list) {}

			Window & create_element(Xml_node node)
			{
				using Label = Window::Label;
				unsigned long const id    = node.attribute_value("id", 0UL);
				Label const label = node.attribute_value("label", Label());
				return *new (window_list._alloc) Window(id, label);
			}

			void update_element(Window &, Xml_node) {}

			static bool element_matches_xml_node(Window const & elem,
			                                     Xml_node node) {
				return elem.has_id(node.attribute_value("id", 0UL)); }

			static bool node_is_element(Genode::Xml_node node) {
				return node.has_type("window"); }


			void destroy_element(Window & elem) {
				destroy(window_list._alloc, &elem); }
		};

	public:

		template <typename FN>
		void for_each_window(FN const &fn) { _list.for_each(fn); }

		template <typename FN>
		void for_window(Window::Label label, FN const &fn)
		{
			for_each_window([&] (Window & window) {
				if (window.has_label(label)) { fn(window); }
			});
		}
};
