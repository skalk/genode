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

#include <base/attached_ram_dataspace.h>
#include <base/registry.h>

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

		enum { CONTROL, CURSOR, QUEUE_COUNT };

		using Virtio_split_queue::Virtio_split_queue;

		void notify(Virtio_gpu_device &);
};


class Vmm::Virtio_gpu_control_request
{
	private:

		using Index            = Virtio_gpu_queue::Descriptor_index;
		using Descriptor       = Virtio_gpu_queue::Descriptor;
		using Descriptor_array = Virtio_gpu_queue::Descriptor_array;

		struct Control_header : Mmio
		{
			enum { SIZE = 24 };

			struct Type : Register<0,  32>
			{
				enum Commands {
					GET_DISPLAY_INFO = 0x0100,
					RESOURCE_CREATE_2D,
					RESOURCE_UNREF,
					SET_SCANOUT,
					RESOURCE_FLUSH,
					TRANSFER_TO_HOST_2D,
					RESOURCE_ATTACH_BACKING,
					RESOURCE_DETACH_BACKING,
					GET_CAPSET_INFO,
					GET_CAPSET,
					GET_EDID,
				};

				enum Responses {
					OK_NO_DATA = 0x1100,
					OK_DISPLAY_INFO,
					OK_CAPSET_INFO,
					OK_CAPSET,
					OK_EDID,
					ERR_UNSPEC = 0x1200,
					ERR_OUT_OF_MEMORY,
					ERR_INVALID_SCANOUT_ID,
					ERR_INVALID_RESOURCE_ID,
					ERR_INVALID_CONTEXT_ID,
					ERR_INVALID_PARAMETER,
				};
			};
			struct Flags    : Register<0x4,  32> {};
			struct Fence_id : Register<0x8,  64> {};
			struct Ctx_id   : Register<0x10, 32> {};

			using Mmio::Mmio;
		};

		struct Display_info_response : Control_header
		{
			enum { SIZE = Control_header::SIZE + 24*16 };

			struct X       : Register<0x18, 32> {};
			struct Y       : Register<0x1c, 32> {};
			struct Width   : Register<0x20, 32> {};
			struct Height  : Register<0x24, 32> {};
			struct Enabled : Register<0x28, 32> {};
			struct Flags   : Register<0x2c, 32> {};

			using Control_header::Control_header;
		};

		struct Resource_create_2d : Control_header
		{
			enum { SIZE = Control_header::SIZE + 16 };

			struct Resource_id : Register<0x18, 32> {};

			struct Format : Register<0x1c, 32>
			{
				enum {
					B8G8R8A8 = 1,
					B8G8R8X8 = 2,
					A8R8G8B8 = 3,
					X8R8G8B8 = 4,
					R8G8B8A8 = 67,
					X8B8G8R8 = 68,
					A8B8G8R8 = 121,
					R8G8B8X8 = 134,
				};
			};

			struct Width       : Register<0x20, 32> {};
			struct Height      : Register<0x24, 32> {};

			using Control_header::Control_header;
		};

		struct Resource_attach_backing : Control_header
		{
			enum { SIZE = Control_header::SIZE + 8 };

			struct Resource_id : Register<0x18, 32> {};
			struct Nr_entries  : Register<0x1c, 32> {};

			struct Memory_entry : Mmio
			{
				enum { SIZE = 16 };

				struct Address : Register<0x0, 64> {};
				struct Length  : Register<0x8, 32> {};

				using Mmio::Mmio;
			};

			using Control_header::Control_header;
		};

		struct Set_scanout : Control_header
		{
			enum { SIZE = Control_header::SIZE + 24 };

			struct X           : Register<0x18, 32> {};
			struct Y           : Register<0x1c, 32> {};
			struct Width       : Register<0x20, 32> {};
			struct Height      : Register<0x24, 32> {};
			struct Scanout_id  : Register<0x28, 32> {};
			struct Resource_id : Register<0x2c, 32> {};

			using Control_header::Control_header;
		};

		struct Resource_flush : Control_header
		{
			enum { SIZE = Control_header::SIZE + 24 };

			struct X           : Register<0x18, 32> {};
			struct Y           : Register<0x1c, 32> {};
			struct Width       : Register<0x20, 32> {};
			struct Height      : Register<0x24, 32> {};
			struct Resource_id : Register<0x28, 32> {};

			using Control_header::Control_header;
		};

		struct Transfer_to_host_2d :Control_header
		{
			enum { SIZE = Control_header::SIZE + 32 };

			struct X           : Register<0x18, 32> {};
			struct Y           : Register<0x1c, 32> {};
			struct Width       : Register<0x20, 32> {};
			struct Height      : Register<0x24, 32> {};
			struct Offset      : Register<0x28, 64> {};
			struct Resource_id : Register<0x30, 32> {};

			using Control_header::Control_header;
		};

		Index _next(Descriptor & desc)
		{
			if (!Descriptor::Flags::Next::get(desc.flags()))
				throw Exception("Invalid request, no next descriptor");
			return desc.next();
		}

