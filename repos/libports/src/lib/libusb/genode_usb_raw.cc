/*
 * \brief  Genode backend for libusb
 * \author Christian Prochaska
 * \author Stefan Kalkowski
 * \date   2016-09-19
 */

/*
 * Copyright (C) 2016-2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#include <base/log.h>
#include <base/registry.h>
#include <base/allocator_avl.h>
#include <base/signal.h>
#include <usb_session/device.h>

#include <fcntl.h>
#include <time.h>
#include <libc/allocator.h>

#include <internal/thread_create.h>

#include "libusbi.h"

using namespace Genode;

#if 0
void handle_events()
{
	struct libusb_context *ctx = nullptr;

	while (usb_connection->source()->ack_avail()) {

		Usb::Packet_descriptor p =
			usb_connection->source()->get_acked_packet();

		if (p.type == Usb::Packet_descriptor::ALT_SETTING) {
			usb_connection->source()->release_packet(p);
			continue;
		}

		Completion *completion = static_cast<Completion*>(p.completion);
		struct usbi_transfer *itransfer = completion->itransfer;

		if (_open)
			ctx = ITRANSFER_CTX(itransfer);

		destroy(libc_alloc, completion);

		if (_open == 0) {
			usb_connection->source()->release_packet(p);
			continue;
		}

		if (!p.succeded || itransfer->flags & USBI_TRANSFER_CANCELLING) {
			if (!p.succeded) {
				if (p.error == Usb::Packet_descriptor::NO_DEVICE_ERROR)
					throw Device_has_vanished();
			}
			itransfer->transferred = 0;
			usb_connection->source()->release_packet(p);
			usbi_signal_transfer_completion(itransfer);
			continue;
		}

		char *packet_content = usb_connection->source()->packet_content(p);

		struct libusb_transfer *transfer =
			USBI_TRANSFER_TO_LIBUSB_TRANSFER(itransfer);

		switch (transfer->type) {

			case LIBUSB_TRANSFER_TYPE_CONTROL: {

				itransfer->transferred = p.control.actual_size;

				struct libusb_control_setup *setup =
					(struct libusb_control_setup*)transfer->buffer;

				if ((setup->bmRequestType & LIBUSB_ENDPOINT_DIR_MASK) ==
				    LIBUSB_ENDPOINT_IN) {
					Genode::memcpy(transfer->buffer + LIBUSB_CONTROL_SETUP_SIZE,
					               packet_content, p.control.actual_size);
				}

				break;
			}

			case LIBUSB_TRANSFER_TYPE_BULK:
			case LIBUSB_TRANSFER_TYPE_BULK_STREAM:
			case LIBUSB_TRANSFER_TYPE_INTERRUPT: {

				itransfer->transferred = p.transfer.actual_size;

				if (IS_XFERIN(transfer))
					Genode::memcpy(transfer->buffer, packet_content,
					               p.transfer.actual_size);

				break;
			}

			case LIBUSB_TRANSFER_TYPE_ISOCHRONOUS: {

				itransfer->transferred = p.transfer.actual_size;

				if (IS_XFERIN(transfer)) {

					Usb::Isoc_transfer &isoc = *(Usb::Isoc_transfer *)packet_content;

					unsigned out_offset = 0;
					for (unsigned i = 0; i < isoc.number_of_packets; i++) {
						size_t const actual_length = isoc.actual_packet_size[i];

						/*
						 * Copy the data from the proper offsets within the buffer as
						 * a short read is still stored at this location.
						 */
						unsigned char       * dst = transfer->buffer + out_offset;
						         char const * src = isoc.data() + out_offset;

						Genode::memcpy(dst, src, actual_length);
						out_offset += transfer->iso_packet_desc[i].length;

						transfer->iso_packet_desc[i].actual_length = actual_length;
						transfer->iso_packet_desc[i].status = LIBUSB_TRANSFER_COMPLETED;
					}
					transfer->num_iso_packets = isoc.number_of_packets;

				}

				break;
			}

			default:
				Genode::error(__PRETTY_FUNCTION__,
				              ": unsupported transfer type");
				usb_connection->source()->release_packet(p);
				continue;
		}

		usb_connection->source()->release_packet(p);

		usbi_signal_transfer_completion(itransfer);
	}

	if (ctx != nullptr)
		usbi_signal_event(ctx);
}
};
#endif


