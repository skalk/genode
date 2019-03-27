/*
 * \brief  Storage handling
 * \author Alexander Boettcher
 * \date   2019-06-15
 */

/*
 * Copyright (C) 2019 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#include <file_system_session/connection.h>

namespace Top
{
	template <typename> class Storage;
	class File;

	using namespace Genode;
	using namespace File_system;

	typedef File_system::Packet_descriptor Packet;
	typedef File_system::Session           Session;
}

class Top::File
{
	private:
public:
		File_system::Connection &_fs;
		File_handle              _file_handle;
		uint64_t                 _fs_offset { 0 };
		uint64_t                 _fs_size   { 0 };
		size_t                   _pos       { 0 };

		size_t const             _max;
		char                     _buffer [8192];

	public:

		File (File_system::Connection &fs, char const *file, size_t max)
		:
			_fs(fs),
			_file_handle { _fs.file(_fs.dir("/", false), file,
			               READ_ONLY, false /* create */) },
			_max(max > sizeof(_buffer) ? sizeof(_buffer) : max)
		{ }

		bool read(Session::Tx::Source &tx)
		{
			if (_fs_offset > _fs_size) return false;
			size_t const request = size_t((_fs_size - _fs_offset < _max) ? _fs_size - _fs_offset : _max);
			if (!request) return false;

			Packet packet { tx.alloc_packet(request), _file_handle,
			                Packet::READ, request, _fs_offset };

			_fs_offset += request;

			tx.submit_packet(packet);

			return true;
		}

		bool update_fs_size(uint64_t size)
		{
			bool const size_change = (size != _fs_size);
			_fs_size = size;
			return size_change;
		}

		uint64_t fs_offset() const { return _fs_offset; }

		void reset() { _fs_offset = 0; }
		void reset_if_eof() { if (_fs_size == _fs_offset) _fs_offset = 0; }

		void adjust_offset(long long const adjust) { _fs_offset += adjust; }

		File_handle file_handle() const { return _file_handle; }
};

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

template <typename T>
class Top::Storage
{
	public:

		static constexpr unsigned const INVALID_ID { ~0U }; /* XXX */

	private:

		Env &env;
		T &_notify;
		Heap heap { env.pd(), env.rm() };
		Allocator_avl avl_alloc { &heap };
		File_system::Connection _fs { env, avl_alloc, "load", "/", false };
		Session::Tx::Source  &tx { *_fs.tx() };

		size_t const _packet_max { tx.bulk_buffer_size() / Session::TX_QUEUE_SIZE };

		Genode::Constructible<Top::File> _data    { };
		Genode::Constructible<Top::File> _subject { };
		Genode::Constructible<Top::File> _select  { };

		Trace::Subject_id _id_req[16];
		Trace::Subject_id _id_round[16];

		Signal_handler<Storage> _handler { env.ep(), *this, &Storage::_handle_fs_event };

		unsigned long long _current_timestamp { 0 };
		enum State {
			IDLE,
			WAIT_FOR_TSC_0,
			WAIT_FOR_TSC_1,
			WAIT_FOR_DATA,
			WAIT_FOR_DATA_UNAVAILABLE,
			READ_DATA,
		} _state { IDLE };

		template <typename TYPE, typename P, typename FN>
		void _apply(P & packet, Genode::Constructible<Top::File> &_file, FN const &fn)
		{
			unsigned offset = unsigned(packet.position() % sizeof(TYPE));
			if (offset > 0)
				Genode::warning(&_file, " : unexpected offset detected ... ", offset, " ", packet.position(), " fs=", _file->file_handle());

			if (offset > 0) offset = sizeof(TYPE) - offset;

			TYPE const * const value = reinterpret_cast<TYPE *>(reinterpret_cast<char *>(tx.packet_content(packet)) + offset);
			char const * const end = reinterpret_cast<char const * >(tx.packet_content(packet)) + packet.length();
			char const * const start = reinterpret_cast<char const * >(value);
			unsigned const count = unsigned((end - start) / sizeof(*value));

			for (unsigned i = 0; i < count; i++) {
				if (!fn(value[i])) {
					_file->adjust_offset(-1LL * (packet.length() - offset - i*sizeof(*value)));
					return;
				}
			}

			if ((end - start) % sizeof(*value))
				_file->adjust_offset(-1LL * ((end - start) % sizeof(*value)));
		}

