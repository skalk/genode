/*
 * \brief  Storage handling
 * \author Alexander Boettcher
 * \date   2019-04-18
 */

/*
 * Copyright (C) 2019-2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#include <file_system_session/connection.h>

namespace Top {
	class Storage;
	class File;

	using namespace Genode;
	using namespace File_system;

	typedef File_system::Packet_descriptor Packet;
	typedef File_system::Session           Session;
}

struct Type_a
{
	Genode::Trace::Subject_id      id;
	Genode::Trace::Execution_time  execution_time;
	Genode::uint16_t               part_ec_time;
	Genode::uint16_t               part_sc_time;
};

struct Type_b
{
	Genode::Trace::Subject_id  id;
	Genode::Session_label      label;
	Genode::Trace::Thread_name thread;
	unsigned                   loc_x;
	unsigned                   loc_y;
};

struct Type_c
{
	Genode::Trace::Subject_id id;
};

class Top::File
{
	private:

		File_system::Connection &_fs;
		File_handle              _file_handle;
		uint64_t                 _fs_offset { 0 };
		size_t                   _pos       { 0 };

		uint16_t                 _cnt_flush_pending { 0 };
		uint16_t                 _cnt_flush_last { 0 };
		uint16_t                 _cnt_packet_err { 0 };
		uint64_t                 _cnt_data_lost { 0 };
		uint64_t                 _cnt_data_last { 0 };

		bool                     _flush_pending { false };
		size_t const             _max;
		char                     _buffer [8192];

	public:

		File (File_system::Connection &fs, char const *file, size_t max)
		:
			_fs(fs),
			_file_handle { _fs.file(_fs.dir("/", false), file,
			                        READ_WRITE, true /* create */) },
			_max(max > sizeof(_buffer) ? sizeof(_buffer) : max)
		{ }

		bool write(void *data, size_t const size)
		{
			if (!size || size > _max || _pos + size >= _max) {
				_cnt_data_lost += size;

				if (!_cnt_data_last || (_cnt_data_last + 10000 < _cnt_data_lost)) {
					_cnt_data_last = _cnt_data_lost;
					Genode::warning("file ", _file_handle, " - lost=", _cnt_data_lost);
				}
				return false;
			}

			memcpy(_buffer + _pos, data, size);

			_pos       += size;
			_fs_offset += size;

			return true;
		}

		void flush_data(Session::Tx::Source &tx)
		{
			if (empty()) return;

			try {
				Packet packet { tx.alloc_packet(_pos), _file_handle,
				                Packet::WRITE, _pos, _fs_offset - _pos};

				memcpy(((char *)tx.packet_content(packet)), _buffer, _pos);

#if 0
				error(this, " fs_offset=", _fs_offset, " send=", _pos);
#endif
				_pos = 0;

				tx.submit_packet(packet);
			} catch (Session::Tx::Source::Packet_alloc_failed) {
				_cnt_packet_err ++;
				if (_cnt_packet_err % 10 == 1)
					error("file ", _file_handle, " - ", _cnt_packet_err, ". packet error, lost=", _cnt_data_lost, " pending flush=", pending());
			}
		}

		bool flush(size_t const space) const { return _pos + space >= _max; }
		bool empty() const { return _pos == 0; }

		bool pending() const { return _flush_pending; }
		void flush_pending() { _cnt_flush_pending ++; _flush_pending = true; }
		void reset_pending() { _flush_pending = false; }

		void stat_pending_cnt()
		{
			if (_cnt_flush_last + 10 > _cnt_flush_pending) return;

			_cnt_flush_last = _cnt_flush_pending;

			Genode::log("file ", _file_handle, " - ", _cnt_flush_pending, " lost=", _cnt_data_lost);
		}
};

class Top::Storage
{
	private:

		Env &env;
		Heap heap { env.pd(), env.rm() };
		Allocator_avl avl_alloc { &heap };
		File_system::Connection fs { env, avl_alloc, "store", "/", true };
		Session::Tx::Source  &tx { *fs.tx() };

		size_t const _packet_max { tx.bulk_buffer_size() / Session::TX_QUEUE_SIZE };

		Top::File    _data       { fs, "data.top_view", _packet_max };
		Top::File    _subject    { fs, "subject.top_view", _packet_max };
		Top::File    _select     { fs, "select.top_view", _packet_max };

		Signal_handler<Storage> _handler { env.ep(), *this, &Storage::_handle_submit };

		void _handle_ack()
		{
			while (tx.ack_avail()) {
				auto packet = tx.get_acked_packet();
				tx.release_packet(packet);
			}
		}

		void _handle_submit()
		{
			_handle_ack();

			if (_data.pending()) {
				if (!tx.ready_to_submit()) {
					Genode::error("no space for submitting new data ?");
					return;
				}
				_data.stat_pending_cnt();
				_data.flush_data(tx);
				_data.reset_pending();
			}

			if (_subject.pending()) {
				if (!tx.ready_to_submit()) {
					Genode::error("no space for submitting new subject ?");
					return;
				}
				_subject.stat_pending_cnt();
				_subject.flush_data(tx);
				_subject.reset_pending();
			}

			if (_select.pending()) {
				if (!tx.ready_to_submit()) {
					Genode::error("no space for submitting new select ?");
					return;
				}
				_select.stat_pending_cnt();
				_select.flush_data(tx);
				_select.reset_pending();
			}
		}

		template <typename T>
		void _write(T value, Top::File &file)
		{
			file.write(&value, sizeof(value));

			/* ask for whether flushing is appropriate */
			if (!file.flush(sizeof(value) * 2)) return;

			if (!tx.ready_to_submit())
			{
				/* check for available acks */
				_handle_ack();
				if (!tx.ready_to_submit()) {
					/* remember that we could not send data */
					file.flush_pending();
					return;
				}
			}
			if (file.pending()) {
				Genode::warning("pending but got not processed before next write ... ");
				file.reset_pending();
			}

			file.flush_data(tx);
		}

	public:

		Storage(Env &env) : env(env)
		{
			fs.sigh(_handler);
		}

		void write(Type_a value) { _write(value, _data); }
		void write(Type_b value) { _write(value, _subject); }
		void write(Type_c value) { _write(value, _select); }

		void force_data_flush()
		{
			_subject.flush_data(tx);
			_select.flush_data(tx);
			_data.flush_data(tx);
		}
};