static int vfs_libusb_fd { -1 };


struct Usb_device
{
	struct Interface : Usb::Interface, Registry<Usb_device::Interface>::Element
	{
		struct Urb : Usb::Interface::Urb
		{
			void *          const buf;
			size_t          const size;
			usbi_transfer * const itransfer;

			Urb(Usb::Interface &iface,
			    Usb::Endpoint  &ep,
			    Usb::Interface::Packet_descriptor::Type type,
			    void * buf, size_t size,
			    usbi_transfer *itransfer)
			:
				Usb::Interface::Urb(iface, ep, type, size),
				buf(buf), size(size), itransfer(itransfer) {}
		};

		Usb_device &_device;

		Interface(Usb_device &device, uint8_t idx);

		void handle_events() { update_urbs(*this); }


		/****************************
		 ** Usb::Interface::Policy **
		 ****************************/

		void produce_out_content(Usb::Interface::Urb &, char*, size_t);
		void consume_in_result(Usb::Interface::Urb &urb, char const *, size_t);
		void completed(Usb::Interface::Urb &,
		               Usb::Interface::Packet_descriptor::Return_value);
	};


	struct Urb : Usb::Device::Urb
	{
		void *          const buf;
		size_t          const size;
		usbi_transfer * const itransfer;

		Urb(Usb::Device &device,
		    uint8_t  request, Type::access_t request_type,
		    uint16_t value, uint16_t index, size_t size,
		    void * buf, size_t buf_size,
		    usbi_transfer *itransfer = nullptr)
		:
			Usb::Device::Urb(device, request, request_type, value, index, size),
			buf(buf), size(buf_size), itransfer(itransfer) {}
	};


	Env                      &_env;
	Allocator                &_alloc;
	Signal_context_capability _handler_cap;
	Usb::Connection           _session { _env };
	Usb::Device               _device  { _session, _alloc, _env.rm() };
	unsigned                  _open { 0 };
	Registry<Interface>       _interfaces {};


	void _wait_for_urb(Urb &urb)
	{
		while (!urb.completed()) {
			_device.update_urbs(*this);

			struct pollfd pollfds {
				.fd      = vfs_libusb_fd,
				.events  = POLLIN,
				.revents = 0
			};

			int poll_result = poll(&pollfds, 1, -1);
			if ((poll_result != 1) || !(pollfds.revents & POLLIN))
				error("could not complete request");
		}
	}

	Usb_device(Env &env, Allocator &alloc, Signal_context_capability cap)
	:
		_env(env), _alloc(alloc), _handler_cap(cap)
	{
		_device.sigh(_handler_cap);
	}

	void close() { _open--; }
	void open()  { _open++; }

	void handle_events()
	{
		_device.update_urbs(*this);
		_interfaces.for_each([&] (Interface &iface) {
			iface.update_urbs(iface); });
	}

	static Constructible<Usb_device>& singleton();


	/*************************
	 ** Usb::Device::Policy **
	 *************************/

	void produce_out_content(Usb::Device::Urb &, char*, size_t);
	void consume_in_result(Usb::Device::Urb &urb, char const *, size_t);
	void completed(Usb::Device::Urb &,
	               Usb::Device::Packet_descriptor::Return_value);
};


Usb_device::Interface::Interface(Usb_device &device, uint8_t idx)
:
	Usb::Interface(device._device, Usb::Interface::Index{idx, 0}, (1UL << 20)),
	Registry<Usb_device::Interface>::Element(device._interfaces, *this),
	_device(device) { }


void Usb_device::Interface::produce_out_content(Usb::Interface::Urb &urb,
                                                char *dst, size_t size)
{
	Urb &u = static_cast<Urb&>(urb);
	memcpy(dst, u.buf, min(size, u.size));
}


