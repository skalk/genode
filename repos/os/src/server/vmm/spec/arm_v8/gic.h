/*
 * \brief  VMM ARM Generic Interrupt Controller device model
 * \author Stefan Kalkowski
 * \date   2019-08-05
 */

/*
 * Copyright (C) 2019 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _SRC__SERVER__VMM__GIC_H_
#define _SRC__SERVER__VMM__GIC_H_

#include <mmio.h>

#include <base/env.h>
#include <drivers/defs/arm_v7.h>
#include <util/list.h>
#include <util/register.h>
#include <util/reconstructible.h>

namespace Vmm { class Gic; }

class Vmm::Gic : public Vmm::Mmio_device
{
	public:

		enum Irqs {
			MAX_SGI  = 16,
			MAX_PPI  = 16,
			MAX_SPI  = 992,
			MAX_IRQ  = 1020,
			SPURIOUS = 1023,
		};

		class Irq : public Genode::List<Irq>::Element
		{
			public:

				struct List : Genode::List<Irq>
				{
					void insert(Irq & irq);
					Irq * highest_enabled();
				};

				struct Irq_handler {
					virtual void eoi()   {};
					virtual void level() {};
				};

				enum Type   { SGI, PPI, SPI };
				enum State  { INACTIVE, ACTIVE, PENDING, ACTIVE_PENDING };
				enum Config { LEVEL, EDGE };

				Irq(unsigned num, Type t, List &pending_list);

				bool     enabled() const;
				bool     active()  const;
				bool     pending() const;
				bool     level()   const;
				unsigned number()  const;
				Genode::uint8_t priority() const;
				Genode::uint8_t target() const;

				void enable();
				void disable();
				void activate();
				void deactivate();
				void assert();
				void deassert();
				void level(bool l);
				void priority(Genode::uint8_t p);
				void target(Genode::uint8_t t);
				void handler(Irq_handler & handler);

			private:

				Irq(Irq const &);
				Irq &operator = (Irq const &);

				bool            _enabled { false    };
				Type            _type    { SPI      };
				State           _state   { INACTIVE };
				Config          _config  { LEVEL    };
				unsigned        _num     { 0 };
				Genode::uint8_t _prio    { 0 };
				Genode::uint8_t _target  { 1 };
				List          & _pending_list;
				Irq_handler   * _handler { nullptr };
		};

		class Gicd_banked
		{
			public:

				Irq & irq(unsigned num);
				void handle_irq();
				bool pending_irq();

				Gicd_banked(Cpu & cpu, Gic & gic);

			private:

				Cpu                      & _cpu;
				Gic                      & _gic;
				Genode::Constructible<Irq> _sgi[MAX_SGI];
				Genode::Constructible<Irq> _ppi[MAX_PPI];
				Irq::List                  _pending_list;
		};

		Gic(const char * const     name,
		    const Genode::uint64_t   addr,
		    const Genode::uint64_t   size,
		    Genode::Env            & env);

	private:

		friend struct Gicd_banked;

		struct Gicd_ctlr : Genode::Register<32>, Mmio_register
		{
			struct Enable : Bitfield<0, 1> {};

			Gicd_ctlr()
			: Mmio_register("GICD_CTLR", Mmio_register::RW, 0, 4, 0) {}
		} _ctrl;

		struct Gicd_typer : Genode::Register<32>, Mmio_register
		{
			struct It_lines_number : Bitfield<0, 5> {};
			struct Cpu_number      : Bitfield<5, 3> {};

			Gicd_typer(unsigned cpus)
			:  Mmio_register("GICD_TYPER", Mmio_register::RO, 0x4, 4,
			                 It_lines_number::bits(31) | Cpu_number::bits(cpus-1)) {}
		} _typer { 1 }; /* FIXME: smp support */

		struct Gicd_iidr : Genode::Register<32>, Mmio_register
		{
			struct Implementer : Bitfield<0, 12> {};
			struct Revision    : Bitfield<12, 4> {};
			struct Variant     : Bitfield<16, 4> {};
			struct Product_id  : Bitfield<24, 8> {};

			Gicd_iidr()
			: Mmio_register("GICD_IIDR", Mmio_register::RO, 0x8, 4, 0x123) {}
		} _iidr;

		struct Irq_reg : Mmio_register
		{
			virtual Register read(Irq & irq)  { return 0; }
			virtual void     write(Irq & irq, Register reg) { }

			Register read(Address_range  & access, Cpu&) override;
			void     write(Address_range & access, Cpu&, Register value) override;

			Irq_reg(Mmio_register::Name name,
			        Mmio_register::Type type,
			        Genode::uint64_t    start,
			        unsigned            bits_per_irq)
			: Mmio_register(name, type, start, bits_per_irq*1024/8) {}
		};

		struct Gicd_isenabler : Irq_reg
		{
			Register read(Irq & irq)              { return irq.enabled(); }
			void     write(Irq & irq, Register v) { if (v) irq.enable();  }

			Gicd_isenabler()
			: Irq_reg("GICD_ISENABLER", Mmio_register::RW, 0x100, 1) {}
		} _isenabler;

		struct Gicd_icenabler : Irq_reg
		{
			Register read(Irq & irq)              { return irq.enabled(); }
			void     write(Irq & irq, Register v) { if (v) irq.disable(); }

			Gicd_icenabler()
			: Irq_reg("GICD_ICENABLER", Mmio_register::RW, 0x180, 1) {}
		} _csenabler;

		struct Gicd_ispendr : Irq_reg
		{
			Register read(Irq & irq)              { return irq.pending(); }
			void     write(Irq & irq, Register v) { if (v) irq.assert();  }

			Gicd_ispendr()
			: Irq_reg("GICD_ISPENDR", Mmio_register::RW, 0x200, 1) {}
		} _ispendr;

		struct Gicd_icpendr : Irq_reg
		{
			Register read(Irq & irq)              { return irq.pending();  }
			void     write(Irq & irq, Register v) { if (v) irq.deassert(); }

			Gicd_icpendr()
			: Irq_reg("GICD_ICPENDR", Mmio_register::RW, 0x280, 1) {}
		} _icpendr;

		struct Gicd_isactiver : Irq_reg
		{
			Register read(Irq & irq)              { return irq.active();   }
			void     write(Irq & irq, Register v) { if (v) irq.activate(); }

			Gicd_isactiver()
			: Irq_reg("GICD_ISACTIVER", Mmio_register::RW, 0x300, 1) {}
		} _isactiver;

		struct Gicd_icactiver : Irq_reg
		{
			Register read(Irq & irq)              { return irq.active();     }
			void     write(Irq & irq, Register v) { if (v) irq.deactivate(); }

			Gicd_icactiver()
			: Irq_reg("GICD_ICACTIVER", Mmio_register::RW, 0x380, 1) {}
		} _icactiver;

		struct Gicd_ipriorityr : Irq_reg
		{
			Register read(Irq & irq)              { return irq.priority(); }
			void     write(Irq & irq, Register v) { irq.priority(v);       }

			Gicd_ipriorityr()
			: Irq_reg("GICD_IPRIORITYR", Mmio_register::RW, 0x400, 8) {}
		} _ipriorityr;

		struct Gicd_itargetr : Irq_reg
		{
			Register read(Irq & irq)              { return irq.target(); }
			void     write(Irq & irq, Register v) { irq.target(v);       }

			Gicd_itargetr()
			: Irq_reg("GICD_ITARGETSR", Mmio_register::RW, 0x800, 8) {}
		} _itargetr;


		struct Gicd_icfgr : Irq_reg
		{
			Register read(Irq & irq)              { return irq.target(); }
			void     write(Irq & irq, Register v) { irq.target(v);       }

			Gicd_icfgr()
			: Irq_reg("GICD_ICFGR", Mmio_register::RW, 0xc00, 8) {}
		} _icfgr;

		/**
		 * FIXME: missing registers:
		 * GICD_IGROUP      = 0x80,
		 * GICD_NSACR       = 0xe00,
		 * GICD_SGIR        = 0xf00,
		 * GICD_CPENDSGIR0  = 0xf10,
		 * GICD_SPENDSGIR0  = 0xf20,
         * GICD identification registers 0xfd0...
		 */

		Genode::Constructible<Irq> _spi[MAX_SPI];
		Irq::List                  _pending_list;
		unsigned                   _cpu_cnt { 1 };
};

#endif /* _SRC__SERVER__VMM__GIC_H_ */
