/*
 * \brief  VDI file as a Block session
 * \author Josef Soentgen
 * \date   2018-11-01
 */

/*
 * Copyright (C) 2018 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _VDI_BLOCK__VDI_FILE_H_
#define _VDI_BLOCK__VDI_FILE_H_

/* Genode includes */
#include <base/attached_ram_dataspace.h>
#include <base/attached_rom_dataspace.h>
#include <base/heap.h>
#include <base/log.h>
#include <block/request_stream.h>
#include <block_session/block_session.h>
#include <util/string.h>
#include <vfs/simple_env.h>

#include <base/debug.h>
#include <vfs/print.h>

/* local includes */
#include <vdi_types.h>

namespace Vdi {
	struct Block;
	struct Meta_data;
	struct File;
} /* namespace Vdi */


struct Vdi::Block
{
	uint32_t value;

	enum {
		BLOCK_FREE = ~0u,
		BLOCK_ZERO = ~1u,
	};

	bool zero()      const { return value == BLOCK_ZERO; }
	bool free()      const { return value == BLOCK_FREE; }
	bool allocated() const { return value  < BLOCK_FREE; }
};


struct Vdi::Meta_data
{
	uint32_t const blocks_offset;
	uint32_t const data_offset;

	uint32_t const block_size;
	uint32_t const sector_size;

	Vdi::Block *table            { nullptr };
	uint32_t    max_blocks       { 0 };
	uint32_t    allocated_blocks { 0 };

	int fd { -1 };

	Meta_data(uint32_t blocks, uint32_t data,
	          uint32_t block_size, uint32_t sector_size)
	:
		blocks_offset(blocks), data_offset(data),
		block_size(block_size), sector_size(sector_size)
	{ }

	template <typename T>
	bool alloc_block(uint64_t const bid, T const &fn)
	{
		if (bid >= max_blocks)
			return false;

		uint64_t const offset = uint64_t(data_offset) +
		                        uint64_t(allocated_blocks) *
		                        uint64_t(block_size);
		fn(offset);

		table[bid].value = allocated_blocks;
		allocated_blocks++;

		return true;
	}
};


#if 0
static void print_uuid(Random_uuid const *uuid)
{
	printf("%8.8x-%4.4x-%4.4x-%2.2x%2.2x-",
	       uuid->dce.time_low, uuid->dce.time_mid,
	       uuid->dce.time_hi_and_version, uuid->dce.clock_seq_hi_and_reserved,
	       uuid->dce.clock_seq_low);
	for (int i = 0; i < 6; i++) {
		printf("%2.2x", uuid->dce.node[i]);
	}
	printf("\n");
}


static void print_block_table(Meta_data const &md)
{
	uint32_t const blocks_offset = md.blocks_offset;
	uint32_t const data_offset   = md.data_offset;
	uint32_t const allocated     = md.allocated_blocks;
	printf("b: %u d: %u a: %u\n", blocks_offset, data_offset, allocated);

	for (uint32_t i = 0; i < allocated; i++) {
		uint32_t const id   = md.table[i].value;
		uint64_t const offset = data_offset + (id * HeaderV1Plus::BLOCK_SIZE);
		printf("  block[%u] offset: %lu\n", i, offset);
	}
}


static void print_sector_offset(Meta_data const &md, uint64_t nr)
{
	uint64_t const offset = lookup_disk_sector(md, nr);
	if (offset == ~0) {
		printf("Not allocated sector %lu\n", nr);
	} else {
		printf("Sector %lu offset: %lu\n", nr, offset);
	}
}
#endif


