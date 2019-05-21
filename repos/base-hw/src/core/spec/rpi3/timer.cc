/*
 * \brief  Timer driver for core
 * \author Stefan Kalkowski
 * \date   2019-05-10
 */

/*
 * Copyright (C) 2019 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#include <drivers/timer/util.h>
#include <kernel/timer.h>
#include <kernel/cpu.h>

using namespace Kernel;


unsigned Timer::interrupt_id() const { return 30; }

unsigned long Timer_driver::_freq() { return Genode::Cpu::Cntfrq_el0::read(); }


Timer_driver::Timer_driver(unsigned) : ticks_per_ms(_freq() / 1000) { }


void Timer::_start_one_shot(time_t const) { }


time_t Timer::_duration() const { return 0; }


time_t Timer::ticks_to_us(time_t const ticks) const {
	return Genode::timer_ticks_to_us(ticks, _driver.ticks_per_ms); }


time_t Timer::us_to_ticks(time_t const us) const {
	return (us / 1000) * _driver.ticks_per_ms; }


time_t Timer::_max_value() const {
	return _driver.ticks_per_ms * 5000; }