		void _handle_fs_event()
		{
			bool read_subject = false;
			bool read_data = false;
			bool read_select = false;

			while (tx.ack_avail()) {
				auto packet = tx.get_acked_packet();
				if (packet.operation() != File_system::Packet_descriptor::Opcode::READ) {
					tx.release_packet(packet);
					continue;
				}

				if (!packet.succeeded()) {
					Genode::warning("not succeeded read packet ?");
					tx.release_packet(packet);
					continue;
				}

				if (_subject.constructed() && _subject->file_handle() == packet.handle()) {
					_apply<Type_b>(packet, _subject, [&] (Type_b const &type) {
						//Genode::log("subject - ", type.id.id, " '", type.label,"'");

						for (unsigned i = 0; i < sizeof(_id_req) / sizeof(_id_req[0]); i++) {
							if (!(_id_req[i] == type.id)) continue;

							_id_req[i].id = INVALID_ID;

//							Genode::log("[", i, "] found subject - ", type.id.id, " '", type.label,"'");

							Genode::String<12> const cpu(type.loc_x, ".", type.loc_y);

							_notify.add_entry(type.id, type.label, type.thread, cpu);

							break;
						}

						return true;
					});

					bool all_read = true;
					for (unsigned i = 0; i < sizeof(_id_req) / sizeof(_id_req[0]); i++) {
						if (_id_req[i].id != INVALID_ID) {
							all_read = false;
							break;
						}
					}

					read_subject = !all_read;

					if (!read_subject)
						_subject->reset();
					else
						_subject->reset_if_eof();
				}

				if (_select.constructed() && _select->file_handle() == packet.handle()) {
					read_select = true;

					_apply<Type_c>(packet, _select, [&] (Type_c const & type) {

						/* XXX better tsc transfer ? */
						if (type.id == INVALID_ID) {
							if (_state == IDLE) {
								_state = WAIT_FOR_TSC_0;
								return true;
							} else {
								read_select = false;
								return false; /* wait for next IDLE */
							}
						}
						if (_state == WAIT_FOR_TSC_0) {
							_state = WAIT_FOR_TSC_1;
							_current_timestamp = type.id.id;
							return true;
						}
						if (_state == WAIT_FOR_TSC_1) {
							_state = WAIT_FOR_DATA;

							_current_timestamp += uint64_t(type.id.id) << 32;

//							Genode::log(Genode::Hex(_current_timestamp), " --> current timestamp");
							read_data = true;
							return true;
						}

						if (!_notify.id_available(type.id)) {
							for (unsigned i = 0; i < sizeof(_id_req) / sizeof(_id_req[0]); i++) {
								if (_id_req[i] == type.id) break;
								if (_id_req[i].id != INVALID_ID) continue;

								_id_req[i] = type.id;
//								Genode::log(Genode::Hex(_current_timestamp), " put to list [", i, "]=", type.id.id);

								if (_subject.constructed()) {
									/* request reading subject info only when stopped */
//									Genode::log("fs offset ", _subject->fs_offset(), " ", _subject->_fs_size);
									if (_subject->fs_offset() == 0)
										read_subject = true;
								}

								break;
							}
						}

						for (unsigned i = 0; i < sizeof(_id_round) / sizeof(_id_round[0]); i++) {
							if (_id_round[i].id != INVALID_ID) continue;
							_id_round[i] = type.id;
//							Genode::log(Genode::Hex(_current_timestamp), " round list  [", i, "]=", type.id.id);
							break;
						}

						return true;
					});
				}

				if (_data.constructed() && _data->file_handle() == packet.handle()) {
					read_data = true;

					unsigned applied_count = 0;
					unsigned drop_time = 0;

					_apply<Type_a>(packet, _data, [&] (Type_a const &type) {

						enum SORT_TIME { EC_TIME = 0, SC_TIME = 1} sort = { EC_TIME };

						if (type.id.id == INVALID_ID) {
							if (_current_timestamp == type.execution_time.thread_context) {
								_state = READ_DATA;
							} else {
								if (_state == READ_DATA) {
									for (unsigned i = 0; i < sizeof(_id_round) / sizeof(_id_round[0]); i++) {
										_id_round[i].id = INVALID_ID;
									}

									_state = IDLE;
									/* stop reading new packets */
									read_data = false;
									read_select = true;
									return false;
								}
							}

							if (!_notify.advance_column_by_storage(_current_timestamp)) {
								/* stop reading new packets */
								read_data = false;
								_state = IDLE;

								for (unsigned i = 0; i < sizeof(_id_round) / sizeof(_id_round[0]); i++) {
									_id_round[i].id = INVALID_ID;
								}

								return false;
							}

							if (type.execution_time.scheduling_context == EC_TIME)
								sort = EC_TIME;
							else
								sort = SC_TIME;
						}

						if (_current_timestamp < _notify.time()) {
							drop_time ++;
							return true;
						}

						if (_state == READ_DATA) {
							for (unsigned i = 0; i < sizeof(_id_round) / sizeof(_id_round[0]); i++) {
								if (_id_round[i].id == INVALID_ID) continue;
								if (_id_round[i].id != type.id.id) continue;

								uint64_t const time = (sort == EC_TIME) ? type.part_ec_time : type.part_sc_time;
								if (_notify.new_data(time, type.id.id, _current_timestamp))
									applied_count++;

								_id_round[i].id = INVALID_ID;
							}
						}

						return true;
					});

					if (drop_time)
						Genode::warning("time ", Genode::Hex(_current_timestamp), " drop=", drop_time);
				}
				tx.release_packet(packet);
			}

			if (read_subject)
				_subject->read(*_fs.tx());

			if (read_select)
				_select->read(*_fs.tx());

			if (read_data && !_data->read(*_fs.tx())) {
				if (_state != READ_DATA) Genode::warning("unexpected state - ", unsigned(_state));
				_state = WAIT_FOR_DATA_UNAVAILABLE;
			}
		}

