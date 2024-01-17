/*
 * \brief  USB device interface
 * \author Stefan Kalkowski
 * \date   2023-08-28
 */

/*
 * Copyright (C) 2023 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _INCLUDE__USB_SESSION__DEVICE_H_
#define _INCLUDE__USB_SESSION__DEVICE_H_

#include <base/allocator_avl.h>
#include <base/exception.h>
#include <base/rpc.h>
#include <base/rpc.h>
#include <packet_stream_tx/client.h>
#include <usb_session/types.h>
#include <usb_session/connection.h>

namespace Usb {
	template <typename SESSION, typename URB> class Urb_handler;
	class Endpoint;
	class Interface;
	class Device;
}


class Usb::Endpoint
{
	public:

		enum Direction { OUT, IN };
		enum Type      { CONTROL, ISOC, BULK, IRQ };

	private:

		enum { MAX_NUMBER = 0xf };

		struct Address : Register<8>
		{
			struct Number    : Bitfield<0, 4> {};
			struct Direction : Bitfield<7, 1> {};
		};

		struct Attributes : Register<8>
		{
			struct Type : Bitfield<0, 2> {};
		};

		Address::access_t    _address    { 0 };
		Attributes::access_t _attributes { 0 };

	public:

		struct Endpoint_not_avail : Exception {};

		Endpoint() : Endpoint(0xff, 0xff) {}

		Endpoint(Address::access_t addr, Attributes::access_t attr)
		: _address(addr), _attributes(attr) {}

		Endpoint(Interface &iface, Direction d, Type t);

		bool valid() { return _address != 0xff || _attributes != 0xff; }

		uint8_t address() { return _address; }

		uint8_t number() { return Address::Number::get(_address); }

		Type type() {
			return (Type) Attributes::Type::get(_attributes); }

		Direction direction() {
			return Address::Direction::get(_address) ? IN : OUT; }
};


template <typename SESSION, typename URB>
class Usb::Urb_handler
{
	protected:

		using Direction         = Endpoint::Direction;
		using Tx                = typename SESSION::Tx;
		using Packet_descriptor = typename SESSION::Packet_descriptor;
		using Payload           = typename Packet_descriptor::Payload;

	public:

		class Urb : Noncopyable
		{
			protected:

				friend class Urb_handler;

				Urb_handler    &_urb_handler;
				Direction const _direction;
				size_t    const _size;
				Payload         _payload { };
				bool            _completed { false };

				Constructible<typename Id_space<URB>::Element> _tag {};

				Fifo_element<URB> _pending_elem { *static_cast<URB*>(this) };

				virtual Packet_descriptor _create() const
				{
					Tagged_packet::Tag const tag { _tag->id().value };
					Packet_descriptor  const p(_payload, tag);
					return p;
				}

				template <typename POLICY>
				void _submit(POLICY &policy, URB &urb, Tx::Source &tx)
				{
					if (!_tag.constructed())
						return;

					Packet_descriptor const p = _create();

					if (_size && _direction == Direction::OUT)
						policy.produce_out_content(urb, tx.packet_content(p),
						                           _size);
					tx.try_submit_packet(p);
				}

			public:

				Urb(Urb_handler &handler,
				    Direction    direction,
				    size_t       size = 0)
				:
					_urb_handler(handler),
					_direction(direction),
					_size(size)
				{
					_urb_handler._pending.enqueue(_pending_elem);
				}

				virtual ~Urb()
				{
					if (pending())
						_urb_handler._pending.remove(_pending_elem);
					else if (in_progress())
						warning("usb-session urb prematurely destructed");
				}

				bool in_progress() const { return _tag.constructed(); }
				bool completed() const { return _completed; }
				bool pending() const { return !in_progress() && !_completed; }
				Direction direction() const { return _direction; }
		};

	protected:

		Allocator_avl                _alloc;
		Packet_stream_tx::Client<Tx> _tx;
		Id_space<URB>                _tags    { };
		Fifo<Fifo_element<URB>>      _pending { };

		template <typename POLICY>
		bool _try_process_ack(POLICY &, Tx::Source &);

		template <typename POLICY>
		bool _try_submit_pending_urb(POLICY &, Tx::Source &);

	public:

		Urb_handler(Capability<typename SESSION::Tx> cap,
		            Region_map &rm, Allocator &md_alloc)
		: _alloc(&md_alloc), _tx(cap, rm, _alloc) {}

		/**
		 * Handle the submission and completion of URBs
		 *
		 * \return  true if progress was made
		 */
		template <typename POLICY>
		bool update_urbs(POLICY &policy)
		{
			typename Tx::Source &tx = *_tx.source();

			bool overall_progress = false;

			for (;;) {

				/* track progress of a single iteration */
				bool progress = false;

				/* process acknowledgements */
				while (_try_process_ack(policy, tx))
					progress = true;

				/* try to submit pending requests */
				while (_try_submit_pending_urb(policy, tx))
					progress = true;

				overall_progress |= progress;

				if (!progress)
					break;
			}

			if (overall_progress)
				tx.wakeup();

			return overall_progress;
		}

		/**
		 * Interface of 'POLICY' argument for 'update_urbs'
		 */
		struct Update_urbs_policy
		{
			/**
			 * Produce content sent to device
			 *
			 * \param dst     destination buffer (located within the I/O
			 *                communication buffer shared with the server)
			 * \param length  size of 'dst' buffer in bytes
			 */
			void produce_out_content(Urb &, char *dst, size_t length);

			/**
			 * Consume data received from device
			 *
			 * \param src     pointer to received data
			 * \param length  number of bytes received
			 */
			void consume_in_result(Urb &, char const *src, size_t length);

			/**
			 * Respond on the completion of the given urb
			 */
			void completed(Urb &, Packet_descriptor::Return_value);
		};

		/**
		 * Call 'fn' with each urb as argument
		 *
		 * This method is intended for the destruction of the urbs associated
		 * with the handler before destructing the 'Urb_handler' object.
		 */
		template <typename FN>
		void dissolve_all_urbs(FN const &fn)
		{
			_pending.dequeue_all([&] (Fifo_element<URB> &elem) {
				fn(elem.object()); });

			auto discard_tag_and_apply_fn = [&] (URB &urb) {
				urb._tag.destruct();
				fn(urb);
				Packet_descriptor const p { urb._payload.offset,
				                            urb._payload.bytes };
				_tx.source()->release_packet(p);
			};

			while (_tags.template apply_any<URB>(discard_tag_and_apply_fn));
		}

		void sigh(Signal_context_capability cap)
		{
			_tx.sigh_ack_avail(cap);
			_tx.sigh_ready_to_submit(cap);
		}
};


