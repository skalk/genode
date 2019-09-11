/*
 * \brief  VMM ARM Generic Interrupt Controller v2 device model
 * \author Stefan Kalkowski
 * \date   2019-08-05
 */

/*
 * Copyright (C) 2019 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#include <cpu.h>
#include <gic.h>

bool Gic::Irq::enabled() const { return _enabled; }

bool Gic::Irq::active()  const {
	return _state == ACTIVE || _state == ACTIVE_PENDING; }

bool Gic::Irq::pending() const {
	return _state == PENDING || _state == ACTIVE_PENDING; }

bool Gic::Irq::level() const { return _config == LEVEL; }

unsigned Gic::Irq::number()  const { return _num; }

Genode::uint8_t Gic::Irq::priority() const { return _prio; }

Genode::uint8_t Gic::Irq::target() const { return _target; }


void Gic::Irq::enable()  { _enabled = true;  }
void Gic::Irq::disable() { _enabled = false; }


void Gic::Irq::activate()
{
	switch (_state) {
	case INACTIVE:       return;
	case PENDING:        return;
	case ACTIVE_PENDING: _state = PENDING;  return;
	case ACTIVE:         _state = INACTIVE; return;
	};
}


void Gic::Irq::deactivate()
{
	switch (_state) {
	case INACTIVE:       return;
	case PENDING:        return;
	case ACTIVE_PENDING: _state = PENDING;  return;
	case ACTIVE:         _state = INACTIVE; return;
	};
}


void Gic::Irq::assert()
{
	if (pending()) return;

	_state = PENDING;
	_pending_list.insert(*this);
}


void Gic::Irq::deassert()
{
	if (_state == INACTIVE) return;

	_state = INACTIVE;
	_pending_list.remove(this);
	if (_handler) _handler->eoi();
}


void Gic::Irq::target(Genode::uint8_t t)
{
	if (_target == t) return;

	_target = t;
}


void Gic::Irq::level(bool l)
{
	if (level() == l) return;

	_config = l ? LEVEL : EDGE;

	if (!_handler) return;

	_handler->level();
}


void Gic::Irq::priority(Genode::uint8_t p)
{
	if (_prio == p) return;

	_prio = p;
}


void Gic::Irq::handler(Gic::Irq::Irq_handler & handler) { _handler = &handler; }


Gic::Irq::Irq(unsigned num, Type t, Irq::List & l)
: _type(t), _num(num), _pending_list(l) {}


void Gic::Irq::List::insert(Irq & irq)
{
	Irq * i = first();
	while (i && i->priority() < irq.priority() && i->next()) i = i->next();
	Genode::List<Irq>::insert(&irq, i);
}


Gic::Irq * Gic::Irq::List::highest_enabled()
{
	Irq * i = first();
	while(i) {
		if (i->enabled()) return i;
		i = i->next();
	}
	return nullptr;
}


void Gic::Gicd_banked::handle_irq()
{
	unsigned i = cpu.state().irqs.virtual_irq;
	if (i > MAX_IRQ) return;

	if (i < MAX_SGI) {
		sgi[i]->deassert();
	} else {
		if (i < MAX_SGI+MAX_PPI) {
			ppi[i-MAX_SGI]->deassert();
		} else {
			gic._spi[i-MAX_SGI-MAX_PPI]->deassert();
		}
	}

	cpu.state().irqs.virtual_irq = 1023;
}


bool Gic::Gicd_banked::pending_irq()
{
	if (cpu.state().irqs.virtual_irq != 1023) return true;

	Irq * i = gic._pending_list.highest_enabled();
	Irq * j = pending_list.highest_enabled();
	Irq * n = j;
	if (i && ((j && j->priority() > i->priority()) || !j)) n = i;
	if (!n) return false;
	cpu.state().irqs.virtual_irq = n->number();
	n->activate();
	return true;
}


Gic::Gicd_banked::Gicd_banked(Cpu & cpu, Gic & gic) : cpu(cpu), gic(gic)
{
	for (unsigned i = 0; i < MAX_SGI; i++)
		sgi[i].construct(i, Irq::SGI, pending_list);

	for (unsigned i = 0; i < MAX_PPI; i++)
		ppi[i].construct(i+MAX_SGI, Irq::PPI, pending_list);

	cpu.state().irqs.last_irq    = 1023;
	cpu.state().irqs.virtual_irq = 1023;
}


void Gic::_enable()  { _enabled = true;  }
void Gic::_disable() { _enabled = false; }


Gic::Irq & Gic::spi(unsigned num) { return *_spi[num]; }


Gic::Gic(const char * const     name,
             const Genode::uint64_t addr,
             const Genode::uint64_t size,
             Genode::Env & env)
: Device(name, addr, size)
{
	for (unsigned i = 0; i < MAX_SPI; i++)
		_spi[i].construct(i+MAX_SGI+MAX_PPI, Irq::SPI, _pending_list);
}


void Gic::read(Genode::uint32_t * reg, Genode::uint64_t off,
               Cpu & cpu)
{
	*reg = 0;

	switch (off) {

	case GICD_CTLR:
		*reg = Ctlr::Enable::bits(_enabled);
		return;

	case GICD_TYPER:
		*reg = Typer::Cpu_number::bits(_cpu_cnt-1) |
		       Typer::It_lines_number::bits(31);
		return;

	case GICD_IIDR:
		*reg = Iidr::Implementer::bits(0x123);
		return;

	case GICD_ISENABLER0:
		cpu.gic().for_range_irq(off, GICD_ISENABLER0, 32, [reg] (Irq & i) {
			if (i.enabled()) *reg |= 1 << i.number(); });
		return;

	case GICD_ICENABLER0:
		cpu.gic().for_range_irq(off, GICD_ICENABLER0, 32, [reg] (Irq & i) {
			if (i.enabled()) *reg |= 1 << i.number(); });
		return;

	case GICD_ISACTIVER0:
		cpu.gic().for_range_irq(off, GICD_ISACTIVER0, 32, [reg] (Irq & i) {
			if (i.active()) *reg |= 1 << i.number(); });
		return;

	case GICD_ICACTIVER0:
		cpu.gic().for_range_irq(off, GICD_ICACTIVER0, 32, [reg] (Irq & i) {
			if (i.active()) *reg |= 1 << i.number(); });
		return;

	case GICD_ISPENDR0:
		cpu.gic().for_range_irq(off, GICD_ISPENDR0, 32, [reg] (Irq & i) {
			if (i.pending()) *reg |= 1 << i.number(); });
		return;

	case GICD_ICPENDR0:
		cpu.gic().for_range_irq(off, GICD_ICPENDR0, 32, [reg] (Irq & i) {
			if (i.pending()) *reg |= 1 << i.number(); });
		return;

	case GICD_ICFGR0: /* sgi cfg is RAZ */ return;
	case GICD_ICFGR1:
		cpu.gic().for_range_irq(off, GICD_ICFGR1, 16, [reg] (Irq & i) {
			if (!i.level()) *reg |= 0b10 << (i.number()*2); });
		return;

	case GICD_SGIR: /* write-only */ return;

	case GICD_CPENDSGIR0: [[fallthrough]];
	case GICD_SPENDSGIR0:
		Genode::error("PENDSGIR0 not yet implemented!");
		return;

	default:

		if (_in_reg_range(off, GICD_ISENABLER1, MAX_SPI, 1)) {
			_for_range_spi(off, GICD_ISENABLER1, 32, [reg] (Irq & i) {
				if (i.enabled()) *reg |= 1 << (i.number() % 32); });
			return;
		}

		if (_in_reg_range(off, GICD_ICENABLER1, MAX_SPI, 1)) {
			_for_range_spi(off, GICD_ICENABLER1, 32, [reg] (Irq & i) {
				if (i.enabled()) *reg |= 1 << (i.number() % 32); });
			return;
		}

		if (_in_reg_range(off, GICD_ISACTIVER1, MAX_SPI, 1)) {
			_for_range_spi(off, GICD_ISACTIVER1, 32, [reg] (Irq & i) {
				if (i.active()) *reg |= 1 << (i.number() % 32); });
			return;
		}

		if (_in_reg_range(off, GICD_ICACTIVER1, MAX_SPI, 1)) {
			_for_range_spi(off, GICD_ICACTIVER1, 32, [reg] (Irq & i) {
				if (i.active()) *reg |= 1 << (i.number() % 32); });
			return;
		}

		if (_in_reg_range(off, GICD_ISPENDR1, MAX_SPI, 1)) {
			_for_range_spi(off, GICD_ISPENDR1, 32, [reg] (Irq & i) {
				if (i.pending()) *reg |= 1 << (i.number() % 32); });
			return;
		}

		if (_in_reg_range(off, GICD_ICPENDR1, MAX_SPI, 1)) {
			_for_range_spi(off, GICD_ICPENDR1, 32, [reg] (Irq & i) {
				if (i.pending()) *reg |= 1 << (i.number() % 32); });
			return;
		}

		if (_in_reg_range(off, GICD_IPRIORITYR8, MAX_SPI, 8)) {
			_for_range_spi(off, GICD_IPRIORITYR8, 4, [reg] (Irq & i) {
				*reg |= i.priority() << ((i.number() % 4)*8); });
			return;
		}

