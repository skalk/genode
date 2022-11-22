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

#include <blit/blit.h>
#include <virtio_gpu.h>


void Vmm::Virtio_gpu_queue::notify(Virtio_gpu_device & dev)
{
	memory_barrier();
	bool inform = false;
	for (Ring_index avail_idx = _avail.current();
	     _cur_idx != avail_idx; _cur_idx.inc()) {
		try {
			Index idx = _avail.get(_cur_idx);
			Virtio_gpu_control_request
				request(idx, _descriptors, _ram, dev);
			_used.add(_cur_idx.idx(), idx, request.size());
		} catch (Exception & e) {
			error(e);
		}
		inform = true;
	}

	if (!inform)
		return;

	_used.write<Used_queue::Idx>(_cur_idx.idx());
	memory_barrier();
	if (_avail.inject_irq()) dev.assert_irq();
}


void Vmm::Virtio_gpu_control_request::_get_display_info()
{
	if (_data.length() < Display_info_response::SIZE)
		throw Exception("Invalid request, request size mismatch ", _request.length());

	Display_info_response dir { _desc_addr(_data) };
	memset((void*)dir.base(), 0, Display_info_response::SIZE);
	dir.write<Control_header::Type>(Control_header::Type::OK_DISPLAY_INFO);

	dir.write<Display_info_response::X>(0);
	dir.write<Display_info_response::Y>(0);
	dir.write<Display_info_response::Width>(_device._fb_mode.area.w());
	dir.write<Display_info_response::Height>(_device._fb_mode.area.h());
	dir.write<Display_info_response::Enabled>(1);
	dir.write<Display_info_response::Flags>(0);
}


void Vmm::Virtio_gpu_control_request::_resource_create_2d()
{
	if (_request.length() < Resource_create_2d::SIZE)
		throw Exception("Invalid request, request size mismatch ", _request.length());

	Resource_create_2d c2d { _desc_addr(_request) };
	Control_header response { _desc_addr(_data) };

	if (c2d.read<Resource_create_2d::Format>() !=
	    Resource_create_2d::Format::B8G8R8X8) {
		warning("Unsupported pixel fomat (id=",
		        c2d.read<Resource_create_2d::Format>(), ")!");
		response.write<Control_header::Type>(Control_header::Type::ERR_INVALID_PARAMETER);
		return;
	}

	using Resource = Virtio_gpu_device::Resource;

	try {
		new (_device._heap)
			Resource(_device._resources, _device._env,
			         c2d.read<Resource_create_2d::Resource_id>(),
			         c2d.read<Resource_create_2d::Width>(),
			         c2d.read<Resource_create_2d::Height>());
		response.write<Control_header::Type>(Control_header::Type::OK_NO_DATA);
	} catch(...) {
		response.write<Control_header::Type>(Control_header::Type::ERR_OUT_OF_MEMORY);
	}
}


void Vmm::Virtio_gpu_control_request::_resource_attach_backing()
{
	if (_request.length() < Resource_attach_backing::SIZE)
		throw Exception("Invalid request, request size mismatch ", _request.length());

	using Resource = Virtio_gpu_device::Resource;
	using Entry    = Resource_attach_backing::Memory_entry;

	Resource_attach_backing rab { _desc_addr(_request) };

	Index          r_idx    { _next(_data)       };
	Descriptor     r_desc   { _array.get(r_idx)  };
	Control_header response { _desc_addr(r_desc) };

	response.write<Control_header::Type>(Control_header::Type::ERR_INVALID_RESOURCE_ID);
	uint32_t id = rab.read<Resource_attach_backing::Resource_id>();
	unsigned nr = rab.read<Resource_attach_backing::Nr_entries>();

	if (_data.length() < nr*Entry::SIZE)
		throw Exception("Invalid request, request size mismatch ", _data.length());

	_device._resources.for_each([&] (Resource & res) {
		if (res.id != id)
			return;

		Resource::Backing * last = res.backings.first();

		if (last) {
			error("Cannot attach multiple backings to one resource!");
			response.write<Control_header::Type>(Control_header::Type::ERR_UNSPEC);
			return;
		}

		try {
			for (unsigned i = 0; i < nr; i++) {
				Entry entry(_desc_addr(_data)+i*Entry::SIZE);
				size_t sz  = entry.read<Entry::Length>();
				addr_t src = _device._ram.local_address(entry.read<Entry::Address>(), sz);
				Resource::Backing * ba = new (_device._heap) Resource::Backing(src, sz);
				res.backings.insert(ba, last);
				last = ba;
			}
			response.write<Control_header::Type>(Control_header::Type::OK_NO_DATA);
		} catch (Exception &) {
			response.write<Control_header::Type>(Control_header::Type::ERR_INVALID_PARAMETER);
		} catch (...) {
			response.write<Control_header::Type>(Control_header::Type::ERR_OUT_OF_MEMORY);
		}
	});
}


