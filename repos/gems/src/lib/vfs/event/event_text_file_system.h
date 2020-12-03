/*
 * \brief  Event file system (text interface)
 * \author Christian Prochaska
 * \date   2020-11-26
 */

/*
 * Copyright (C) 2020 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _EVENT_TEXT_FILE_SYSTEM_H_
#define _EVENT_TEXT_FILE_SYSTEM_H_

/* Genode includes */
#include <event_session/connection.h>
#include <util/utf8.h>
#include <vfs/single_file_system.h>


class Event_text_file_system : public Vfs::Single_file_system
{
	private:

		Event::Connection _event;

		class Event_vfs_handle : public Single_vfs_handle
		{
			private:
				Event::Connection &_event;

			public:

				Event_vfs_handle(Directory_service &ds,
				                 File_io_service   &fs,
				                 Genode::Allocator &alloc,
				                 Event::Connection &event)
				: Single_vfs_handle(ds, fs, alloc, 0),
				  _event(event) { }

				Read_result read(char *dst, Vfs::file_size count,
				                 Vfs::file_size &out_count) override
				{
					return READ_ERR_IO;
				}

				Write_result write(char const *src, Vfs::file_size count,
				                   Vfs::file_size &out_count) override
				{
					out_count = 0;

					for (Genode::Utf8_ptr utf8(src);
					     (utf8.complete() &&
					      (out_count + utf8.length()) <= count);
					     utf8 = utf8.next()) {

						_event.with_batch([&] (Event::Session_client::Batch &batch) {
							batch.submit(Input::Press_char{Input::KEY_UNKNOWN,
							                               utf8.codepoint()});
							batch.submit(Input::Release{Input::KEY_UNKNOWN});
						});

						out_count += utf8.length();
					}

					return WRITE_OK;
				}

				bool read_ready() { return true; }
		};

	public:

		Event_text_file_system(Genode::Env &env)
		:
			Single_file_system(Vfs::Node_type::CONTINUOUS_FILE, name(),
			                   Vfs::Node_rwx::wo(), "<text/>"),
			_event(env)
		{ }

		static char const *name()   { return "text"; }
		char const *type() override { return "text"; }

		/*********************************
		 ** Directory service interface **
		 *********************************/

		Open_result open(char const         *path, unsigned,
		                 Vfs::Vfs_handle   **out_handle,
		                 Genode::Allocator  &alloc) override
		{
			if (!_single_file(path))
				return OPEN_ERR_UNACCESSIBLE;

			*out_handle = new (alloc)
				Event_vfs_handle(*this, *this, alloc, _event);

			return OPEN_OK;
		}

		/********************************
		 ** File I/O service interface **
		 ********************************/

		Ftruncate_result ftruncate(Vfs::Vfs_handle *, Vfs::file_size size) override
		{
			if (size == 0)
				return FTRUNCATE_OK;

			return FTRUNCATE_ERR_NO_PERM;
		}
};

#endif /* _EVENT_FILE_SYSTEM_H_ */