void Usb_device::Interface::consume_in_result(Usb::Interface::Urb &urb,
                                              char const *src, size_t size)
{
	Urb &u = static_cast<Urb&>(urb);
	memcpy(u.buf, src, min(size, u.size));
	u.itransfer->transferred = size;
}


void Usb_device::Interface::completed(Usb::Interface::Urb &urb,
                                      Usb::Interface::Packet_descriptor::Return_value v)
{
	if (v != Usb::Interface::Packet_descriptor::OK)
		error("tranfer failed, return value ", (int)v);

	Urb &u = static_cast<Urb&>(urb);

	if (!u.itransfer)
		return;

	libusb_context *ctx = _device._open ? ITRANSFER_CTX(u.itransfer) : nullptr;
	usbi_signal_transfer_completion(u.itransfer);
	if (ctx) usbi_signal_event(ctx);
	destroy(_device._alloc, &u);
}


void Usb_device::produce_out_content(Usb::Device::Urb &urb,
                                     char *dst, size_t size)
{
	Urb &u = static_cast<Urb&>(urb);
	memcpy(dst, u.buf, min(size, u.size));
}


void Usb_device::consume_in_result(Usb::Device::Urb &urb,
                                   char const *src, size_t size)
{
	Urb &u = static_cast<Urb&>(urb);
	memcpy(u.buf, src, min(size, u.size));
	if (u.itransfer) u.itransfer->transferred = size;
}


void Usb_device::completed(Usb::Device::Urb &urb,
                           Usb::Device::Packet_descriptor::Return_value v)
{
	if (v != Usb::Device::Packet_descriptor::OK)
		error("control tranfer failed, return value ", (int)v);

	Urb &u = static_cast<Urb&>(urb);

	if (!u.itransfer)
		return;

	libusb_context *ctx = _open ? ITRANSFER_CTX(u.itransfer) : nullptr;
	usbi_signal_transfer_completion(u.itransfer);
	if (ctx) usbi_signal_event(ctx);
	destroy(_alloc, &u);
}


Constructible<Usb_device>& Usb_device::singleton()
{
	static Constructible<Usb_device> dev {};
	return dev;
}


static Usb_device& device()
{
	struct Libusb_not_initialized {};

	if (!Usb_device::singleton().constructed())
		throw Libusb_not_initialized();

	return *Usb_device::singleton();
}


void libusb_genode_backend_init(Env &env, Allocator &alloc,
                                Signal_context_capability handler)
{
	Usb_device::singleton().construct(env, alloc, handler);
}


bool libusb_genode_backend_ready()
{
	return true;
}


static int genode_init(struct libusb_context* ctx)
{
	if (vfs_libusb_fd == -1) {
		vfs_libusb_fd = open("/dev/libusb", O_RDONLY);
		if (vfs_libusb_fd == -1) {
			Genode::error("could not open /dev/libusb");
			return LIBUSB_ERROR_OTHER;
		}
	} else {
		Genode::error("tried to init genode usb context twice");
		return LIBUSB_ERROR_OTHER;
	}

	return LIBUSB_SUCCESS;
}


static void genode_exit(void)
{
	Usb_device::singleton().constructed();
		Usb_device::singleton().destruct();

	if (vfs_libusb_fd != -1) {
		close(vfs_libusb_fd);
		vfs_libusb_fd = -1;
	}
}


int genode_get_device_list(struct libusb_context *ctx,
                           struct discovered_devs **discdevs)
{
	unsigned long session_id;
	struct libusb_device *dev;

	uint8_t busnum = 1;
	uint8_t devaddr = 1;

	session_id = busnum << 8 | devaddr;
	usbi_dbg("busnum %d devaddr %d session_id %ld", busnum, devaddr,
		session_id);

	dev = usbi_get_device_by_session_id(ctx, session_id);

