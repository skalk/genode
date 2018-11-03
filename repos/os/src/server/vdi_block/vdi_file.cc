/*
 * \brief  VDI file as a Block session
 * \author Alexander Boettcher
 * \date   2020-03-19
 */

/*
 * Copyright (C) 2020 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* local includes */
#include "vdi_file.h"

Vdi::File::Response
Vdi::File::_read_split(::Block::Operation const operation,
                       void * dst, Genode::size_t const dst_size,
                       uint64_t const file_offset,
                       uint32_t const max_offset_read)
{
	if (_state_fs_read.state == Read::NONE) {
		_state_fs_read.bytes_read = 0;
		_state_fs_read.offset     = file_offset;
		if (max_offset_read < dst_size) {
			_state_fs_read.remaining = max_offset_read;
			_state_fs_read.operation = operation;
		} else {
			_state_fs_read.remaining = dst_size;
			_state_fs_read.operation.type = ::Block::Operation::Type::INVALID;
		}

		_state_fs_read.state = Read::READ;
	}

	Genode::uint32_t const read_before = _state_fs_read.bytes_read;

	do {
		_read(reinterpret_cast<char *>(dst), _state_fs_read.remaining);
	} while (_state_fs_read.state == Read::LOOP_READ);

	if (_state_fs_read.state != Read::NONE)
		return Response::RETRY;

	/* sanity check, READ::NONE should be same as remaining == 0 - XXX remove me */
	if (_state_fs_read.remaining) {
		Genode::error("invalid read state - unexpected remaining state");
		return Response::REJECTED;
	}

	if (_state_fs_read.operation.type == ::Block::Operation::Type::INVALID) {
		_state_fs_read.dst_offset = 0;
		return Response::ACCEPTED;
	}

	if (read_before > _state_fs_read.bytes_read) {
		Genode::error("invalid read state");
		return Response::REJECTED;
	}

	uint32_t const blocks = _state_fs_read.bytes_read / _block_ops.block_size;
	if (_state_fs_read.bytes_read % _block_ops.block_size) {
		Genode::error("invalid read state - bytes read");
		return Response::REJECTED;
	}

	if (_state_fs_read.operation.count <= blocks) {
		Genode::error("count of blocks is too small ",
		              _state_fs_read.operation.count, " vs ", blocks,
		              " read_before=", read_before);
		return Response::REJECTED;
	}

	_state_fs_read.operation.block_number += blocks;
	_state_fs_read.operation.count        -= blocks;
	_state_fs_read.dst_offset             += blocks * _block_ops.block_size;

	return Response::ACCEPTED;
}

