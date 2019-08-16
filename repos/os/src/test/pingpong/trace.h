/*
 * \brief  Trace support
 * \author Alexander Boettcher
 * \date   2019-08-06
 */

/*
 * Copyright (C) 2019 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#pragma once

#include <base/attached_dataspace.h>
#include <rom_session/connection.h>
#include <trace/timestamp.h>
#include <trace_session/connection.h>

namespace Test
{
	using namespace Genode;
	class Tracing;
};

struct Test::Tracing
{
	enum {
		TRACE_RAM_QUOTA = 32 * 4096,
		ARG_BUFFER_RAM  =  8 * 1024,
		PARENT_LEVELS   =  0,
		MAX_SUBJECTS    = 32
	};

	Env               &_env;
	Trace::Connection  _trace     { _env, TRACE_RAM_QUOTA,
	                                ARG_BUFFER_RAM, PARENT_LEVELS };
	Trace::Policy_id   _policy_id { };
	Trace::Subject_id  _subjects  [MAX_SUBJECTS];
	unsigned           _subject_count { 0 };
	Trace::Subject_id  _caller_id { };
	Trace::Subject_id  _callee_id { };

	Tracing(Env &env) : _env(env)
	{
		String<32> policy_module ("rpc_tsc");

		Rom_connection rom(_env, policy_module.string());

		uint64_t rom_size = Dataspace_client(rom.dataspace()).size();

		_policy_id = _trace.alloc_policy(rom_size);
		Dataspace_capability ds_cap = _trace.policy(_policy_id);

		Attached_dataspace ram_ds (_env.rm(), ds_cap);
		Attached_dataspace rom_ds (_env.rm(), rom.dataspace());

		memcpy(ram_ds.local_addr<void>(), rom_ds.local_addr<void>(), rom_size);

		_subject_count = _trace.subjects(_subjects, MAX_SUBJECTS);
	}

	Genode::Trace::Execution_time idle_time()
	{
		Genode::Affinity::Location const affinity = Genode::Thread::myself()->affinity(); 

		/* add and update existing entries */
		for (unsigned i = 0; i < _subject_count; i++) {
			Genode::Trace::Subject_info const info (_trace.subject_info(_subjects[i]));
			if (info.thread_name() == "idle" &&
			    info.session_label() == "kernel" &&
			    affinity.xpos() == info.affinity().xpos() &&
			    affinity.ypos() == info.affinity().ypos())
				return info.execution_time();
		}

		Genode::warning("no trace idle times available");

		return Genode::Trace::Execution_time {};
	}

	void start_tracing()
	{
		/* call it again to get call_me subject in, which could be not avail in constructor */
		_subject_count = _trace.subjects(_subjects, MAX_SUBJECTS);

		for (unsigned i = 0; i < _subject_count; i++) {
			Genode::Trace::Subject_info const info (_trace.subject_info(_subjects[i]));
			if (!(info.session_label() == "init -> pingpong"))
				continue;

			if (info.thread_name() == "call_me") {
				_callee_id = _subjects[i];
				_trace.trace(_subjects[i], _policy_id, 16 * 1024);
			}

			if (info.thread_name() == "ep") {
				_caller_id = _subjects[i];
				_trace.trace(_subjects[i], _policy_id, 16 * 1024);
			}

			if (_caller_id.id && _callee_id.id)
				break;
		}

		if (!_caller_id.id || !_callee_id.id) {
			Genode::error("caller or callee not found as trace subject");
			Genode::Lock lock;
			while (true) lock.lock();
		}
	}

	void stop_tracing()
	{
		for (unsigned i = 0; i < _subject_count; i++) {
			Genode::Trace::Subject_info const info (_trace.subject_info(_subjects[i]));
			if (!(info.session_label() == "init -> pingpong"))
				continue;

/*
			Genode::log(info.thread_name(), " ",
			            Genode::Trace::Subject_info::state_name(info.state()),
			            " ", _subjects[i].id);
*/

			if (info.state() == Genode::Trace::Subject_info::TRACED)
				_trace.pause(_subjects[i]);
		}
	}

	template <typename FN>
	void for_each(Trace::Buffer &buffer_caller, Trace::Buffer &buffer_callee,
	              Genode::Trace::Timestamp const tsc_start,
	              Genode::Trace::Timestamp const *times,
	              unsigned const times_cnt, unsigned const skip_first,
	              FN const &fn)
	{

		unsigned entry_i = 0;

		Trace::Buffer::Entry entry_caller  { buffer_caller.first() };
		Trace::Buffer::Entry entry_callee  { buffer_callee.first() };

		for (; !entry_caller.last(); entry_caller = buffer_caller.next(entry_caller)) {
			if (!entry_caller.length()) break;

			Trace::Timestamp tsc_call_start = 0;
			Trace::Timestamp tsc_call_end   = 0;
			Trace::Timestamp tsc_dis_start  = 0;
			Trace::Timestamp tsc_dis_end    = 0;

			if (!_evaluate_entry(entry_caller, 'C', tsc_call_start))
				/* on caller side other RPC is tracked which we aren't interested in */
				continue;

			entry_caller = buffer_caller.next(entry_caller);
			if (!_evaluate_entry(entry_caller, 'R', tsc_call_end)) {
				Genode::error("parsing error ", __LINE__);
				return;
			}

			if (!entry_callee.length()) {
				Genode::error("parsing error A");
				return;
			}

			if (!_evaluate_entry(entry_callee, 'D', tsc_dis_start))
				return;

			entry_callee = buffer_callee.next(entry_callee);
			if (!_evaluate_entry(entry_callee, 'Y', tsc_dis_end))
				return;

			Trace::Timestamp const tsc_diff_a = tsc_call_end  - tsc_call_start;
			Trace::Timestamp const tsc_diff_b = tsc_dis_start - tsc_call_start;
			Trace::Timestamp const tsc_diff_c = tsc_dis_end   - tsc_dis_start;
			Trace::Timestamp const tsc_diff_d = tsc_call_end  - tsc_dis_end;
			Trace::Timestamp tsc_diff_e = 0;
			Trace::Timestamp tsc_diff_f = 0;
			if (entry_i >= skip_first) {
				if (entry_i - skip_first < times_cnt) {
					if (times[entry_i - skip_first] < tsc_call_end)
						Genode::error("insane tsc values e");
					else
						tsc_diff_e = times[entry_i - skip_first] - tsc_call_end;
				}
				else
					Genode::error("entry count to big ", entry_i - skip_first,
					              "/", times_cnt);

				if (entry_i > skip_first) {
					if (tsc_call_start < times[entry_i - skip_first - 1])
						Genode::error("insane tsc values f");
					else
						tsc_diff_f = tsc_call_start - times[entry_i - skip_first - 1];
				} else
				if (entry_i == skip_first)
					tsc_diff_f = tsc_call_start - tsc_start;
			}

			fn(tsc_diff_a, tsc_diff_b, tsc_diff_c, tsc_diff_d, tsc_diff_f, tsc_diff_e);

			entry_callee = buffer_callee.next(entry_callee);

			entry_i ++;
		}

		if (entry_caller.length() || entry_callee.length())
			Genode::error("more unevaluated entries !? ", entry_caller.length(), "/", entry_callee.length());
	}

	template <int T>
	Genode::String<T> _align_right(Genode::uint64_t value, bool zero = false)
	{
		Genode::String<T> result(value);

		for (Genode::uint64_t i = 1, pow = 10; i < (T - 1); i++, pow *= 10) {
			if (value < pow)
				result = Genode::String<T>(zero ? "0" : " ", result);
		}

		return result;
	}

	void evaluate_tracing(unsigned const skip_first,
	                      Genode::Trace::Timestamp const *times,
	                      unsigned const times_cnt,
	                      Genode::Trace::Timestamp const time_start)
	{
		Dataspace_capability const ds_cap_caller { _trace.buffer(_caller_id) };
		Attached_dataspace         ram_ds_caller { _env.rm(), ds_cap_caller };
		Trace::Buffer             *buffer_caller { ram_ds_caller.local_addr<Trace::Buffer>() };

		Dataspace_capability const ds_cap_callee { _trace.buffer(_callee_id) };
		Attached_dataspace         ram_ds_callee { _env.rm(), ds_cap_callee };
		Trace::Buffer             *buffer_callee { ram_ds_callee.local_addr<Trace::Buffer>() };

		Trace::Timestamp tsc_sum_a = 0;
		Trace::Timestamp tsc_best_a = ~0ULL;
		Trace::Timestamp tsc_worst_a = 0;

		Trace::Timestamp tsc_sum_b = 0;
		Trace::Timestamp tsc_best_b = ~0ULL;
		Trace::Timestamp tsc_worst_b = 0;

		Trace::Timestamp tsc_sum_c = 0;
		Trace::Timestamp tsc_best_c = ~0ULL;
		Trace::Timestamp tsc_worst_c = 0;

		Trace::Timestamp tsc_sum_d = 0;
		Trace::Timestamp tsc_best_d = ~0ULL;
		Trace::Timestamp tsc_worst_d = 0;

		Trace::Timestamp tsc_sum_e = 0;
		Trace::Timestamp tsc_best_e = ~0ULL;
		Trace::Timestamp tsc_worst_e = 0;

		Trace::Timestamp tsc_sum_f = 0;
		Trace::Timestamp tsc_best_f = ~0ULL;
		Trace::Timestamp tsc_worst_f = 0;

		unsigned         entries = 0;

		for_each(*buffer_caller, *buffer_callee,
		         time_start, times, times_cnt, skip_first,
		         [&] (Trace::Timestamp const tsc_diff_a,
		              Trace::Timestamp const tsc_diff_b,
		              Trace::Timestamp const tsc_diff_c,
		              Trace::Timestamp const tsc_diff_d,
		              Trace::Timestamp const tsc_diff_e,
		              Trace::Timestamp const tsc_diff_f)
		{
			if (entries >= skip_first) {
				tsc_sum_a += tsc_diff_a;
				tsc_sum_b += tsc_diff_b;
				tsc_sum_c += tsc_diff_c;
				tsc_sum_d += tsc_diff_d;
				tsc_sum_e += tsc_diff_e;
				tsc_sum_f += tsc_diff_f;

				if (tsc_diff_a > tsc_worst_a) { /* -a- contains sum of run */
					tsc_worst_a = tsc_diff_a;
					tsc_worst_b = tsc_diff_b;
					tsc_worst_c = tsc_diff_c;
					tsc_worst_d = tsc_diff_d;
					tsc_worst_e = tsc_diff_e;
					tsc_worst_f = tsc_diff_f;
				}

				if (tsc_diff_a < tsc_best_a) { /* -a- contains sum of run */
					tsc_best_a = tsc_diff_a;
					tsc_best_b = tsc_diff_b;
					tsc_best_c = tsc_diff_c;
					tsc_best_d = tsc_diff_d;
					tsc_best_e = tsc_diff_e;
					tsc_best_f = tsc_diff_f;
				}
			}

			entries ++;
		});

		unsigned const count = entries - skip_first;

		Trace::Timestamp const tsc_mid_a = tsc_sum_a / count;
		Trace::Timestamp const tsc_mid_b = tsc_sum_b / count;
		Trace::Timestamp const tsc_mid_c = tsc_sum_c / count;
		Trace::Timestamp const tsc_mid_d = tsc_sum_d / count;
		Trace::Timestamp const tsc_mid_e = tsc_sum_e / count;
		Trace::Timestamp const tsc_mid_f = tsc_sum_f / count;

		Genode::log("\nU->C->D->Y->R->U");
		Genode::log("skip=", skip_first, " entries=", entries);
		Genode::log("rounds ", count, ", cycles ", tsc_sum_a + tsc_sum_e + tsc_sum_f);
		Genode::log("average cyles/count ", (tsc_sum_a + tsc_sum_e + tsc_sum_f) / count);
		Genode::log("");
		Genode::log("sum C->D->Y->R ", _align_right<10>(tsc_sum_a),
			        " C->D ",          _align_right<10>(tsc_sum_b),
			        " D->F->Y ",       _align_right<10>(tsc_sum_c),
			        " Y->R ",          _align_right<10>(tsc_sum_d),
			        " U->C ",          _align_right<10>(tsc_sum_e),
			        " R->U ",          _align_right<10>(tsc_sum_f));
		Genode::log("mid C->D->Y->R ", _align_right<10>(tsc_mid_a),
			        " C->D ",          _align_right<10>(tsc_mid_b),
			        " D->F->Y ",       _align_right<10>(tsc_mid_c),
			        " Y->R ",          _align_right<10>(tsc_mid_d),
			        " U->C ",          _align_right<10>(tsc_mid_e),
			        " R->U ",          _align_right<10>(tsc_mid_f));
		Genode::log("bes C->D->Y->R ", _align_right<10>(tsc_best_a),
			        " C->D ",          _align_right<10>(tsc_best_b),
			        " D->F->Y ",       _align_right<10>(tsc_best_c),
			        " Y->R ",          _align_right<10>(tsc_best_d),
			        " U->C ",          _align_right<10>(tsc_best_e),
			        " R->U ",          _align_right<10>(tsc_best_f));
		Genode::log("wor C->D->Y->R ", _align_right<10>(tsc_worst_a),
			        " C->D ",          _align_right<10>(tsc_worst_b),
			        " D->F->Y ",       _align_right<10>(tsc_worst_c),
			        " Y->R ",          _align_right<10>(tsc_worst_d),
			        " U->C ",          _align_right<10>(tsc_worst_e),
			        " R->U ",          _align_right<10>(tsc_worst_f));

		entries = 0;

		for_each(*buffer_caller, *buffer_callee,
		         time_start, times, times_cnt, skip_first,
		         [&] (Trace::Timestamp const tsc_diff_a,
		              Trace::Timestamp const tsc_diff_b,
		              Trace::Timestamp const tsc_diff_c,
		              Trace::Timestamp const tsc_diff_d,
		              Trace::Timestamp const tsc_diff_e,
		              Trace::Timestamp const tsc_diff_f)
		{
			bool show = entries < skip_first;

			unsigned const div_a = tsc_mid_a / 10;
			unsigned const div_b = tsc_mid_b / 10;
			unsigned const div_c = tsc_mid_c / 10;
			unsigned const div_d = tsc_mid_d / 10;
			unsigned const div_e = tsc_mid_e / 10;
			unsigned const div_f = tsc_mid_f / 10;

			bool const err_a = !((tsc_mid_a - div_a <= tsc_diff_a) &&
			                     (tsc_diff_a <= tsc_mid_a + div_a));
			bool const err_b = !((tsc_mid_b - div_b <= tsc_diff_b) &&
			                     (tsc_diff_b <= tsc_mid_b + div_b));
			bool const err_c = !((tsc_mid_c - div_c <= tsc_diff_c) &&
			                     (tsc_diff_c <= tsc_mid_c + div_c));
			bool const err_d = !((tsc_mid_d - div_d <= tsc_diff_d) &&
			                     (tsc_diff_d <= tsc_mid_d + div_d));
			bool const err_e = !((tsc_mid_e - div_e <= tsc_diff_e) &&
			                     (tsc_diff_e <= tsc_mid_e + div_e));
			bool const err_f = !((tsc_mid_f - div_f <= tsc_diff_f) &&
			                     (tsc_diff_f <= tsc_mid_f + div_f));

			show = show || err_a || err_b || err_c || err_d || err_e || err_f;

			entries ++;

			if (!show) return;

			log(_align_right<4>(entries - 1),
			    " C->D->Y->R ", _align_right<10>(tsc_diff_a),
			    " C->D ",       _align_right<10>(tsc_diff_b),
			    " D->F->Y ",    _align_right<10>(tsc_diff_c),
			    " Y->R ",       _align_right<10>(tsc_diff_d),
			    " U->C ",       _align_right<10>(tsc_diff_e),
			    " R->U ",       _align_right<10>(tsc_diff_f),
			    " - ",
			    (entries - 1 >= skip_first) ? "" : "SKIP ",
			    err_a ? "a" : "", err_b ? "b" : "",
			    err_c ? "c" : "", err_d ? "d" : "",
			    err_e ? "e" : "", err_f ? "f" : "");
		});
	}


	bool _evaluate_entry(Trace::Buffer::Entry &entry, char const type,
	                     Trace::Timestamp &tsc)
	{
		if (!entry.length()) return false;

		char const type_e = *reinterpret_cast<char const *>(entry.data());
		String<15> name { reinterpret_cast<char const *>(entry.data() + 1 + sizeof(tsc)) };

		if (name != "pingpong" || type_e != type)
			return false;

		tsc = *reinterpret_cast<Trace::Timestamp const *>(entry.data() + 1);

		return true;
	}
};