#if 0
		if (_in_reg_range(off, GICD_IPRIORITYR0, MAX_SGI+MAX_PPI, 8)) {
			*reg = state.gicd_banked.ipriorityr[_reg_idx(GICD_IPRIORITYR0, off)];
			return;
		}
#endif

		if (_in_reg_range(off, GICD_ITARGETSR0, MAX_SGI+MAX_PPI, 8)) {
			const Genode::uint8_t cpu_target = 1 << cpu.cpu_id();
			*reg = cpu_target | cpu_target << 8 |
			       cpu_target << 16 | cpu_target << 24;
			return;
		}

		if (_in_reg_range(off, GICD_ITARGETSR8, MAX_SPI, 8)) {
			_for_range_spi(off, GICD_ITARGETSR8, 4, [reg] (Irq & i) {
				*reg |= i.target() << ((i.number() % 4)*8); });
			return;
		}

		if (_in_reg_range(off, GICD_ICFGR2, MAX_SPI, 2)) {
			_for_range_spi(off, GICD_ICFGR2, 16, [reg] (Irq & i) {
				if (!i.level()) *reg |= 0b10 << ((i.number() % 16)*2); });
			return;
		}

		Genode::warning("GIC: unsupported read offset ", (void*)off);
		*reg = 0;
	};
}