void Vdi::File::_execute_alloc_block()
{
	uint32_t const max_bid = _md->max_blocks;
	uint32_t const bid = sector_to_block(_state_fs.block_nr);
	if (bid >= max_bid) {
		Genode::error(__func__, " bid too large", bid, "/", max_bid);
		_state_fs.state = Write::ALLOC_BLOCK_ERROR;
		return;
	}

	if (_state_fs.state == ALLOC_BLOCK) {

		uint64_t const offset = _md->data_offset +
		                       (_md->allocated_blocks * _md->block_size);

		try {
			do {
				Vfs::file_size written = 0;

				_vdi_file->seek(offset + _state_fs.written);
				Write_result res = _vdi_file->fs().write(_vdi_file,
				                                         _zero_addr,
				                                         Genode::min(_md->block_size - _state_fs.written, _zero_size),
				                                         written);
				if (res != Vfs::File_io_service::WRITE_OK) {
					_state_fs.state = Write::ALLOC_BLOCK_ERROR;
					Genode::error(__func__, " state: ", written, " ",
					              _zero_size, " ", (int)res);
					return;
				}

				_state_fs.written += written;
			} while (_state_fs.written < _md->block_size);
		} catch (Vfs::File_io_service::Insufficient_buffer) {
			/* will be resumed later, keep state */
			return;
		}

		_md->table[bid].value = _md->allocated_blocks;
		_md->allocated_blocks++;
		_state_fs.state = Write::SYNC_HEADER;
	}

	if (_state_fs.state == Write::SYNC_HEADER   ||
	    _state_fs.state == Write::SYNC_HEADER_1 ||
	    _state_fs.state == Write::SYNC_HEADER_2)
	{
		_sync_header(bid);

		if (_state_fs.state != Write::SYNC_OK)
			/* will be resumed later, keep state */
			return;

		_state_fs.state = Write::ALLOC_BLOCK_SYNC;
	}

	if (_state_fs.state == Write::ALLOC_BLOCK_SYNC) {
		if (!_vdi_file->fs().queue_sync(_vdi_file))
			return;
		_state_fs.state = Write::ALLOC_BLOCK_SYNC_QUEUED;
	}

	if (_state_fs.state == Write::ALLOC_BLOCK_SYNC_QUEUED) {
		Sync_result res = _complete_sync_fs();
		switch (res) {
		case Vfs::File_io_service::SYNC_QUEUED:
			_state_fs.state = Write::ALLOC_BLOCK_SYNC_QUEUED;
			return;
		case Vfs::File_io_service::SYNC_ERR_INVALID:
			_state_fs.state = Write::ERROR;
			return;
		case Vfs::File_io_service::SYNC_OK:
			_state_fs.state = Write::IDLE;
		}
	}
}

bool Vdi::File::_cross_vdi_block(::Block::Operation const operation) const
{
#if 0
	Genode::String<16> const op_string((operation.type == ::Block::Operation::Type::READ) ? "read" :
	                                   (operation.type == ::Block::Operation::Type::WRITE) ? "write" :
	                                   (operation.type == ::Block::Operation::Type::SYNC) ? "sync" :
	                                   (operation.type == ::Block::Operation::Type::TRIM) ? "trim" :
	                                   (operation.type == ::Block::Operation::Type::INVALID) ? "invalid" : "unknown");
#endif

	uint64_t const range_size  = HeaderV1Plus::BLOCK_SIZE;
	uint64_t const range_start = operation.block_number * _block_ops.block_size;
	uint64_t const range_end   = (operation.block_number + operation.count) * _block_ops.block_size;

#if 0
	if ((operation.type != ::Block::Operation::Type::READ) &&
	    (operation.type != ::Block::Operation::Type::WRITE))
		Genode::log(op_string," ", operation.block_number, "+", operation.count);
#endif

	bool cross_vdi_block = (range_start / range_size) != ((range_end - 1) / range_size);

#if 0
	if (cross_vdi_block && operation.type == ::Block::Operation::Type::READ) {
		Genode::log(op_string," ", operation.block_number, "+", operation.count,
		            " crossing 1MB range ", Genode::Hex(range_start), "-", Genode::Hex(range_end),
		            (_state_fs_read.state == Read::NONE) ? " none" :
		            (_state_fs_read.state == Read::CHECK) ? " check" :
		            (_state_fs_read.state == Read::READ) ? " read" : " unknown");
	}
	if (cross_vdi_block && operation.type == ::Block::Operation::Type::WRITE) {
		Genode::log(op_string," ", operation.block_number, "+", operation.count,
		            " crossing 1MB range ", Genode::Hex(range_start), "-", Genode::Hex(range_end),
		            (_state_fs.state == Write::ERROR) ? " error" :
		            (_state_fs.state == Write::IDLE) ? " idle" :
		            (_state_fs.state == Write::ALLOC_BLOCK) ? " alloc_block" :
		            (_state_fs.state == Write::ALLOC_BLOCK_ERROR) ? " alloc_block_error" :
		            (_state_fs.state == Write::ALLOC_BLOCK_SYNC) ? " alloc_block_sync" :
		            (_state_fs.state == Write::ALLOC_BLOCK_SYNC_QUEUED) ? " alloc_block_sync_queued" :
		            (_state_fs.state == Write::SYNC_HEADER) ? " sync_header" :
		            (_state_fs.state == Write::SYNC_HEADER_1) ? " sync_header_1" :
		            (_state_fs.state == Write::SYNC_HEADER_2) ? " sync_header_2" :
		            (_state_fs.state == Write::SYNC_OK) ? " sync_ok" :
		            (_state_fs.state == Write::WRITE) ? " write" : " unknown");
	}
#endif

	return cross_vdi_block;
}

