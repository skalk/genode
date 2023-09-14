/*
 * \brief  Genode USB client provider C-API
 * \author Sebastian Sumpf
 * \author Stefan Kalkowski
 * \date   2023-06-29
 */

/*
 * Copyright (C) 2023 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#include <base/id_space.h>
#include <base/heap.h>
#include <util/bit_allocator.h>
#include <util/reconstructible.h>
#include <util/list_model.h>

#include <usb_session/device.h>
#include <genode_c_api/usb_client.h>

using namespace Genode;

struct Device;

struct Endpoint : List_model<Endpoint>::Element
{
	uint8_t const address;
	uint8_t const attributes;

	Endpoint(Xml_node const &n)
	:
		address(n.attribute_value<uint8_t>("address", 0xff)),
		attributes(n.attribute_value<uint8_t>("attributes", 0xff)) {}

	bool matches(Xml_node const &node) const {
		return address == node.attribute_value<uint8_t>("address", 0xff); }

	static bool type_matches(Xml_node const &node) {
		return node.has_type("endpoint"); }
};


class Interface : public List_model<::Interface>::Element
{
	private:

		Device                       &_device;
		Constructible<Usb::Interface> _iface {};
		List_model<Endpoint>          _endpoints {};
		uint8_t const                 _number;
		uint8_t const                 _alt_setting;
		bool                          _active;
		size_t                        _buf_size { 4096 * 8 }; /* FIXME: read from elsewhere */

		Usb::Interface &_session();

	public:

		class Urb : Usb::Endpoint, public Usb::Interface::Urb
		{
			private:

				friend class ::Interface;
				using Pdesc = Usb::Interface::Packet_descriptor;

				static Pdesc::Type _type(genode_usb_client_iface_type_t t)
				{
					switch(t) {
						case ::BULK:  return Pdesc::BULK;
						case ::IRQ:   return Pdesc::IRQ;
						case ::ISOC:  return Pdesc::ISOC;
						case ::FLUSH: return Pdesc::FLUSH;
					};
					return Pdesc::FLUSH;
				}

				struct Driver_data { void * const data; } _driver_data;

			public:

				Urb(::Interface                   &iface,
				    ::Endpoint                    &endp,
				    genode_usb_client_iface_type_t type,
				    size_t                         size,
				    void                          *opaque_data)
				:
					Usb::Endpoint(endp.address, endp.attributes),
					Usb::Interface::Urb(iface._session(), *this, _type(type), size),
					_driver_data{opaque_data} {}
		};

		struct Policy
		{
			genode_usb_client_produce_out_t out;
			genode_usb_client_consume_in_t  in;
			genode_usb_client_complete_t    complete;

			void produce_out_content(Usb::Interface::Urb &urb,
			                         char *dst, size_t length)
			{
				void * opaque_data = static_cast<Urb&>(urb)._driver_data.data;
				out(opaque_data, { (void*)dst, length });
			}

			void consume_in_result(Usb::Interface::Urb &urb,
			                       char const *src, size_t length)
			{
				void * opaque_data = static_cast<Urb&>(urb)._driver_data.data;
				in(opaque_data, { (void*)src, length });
			}

			void completed(Usb::Interface::Urb &urb,
			               Usb::Interface::Packet_descriptor::Return_value v)
			{
				using Retval = Usb::Interface::Packet_descriptor::Return_value;
				genode_usb_client_ret_val_t ret;
				void * opaque_data = static_cast<Urb&>(urb)._driver_data.data;
				switch (v) {
				case Retval::NO_DEVICE: ret = NO_DEVICE; break;
				case Retval::INVALID:   ret = INVALID;   break;
				case Retval::OK:        ret = OK;        break;
				default:
					error("unhandled packet should not happen!");
					ret = INVALID;
				};
				complete(opaque_data, ret);
			}
		};

		Interface(Device &device, Xml_node const &n)
		:
			_device(device),
			_number(n.attribute_value<uint8_t>("number", 0xff)),
			_alt_setting(n.attribute_value<uint8_t>("alt_setting", 0xff)),
			_active(n.attribute_value("active", false)) {}

		~Interface()
		{
			if (_iface.constructed())
				_iface->dissolve_all_urbs([] (Usb::Interface::Urb&) {});
		}

		bool active() const { return _active; }

		bool matches(Xml_node const &n) const
		{
			uint8_t nr  = n.attribute_value<uint8_t>("number", 0xff);
			uint8_t alt = n.attribute_value<uint8_t>("alt_setting", 0xff);
			return _number == nr && _alt_setting == alt;
		}

		static bool type_matches(Xml_node const &node) {
			return node.has_type("interface"); }

		void update(Allocator &alloc, Xml_node const &node)
		{
			_active = node.attribute_value("active", false);
			_endpoints.update_from_xml(node,

				/* create */
				[&] (Xml_node const &node) -> Endpoint & {
					return *new (alloc) Endpoint(node); },

				/* destroy */
				[&] (Endpoint &endp) {
					destroy(alloc, &endp); },

				/* update */
				[&] (Endpoint &, Xml_node const &) { }
			);
		}

		void update(genode_usb_client_produce_out_t out,
		            genode_usb_client_consume_in_t  in,
		            genode_usb_client_complete_t    complete)
		{
			if (!_iface.constructed())
				return;

			Policy policy { out, in, complete };
			_iface->update_urbs(policy);
		}

		template <typename FN>
		void with_endpoint(uint8_t index, FN const &fn)
		{
			_endpoints.for_each([&] (Endpoint &endp) {
				if (endp.address == index) fn(endp);
			});
		}
};


