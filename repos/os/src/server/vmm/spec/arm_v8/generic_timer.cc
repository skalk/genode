/*
 * \brief  VMM ARM Generic timer device model
 * \author Stefan Kalkowski
 * \date   2019-08-20
 */

/*
 * Copyright (C) 2019 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#include <cpu.h>
#include <generic_timer.h>

using Vmm::Generic_timer;

Genode::uint64_t Generic_timer::_ticks_per_ms()
{
	static Genode::uint64_t ticks_per_ms = 0;
	if (!ticks_per_ms) {
		Genode::uint32_t freq = 62500000;
		// FIXME asm volatile("mrs %0, cntfrq_el0" : "=r" (freq));
		ticks_per_ms = freq / 1000;
	}
	return ticks_per_ms;
}


bool Generic_timer::_enabled() {
	return Ctrl::Enabled::get(_cpu.state().timer.control); }


bool Generic_timer::_masked()  {
	return Ctrl::Imask::get(_cpu.state().timer.control);   }


bool Generic_timer::_pending() {
	return Ctrl::Istatus::get(_cpu.state().timer.control); }


void Generic_timer::_handle_timeout(Genode::Duration)
{
	_cpu.handle_signal([this] (void) {
		_cpu.state().timer.count    = _cpu.state().timer.compare + 1;
		_time = 0;
		if (_enabled() && !_masked()) handle_irq();
	});
}


Genode::uint64_t Generic_timer::_usecs_left()
{
	Genode::uint64_t ticks = _cpu.state().timer.compare -
	                         _cpu.state().timer.count;
	if (_cpu.state().timer.count > _cpu.state().timer.compare) return 0;
#if 0
	if (_cpu.state().timer.kcontrol & (1 << 2)) {
		bool dir   = _cpu.state().timer.kcontrol & (1 << 3);
		unsigned i = (_cpu.state().timer.kcontrol >> 4) & 0b1111;
		Genode::uint64_t mask = (1 << i) - 1;
		Genode::uint64_t t    = mask - (_cpu.state().timer.count & mask);
		t += (((dir ? 1 : 0) << i) == (_cpu.state().timer.count & (1 << i)))
			? 0 : mask;
		Genode::log("Sonderfall ", t, " ", ticks);
		if (t < ticks) ticks = t;
	}
#endif
	return Genode::timer_ticks_to_us(ticks, _ticks_per_ms());
}


Generic_timer::Generic_timer(Genode::Env & env,
                             Gic::Irq    & irq,
                             Cpu         & cpu)
: _timer(env),
  _timeout(_timer, *this, &Generic_timer::_handle_timeout),
  _irq(irq),
  _cpu(cpu)
{
	_cpu.state().timer.irq = true;
	_irq.handler(*this);
}


void Generic_timer::schedule_timeout()
{
	if (_pending()) {
		handle_irq();
		return;
	}

	if (_enabled()) {
		_time = _timer.curr_time().trunc_to_plain_us().value;
		if (_usecs_left()) {
			_timeout.schedule(Genode::Microseconds(_usecs_left()));
		} else _handle_timeout(Genode::Duration(Genode::Microseconds(0)));
	}
}


void Generic_timer::cancel_timeout()
{
	if (!_time) return;
	Genode::error("Timer cancel_timeout");
	_timeout.discard();
	//Genode::uint64_t time_passed =
	//	_timer.curr_time().trunc_to_plain_us().value - _time;
	//FIXME
	_time = 0;
}


void Generic_timer::handle_irq()
{
	_irq.assert();
	_cpu.state().timer.irq = false;
}


void Generic_timer::eoi()
{
	_cpu.state().timer.irq = true;
};


void Generic_timer::dump()
{
	using namespace Genode;

	log("  timer.ctl  = ", Hex(_cpu.state().timer.control, Hex::PREFIX, Hex::PAD));
	log("  timer.cmp  = ", Hex(_cpu.state().timer.compare, Hex::PREFIX, Hex::PAD));
}