	if (!dev) {
	
		usbi_dbg("allocating new device for %d/%d (session %ld)",
		 	 busnum, devaddr, session_id);
		dev = usbi_alloc_device(ctx, session_id);
		if (!dev)
			return LIBUSB_ERROR_NO_MEM;

		/* initialize device structure */
		dev->bus_number = busnum;
		dev->device_address = devaddr;
		dev->speed = LIBUSB_SPEED_SUPER;

		int result = usbi_sanitize_device(dev);
		if (result < 0) {
			libusb_unref_device(dev);
			return result;
		}

	} else {
		usbi_dbg("session_id %ld already exists", session_id);
	}

	if (discovered_devs_append(*discdevs, dev) == NULL) {
		libusb_unref_device(dev);
		return LIBUSB_ERROR_NO_MEM;
	}

	libusb_unref_device(dev);

	return LIBUSB_SUCCESS;
}


static int genode_open(struct libusb_device_handle *dev_handle)
{
	device().open();
	return usbi_add_pollfd(HANDLE_CTX(dev_handle), vfs_libusb_fd,
	                       POLLIN);
}


static void genode_close(struct libusb_device_handle *dev_handle)
{
	device().close();
	usbi_remove_pollfd(HANDLE_CTX(dev_handle), vfs_libusb_fd);
}


static int genode_get_device_descriptor(struct libusb_device *,
                                        unsigned char* buffer,
                                        int *host_endian)
{
	Usb_device::Urb urb(device()._device, LIBUSB_REQUEST_GET_DESCRIPTOR,
	                    LIBUSB_ENDPOINT_IN, (LIBUSB_DT_DEVICE << 8) | 0, 0,
	                    LIBUSB_DT_DEVICE_SIZE, buffer,
	                    sizeof(libusb_device_descriptor));
	device()._wait_for_urb(urb);
	*host_endian = 0;
	return LIBUSB_SUCCESS;
}


static int genode_get_config_descriptor(struct libusb_device *,
                                        uint8_t idx,
                                        unsigned char *buffer,
                                        size_t len,
                                        int *host_endian)
{
	/* read minimal config descriptor */
	genode_usb_config_descriptor desc;
	Usb_device::Urb cfg(device()._device, LIBUSB_REQUEST_GET_DESCRIPTOR,
	                    LIBUSB_ENDPOINT_IN, (LIBUSB_DT_CONFIG << 8) | idx, 0,
	                    sizeof(desc), &desc, sizeof(desc));
	device()._wait_for_urb(cfg);

	/* read again whole configuration */
	Usb_device::Urb all(device()._device, LIBUSB_REQUEST_GET_DESCRIPTOR,
	                    LIBUSB_ENDPOINT_IN, (LIBUSB_DT_CONFIG << 8) | idx, 0,
	                    desc.total_length, buffer, len);
	device()._wait_for_urb(all);

	*host_endian = 0;
	return desc.total_length;
}


static int genode_get_active_config_descriptor(struct libusb_device *device,
                                               unsigned char *buffer,
                                               size_t len,
                                               int *host_endian)
{
	return genode_get_config_descriptor(device, 0, buffer, len, host_endian);
}


static int genode_set_configuration(struct libusb_device_handle *dev_handle,
                                    int config)
{
	Genode::error(__PRETTY_FUNCTION__,
	              ": not implemented (return address: ",
	              Genode::Hex((Genode::addr_t)__builtin_return_address(0)),
	              ") \n");
	return LIBUSB_ERROR_NOT_SUPPORTED;
}


static int genode_claim_interface(struct libusb_device_handle *dev_handle,
                                  int interface_number)
{
	bool found = false;

	error("claim iface ", interface_number);
	device()._interfaces.for_each([&] (Usb_device::Interface const &iface) {
		if (iface.index().number == interface_number) found = true; });

	if (found) {
		error(__PRETTY_FUNCTION__, ": interface already claimed");
		return LIBUSB_ERROR_BUSY;
	}

	if (interface_number > 0xff || interface_number < 0) {
		error(__PRETTY_FUNCTION__, ": invalid interface number ",
		      interface_number);
		return LIBUSB_ERROR_OTHER;
	}

	new (device()._alloc)
		Usb_device::Interface(device(), (uint8_t) interface_number);
	error("claim iface done");
	return LIBUSB_SUCCESS;
}