void Vmm::Virtio_gpu_control_request::_set_scanout()
{
	if (_request.length() < Set_scanout::SIZE)
		throw Exception("Invalid request, request size mismatch ", _request.length());

	Set_scanout scr { _desc_addr(_request) };
	Control_header response { _desc_addr(_data) };

	uint32_t id  = scr.read<Set_scanout::Resource_id>();
	uint32_t sid = scr.read<Set_scanout::Scanout_id>();
	response.write<Control_header::Type>(id ? Control_header::Type::ERR_INVALID_RESOURCE_ID
	                                        : Control_header::Type::OK_NO_DATA);

	using Resource = Virtio_gpu_device::Resource;
	using Scanout  = Resource::Scanout;
	_device._resources.for_each([&] (Resource & res) {
		if (!id || id == res.id)
			res.scanouts.for_each([&] (Scanout & sc) {
				if (sc.id == sid) destroy(_device._heap, &sc); });

		if (res.id != id)
			return;

		try {
			new (_device._heap) Scanout(res.scanouts, sid,
			                            scr.read<Set_scanout::X>(),
			                            scr.read<Set_scanout::Y>(),
			                            scr.read<Set_scanout::Width>(),
			                            scr.read<Set_scanout::Height>());
			response.write<Control_header::Type>(Control_header::Type::OK_NO_DATA);
		} catch(...) {
			response.write<Control_header::Type>(Control_header::Type::ERR_OUT_OF_MEMORY);
		}
	});
}


void Vmm::Virtio_gpu_control_request::_resource_flush()
{
	if (_request.length() < Resource_flush::SIZE)
		throw Exception("Invalid request, request size mismatch ", _request.length());

	Resource_flush rf { _desc_addr(_request) };
	Control_header response { _desc_addr(_data) };

	uint32_t id  = rf.read<Resource_flush::Resource_id>();
	response.write<Control_header::Type>(Control_header::Type::ERR_INVALID_RESOURCE_ID);

	using Resource = Virtio_gpu_device::Resource;
	_device._resources.for_each([&] (Resource & res) {
		if (res.id != id)
			return;

		uint32_t x = rf.read<Resource_flush::X>();
		uint32_t y = rf.read<Resource_flush::Y>();
		uint32_t w = rf.read<Resource_flush::Width>();
		uint32_t h = rf.read<Resource_flush::Height>();

		if (x > res.area.w()     ||
		    y > res.area.h()     ||
		    w > res.area.w()     ||
		    h > res.area.h()     ||
		    x + w > res.area.w() ||
		    y + h > res.area.h()) {
			response.write<Control_header::Type>(Control_header::Type::ERR_INVALID_PARAMETER);
			return;
		}

		enum { BYTES_PER_PIXEL = Virtio_gpu_device::BYTES_PER_PIXEL };
		response.write<Control_header::Type>(Control_header::Type::OK_NO_DATA);

		addr_t src = (addr_t)res.buffer.local_addr<void>() +
		             (res.area.w() * y + x) * BYTES_PER_PIXEL;
		addr_t dst = (addr_t)_device._fb_ds->local_addr<void>() +
		             (_device._fb_mode.area.w() * y + x) * BYTES_PER_PIXEL;

		for (unsigned i = 0; i < h; i++) {
			memcpy((void*)dst, (void*)src, w*BYTES_PER_PIXEL);
			src += res.area.w() * BYTES_PER_PIXEL;
			dst += _device._fb_mode.area.w() * BYTES_PER_PIXEL;
		}

		_device._gui.framebuffer()->refresh(x, y, w, h);
	});
}


void Vmm::Virtio_gpu_control_request::_transfer_to_host_2d()
{
	if (_request.length() < Transfer_to_host_2d::SIZE)
		throw Exception("Invalid request, request size mismatch ", _request.length());

	Transfer_to_host_2d tth      { _desc_addr(_request) };
	Control_header      response { _desc_addr(_data)    };

	uint32_t id = tth.read<Transfer_to_host_2d::Resource_id>();
	response.write<Control_header::Type>(Control_header::Type::ERR_INVALID_RESOURCE_ID);

	enum { BYTES_PER_PIXEL = Virtio_gpu_device::BYTES_PER_PIXEL };

	using Resource = Virtio_gpu_device::Resource;
	_device._resources.for_each([&] (Resource & res)
	{
		if (res.id != id)
			return;

		uint32_t x = tth.read<Transfer_to_host_2d::X>();
		uint32_t y = tth.read<Transfer_to_host_2d::Y>();
		uint32_t w = tth.read<Transfer_to_host_2d::Width>();
		uint32_t h = tth.read<Transfer_to_host_2d::Height>();
		addr_t off = tth.read<Transfer_to_host_2d::Offset>();

		if (x > res.area.w()     ||
		    y > res.area.h()     ||
		    w > res.area.w()     ||
		    h > res.area.h()     ||
		    x + w > res.area.w() ||
		    y + h > res.area.h()) {
			response.write<Control_header::Type>(Control_header::Type::ERR_INVALID_PARAMETER);
			return;
		}

		addr_t dst    = (addr_t)res.buffer.local_addr<void>() +
		                (y * res.area.w() + x) * BYTES_PER_PIXEL;
		size_t line   = res.area.w() * BYTES_PER_PIXEL;
		size_t size   = h * line;
		size_t copy   = w * BYTES_PER_PIXEL;
		size_t copied = 0;

		res.for_each_backing(off, size, [&] (addr_t ram_addr, size_t ram_size)
		{
			while (ram_size && (copied < size)) {
				size_t bytes = min(copy - (copied%line), ram_size);
				memcpy((void*)dst, (void*)ram_addr, bytes);
				if (((copied+bytes) % line) >= copy)
					bytes = line;
				copied   += bytes;
				dst      += bytes;
				ram_addr += bytes;
				if (ram_size <= bytes)
					break;
				ram_size -= bytes;
			}
		});

		response.write<Control_header::Type>(Control_header::Type::OK_NO_DATA);
	});
}
