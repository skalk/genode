/*
 * \brief
 * \author Alexander Boettcher
 * \date   2019-11-05
 */

/*
 * Copyright (C) 2019 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

struct Entry
: Genode::List<Entry>::Element, Genode::Avl_node<Entry>
{
	private:

		Genode::Trace::Subject_id  const _id;
		Genode::Trace::Thread_name const _thread;
		Genode::Session_label      const _label;
		Genode::String<12>         const _cpu;

	public:

		Entry(Genode::Trace::Subject_id const &id,
		      Genode::Trace::Thread_name const &thread,
		      Genode::Session_label const &label,
		      Genode::String<12> const &cpu)
		:
			_id(id),
			_thread(thread),
			_label(label),
			_cpu(cpu)
		{ }

		~Entry() { }

		Genode::Trace::Thread_name const &thread_name() const { return _thread; }
		Genode::Session_label const &session_label() const { return _label; }
		Genode::String<12> const &cpu() const { return _cpu; }

		/*****************
		 * Avl interface *
		 *****************/

		Entry *find_by_id(Genode::Trace::Subject_id const id)
		{
			if (id == _id) return this;
			Entry *obj = this->child(id.id > _id.id);
			return obj ? obj->find_by_id(id) : nullptr;
		}

		bool higher(Entry *e) { return e->_id.id > _id.id; }
};