static void print_headers(Preheader const &ph, HeaderV1Plus const &h)
{
	using namespace Genode;

	log("--- PreHeader ---");
	log("Info: '", ph.info, "'");
	log("Signature okay: ", ph.valid() ? "yes" : "no");
	log("Version: ", ph.major(), ".", ph.minor());
	log("--- HeaderV1Plus ---");
	log("Size:          ", h.size);
	log("Type:          ", h.type);
	log("Flags:         ", Hex(h.flags));
	log("Blocks offset: ", h.blocks_offset);
	log("Data offset:   ", h.data_offset);
	log("Legacy cylinders:   ", h.legacy_geometry.cylinders);
	log("Legacy heads:       ", h.legacy_geometry.heads);
	log("Legacy sectors:     ", h.legacy_geometry.sectors);
	log("Legacy sector_size: ", h.legacy_geometry.sector_size);
	log("Disk size:          ", h.disk_size);
	log("Block size:         ", h.block_size);
	log("Block size extra:   ", h.block_size_extra);
	log("Blocks:             ", h.blocks);
	log("Allocated blocks:   ", h.allocated_blocks);
	log("Image UUID:       "); //print_uuid(&h.image_uuid);
	log("Modify UUID:      "); //print_uuid(&h.modify_uuid);
	if (h.prev_uuid.valid()) {
		log("Prev UUID:        "); //print_uuid(&h.prev_uuid);
		log("Prev modify UUID: "); //print_uuid(&h.prev_modify_uuid);
	}
}


static Genode::Xml_node vfs_config(Genode::Xml_node const &config)
{
	try {
		return config.sub_node("vfs");
	} catch (...) {
		Genode::error("VFS not configured");
		throw;
	}
}

class Vdi::File: public Vfs::Io_response_handler
{
	private:

		using Read_result  = Vfs::File_io_service::Read_result;
		using Sync_result  = Vfs::File_io_service::Sync_result;
		using Write_result = Vfs::File_io_service::Write_result;

		/****************************************
		 ** Vfs::Io_response_handler interface **
		 ****************************************/

		void read_ready_response() override
		{
			Genode::error(__LINE__, " unimplemented");
		}

		void io_progress_response() override
		{
#if 0
			Genode::log("io response write=", (int)_state_fs.state,
			            " read=", (int)_state_fs_read.state,
			            " sync=", (int)_state_fs_sync.state,
			            " cap=", _block_notify.cap);
#endif

			if (   (_state_fs_read.state == NONE || _state_fs_read.state == UNKNOWN)
			    && (_state_fs_sync.state == Sync::IDLING || _state_fs_sync.state == Sync::FAULT)
				&& (_state_fs.state == Write::IDLE || _state_fs.state == Write::ERROR))
				return;

			if (_block_notify.cap.valid())
				Genode::Signal_transmitter(_block_notify.cap).submit();
		}

		void _execute_alloc_block();

		File(File const &) = delete;
		File operator=(File const&) = delete;

		Genode::Env  &_env;
		Genode::Heap  _heap { _env.ram(), _env.rm() };

		Genode::Attached_ram_dataspace  _header_buffer { _env.ram(), _env.rm(), 2u<<20 };
		Vfs::file_size                  _header_size   { _header_buffer.size() };
		char                           *_header_addr   { _header_buffer.local_addr<char>() };

		Genode::Attached_ram_dataspace  _zero_buffer { _env.ram(), _env.rm(), 64u<<10 };
		Vfs::file_size const            _zero_size   { _zero_buffer.size() };
		char                           *_zero_addr   { _zero_buffer.local_addr<char>() };

		::Block::Session::Info       _block_ops   { };

		Vfs::Simple_env _vfs_env;

		Vfs::Vfs_handle *_vdi_file { nullptr };

		Genode::Constructible<Vdi::Meta_data> _md { };

		enum Write {
			ERROR,
			IDLE,
			ALLOC_BLOCK,
			ALLOC_BLOCK_ERROR,
			ALLOC_BLOCK_SYNC,
			ALLOC_BLOCK_SYNC_QUEUED,
			SYNC_HEADER,
			SYNC_HEADER_1,
			SYNC_HEADER_2,
			SYNC_OK,
			WRITE
		};

		enum Read { NONE, READ, LOOP_READ, CHECK, UNKNOWN, END };

		struct {
			enum Write         state;
			Vfs::file_size     written;
			Vfs::file_size     max;
			uint64_t           block_nr;
			Vfs::file_size     dst_offset;
			::Block::Operation operation;
		} _state_fs { Write::IDLE, 0, 0, 0, 0,
		              { ::Block::Operation::Type::INVALID, 0, 0}
		            };