		Descriptor_array  & _array;
		Ram               & _ram;
		Virtio_gpu_device & _device;

		addr_t _desc_addr(Descriptor const & desc) const {
			return _ram.local_address(desc.address(), desc.length()); }

		Index               _request_idx;
		Descriptor          _request    { _array.get(_request_idx) };
		Control_header      _ctrl_hdr   { _desc_addr(_request)     };
		Index               _data_idx   { _next(_request)          };
		Descriptor          _data       { _array.get(_data_idx)    };

		void _get_display_info();
		void _resource_create_2d();
		void _resource_attach_backing();
		void _set_scanout();
		void _resource_flush();
		void _transfer_to_host_2d();

	public:

		Virtio_gpu_control_request(Index               id,
		                           Descriptor_array  & array,
		                           Ram               & ram,
		                           Virtio_gpu_device & device)
		: _array(array), _ram(ram), _device(device), _request_idx(id)
		{
			if (_request.length() < Control_header::SIZE)
				throw Exception("Invalid request, control header size mismatch ",
				                _request.length());

			switch (_ctrl_hdr.read<Control_header::Type>()) {
			case Control_header::Type::GET_DISPLAY_INFO:
				_get_display_info();
				break;
			case Control_header::Type::RESOURCE_CREATE_2D:
				_resource_create_2d();
				break;
			case Control_header::Type::RESOURCE_ATTACH_BACKING:
				_resource_attach_backing();
				break;
			case Control_header::Type::SET_SCANOUT:
				_set_scanout();
				break;
			case Control_header::Type::RESOURCE_FLUSH:
				_resource_flush();
				break;
			case Control_header::Type::TRANSFER_TO_HOST_2D:
				_transfer_to_host_2d();
				break;
			default:
				error("Unknown control request ",
				      _ctrl_hdr.read<Control_header::Type>());
			};
		}

		size_t size() { return Control_header::SIZE; }
};


class Vmm::Virtio_gpu_device : public Virtio_device<Virtio_gpu_queue, 2>
{
	private:

		friend class Virtio_gpu_control_request;

		Env                                  & _env;
		Heap                                 & _heap;
		Gui::Connection                        _gui { _env };
		Cpu::Signal_handler<Virtio_gpu_device> _handler;
		Constructible<Attached_dataspace>      _fb_ds { };
		Framebuffer::Mode                      _fb_mode { _gui.mode() };
		Gui::Session::View_handle              _view = _gui.create_view();
		bool                                   _mode_changed { true };

		using Area = Genode::Area<>;
		using Rect = Genode::Rect<>;

		enum { BYTES_PER_PIXEL = 4 };

		struct Resource : Registry<Resource>::Element
		{
			struct Backing : List<Backing>::Element
			{
				addr_t addr;
				size_t size;

				Backing(addr_t addr, size_t size) : addr(addr), size(size) { }
			};

			struct Scanout : Registry<Scanout>::Element, Rect
			{
				uint32_t id;

				Scanout(Registry<Scanout> & registry,
				        uint32_t id,
				        uint32_t x, uint32_t y,
				        uint32_t w, uint32_t h)
				:
					Registry<Scanout>::Element(registry, *this),
					Rect(Point((int)x,(int)y), Area((int)w,(int)h)) { }

				using Rect::Rect;
			};

			uint32_t               id;
			Area                   area;
			Attached_ram_dataspace buffer;
			List<Backing>          backings;
			Registry<Scanout>      scanouts;

			Resource(Registry<Resource> & registry,
			         Env                & env,
			         uint32_t             id,
			         uint32_t             w,
			         uint32_t             h)
			:
				Registry<Resource>::Element(registry, *this),
				id(id),
				area((int)w, (int)h),
				buffer(env.ram(), env.rm(), w*h*BYTES_PER_PIXEL) { }

			template <typename FN>
			void for_each_backing(addr_t offset, size_t size, FN const & fn)
			{
				Backing * b = backings.first();
				while (size) {
					if (!b)
						return;

					if (b->size <= offset) {
						offset -= b->size;
						b       = b->next();
						continue;
					}

					size_t buf_sz = min(size, b->size-offset);
					fn(b->addr + offset, buf_sz);
					size  -= buf_sz;
					offset = 0;
					b      = b->next();
				}
			}
		};

		Registry<Resource> _resources {};

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
			if (idx < Virtio_gpu_queue::QUEUE_COUNT)
				_queue[idx]->notify(*this);
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
		                  Env              & env,
		                  Heap             & heap)
		:
			Virtio_device<Virtio_gpu_queue, 2>(name, addr, size,
			                                   irq, cpu, bus, ram, GPU),
			_env(env), _heap(heap),
			_handler(cpu, env.ep(), *this, &Virtio_gpu_device::_mode_change)
		{
			_gui.mode_sigh(_handler);
			_mode_change();
		}

		void assert_irq() { _assert_irq(); }
};

#endif /* _VIRTIO_GPU_H_ */
