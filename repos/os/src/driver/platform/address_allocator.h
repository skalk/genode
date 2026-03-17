/*
 * \brief  Very simple address range allocator
 * \author Stefan Kalkowski
 * \date   2026-01-14
 *
 * WARNING: DO NOT COPY IT!!!
 * This simple kind of range allocator is inefficient, and open to
 * fragmentation. It is for the limited use-case in the platform driver
 * useful only, and get replaced whenever a general useful alternative
 * is available.
 */

/*
 * Copyright (C) 2026 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _SRC__DRIVER__ADDRESS_ALLOCATOR_H_
#define _SRC__DRIVER__ADDRESS_ALLOCATOR_H_

/* Genode includes */
#include <base/memory.h>
#include <util/avl_tree.h>

namespace Driver {
	using namespace Genode;

	class Address_tree;
	class Address_reservations;
	class Address_allocator;
}

class Driver::Address_tree : Genode::Noncopyable
{
	public:

		struct Range : Range_allocator::Range
		{
			void print(Output &output) const
			{
				Genode::print(output, "Range(", Hex(start), ", ", Hex(end), ")");
			}
		};

		class Node : private Avl_node<Node>
		{
			private:

				friend class Address_reservations;
				friend class Address_allocator;
				friend class Avl_node<Node>;
				friend class Avl_tree<Node>;

				Avl_tree<Node> &_tree;

				bool higher(Node *a) {
					return a->range.start >= range.start; }

			public:

				Range const range;

				Node(Avl_tree<Node> &tree, Range range)
				:
					_tree(tree),
					range(range)
				{
					tree.insert(this);
				}

				~Node() { _tree.remove(this); }

				bool collides(Range r) const
				{
					return ((range.end) > r.start &&
				            (range.start < r.end));
				};

				void with(Range r, auto const &fn)
				{
					if (range.start == r.start &&
					    range.end == r.end) {
						fn(*this);
						return;
					}

					bool side = r.start < range.start;
					if (child(side)) child(side)->with(r, fn);
				}
		};

	protected:

		Avl_tree<Node> _addr_tree {};
};


class Driver::Address_reservations : Address_tree
{
	private:

		using Md_allocator = Memory::Constrained_obj_allocator<Node>;

		Md_allocator _md_alloc;

	public:

		Address_reservations(Memory::Constrained_allocator &alloc)
		:
			_md_alloc(alloc) {}

		void reserve(Range range)
		{
			if (range.start >= range.end) {
				error("Invalid range: ", range);
				return;
			}

			/* check for the node nearest to start from below */
			for(Node *node = _addr_tree.first(); node;) {
				if (node->collides(range)) {
					error("Range conflict of ", range, " with ", node->range);
					return;
				}
				node = node->child(range.start>node->range.start);
			}

			/* check for the node nearest to end from above */
			for(Node *node = _addr_tree.first(); node;) {
				if (node->collides(range)) {
					error("Range conflict of ", range, " with ", node->range);
					return;
				}
				node = node->child(range.end>node->range.end);
			}

			_md_alloc.create(_addr_tree, range).with_result(
				[&] (auto &a) { a.deallocate = false; },
				[] (auto) {
					/* Can actually not happen, should block for resources */
					error("Allocation of reservation failed");
				});
		}

		void cancel_reservation(Range range)
		{
			bool found = false;

			if (_addr_tree.first())
				_addr_tree.first()->with(range,
					[&] (Node &node) {
						found = true;
						Md_allocator::Allocation a(_md_alloc, { node });
					});

			if (!found)
				error("Could not cancel reservation ", range);
		}

		void for_each(auto const &fn) const {
			_addr_tree.for_each(fn); }
};

class Driver::Address_allocator : Address_tree
{
	public:

		using Metadata_allocator = Memory::Constrained_obj_allocator<Node>;

		struct Attr
		{
			Node &node;
			Metadata_allocator &alloc;

			Range range() { return node.range; }
		};

	private:

		Avl_tree<Node> _addr_tree {};

		addr_t _start { 0 };
		addr_t _end   { ~0UL };

	public:

		using Error      = Alloc_error;
		using Allocation = Genode::Allocation<Address_allocator>;
		using Result     = Allocation::Attempt;

		Result alloc(Address_reservations &reservations,
		             Metadata_allocator &md_alloc,
		             size_t size, Align align)
		{
			addr_t start = _start;
			bool   found = _addr_tree.first() == nullptr;

			auto collides_with_reservation = [&] (Range r) {
				reservations.for_each([&] (Node const &res) {
					if (!res.collides(r))
						return;

					if (res.range.start > r.start &&
					    (res.range.start - r.start) >= size)
							return;

					if (res.range.end < r.end &&
					    (r.end - res.range.end) >= size) {
						start = res.range.end;
						return;
					}

					found = false;
				});
				return !found;
			};

			if (!found)
				_addr_tree.for_each([&] (Node const &node) {
					if (found)
						return;
					if (start+size <= node.range.start) {
						found = true;
						if (!collides_with_reservation({start, node.range.start}))
							return;
					}
					start = align_addr(node.range.end, align);
				});

			if (!found && (_end-start+1) >= size) {
				found = true;
				collides_with_reservation({start, _end});
			}

			if (found)
				return md_alloc.create(_addr_tree,
				                       Range{start, start+size}).convert<Result>(
					[&] (auto &a) -> Result {
						a.deallocate = false;
						return { *this, { a.obj, md_alloc } }; },
					[] (auto err) -> Error { return err; });

			return Alloc_error::DENIED;
		}

		void _free(Allocation &a)
		{
			a.alloc.destroy(a.node);
			a.deallocate = true;
		}
};

#endif /* _SRC__DRIVER__ADDRESS_ALLOCATOR_H_ */
