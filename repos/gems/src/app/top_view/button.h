/*
 * \brief  Button state helper
 * \author Alexander Boettcher
 * \date   2019-03-18
 */

/*
 * Copyright (C) 2019-2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

struct Button_state
{
	unsigned  first;
	unsigned  last;
	unsigned  current;
	unsigned  max     { 4 };
	bool      hovered { false };
	bool      prev    { false };
	bool      next    { false };

	Button_state()
	: first(0), last(9), current(first)
	{ }

	Button_state(unsigned const f, unsigned const l, unsigned c = ~0U)
	: first(f), last(l), current((c == ~0U) ? f : c)
	{ }

	bool active() { return hovered || prev || next; }
	void reset() { hovered = prev = next = false; }

	bool advance()
	{
		bool update = false;

		if (prev && current > first) {
			current -= 1;
			update = true;
		}
		if (next && current < last) {
			current += 1;
			update = true;
		}

		return update;
	}

	void inc() { current = (current >= last) ? first : (current + 1); }
	void dec() { current = (current == first) ? last : (current - 1); }

	void set(unsigned const value)
	{
		if (value > last)
			current = last;
		else if (value < first)
			current = first;
		else
			current = value;
	}

	unsigned range() const { return last - first + 1; }
};

template <unsigned DIGITS, unsigned START, unsigned END, unsigned INITIAL>
struct Button_hub
{
	Button_state _button[DIGITS];

	Button_hub()
	{
		for (unsigned i = 0; i < DIGITS; i++) {
			_button[i] = Button_state(START, END);
			_button[i].current = INITIAL;
		}
	}
	bool update_inc()
	{
		bool update = false;
		for (unsigned i = 0; i < DIGITS; i++) {
			if (_button[i].hovered) {
				update = true;
				_button[i].inc();
			}
		}
		return update;
	}

	bool update_dec()
	{
		bool update = false;
		for (unsigned i = 0; i < DIGITS; i++) {
			if (_button[i].hovered) {
				update = true;
				_button[i].dec();
			}
		}
		return update;
	}

	void reset()
	{
		for (unsigned i = 0; i < DIGITS; i++) {
			_button[i].reset();
		}
	}

	void set(unsigned value)
	{
		for (unsigned i = 0; i < DIGITS; i++) {
			Button_state &button = _button[i];
			button.set(value % button.range());
			value /= button.range();
		}
	}

	unsigned value() const
	{
		unsigned value = 0;
		for (unsigned i = DIGITS; i > 0; i--) {
			value *= _button[i-1].range();
			value += _button[i-1].current;
		}
		return value;
	}

	Button_state & button(unsigned i) { return _button[i]; }

	template <typename FUNC>
	void for_each(FUNC const &fn) {
		for (unsigned i = DIGITS; i > 0; i--) { fn(_button[i - 1], i - 1); } }
};