class Usb::Interface
:
	Noncopyable,
	Interface_capability,
	public Interface_session
{
	public:

		struct Index
		{
			uint8_t number;
			uint8_t alt_setting;
		};

		struct Type
		{
			uint8_t cla;
			uint8_t subcla;
			uint8_t prot;
		};

		class Urb : public Urb_handler<Interface_session, Urb>::Urb
		{
			private:

				using Base = Urb_handler<Interface_session, Urb>::Urb;

				Packet_descriptor::Type _type;
				Endpoint &_ep;

				Packet_descriptor _create() const override
				{
					Packet_descriptor p = Base::_create();
					p.index = _ep.address();
					p.type  = _type;
					return p;
				}

			public:

				Urb(Interface &iface, Endpoint &ep,
				    Packet_descriptor::Type type,
				    size_t size = 0)
				:
					Base(iface._urb_handler, ep.direction(), size),
					_type(type), _ep(ep) {}
		};

		using Policy = Urb_handler<Interface_session, Urb>::Update_urbs_policy;

		struct Alt_setting;

	private:

		friend class Endpoint;
		friend class Urb;

		enum { MAX_EPS = 16 };

		Device                             &_device;
		Index                               _idx;
		Urb_handler<Interface_session, Urb> _urb_handler;
		Endpoint                            _eps[2][MAX_EPS] { };

	public:

		struct Interface_not_avail : Exception {};

		Interface(Device &device, Index idx, size_t buffer_size);
		Interface(Device &device, Type type, size_t buffer_size);
		Interface(Device &device, size_t buffer_size);
		~Interface();

		Index index() const { return _idx; }

		void sigh(Signal_context_capability cap) {
			_urb_handler.sigh(cap); }

		template <typename POLICY>
		bool update_urbs(POLICY &policy) {
			return _urb_handler.update_urbs(policy); }

		template <typename FN>
		void dissolve_all_urbs(FN const &fn) {
			_urb_handler.dissolve_all_urbs(fn); }

		template <typename FN>
		void for_each_endpoint(FN const &fn)
		{
			for (unsigned d = Endpoint::OUT; d <= Endpoint::IN; d++)
				for (unsigned n = 0; n < MAX_EPS; n++)
					if (_eps[d][n].valid()) fn(_eps[d][n]);
		}
};