class Device : public List_model<Device>::Element
{
	public:

		using Name = String<64>;

	private:

		Name                      const _name;
		Id_space<Device>::Element const _elem;
		Usb::Device                     _device;
		Signal_context_capability       _sigh_cap;
		void                           *_driver_data { nullptr };
		List_model<::Interface>         _ifaces {};

		/**
		 * Noncopyable
		 */
		Device(Device const &);
		Device &operator = (Device const &);

	public:

		struct Urb : Usb::Device::Urb
		{
			using Request_type =
				Usb::Device::Packet_descriptor::Request_type::access_t;

			struct Driver_data { void * const data; } _driver_data;

			Urb(Device & device,
			    uint8_t  request,
			    uint8_t  request_type,
			    uint16_t value,
			    uint16_t index,
			    size_t   size,
			    void    *opaque_data)
			:
				Usb::Device::Urb(device._device, request,
				                 (Request_type)request_type,
				                 value, index, size),
				_driver_data{opaque_data} {}
		};

		struct Policy
		{
			genode_usb_client_produce_out_t out;
			genode_usb_client_consume_in_t  in;
			genode_usb_client_complete_t    complete;

			void produce_out_content(Usb::Device::Urb &urb,
			                         char *dst, size_t length)
			{
				void * opaque_data = static_cast<Urb&>(urb)._driver_data.data;
				out(opaque_data, { (void*)dst, length });
			}

			void consume_in_result(Usb::Device::Urb &urb,
			                       char const *src, size_t length)
			{
				void * opaque_data = static_cast<Urb&>(urb)._driver_data.data;
				in(opaque_data, { (void*)src, length });
			}

			void completed(Usb::Device::Urb &urb,
			               Usb::Device::Packet_descriptor::Return_value v)
			{
				using Retval = Usb::Device::Packet_descriptor::Return_value;
				genode_usb_client_ret_val_t ret;
				void * opaque_data = static_cast<Urb&>(urb)._driver_data.data;
				switch (v) {
				case Retval::NO_DEVICE: ret = NO_DEVICE; break;
				case Retval::INVALID:   ret = INVALID;   break;
				case Retval::TIMEOUT:   ret = TIMEOUT;   break;
				case Retval::OK:        ret = OK;        break;
				default:
					error("unhandled packet should not happen!");
					ret = INVALID;
				};
				complete(opaque_data, ret);
			}
		};

		Device(Name                     &name,
		       Usb::Connection          &usb,
		       Allocator                &alloc,
		       Region_map               &rm,
		       Id_space<Device>         &space,
		       Signal_context_capability cap)
		:
			_name(name),
			_elem(*this, space),
			_device(usb, alloc, rm, name),
			_sigh_cap(cap)
		{
			_device.sigh(_sigh_cap);
		}

		~Device() {
			_device.dissolve_all_urbs([] (Usb::Device::Urb&) {}); }

		Usb::Device &session() { return _device; }

		Name name() { return _name; }

		Signal_context_capability sigh_cap() { return _sigh_cap; }

		genode_usb_client_dev_handle_t handle() const {
			return _elem.id().value; }

		void  driver_data(void *data) { _driver_data = data; }
		void* driver_data()           { return _driver_data; }

		bool matches(Xml_node const &node) const {
			return _name == node.attribute_value("name", Name()); }

		static bool type_matches(Xml_node const &node) {
			return node.has_type("device"); }

		void update(Allocator &alloc, Xml_node const &node)
		{
			Xml_node active_config = node;

			node.for_each_sub_node("config", [&] (Xml_node const &node) {
				if (node.attribute_value("active", false))
					active_config = node;
			});

			_ifaces.update_from_xml(active_config,

				/* create */
				[&] (Xml_node const &node) -> ::Interface & {
					return *new (alloc) ::Interface(*this, node); },

				/* destroy */
				[&] (::Interface &iface) {
					iface.update(alloc, Xml_node("<empty/>"));
					destroy(alloc, &iface); },

				/* update */
				[&] (::Interface &iface, Xml_node const &node) {
					iface.update(alloc, node); }
			);
		}

		void update(genode_usb_client_produce_out_t out,
		            genode_usb_client_consume_in_t  in,
		            genode_usb_client_complete_t    complete)
		{
			Policy policy { out, in, complete };
			_device.update_urbs(policy);
			_ifaces.for_each([&] (::Interface &iface) {
				iface.update(out, in, complete); });
		}