		struct {
			enum Read          state;
			Vfs::file_size     bytes_read;
			Vfs::file_size     remaining;
			Vfs::file_size     offset;
			::Block::Operation operation;
			Vfs::file_size     dst_offset;
		} _state_fs_read { Read::NONE, 0, 0, 0,
		                   { ::Block::Operation::Type::INVALID, 0, 0},
		                   0
		                 };

		enum Sync { IDLING, FAULT, SYNC_QUEUED };

		struct {
			enum Sync state;
		} _state_fs_sync { Sync::IDLING };

		struct {
			Genode::Signal_context_capability cap;
		} _block_notify { };

		static inline constexpr uint32_t sectors_per_block()
		{
			return HeaderV1Plus::BLOCK_SIZE / HeaderV1Plus::SECTOR_SIZE;
		}

		static inline uint64_t sector_to_block(uint64_t const nr)
		{
			return nr / sectors_per_block();
		}

		template <typename BLOCK_MISS, typename BLOCK_READY>
		auto _apply_block(uint64_t const nr, BLOCK_MISS missing, BLOCK_READY found)
		{
			uint32_t const dis = (nr % sectors_per_block()) * HeaderV1Plus::SECTOR_SIZE;
			uint32_t const max_bid = _md->max_blocks;
			uint64_t const bid = sector_to_block(nr);

			if (bid >= max_bid || !_md->table[bid].allocated())
				return missing(HeaderV1Plus::BLOCK_SIZE - dis);

			uint64_t const pid = _md->table[bid].value;

			uint64_t const data = _md->data_offset;

			return found(data + (pid * HeaderV1Plus::BLOCK_SIZE) + dis,
			             HeaderV1Plus::BLOCK_SIZE - dis);
		}

		void _sync_header(uint64_t const bid)
		{
			if (bid >= _md->max_blocks) {
				Genode::error(__func__, " invalid bid");
				_state_fs.state = Write::ERROR;
				return;
			}

			HeaderV1Plus *h = (HeaderV1Plus*)(_header_addr + sizeof(Preheader));
			Vdi::Block *table = (Vdi::Block*)(_header_addr + h->blocks_offset);
			Vfs::file_offset const offset = h->blocks_offset + (bid * sizeof (uint32_t));

			if (_state_fs.state == SYNC_HEADER) {

				h->allocated_blocks = _md->allocated_blocks;
				table[bid].value     = _md->table[bid].value;

				/* update block table */
				_state_fs.written  = 0;
				_state_fs.max      = sizeof(uint32_t);

				_state_fs.state    = SYNC_HEADER_1;
			}

			if (_state_fs.state == SYNC_HEADER_1) {
				_write(reinterpret_cast<char *>(&table[bid]), sizeof(uint32_t), offset);

				if (_state_fs.written < _state_fs.max)
					/* will be resumed later, keep state */
					return;

				/* update header */
				_state_fs.written  = 0;
				_state_fs.max      = HeaderV1Plus::SECTOR_SIZE;

				_state_fs.state = SYNC_HEADER_2;
			}

			if (_state_fs.state == SYNC_HEADER_2) {
				/* update header */
				_write(reinterpret_cast<char *>(_header_addr),
				        HeaderV1Plus::SECTOR_SIZE, 0 /*offset*/);

				if (_state_fs.written < _state_fs.max)
					/* will be resumed later, keep state */
					return;

				_state_fs.state = Write::SYNC_OK;
			}
		}

		void _allocate_block(uint64_t nr)
		{
			if (_md->allocated_blocks >= _md->max_blocks) {
				Genode::error("allocated blocks > max blocks");
				_state_fs.state = Write::ERROR;
				return;
			}

			if (_state_fs.state != Write::IDLE) {
				Genode::error("several allocate block requests");
				_state_fs.state = Write::ERROR;
				return;
			}

			_state_fs.state    = Write::ALLOC_BLOCK;
			_state_fs.written  = 0;
			_state_fs.max      = _md->block_size;
			_state_fs.block_nr = nr;

			_execute_alloc_block();
		}

		Vfs::File_io_service::Sync_result _complete_sync_fs() {
			return _vdi_file->fs().complete_sync(_vdi_file); }