class Usb::Device
:
	Noncopyable,
	Device_capability,
	public Device_session
{
	public:

		using Interface = Usb::Interface;
		using Name = Usb::Session::Device_name;

		class Urb : public Urb_handler<Device_session, Urb>::Urb
		{
			public:

				using Type = Packet_descriptor::Request_type;

			private:

				using Base = Urb_handler<Device_session, Urb>::Urb;

				uint8_t        _request;
				Type::access_t _request_type;
				uint16_t       _value;
				uint16_t       _index;

				Packet_descriptor _create() const override
				{
					Packet_descriptor p = Base::_create();
					p.request_type = _request_type;
					p.request      = _request;
					p.value        = _value;
					p.index        = _index;
					return p;
				}

			public:

				Urb(Device &device,
				    uint8_t  request, Type::access_t request_type,
				    uint16_t value, uint16_t index, size_t size = 0)
				:
					Base(device._urb_handler,
					     Type::D::get(request_type) ? Endpoint::Direction::IN
					                                : Endpoint::Direction::OUT,
					     size),
					_request(request), _request_type(request_type),
					_value(value), _index(index) {}
		};

		using Policy = Urb_handler<Device_session, Urb>::Update_urbs_policy;

	private:

		friend class Usb::Interface;
		friend class Endpoint;
		friend class Urb;

		::Usb::Connection &_session;
		Allocator         &_md_alloc;
		Region_map        &_rm;
		Name         const _name { };

		Urb_handler<Device_session, Urb> _urb_handler;

		Interface_capability _interface_cap(uint8_t num, size_t buf_size);

		Name _first_device_name();

		template <typename FN>
		void _for_each_iface(FN const & fn);

		Interface::Index _interface_index(Interface::Type);

	public:

		Device(Connection &usb_session, Allocator &md_alloc,
		       Region_map &rm, Name name);

		Device(Connection &session, Allocator &md_alloc, Region_map &rm);

		~Device() { _session.release_device(*this); }

		void sigh(Signal_context_capability cap) {
			_urb_handler.sigh(cap); }

		template <typename POLICY>
		bool update_urbs(POLICY &policy) {
			return _urb_handler.update_urbs(policy); }

		template <typename FN>
		void dissolve_all_urbs(FN const &fn) {
			_urb_handler.dissolve_all_urbs(fn); }
};


struct Usb::Interface::Alt_setting : Device::Urb
{
	using P  = Device::Packet_descriptor;
	using Rt = P::Request_type;

	Alt_setting(Device &dev, Interface &iface)
	:
		Device::Urb(dev,
		            P::Request::SET_INTERFACE,
		            Rt::value(P::Recipient::IFACE, P::Type::STANDARD,
		                      P::Direction::IN),
		            iface.index().number, iface.index().alt_setting) {}
};


template <typename SESSION, typename URB>
template <typename POLICY>
bool Usb::Urb_handler<SESSION, URB>::_try_process_ack(POLICY &policy,
                                                      Tx::Source &tx)
{
	if (!tx.ack_avail())
		return false;

	Packet_descriptor const p = tx.try_get_acked_packet();

	typename Id_space<URB>::Id const id { p.tag.value };

	try {
		_tags.template apply<URB>(id, [&] (URB &urb) {

			if (urb._direction == Direction::IN &&
			    p.return_value == Packet_descriptor::OK)
				policy.consume_in_result(urb, tx.packet_content(p),
				                         p.payload_return_size);

			urb._completed = true;
			urb._tag.destruct();
			policy.completed(urb, p.return_value);
		});
	} catch (typename Id_space<URB>::Unknown_id) {
		warning("spurious usb-session urb acknowledgement");
	}

	tx.release_packet(p);
	return true;
}


template <typename SESSION, typename URB>
template <typename POLICY>
bool Usb::Urb_handler<SESSION, URB>::_try_submit_pending_urb(POLICY &policy,
                                                             Tx::Source &tx)
{
	if (_pending.empty())
		return false;

	if (!tx.ready_to_submit())
		return false;

	/*
	 * Allocate space for the payload in the packet-stream buffer.
	 */

	Payload payload { };
	try {
		_pending.head([&] (Fifo_element<URB> const &elem) {

			Urb const &urb = elem.object();
			size_t const align = Tagged_packet::PACKET_ALIGNMENT;
			payload = {
				.offset = tx.alloc_packet(urb._size, align).offset(),
				.bytes  = urb._size
			};
		});
	} catch (typename Tx::Source::Packet_alloc_failed) {
		/* the packet-stream buffer is saturated */
		return false;
	}

	/*
	 * All preconditions for the submission of the urb are satisfied.
	 * So the urb can go from the pending to the in-progress stage.
	 */

	_pending.dequeue([&] (Genode::Fifo_element<URB> &elem) {

		Urb &urb = elem.object();

		/* let the urb join the tag ID space, allocating a tag */
		urb._tag.construct(elem.object(), _tags);

		urb._payload = payload;
		urb._submit(policy, elem.object(), tx);
	});

	return true;
}