		template <typename FN>
		void with_active_interfaces(FN const &fn)
		{
			_ifaces.for_each([&] (::Interface &iface) {
				if (iface.active()) fn(iface); });
		}
};


struct Session
{
	Env                      &_env;
	Allocator                &_alloc;
	Signal_context_capability _handler_cap;
	Usb::Connection           _usb { _env };
	List_model<Device>        _model {};
	Id_space<Device>          _space {};

	Session(Env &env, Allocator &alloc, Signal_context_capability cap)
	:
		_env(env), _alloc(alloc), _handler_cap(cap)
	{
		_usb.sigh(cap);
	}

	~Session() {
		_model.for_each([&] (Device &dev) { destroy(_alloc, &dev); }); }

	void update(genode_usb_client_dev_add_t add,
	            genode_usb_client_dev_del_t del)
	{
		_usb.with_xml([&] (Xml_node node) {
			_model.update_from_xml(node,

				/* create */
				[&] (Xml_node const &node) -> Device &
				{
					Device::Name name = node.attribute_value("name",
					                                         Device::Name());
					Device &dev = *new (_alloc) Device(name, _usb, _alloc,
					                                   _env.rm(), _space,
					                                   _handler_cap);
					return dev;
				},

				/* destroy */
				[&] (Device &dev)
				{
					if (dev.driver_data()) del(dev.handle(), dev.driver_data());
					dev.update(_alloc, Xml_node("<empty/>"));
					destroy(_alloc, &dev);
				},

				/* update */
				[&] (Device &dev, Xml_node const &node)
				{
					dev.update(_alloc, node);
				}
			);
		});

		/* add new devices for C-API client after it got successfully added */
		_model.for_each([&] (Device &dev) {
			if (!dev.driver_data())
				dev.driver_data(add(dev.handle(), dev.name().string()));
		});
	}
};


Usb::Interface &::Interface::_session()
{
	if (!_iface.constructed()) {
		_iface.construct(_device.session(),
		                 Usb::Interface::Index{_number, _alt_setting},
		                 _buf_size);
		_iface->sigh(_device.sigh_cap());
	}
	return *_iface;
};


static ::Session * _usb_session = nullptr;


extern "C"
void genode_usb_client_init(struct genode_env            *env,
                            struct genode_allocator      *alloc,
                            struct genode_signal_handler *handler)
{
	static ::Session session(*static_cast<Env*>(env),
	                         *static_cast<Allocator*>(alloc),
	                         cap(handler));
	_usb_session = &session;
};


extern "C"
void genode_usb_client_update(genode_usb_client_dev_add_t add,
                              genode_usb_client_dev_del_t del)
{
	if (_usb_session) _usb_session->update(add, del);
}


extern "C"
genode_usb_client_ret_val_t
genode_usb_client_device_control(genode_usb_client_dev_handle_t handle,
                                 genode_uint8_t                 request,
                                 genode_uint8_t                 request_type,
                                 genode_uint16_t                value,
                                 genode_uint16_t                index,
                                 unsigned long                  size,
                                 void                          *opaque_data)
{
	try {
		if (!_usb_session)
			return NO_DEVICE;

		return _usb_session->_space.apply<Device>({ handle },
		                                          [&] (Device & device) {
			new (_usb_session->_alloc)
				Device::Urb(device, request, request_type,
				            value, index, size, opaque_data);
			return OK;
		});
	} catch(Id_space<Device>::Unknown_id&) {
		return NO_DEVICE;
	} catch(...) {
		return NO_MEMORY;
	}
}


extern "C"
void genode_usb_client_device_update(genode_usb_client_produce_out_t out,
                                     genode_usb_client_consume_in_t  in,
                                     genode_usb_client_complete_t    complete)
{
	try {
		if (!_usb_session)
			return;
		_usb_session->_model.for_each([&] (Device & device) {
			device.update(out, in, complete); });

	} catch(...) { }
}


extern "C"
genode_usb_client_ret_val_t
genode_usb_client_iface_transfer(genode_usb_client_dev_handle_t handle,
                                 genode_usb_client_iface_type_t type,
                                 genode_uint8_t                 index,
                                 unsigned long                  size,
                                 void                          *opaque_data)
{
	try {
		if (!_usb_session)
			return NO_DEVICE;

		genode_usb_client_ret_val_t ret = NO_DEVICE;

		_usb_session->_space.apply<Device>({ handle },
		                                   [&] (Device & device) {
			device.with_active_interfaces([&] (::Interface &iface) {
				iface.with_endpoint(index, [&] (Endpoint &endp) {
					new (_usb_session->_alloc)
						::Interface::Urb(iface, endp, type, size, opaque_data);
					ret = OK;
				});
			});
		});

		return ret;
	} catch(Id_space<Device>::Unknown_id&) {
		return NO_DEVICE;
	} catch(...) {
		return NO_MEMORY;
	}
}