static int genode_release_interface(struct libusb_device_handle *dev_handle,
                                    int interface_number)
{
#if 0
	Usb_device *usb_device = *(Usb_device**)dev_handle->dev->os_priv;

	try {
		usb_device->usb_connection->release_interface(interface_number);
	} catch (Usb::Session::Interface_not_found) {
		Genode::error(__PRETTY_FUNCTION__, ": interface not found");
		return LIBUSB_ERROR_NOT_FOUND;
	} catch (...) {
		Genode::error(__PRETTY_FUNCTION__, ": unknown exception");
		return LIBUSB_ERROR_OTHER;
	}
#endif

	error(__func__, " not implemented yet!");
	return LIBUSB_SUCCESS;
}


static int genode_set_interface_altsetting(struct libusb_device_handle* dev_handle,
                                           int interface_number,
                                           int altsetting)
{
	using P  = Usb::Device::Packet_descriptor;
	using Rt = P::Request_type;

	if (interface_number < 0 || interface_number > 0xff ||
	    altsetting < 0 || altsetting > 0xff)
		return LIBUSB_ERROR_INVALID_PARAM;

	error("set alt setting for iface ", interface_number, " to ", altsetting);
	Usb_device::Urb urb(device()._device, P::Request::SET_INTERFACE,
	                    Rt::value(P::Recipient::IFACE, P::Type::STANDARD,
	                              P::Direction::IN),
	                    (uint8_t)interface_number, (uint8_t)altsetting,
	                    0, nullptr, 0);
	device()._wait_for_urb(urb);
	error("alt setting done");
	return LIBUSB_SUCCESS;
}