		void _write(char * const base, Vfs::file_size const base_size,
		            Genode::uint64_t const fs_offset)
		{
			do {
				if (_state_fs.written > _state_fs.max ||
				    _state_fs.written > base_size) {
					Genode::error("size errors");
					_state_fs.state = Write::ERROR;
					return;
				}

				auto rest = Genode::min(base_size - _state_fs.written,
				                        _state_fs.max - _state_fs.written);
				char * dst = base + _state_fs.written;
				Vfs::file_size written = 0;

				try {
					_vdi_file->seek(fs_offset + _state_fs.written);
					Write_result res = _vdi_file->fs().write(_vdi_file,
					                                         dst,
					                                         rest,
					                                         written);
					if (res != Vfs::File_io_service::WRITE_OK)
					{
						_state_fs.state = Write::ERROR;
						Genode::error(".... write error");
						return;
					}
				} catch (Vfs::File_io_service::Insufficient_buffer) {
					/* will be resumed later, keep state on WRITE */
					return;
				}

				_state_fs.written += written;
			} while (_state_fs.written < _state_fs.max);
		}

		void _read(char * dst, Vfs::file_size const dst_size)
		{
			Vfs::Vfs_handle &handle = *_vdi_file;

			if (_state_fs_read.state == Read::LOOP_READ)
				_state_fs_read.state = Read::READ;

			if (_state_fs_read.state == Read::READ) {
				handle.seek(_state_fs_read.offset);

				if (!handle.fs().queue_read(&handle, _state_fs_read.remaining)) {
					/* will be resumed later, keep READ state */
					return;
				}

				_state_fs_read.state = Read::CHECK;
			}

			if (_state_fs_read.state == Read::CHECK) {
				if (_state_fs_read.remaining > dst_size) {
					Genode::error("buffer insufficient to read data");
					_state_fs_read.state = Read::UNKNOWN;
					return;
				}

				char          *p = dst + _state_fs_read.bytes_read;
				Vfs::file_size n = 0;

				Read_result read_result =
					handle.fs().complete_read(&handle, p,
					                          _state_fs_read.remaining,
					                          n);

				switch (read_result) {
				case Vfs::File_io_service::READ_OK:
				{
					if (_state_fs_read.remaining != n) {
						if (n)
							_state_fs_read.state = Read::LOOP_READ;
						else {
							/* end of file */
							_state_fs_read.state = Read::END;
						}
					}

					_state_fs_read.bytes_read += n;
					_state_fs_read.offset     += n;

					if (_state_fs_read.remaining >= n)
						_state_fs_read.remaining  -= n;
					else
						_state_fs_read.remaining = 0;

					if (_state_fs_read.remaining == 0)
						_state_fs_read.state = Read::NONE;

					break;
				}
				case Vfs::File_io_service::READ_QUEUED:
					if (n)
						Genode::error("read queued with n=", n);
					break;
				default:
					Genode::error("read not ok res=", (int)read_result, " ", n);
				}
			}
		}

		typedef ::Block::Request_stream::Response Response;

		Response _sync()
		{
			switch (_state_fs_sync.state) {
			case Sync::FAULT:
				return Response::REJECTED;

			case Sync::IDLING:
				if (!_vdi_file->fs().queue_sync(_vdi_file))
					return Response::RETRY;
				_state_fs_sync.state = Sync::SYNC_QUEUED;

				[[fallthrough]];

			case Sync::SYNC_QUEUED: {
				Sync_result res = _complete_sync_fs();
				switch (res) {
				case Vfs::File_io_service::SYNC_QUEUED:
					_state_fs_sync.state = Sync::SYNC_QUEUED;
					return Response::RETRY;
				case Vfs::File_io_service::SYNC_ERR_INVALID:
					Genode::error("sync fault - out of service");
					_state_fs_sync.state = Sync::FAULT;
					return Response::REJECTED;
				case Vfs::File_io_service::SYNC_OK:
					_state_fs_sync.state = Sync::IDLING;
					return Response::ACCEPTED;
				}
			}
			}
			return Response::REJECTED;
		}

