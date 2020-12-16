/*
 * \brief  Mobile shell - fill the window layouter role
 * \author Stefan Kalkowski
 * \date   2020-12/08
 */

/*
 * Copyright (C) 2020 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#include <base/component.h>
#include <base/heap.h>
#include <gui_session/connection.h>
#include <os/reporter.h>

#include <buttons.h>
#include <windows.h>

namespace Shell { class Main; }


struct Shell::Main : public Button_list::Change_handler,
                     public Window_list::Change_handler
{
	public:

		Main(Env & env);
		void button_list_changed() override;
		void window_list_changed() override;

	private:

		enum { PANEL_HEIGHT = 45, KEYBOARD_DIV = 256, KEYBOARD_FAC = 100 };

		void _handle_mode();
		void _update_window_layout();

		Env                & _env;
		Heap                 _heap         { _env.ram(), _env.rm() };
		Gui::Connection      _gui          { _env };
		Framebuffer::Mode    _mode         { _gui.mode() };
		Signal_handler<Main> _mode_handler { _env.ep(), *this,
		                                     &Main::_handle_mode };
		Button_list          _button_list  { _env, _heap, *this };
		Window_list          _window_list  { _env, _heap, *this };
		Expanding_reporter   _window_layout_reporter { _env, "window_layout",
		                                                     "window_layout"};
		Expanding_reporter   _focus_reporter  { _env, "focus", "focus"};
		Expanding_reporter   _resize_reporter { _env, "resize_request",
		                                              "resize_request"};
};


void Shell::Main::button_list_changed() {
	_update_window_layout(); }


void Shell::Main::window_list_changed() {
	_update_window_layout(); }


void Shell::Main::_handle_mode() {
	_mode = _gui.mode(); }


void Shell::Main::_update_window_layout()
{
	bool keyb_present = false;
	Button::Label btn_hold;

	_button_list.for_each_button([&] (Button & button) {
		if (button.has_label("keyboard")) {
			keyb_present = button.hold();
			return;
		}

		if (button.hold()) {
			btn_hold = button.label();
		}
	});

	bool     resize_needed = false;
	int      ypos;
	int      h;
	unsigned keyb_h = (_mode.area.w() * KEYBOARD_FAC / KEYBOARD_DIV);
	unsigned app_h  = (keyb_present ? (_mode.area.h() - keyb_h) : _mode.area.h())
	                  - PANEL_HEIGHT;
	_window_list.for_each_window([&] (Window & window) {
		if (window.has_label("panel")) {
			ypos = _mode.area.h() - PANEL_HEIGHT;
			h    = PANEL_HEIGHT;
		} else if (window.has_label("keyboard")) {
			ypos = keyb_present ? app_h : _mode.area.h();
			h    = keyb_h;
		} else if (window.has_label(btn_hold)) {
			ypos = 0;
			h    = app_h;
			window.focus(true);
		} else {
			window.focus(false);
			ypos = _mode.area.h();
			h    = _mode.area.h() - PANEL_HEIGHT;
		}
		window.position(0, ypos, _mode.area.w(), h);
		if (window.resize_needed()) { resize_needed = true; }
	});

	if (resize_needed) {
		_resize_reporter.generate([&] (Xml_generator &xml) {
			_window_list.for_each_window([&] (Window & window) {
				if (window.resize_needed()) { window.resize(xml); };
			});
		});
	}

	_focus_reporter.generate([&] (Xml_generator &xml) {
		_window_list.for_each_window([&] (Window & window) {
			if (window.has_label(btn_hold)) { window.generate(xml); }
		});
	});

	_window_layout_reporter.generate([&] (Xml_generator &xml) {
		_window_list.for_window("panel", [&] (Window & window) {
			window.generate(xml);
		});

		_window_list.for_window("keyboard", [&] (Window & window) {
			window.generate(xml);
		});

		_window_list.for_window(btn_hold, [&] (Window & window) {
			window.generate(xml);
		});

		_window_list.for_each_window([&] (Window & window) {
			if (window.has_label("panel")    ||
			    window.has_label("keyboard") ||
			    window.has_label(btn_hold)) { return; }

			window.generate(xml);
		});
	});
}


Shell::Main::Main(Env & env)
: _env(env)
{
	_gui.mode_sigh(_mode_handler);
	_button_list.handle_rom();
	_window_list.handle_rom();
}


void Component::construct(Genode::Env &env) {
	static Shell::Main application(env); }