static int genode_submit_transfer(struct usbi_transfer * itransfer)
{
	struct libusb_transfer *transfer =
		USBI_TRANSFER_TO_LIBUSB_TRANSFER(itransfer);
	Usb::Interface::Packet_descriptor::Type type =
		Usb::Interface::Packet_descriptor::FLUSH;
	size_t size = transfer->length;

	switch (transfer->type) {

		case LIBUSB_TRANSFER_TYPE_CONTROL: {

			struct libusb_control_setup *setup =
				(struct libusb_control_setup*)transfer->buffer;

			void * addr = transfer->buffer + LIBUSB_CONTROL_SETUP_SIZE;
			new (device()._alloc)
				Usb_device::Urb(device()._device, setup->bRequest,
				                setup->bmRequestType, setup->wValue,
				                setup->wIndex, setup->wLength, addr,
				                setup->wLength, itransfer);
			device().handle_events();
			return LIBUSB_SUCCESS;
		}

		case LIBUSB_TRANSFER_TYPE_BULK:
		case LIBUSB_TRANSFER_TYPE_BULK_STREAM:
			type = Usb::Interface::Packet_descriptor::BULK;
			break;
		case LIBUSB_TRANSFER_TYPE_INTERRUPT:
			type = Usb::Interface::Packet_descriptor::IRQ;
			break;
		case LIBUSB_TRANSFER_TYPE_ISOCHRONOUS:
			type  = Usb::Interface::Packet_descriptor::ISOC;
			size += sizeof(genode_usb_isoc_transfer_header) +
			        transfer->num_iso_packets*sizeof(genode_usb_isoc_descriptor);
			break;

		default:
			usbi_err(TRANSFER_CTX(transfer),
				"unknown endpoint type %d", transfer->type);
			return LIBUSB_ERROR_INVALID_PARAM;
	}

	bool found = false;
	device()._interfaces.for_each([&] (Usb_device::Interface &iface) {
		iface.for_each_endpoint([&] (Usb::Endpoint ep) {
			if (found || transfer->endpoint != ep.address())
				return;
			found = true;
			new (device()._alloc)
				Usb_device::Interface::Urb(iface, ep, type, transfer->buffer,
				                           transfer->length, itransfer);

#if 0
			if (transfer->type == LIBUSB_TRANSFER_TYPE_ISOCHRONOUS) {
				genode_usb_isoc_descriptor desc[transfer->num_iso_packets];

			for (int i = 0; i < transfer->num_iso_packets; i++) {
				desc[i].actual_size = 0;
				desc[i].size        = transfer->iso_packet_desc[i].length;
			}
			buffer = device()._alloc.alloc(size);

			((genode_usb_isoc_transfer_header*)(buffer))->number_of_packets =
				transfer->num_iso_packets;

			void * dst = 
				(void*)((size_t)buffer+sizeof(genode_usb_isoc_transfer_header));
			memcpy(dst, &desc, sizeof(desc));
			dst = (void*)((size_t)dst+sizeof(desc));
			memcpy(dst, transfer->buffer,
			       size-sizeof(genode_usb_isoc_transfer_header)-sizeof(desc));
#endif

			iface.handle_events();
		});
	});

	return found ? LIBUSB_SUCCESS : LIBUSB_ERROR_NOT_FOUND;
}


static int genode_cancel_transfer(struct usbi_transfer * itransfer)
{
	return LIBUSB_SUCCESS;
}


static void genode_clear_transfer_priv(struct usbi_transfer * itransfer) { }


static int genode_handle_events(struct libusb_context *, struct pollfd *,
                                POLL_NFDS_TYPE, int)
{
	device().handle_events();
	return LIBUSB_SUCCESS;
}


static int genode_handle_transfer_completion(struct usbi_transfer * itransfer)
{
	enum libusb_transfer_status status = LIBUSB_TRANSFER_COMPLETED;

	if (itransfer->flags & USBI_TRANSFER_CANCELLING)
		status = LIBUSB_TRANSFER_CANCELLED;

	return usbi_handle_transfer_completion(itransfer, status);
}


static int genode_clock_gettime(int clkid, struct timespec *tp)
{
	switch (clkid) {
		case USBI_CLOCK_MONOTONIC:
			return clock_gettime(CLOCK_MONOTONIC, tp);
		case USBI_CLOCK_REALTIME:
			return clock_gettime(CLOCK_REALTIME, tp);
		default:
			return LIBUSB_ERROR_INVALID_PARAM;
	}
}


const struct usbi_os_backend genode_usb_raw_backend = {
	/*.name =*/ "Genode",
	/*.caps =*/ 0,
	/*.init =*/ genode_init,
	/*.exit =*/ genode_exit,
	/*.get_device_list =*/ genode_get_device_list,
	/*.hotplug_poll =*/ NULL,
	/*.open =*/ genode_open,
	/*.close =*/ genode_close,
	/*.get_device_descriptor =*/ genode_get_device_descriptor,
	/*.get_active_config_descriptor =*/ genode_get_active_config_descriptor,
	/*.get_config_descriptor =*/ genode_get_config_descriptor,
	/*.get_config_descriptor_by_value =*/ NULL,


	/*.get_configuration =*/ NULL,
	/*.set_configuration =*/ genode_set_configuration,
	/*.claim_interface =*/ genode_claim_interface,
	/*.release_interface =*/ genode_release_interface,

	/*.set_interface_altsetting =*/ genode_set_interface_altsetting,
	/*.clear_halt =*/ NULL,
	/*.reset_device =*/ NULL,

	/*.alloc_streams =*/ NULL,
	/*.free_streams =*/ NULL,

	/*.kernel_driver_active =*/ NULL,
	/*.detach_kernel_driver =*/ NULL,
	/*.attach_kernel_driver =*/ NULL,

	/*.destroy_device =*/ NULL,

	/*.submit_transfer =*/ genode_submit_transfer,
	/*.cancel_transfer =*/ genode_cancel_transfer,
	/*.clear_transfer_priv =*/ genode_clear_transfer_priv,

	/*.handle_events =*/ genode_handle_events,
	/*.handle_transfer_completion =*/ genode_handle_transfer_completion,

	/*.clock_gettime =*/ genode_clock_gettime,

#ifdef USBI_TIMERFD_AVAILABLE
	/*.get_timerfd_clockid =*/ NULL,
#endif

	/*.device_priv_size =*/ 0,
	/*.device_handle_priv_size =*/ 0,
	/*.transfer_priv_size =*/ 0,
};
