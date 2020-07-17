#pragma once

#include <util/avl_string.h>

namespace Driver {
	template <unsigned, typename> class Avl_string_element;
	class Clock;
	class Fixed_clock;
	class Fixed_divider;

	using namespace Genode;
}


template <unsigned STRING_LEN, typename T>
class Driver::Avl_string_element : public Avl_string_base
{
	String<STRING_LEN> _name;
	T                & _object;

	public:

		Avl_string_element(String<STRING_LEN> & name, T & o)
		: Avl_string_base(name.string()), _name(name), _object(o) {}

		String<STRING_LEN> name()   const { return _name;   }
		T &                object() const { return _object; }
};


class Driver::Clock
{
	protected:

		enum { NAME_LEN = 64 };

		using Node = Avl_string_element<NAME_LEN, Clock>;

		Node _tree_elem;

		/*
		 * Noncopyable
		 */
		Clock(Clock const &);
		Clock &operator = (Clock const &);

	public:

		using Name               = Genode::String<NAME_LEN>;
		using Clock_tree         = Avl_tree<Avl_string_base>;
		using Clock_tree_element = Avl_string_element<NAME_LEN, Clock>;

		Clock(Name       & name,
		      Clock_tree & tree)
		: _tree_elem(name, *this) { tree.insert(&_tree_elem); }

		virtual ~Clock() {}

		virtual void          set_rate(unsigned long rate) = 0;
		virtual unsigned long get_rate()             const = 0;
		virtual void          enable()  {}
		virtual void          disable() {}

		Name name() { return _tree_elem.name(); }
};


class Driver::Fixed_clock : public Driver::Clock
{
	private:

		unsigned long _rate;

	public:

		Fixed_clock(Name          name,
		            unsigned long rate,
		            Clock_tree  & tree)
		: Clock(name, tree), _rate(rate) {}

		void          set_rate(unsigned long) override {}
		unsigned long get_rate()        const override { return _rate; }
};


class Driver::Fixed_divider : public Driver::Clock
{
	private:

		Clock  & _parent;
		unsigned _divider;

	public:

		Fixed_divider(Name         name,
		              Clock      & parent,
		              unsigned     divider,
		              Clock_tree & tree)
		: Clock(name, tree), _parent(parent), _divider(divider) {}

		void          set_rate(unsigned long) override {}
		unsigned long get_rate()        const override {
			return _parent.get_rate() / _divider; }
};