		bool _ping(Genode::Constructible<Top::File> &file,
		           char const * const name, char const * const namex)
		{
			try {
				File_system::Node_handle node   = _fs.node(namex);
				File_system::Status      status = _fs.status(node);
				_fs.close(node);

//				Genode::log(namex, " size=", status.size);

				if (!file.constructed()) {
					try {
						file.construct(_fs, name, _packet_max);
						Genode::log("opening ", name, " fs=", file->file_handle());
					} catch (...) { warning(name, " not available"); };
				}

				if (file.constructed() && file->update_fs_size(status.size))
					return true;
			} catch (...) { }

			return false;
		}

	public:

		Storage(Env &env, T &notify) : env(env), _notify(notify)
		{
			for (unsigned i = 0; i < sizeof(_id_req) / sizeof(_id_req[0]); i++) {
				_id_req[i] = Trace::Subject_id(INVALID_ID);
				_id_round[i] = Trace::Subject_id(INVALID_ID);
			}

			_fs.sigh(_handler);

			_fs.watch("/");
		}

		void ping()
		{
			bool fs = false;

			if (_ping(_subject, "subject.top_view", "/subject.top_view")) fs = true;
			if (_ping(_select, "select.top_view", "/select.top_view")) fs = true;

			if (_ping(_data, "data.top_view", "/data.top_view")) {
				/* if reading data stopped due to not available data, start again */
				if (_data.constructed() && _state == WAIT_FOR_DATA_UNAVAILABLE) {
					if (_data->read(*_fs.tx()))
						_state = READ_DATA;
				}

				fs = true;
			}

			if (fs) {
				/* subject and data read will be triggered by _select if required */
				if (_select.constructed())
					_select->read(*_fs.tx());
			}
		}
};