void Gic::write(Genode::uint32_t * reg, Genode::uint64_t off,
                Cpu & cpu)
{
	using namespace Genode;

	switch (off) {

	case GICD_CTLR:
		Ctlr::Enable::get(*reg) ? _enable() : _disable();
		return;

	case GICD_ISENABLER0:
		cpu.gic().for_range_irq(off, GICD_ISENABLER0, 32, [reg] (Irq & i) {
			i.enable(); });
		return;

	case GICD_ICENABLER0:
		cpu.gic().for_range_irq(off, GICD_ICENABLER0, 32, [reg] (Irq & i) {
			i.disable(); });
		return;

	case GICD_ISACTIVER0:
		cpu.gic().for_range_irq(off, GICD_ISACTIVER0, 32, [reg] (Irq & i) {
			i.activate(); });
		return;

	case GICD_ICACTIVER0:
		cpu.gic().for_range_irq(off, GICD_ICACTIVER0, 32, [reg] (Irq & i) {
			i.deactivate(); });
		return;

	case GICD_ISPENDR0:
		cpu.gic().for_range_irq(off, GICD_ISPENDR0, 32, [reg] (Irq & i) {
			i.assert(); });
		return;

	case GICD_ICPENDR0:
		cpu.gic().for_range_irq(off, GICD_ICPENDR0, 32, [reg] (Irq & i) {
			i.deassert(); });
		return;

#if 0
	case GICD_ICFGR1:
	case GICD_SGIR:
	case GICD_CPENDSGIR0:
	case GICD_SPENDSGIR0:
#endif

	default:

		if (_in_reg_range(off, GICD_ISENABLER1, MAX_SPI, 1)) {
			_for_range_spi(off, GICD_ISENABLER1, 32, [reg] (Irq & i) {
				if (*reg | (1 << (i.number() % 32))) i.enable(); });
			return;
		}

		if (_in_reg_range(off, GICD_ICENABLER1, MAX_SPI, 1)) {
			_for_range_spi(off, GICD_ICENABLER1, 32, [reg] (Irq & i) {
				if (*reg | (1 << (i.number() % 32))) i.disable(); });
			return;
		}

		if (_in_reg_range(off, GICD_ISACTIVER1, MAX_SPI, 1)) {
			_for_range_spi(off, GICD_ISACTIVER1, 32, [reg] (Irq & i) {
				if (*reg | (1 << (i.number() % 32))) i.activate(); });
			return;
		}

		if (_in_reg_range(off, GICD_ICACTIVER1, MAX_SPI, 1)) {
			_for_range_spi(off, GICD_ICACTIVER1, 32, [reg] (Irq & i) {
				if (*reg | (1 << (i.number() % 32))) i.deactivate(); });
			return;
		}

		if (_in_reg_range(off, GICD_ISPENDR1, MAX_SPI, 1)) {
			_for_range_spi(off, GICD_ISPENDR1, 32, [reg] (Irq & i) {
				if (*reg | (1 << (i.number() % 32))) i.assert(); });
			return;
		}

		if (_in_reg_range(off, GICD_ICPENDR1, MAX_SPI, 1)) {
			_for_range_spi(off, GICD_ICPENDR1, 32, [reg] (Irq & i) {
				if (*reg | (1 << (i.number() % 32))) i.deassert(); });
			return;
		}

		if (_in_reg_range(off, GICD_IPRIORITYR0, MAX_SGI+MAX_PPI, 8)) {
			cpu.gic().for_range_irq(off, GICD_IPRIORITYR0, 4, [reg] (Irq & i) {
				i.priority(*reg >> ((i.number() % 4)*8)); });
			return;
		}

		if (_in_reg_range(off, GICD_IPRIORITYR8, MAX_SPI, 8)) {
			_for_range_spi(off, GICD_IPRIORITYR8, 4, [reg] (Irq & i) {
				i.priority(*reg >> ((i.number() % 4)*8)); });
			return;
		}

		if (_in_reg_range(off, GICD_ITARGETSR8, MAX_SPI, 8)) {
			_for_range_spi(off, GICD_ITARGETSR8, 4, [reg] (Irq & i) {
				i.target(*reg >> ((i.number() % 4)*8)); });
			return;
		}

		if (_in_reg_range(off, GICD_ICFGR2, MAX_SPI, 2)) {
			_for_range_spi(off, GICD_ICFGR2, 16, [reg] (Irq & i) {
				i.level(!((*reg >> ((i.number() % 16)*2)) & 0b10)); });
			return;
		}

		Genode::warning("GIC: unsupported write offset ",
						(void*)off, " = ", Genode::Hex(*reg));
	};

#if 0
	/* only allow cpu0 as target by now */
	if (off >= GICD_ITARGETSR0 && off <= GICD_ITARGETSRL &&
	    *reg == 0x01010101)
		return;

	/* only allow level triggered && active low */
	if (off >= GICD_ICFGR2 && off <= GICD_ICFGRL &&
	    *reg == 0)
		return;

	/* ignore priority settings */
	if (off >= GICD_IPRIORITYR0 && off <= GICD_IPRIORITYRL)
		return;

	/* set enable registers */
	if (off >= GICD_ISENABLER0 && off <= GICD_ISENABLERL) {
		addr_t idx = ((addr_t)off - GICD_ISENABLER0) * 8;
		for (unsigned i = 0; i < 32; i++)
			if (((*reg >> i) & 1))
				_enable_irq(idx+i);
		return;
	}

	/* clear enable registers */
	if (off >= GICD_ICENABLER0 && off <= GICD_ICENABLERL) {
		addr_t idx = ((addr_t)off - GICD_ICENABLER0) * 8;
		for (unsigned i = 0; i < 32; i++)
			if (((*reg >> i) & 1))
				_disable_irq(idx+i);
		return;
	}

	switch (off) {
	case GICD_CTLR:
		_distr_enabled = (*reg & 0b1);
		return;
	default:
		throw Error("GIC: unsupported write offset %llx", off);
	};
#endif
}