		Response _read_split(::Block::Operation const operation,
		                     void * dst, Vfs::file_size,
		                     uint64_t const file_offset,
		                     uint32_t const max_offset_read);

		bool _cross_vdi_block(::Block::Operation const) const;

	public:

		struct Could_not_open_file : Genode::Exception { };

		File(Genode::Env &env, Genode::Xml_node config)
		:
			_env(env),
			_vfs_env(_env, _heap, vfs_config(config))
		{
			bool const writeable = config.attribute_value("writeable", false);

			_block_ops.writeable = writeable;

			Genode::String<256> file;
			file = config.attribute_value("file", file);
			if (!file.valid()) {
				Genode::error("mandatory file attribute missing");
				throw Could_not_open_file();
			}
			Vfs::Directory_service::Open_result open_result =
			_vfs_env.root_dir().open(file.string(),
			                         writeable ? Vfs::Directory_service::OPEN_MODE_RDWR
			                                   : Vfs::Directory_service::OPEN_MODE_RDONLY,
			                         &_vdi_file, _heap);
			if (open_result != Vfs::Directory_service::OPEN_OK) {
				Genode::error("Could not open '", file, "'");
				throw Could_not_open_file();
			}

			_vdi_file->handler(this);

			Genode::log("Provide '", file.string(), "' as block device,"
			            " writeable: ", writeable ? "yes" : "no");
		}

		void set_notify_cap(Genode::Signal_context_capability const signal) {
			_block_notify.cap = signal; }

		bool init(Genode::Signal_context_capability const init_signal)
		{
			if (_state_fs_read.bytes_read != _header_size) {
				if (_state_fs_read.state == Read::NONE) {
					set_notify_cap(init_signal);

					_state_fs_read.bytes_read = 0;
					_state_fs_read.remaining  = _header_size;
					_state_fs_read.offset     = 0;
					_state_fs_read.state      = Read::READ;
				}

				do {
					_read(_header_addr, _header_size);
				} while (_state_fs_read.state == Read::LOOP_READ);

				if (_state_fs_read.state == Read::END) {
					if (_state_fs_read.bytes_read >= sizeof(Preheader) + sizeof(HeaderV1Plus)) {
						_state_fs_read.state = Read::NONE;
						_header_size = _state_fs_read.bytes_read;
					} else {
						_state_fs_read.state = Read::UNKNOWN;
						Genode::error("read header to short");
					}
				}
			}

			if (_state_fs_read.state != Read::NONE ||
			    _state_fs_read.bytes_read != _header_size)
				return false;

			set_notify_cap(Genode::Signal_context_capability());

			Preheader    const &ph = *(Preheader*)(_header_addr);
			HeaderV1Plus const  &h = *(HeaderV1Plus*)(_header_addr + sizeof(Preheader));

			print_headers(ph, h);

			if (!ph.valid()) {
				_state_fs_read.state = Read::UNKNOWN;
				Genode::error("signature error");
				return false;
			}

			if (h.blocks_offset + h.blocks * sizeof(Vdi::Block::value) > _state_fs_read.bytes_read) {
				Genode::error("block count error");
				_state_fs_read.state = Read::UNKNOWN;
				return false;
			}

			_md.construct(h.blocks_offset, h.data_offset,
			              HeaderV1Plus::BLOCK_SIZE, HeaderV1Plus::SECTOR_SIZE);
			_md->max_blocks       = h.blocks;
			_md->allocated_blocks = h.allocated_blocks;
			_md->table            = (Vdi::Block*)(_header_addr + h.blocks_offset);

			_block_ops.block_size = HeaderV1Plus::SECTOR_SIZE;
			_block_ops.block_count = h.disk_size / _block_ops.block_size;

			Genode::log("block_size: ", _block_ops.block_size,
			            " block_count: ", _block_ops.block_count);

			return true;
		}

		~File()
		{
			/* XXX close file */
		}

		::Block::Session::Info info() const { return _block_ops; }

		Response handle(::Block::Request const & request,
		                ::Block::Request_stream::Payload const &payload);
};

#endif /* _VDI_BLOCK__VDI_FILE_H_ */