Vdi::File::Response
Vdi::File::handle(::Block::Request const & request,
                  ::Block::Request_stream::Payload const &payload)
{
	Response response {Response::REJECTED};

	if (_state_fs.state == Write::ERROR)
		return response;

	switch (request.operation.type) {
	case ::Block::Operation::Type::READ:
		payload.with_content(request, [&] (void *addr, Genode::size_t const dst_size) {
			char * dst = reinterpret_cast<char *>(addr);
			Genode::size_t dst_offset = 0;

			::Block::Operation operation = request.operation;

			bool loop = false;

			do {

				if (_state_fs_read.dst_offset)
				{
					operation  = _state_fs_read.operation;
					dst_offset = _state_fs_read.dst_offset;

#if 0
					Genode::log("read ", request.operation.block_number, "+", request.operation.count,
					            " -> ", operation.block_number, "+", operation.count,
					            " - dst_offset=", dst_offset);
#endif
				}

#if 0
				/* debug */
				_cross_vdi_block(operation);
#endif

				Vfs::file_size const len = operation.count * _block_ops.block_size;

				loop = _apply_block(operation.block_number, [&](Genode::uint32_t const max_offset_read) {
					if (dst_size - dst_offset != len) {
						Genode::warning("read ", dst_size, "-", dst_offset, "=", dst_size - dst_offset, " !=", len);
						return false;
					}

					if (dst_size < dst_offset) {
						Genode::error("read dst_size < dst_offset - ", dst_size, "<", dst_offset);
						return false;
					}

					uint32_t const memset_size = Genode::min(dst_size - dst_offset, max_offset_read);

					Genode::memset(dst + dst_offset, 0, memset_size);

					if (memset_size == dst_size - dst_offset) {
						response = Response::ACCEPTED;
						_state_fs_read.dst_offset = 0;
						return false;
					}

					_state_fs_read.dst_offset += memset_size;

					uint32_t const blocks = memset_size / _block_ops.block_size;

					if (operation.count < blocks) {
						Genode::error("read - count of blocks is too small ");
						return false;
					}

					_state_fs_read.operation = operation;
					_state_fs_read.operation.block_number += blocks;
					_state_fs_read.operation.count        -= blocks;

					return true; /* loop retry */
				}, [&](uint64_t const offset, uint32_t const max_offset_read) {

					if (dst_offset > dst_size || dst_size - dst_offset != len) {
						Genode::error("partial reads, error ahead");
						response = Response::REJECTED;
						return false;
					}

					response = _read_split(operation, dst + dst_offset,
					                       dst_size - dst_offset,
					                       offset, max_offset_read);

					if (response == Response::ACCEPTED &&
					    _state_fs_read.operation.type != ::Block::Operation::Type::INVALID)
						return true; /* loop retry */

					return false;
				});
			} while (loop);
		});
		break;
	case ::Block::Operation::Type::WRITE: {
		if (_state_fs.state != Write::WRITE) {
			if (_state_fs.state != Write::IDLE)
				_execute_alloc_block();

			if (_state_fs.state != Write::IDLE)
				return Response::RETRY;
		}

		Genode::size_t dst_offset = 0;

		::Block::Operation operation = request.operation;

		bool loop = false;

		do {
			if (_state_fs.operation.type != ::Block::Operation::Type::INVALID) {
				operation  = _state_fs.operation;
				dst_offset = _state_fs.dst_offset;

#if 0
				Genode::log("write ", request.operation.block_number, "+", request.operation.count,
					        " -> ", _state_fs.operation.block_number, "+", _state_fs.operation.count,
					        " - dst_offset=", dst_offset);
#endif
			}

			bool const cross_vdi_block = _cross_vdi_block(operation);

			loop = _apply_block(operation.block_number, [&](Genode::uint32_t) {
				if (_state_fs.state == Write::WRITE) {
					Genode::error("during data write sector in vdi vanished ?");
					_state_fs.state = Write::ERROR;
					response = Response::REJECTED;
					return false;
				}

				_allocate_block(operation.block_number);

				if (_state_fs.state == ALLOC_BLOCK_SYNC_QUEUED ||
				    _state_fs.state == ALLOC_BLOCK_SYNC ||
				    _state_fs.state == SYNC_HEADER ||
				    _state_fs.state == SYNC_HEADER_1 ||
				    _state_fs.state == SYNC_HEADER_2 ||
				    _state_fs.state == ALLOC_BLOCK) {

					response = Response::RETRY;
					return false;
				}

				if (_state_fs.state != Write::IDLE) {
					Genode::error("unknown state Block::Write state_fs=",
					              (int)_state_fs.state);
					response = Response::REJECTED;
					return false;
				}

				return true;
			}, [&](uint64_t const offset, uint32_t const max_offset_write) {

				Vfs::file_size const len = operation.count * _block_ops.block_size;
				bool retry = false;

				payload.with_content(request, [&] (void *addr, Genode::size_t const dst_size) {
					char * dst = reinterpret_cast<char *>(addr);

					if (dst_offset > dst_size || len < dst_size - dst_offset) {
						Genode::error("remaining size to write is bogus - stop");
						_state_fs.state  = Write::ERROR;
						return;
					}

					if (_state_fs.state == Write::IDLE) {
						_state_fs.block_nr = operation.block_number;
						_state_fs.written  = 0;
						_state_fs.max      = Genode::min(max_offset_write, len);
						_state_fs.state    = Write::WRITE;
					}

					if (_state_fs.max > dst_size - dst_offset) {
						Genode::error("write larger than buffer - stop");
						_state_fs.state  = Write::ERROR;
						return;
					}

					if (_state_fs.state == Write::WRITE) {

						_write(dst + dst_offset, dst_size - dst_offset, offset);

						if (_state_fs.written >= _state_fs.max) {
							_state_fs.state = Write::IDLE;

							if (!cross_vdi_block) {
								_state_fs.operation.type = ::Block::Operation::Type::INVALID;
								_state_fs.dst_offset = 0;
							} else {
								uint32_t const blocks = _state_fs.max / _block_ops.block_size;

								if (operation.count < blocks) {
									Genode::error("write - count of blocks is too small ", operation.count, " ", blocks);
									response = Response::REJECTED;
									return;
								}

								_state_fs.operation = operation;
								_state_fs.operation.block_number += blocks;
								_state_fs.operation.count -= blocks;

								if (_state_fs.operation.count == 0) {
									_state_fs.operation.type = ::Block::Operation::Type::INVALID;
									_state_fs.dst_offset = 0;
									/* we should ever get into !cross_vdi_block case */
									Genode::warning("write - insane state");
								} else {
#if 0
									Genode::log("partial write done ",
									            _state_fs.written, "/", _state_fs.max, " vs ", len,
									            " -> next"
									            " block_number=", _state_fs.operation.block_number,
									            " count=", _state_fs.operation.count);
#endif

									_state_fs.dst_offset += _state_fs.max;
									dst_offset += _state_fs.max;
									retry = true;
								}
							}
						}

						if (_state_fs.state == Write::IDLE)
							response = Response::ACCEPTED;
						else
							response = Response::RETRY;
					}
				});

				return retry; /* loop */
			});

		} while (loop);

		break;
	}
	default:
		break;
	}

	return response;
}