Usb::Endpoint::Endpoint(Interface &iface, Direction d, Type t)
{
	bool found = false;

	iface.for_each_endpoint([&] (Endpoint ep) {
		if (ep.type() == t && ep.direction() == d) {
			_attributes = ep._attributes;
			_address    = ep._address;
			found       = true;
		}
	});

	if (!found)
		throw Endpoint_not_avail();
};


Usb::Interface::Interface(Device &device, Index idx, size_t buffer_size)
:
	Interface_capability(device._interface_cap(idx.number, buffer_size)),
	_device(device), _idx(idx),
	_urb_handler(call<Rpc_tx_cap>(), device._rm, device._md_alloc)
{
	static constexpr uint16_t INVALID = 256;

	device._for_each_iface([&] (Xml_node node) {
		if (node.attribute_value<uint16_t>("number", INVALID) != idx.number)
			return;
		node.for_each_sub_node("endpoint", [&] (Xml_node node) {
			Endpoint ep { node.attribute_value<uint8_t>("address", 0),
			              node.attribute_value<uint8_t>("attributes", 0) };
			if (!_eps[ep.direction()][ep.number()].valid())
				_eps[ep.direction()][ep.number()] = ep;
		});
	});
}


Usb::Interface::Interface(Device &device, Type type, size_t buffer_size)
: Interface(device, device._interface_index(type), buffer_size) { }


Usb::Interface::Interface(Device &device, size_t buffer_size)
: Interface(device, Index { 0, 0 }, buffer_size) { }


Usb::Interface::~Interface() {
	_device.call<Device_session::Rpc_release_interface>(*this); }


Usb::Interface_capability
Usb::Device::_interface_cap(uint8_t num, size_t buf_size)
{
	_session.upgrade_ram(buf_size);
	return _session.retry_with_upgrade(Ram_quota{6*1024}, Cap_quota{6}, [&] () {
		return call<Device_session::Rpc_acquire_interface>(num, buf_size); });
}


Usb::Device::Name Usb::Device::_first_device_name()
{
	Name ret;
	_session.with_xml([&] (Xml_node & xml) {
		xml.with_optional_sub_node("device", [&] (Xml_node node) {
			ret = node.attribute_value("name", Name()); });
	});
	return ret;
}


template <typename FN>
void Usb::Device::_for_each_iface(FN const & fn)
{
	_session.with_xml([&] (Xml_node & xml) {
		xml.for_each_sub_node("device", [&] (Xml_node node) {
			if (node.attribute_value("name", Name()) == _name)
				node.for_each_sub_node("config", [&] (Xml_node node) {
					if (node.attribute_value("active", false))
						node.for_each_sub_node("interface", fn);
				});
		});
	});
}


Usb::Interface::Index
Usb::Device::_interface_index(Interface::Type t)
{
	static constexpr uint16_t INVALID = 256;

	uint16_t num = INVALID, alt = INVALID;

	_for_each_iface([&] (Xml_node node) {
		uint16_t c = node.attribute_value("class",    INVALID);
		uint16_t s = node.attribute_value("subclass", INVALID);
		uint16_t p = node.attribute_value("protocol", INVALID);
		if (c == t.cla && s == t.subcla && p == t.prot) {
			num = node.attribute_value("number",      INVALID);
			alt = node.attribute_value("alt_setting", INVALID);
		}
	});

	if (num < INVALID && alt < INVALID)
		return { (uint8_t)num, (uint8_t)alt };

	throw Interface::Interface_not_avail();
}


Usb::Device::Device(Connection &session,
                    Allocator  &md_alloc,
                    Region_map &rm,
                    Name        name)
:
	Device_capability(session.acquire_device(name)),
	_session(session),
	_md_alloc(md_alloc),
	_rm(rm),
	_name(name),
	_urb_handler(call<Rpc_tx_cap>(), rm, md_alloc) {}


Usb::Device::Device(Connection &session,
                    Allocator  &md_alloc,
                    Region_map &rm)
:
	Device_capability(session.acquire_device()),
	_session(session),
	_md_alloc(md_alloc),
	_rm(rm),
	_name(_first_device_name()),
	_urb_handler(call<Rpc_tx_cap>(), rm, md_alloc) {}

#endif /* _INCLUDE__USB_SESSION__DEVICE_H_ */
