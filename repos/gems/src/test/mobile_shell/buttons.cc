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

#include <buttons.h>


void Shell::Button::press(bool const hold) {
	_hold = hold; }


bool Shell::Button::hold() const {
	return _hold; }


bool Shell::Button::has_label(Label const label) const {
	return label == _label; }


Shell::Button::Label Shell::Button::label() const {
	return _label; }


Shell::Button::Button(Label label, bool hold)
: _label(label), _hold(hold) { }


void Shell::Button_list::handle_rom()
{
	_rom.update();

	Update_policy policy(*this);
	_list.update_from_xml(policy, _rom.xml());

	_change_handler.button_list_changed();
}


Shell::Button_list::Button_list(Env            & env,
                                Allocator      & alloc,
                                Change_handler & handler)
: _env(env),
  _alloc(alloc),
  _change_handler(handler)
{
	_rom.sigh(_rom_handler);
}
