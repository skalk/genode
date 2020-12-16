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

#include <windows.h>


bool Shell::Window::has_id(unsigned id) const
{
	return _id == id;
}


bool Shell::Window::has_label(Label label) const
{
	return _label == label;
}


bool Shell::Window::resize_needed() const
{
	return !_resized;
}


void Shell::Window::position(int x, int y, unsigned w, unsigned h)
{
	_resized  = (w == _geometry.w()) && (h == _geometry.h());
	_geometry = Decorator::Rect(Point(x,y), Area(w,h));
}


void Shell::Window::focus(bool focus)
{
	_focused = focus;
}


void Shell::Window::generate(Xml_generator & xml) const
{
	xml.node("window", [&]() {
		xml.attribute("id",     _id);
		xml.attribute("title",  _label);
		xml.attribute("xpos",   _geometry.x1());
		xml.attribute("ypos",   _geometry.y1());
		xml.attribute("width",  _geometry.w());
		xml.attribute("height", _geometry.h());
		if (_focused) { xml.attribute("focused", "yes"); }
	});
}


void Shell::Window::resize(Xml_generator & xml)
{
	xml.node("window", [&]() {
		xml.attribute("id",     _id);
		xml.attribute("width",  _geometry.w());
		xml.attribute("height", _geometry.h());
	});
	_resized = true;
}


Shell::Window::Window(unsigned id, Label label)
: _id(id), _label(label) {}


void Shell::Window_list::handle_rom()
{
	_rom.update();

	Update_policy policy(*this);
	_list.update_from_xml(policy, _rom.xml());

	_change_handler.window_list_changed();
}


Shell::Window_list::Window_list(Env            & env,
                                Allocator      & alloc,
                                Change_handler & handler)
: _env(env),
  _alloc(alloc),
  _change_handler(handler)
{
	_rom.sigh(_rom_handler);
}
