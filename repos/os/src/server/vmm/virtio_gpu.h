/*
 * \brief  Virtio GPU implementation
 * \author Stefan Kalkowski
 * \date   2021-02-19
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _VIRTIO_GPU_H_
#define _VIRTIO_GPU_H_

#include <gui_session/connection.h>
#include <virtio_device.h>

namespace Vmm {
	class Virtio_gpu_queue;
	class Virtio_gpu_control_request;
	class Virtio_gpu_device;
	using namespace Genode;
}


class Vmm::Virtio_gpu_queue : public Virtio_split_queue
{
	private:

		Ring_index _used_idx;

		friend class Virtio_gpu_control_request;

	public:

		enum Type { CONTROL, CURSOR };

		using Virtio_split_queue::Virtio_split_queue;

		bool notify(Virtio_gpu_device &, Type t);

		void ack(Descriptor_index id, size_t written)
		{
			_used.add(_used_idx, id, written);
			_used_idx.inc();
			_used.write<Used_queue::Idx>(_used_idx.idx());
			memory_barrier();
		}
};


class Vmm::Virtio_gpu_control_request
{
	public:

		struct Invalid_request : Genode::Exception {};

	private:

		using Index            = Virtio_gpu_queue::Descriptor_index;
		using Descriptor       = Virtio_gpu_queue::Descriptor;
		using Descriptor_array = Virtio_gpu_queue::Descriptor_array;

		struct Control_header : Mmio
		{
			enum { SIZE = 24 };

			struct Type : Register<0,  32>
			{
				enum {
					GET_DISPLAY_INFO = 0x0100,
				};
			};
			struct Flags    : Register<4,  32> {};
			struct Fence_id : Register<8,  64> {};
			struct Ctx_id   : Register<16, 32> {};
			struct Padding  : Register<20, 32> {};

			using Mmio::Mmio;
		};

		enum Status { OK, IO_ERROR, UNSUPPORTED };

		Index _next(Descriptor & desc)
		{
			if (!Descriptor::Flags::Next::get(desc.flags())) {
				throw Invalid_request(); }
			return desc.next();
		}

		Descriptor_array & _array;
		Ram              & _ram;

		addr_t _desc_addr(Descriptor const & desc) const {
			return _ram.local_address(desc.address(), desc.length()); }

		Index              _request_idx;
		Descriptor         _request    { _array.get(_request_idx) };
		Control_header     _ctrl_hdr   { _desc_addr(_request)     };
		Index              _data_idx   { _next(_request)          };
		Descriptor         _data       { _array.get(_data_idx)    };
		Index              _status_idx { _next(_data)             };
		Descriptor         _status     { _array.get(_status_idx)  };
		size_t             _written    { 0 };

		void _done(Virtio_gpu_queue & queue)
		{
			*((uint8_t*)_desc_addr(_status)) = OK;
			queue.ack(_request_idx, _written);
		}

	public:

		Virtio_gpu_control_request(Index              id,
		                           Descriptor_array & array,
		                           Ram              & ram,
		                           Virtio_gpu_device &)
		: _array(array), _ram(ram), _request_idx(id)
		{
			if (_request.length() != Control_header::SIZE ||
			    _status.length()  != sizeof(uint8_t)) {
				throw Invalid_request(); }
		}
};


class Vmm::Virtio_gpu_device : public Virtio_device<Virtio_gpu_queue, 2>
{
	private:

		Env                                  & _env;
		Gui::Connection                        _gui { _env };
		Cpu::Signal_handler<Virtio_gpu_device> _handler;
		Constructible<Attached_dataspace>      _fb_ds { };
		Framebuffer::Mode                      _fb_mode { _gui.mode() };
		Gui::Session::View_handle              _view = _gui.create_view();
		bool                                   _mode_changed { true };

		struct Configuration_area : Mmio_register
		{
			Virtio_gpu_device & dev;

			enum {
				EVENTS_READ  = 0,
				EVENTS_CLEAR = 4,
				SCANOUTS     = 8 };

			Register read(Address_range & range,  Cpu&) override
			{
				if (range.start == EVENTS_READ && range.size == 4)
					return dev._mode_changed ? 1 : 0;

				/* we support no multi-head, just return 1 */
				if (range.start == SCANOUTS && range.size == 4)
					return 1;

				return 0;
			}

			void write(Address_range & range,  Cpu&, Register v) override
			{
				if (range.start == EVENTS_CLEAR && range.size == 4 && v == 1)
					dev._mode_changed = false;
			}

			Configuration_area(Virtio_gpu_device & device)
			: Mmio_register("GPU config area", Mmio_register::RO, 0x100, 16),
			  dev(device) { device.add(*this); }
		} _config_area{ *this };

		void _mode_change()
		{
			Genode::Mutex::Guard guard(_mutex);

			_fb_mode = _gui.mode();

			_gui.buffer(_fb_mode, false);

			if (_fb_mode.area.count() > 0)
				_fb_ds.construct(_env.rm(),
				                 _gui.framebuffer()->dataspace());

			using Command = Gui::Session::Command;
			_gui.enqueue<Command::Geometry>(_view, Rect(Point(0, 0), _fb_mode.area));
			_gui.enqueue<Command::To_front>(_view, Gui::Session::View_handle());
			_gui.execute();

			_mode_changed = true;
		}

		void _notify(unsigned idx) override
		{
			Virtio_gpu_queue::Type t;
			switch (idx) {
			case Virtio_gpu_queue::CONTROL: t = Virtio_gpu_queue::CONTROL;
			case Virtio_gpu_queue::CURSOR:  t = Virtio_gpu_queue::CURSOR;
			default:
				return;
			};
			_queue[idx]->notify(*this, t);
		}

		enum Device_id { GPU = 16 };

	public:

		Virtio_gpu_device(const char * const name,
		                  const uint64_t     addr,
		                  const uint64_t     size,
		                  unsigned           irq,
		                  Cpu              & cpu,
		                  Mmio_bus         & bus,
		                  Ram              & ram,
		                  Genode::Env      & env)
		:
			Virtio_device<Virtio_gpu_queue, 2>(name, addr, size,
			                                   irq, cpu, bus, ram, GPU),
			_env(env),
			_handler(cpu, env.ep(), *this, &Virtio_gpu_device::_mode_change)
		{
			_gui.mode_sigh(_handler);
			_mode_change();
		}
};


bool Vmm::Virtio_gpu_queue::notify(Virtio_gpu_device & dev, Type t)
{
	memory_barrier();
	for (Ring_index avail_idx = _avail.current();
	     _cur_idx != avail_idx; _cur_idx.inc())
		if (t == CONTROL)
			Virtio_gpu_control_request request(_avail.get(_cur_idx), _descriptors, _ram, dev);
	return false;
}

#endif /* _VIRTIO_GPU_H_ */
