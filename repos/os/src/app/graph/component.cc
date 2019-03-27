/*
 * \brief  Utility to draw graphs in a coordinate system
 * \author Alexander Boettcher
 * \date   2019-03-12
 */

/*
 * Copyright (C) 2019 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#include <base/attached_rom_dataspace.h>
#include <base/component.h>
#include <base/heap.h>

#include <base/trace/types.h>

#include <os/pixel_rgb888.h>
#include <gui_session/connection.h>
#include <util/color.h>

#include <gems/vfs_font.h>

#include "trace.h"
#include "storage.h"

using Genode::Pixel_rgb888;
using Genode::Trace::Subject_id;
using Genode::uint64_t;

class Graph;

enum { MAX_GRAPHS = 8 };

struct Checkpoint
{
	Gui::Point       _points[MAX_GRAPHS];
	uint64_t         _values[MAX_GRAPHS];
	Subject_id       _id[MAX_GRAPHS];
	uint64_t         _time { 0 };
	Genode::uint8_t  _used { 0 };
	bool             _done { false };

	bool unused(unsigned const i) const { return i < MAX_GRAPHS && !_points[i].x() && !_points[i].y(); }
};

typedef Genode::Constructible<Genode::Attached_dataspace> Reconstruct_ds;
typedef Genode::Signal_handler<Graph> Signal_handler;

class Graph
{
	private:

		Genode::Env                     &_env;
		Genode::Heap                     _heap       { _env.ram(), _env.rm() };
		Genode::Attached_rom_dataspace   _config     { _env, "config" };
		Gui::Connection                  _gui        { _env };
		Input::Session_client           &_input      { *_gui.input() };
		Gui::Session::View_handle        _view_all   { _gui.create_view() };
		Gui::Session::View_handle        _view       { _gui.create_view(_view_all) };
		Gui::Session::View_handle        _view_2     { _gui.create_view(_view_all) };
		Gui::Session::View_handle        _view_text  { _gui.create_view(_view_all) };
		Gui::Session::View_handle        _view_scale { _gui.create_view(_view_all) };

		Genode::Avl_tree<Entry>    _entries   { };
		Entry const                _entry_unknown { 0, "unknown", "", "" };

		unsigned       _width        { 1000 };
		unsigned       _height       { 425 };
		unsigned const _x_root       { 50 };
		unsigned const _y_detract    { 25 }; 
		unsigned const _scale_10_len { 10 };
		unsigned const _scale_5_len  { 5 };
		unsigned const _marker_half  { 4 };
		unsigned const _line_half    { 5 };
		unsigned const _invisible    { 300 };
		unsigned       _scale_e      { 2 };
		unsigned const _x_scale      { _x_root + _scale_10_len + 1};
		unsigned const _step_width   { 20 };
		unsigned const _step_dot     { 10 };


		unsigned      _max_width     { _width };
		unsigned      _max_height    { _height };

		unsigned height_mode() const { return _height + _invisible; }
		unsigned y_root()      const { return _height - _y_detract; }
		unsigned scale_e()     const { return _scale_e; }
		unsigned scale_10()    const { return (y_root() - _y_detract/2) / scale_e(); }
		unsigned scale_5()     const { return scale_10() / 2; }

		Signal_handler _signal_input { _env.ep(), *this, &Graph::_handle_input };
		Reconstruct_ds _ds           { };

		Signal_handler _signal_mode { _env.ep(), *this, &Graph::_handle_mode};

		Signal_handler _config_handler = { _env.ep(), *this, &Graph::_handle_config};

		Genode::Color const           _white     { 255, 255, 255 };
		Genode::Color const           _red       { 255,   0,   0 };
		Genode::Color const           _green     {   0, 255,   0 };
		Genode::Color const           _blue      {   0,   0, 255 };
		Genode::Color const           _black     {   0,   0,   0 };

		Checkpoint                    _column[256];
		static unsigned constexpr     _column_max { sizeof(_column) / sizeof(_column[0]) };
		unsigned                      _column_warp { 0 };
		unsigned                      _column_offset { 0 };
		Genode::uint16_t              _column_cur { 0 };
		Genode::uint16_t              _column_last { 0 };
		Genode::uint16_t              _sliding_offset { 0 };
		bool                          _sliding { false };
		bool                          _verbose { false };
		unsigned                      _hovered_vline { ~0U };
		uint64_t                      _time_storage_wait_for { 0 };
		uint64_t                      _freq_khz { 2000000 };

		Genode::Root_directory _root { _env, _heap, _config.xml().sub_node("vfs") };
		Genode::Vfs_font       _font { _heap, _root, "fonts/monospace/regular" };

		Genode::Constructible<Top::Storage<Graph>> _storage { };

		/**
		 * GUI data initialisation
		 */
		Genode::Dataspace_capability _setup(int width, int height)
		{
			using namespace Gui;

			Framebuffer::Mode const mode { Area(width, height_mode()) };

			_gui.buffer(mode, false /* no alpha */);

			Point const p_start(0,0);

			Rect const r_all(p_start, Area(width, height));
			_gui.enqueue<Session::Command::Geometry>(_view_all, r_all);

			Rect const r_view(Point(_x_root + 1, 0), Area(width - _x_root - 1, height));
			_gui.enqueue<Session::Command::Geometry>(_view, r_view);

			Rect const r_view2(Point(_x_root + _step_width + 1, 0), Area(width - _step_width - _x_root - 1, height));
			_gui.enqueue<Session::Command::Geometry>(_view_2, r_view2);

			_gui.enqueue<Session::Command::Offset>(_view, Point(-_x_root - 1, 0));
			_gui.enqueue<Session::Command::Offset>(_view_2, Point(_width, 0));

			Rect const r_scale(p_start, Area(_x_scale, height));
			_gui.enqueue<Session::Command::Geometry>(_view_scale, r_scale);

			_gui.enqueue<Session::Command::To_front>(_view, _view_2);
			_gui.enqueue<Session::Command::To_front>(_view_scale, _view);
			_gui.execute();

			return _gui.framebuffer()->dataspace();
		}

		Signal_handler _graph_handler { _env.ep(), *this, &Graph::_handle_graph};
		Genode::Attached_rom_dataspace _graph { _env, "graph" };

		void _handle_input();
		void _handle_graph();
		void _handle_mode();
		void _handle_data();
		void _handle_config();

		Genode::Color _color(unsigned i)
		{
			i %= MAX_GRAPHS;
			switch (i) {
			case 0: return _red;
			case 1: return _green;
			case 2: return _blue;
			case 3: return Genode::Color {   0, 255, 255 };
			case 4: return Genode::Color { 255,   0, 255 };
			case 5: return Genode::Color { 255,   0, 128 };
			case 6: return Genode::Color { 255, 128,   0 };
			default:
				return Genode::Color { 255, 255,   0 };
			}
		}

		unsigned _sliding_size() const {
			return ((_width - _x_root - _line_half) / _step_width) - 1; }

		void _init_screen(bool reset_points = true)
		{
			/* vertical line and scale */
			vline(_x_root, _white);

			for (unsigned i = y_root() - scale_10() * scale_e(); i < y_root(); i += scale_10()) {

				Gui::Point point(_x_root, i);
				hline(point, _scale_10_len, _white, true);
				hline_dotted(point, _width - _x_root, _white, _step_dot);

				Genode::String<4> text(((y_root() - i) / scale_10()) * 10);

				auto const text_size = _font.bounding_box().w() *
				                       (text.length() - 1) + _scale_10_len;
				int xpos = 0;
				int ypos = 0;
				if (_x_root > text_size) xpos = int(_x_root - text_size);
				if (i > _font.height() / 2) ypos = i - _font.height() / 2;

				_text(text.string(), Text_painter::Position(xpos, ypos), _white);

				Gui::Point point_5(_x_root, i + scale_10() - scale_5());
				hline(point_5, _scale_5_len, _white, true);
				hline_dotted(point_5, _width - _x_root, _white, _step_dot);
			}

			/* horizontal line */
			hline(y_root() , _white);

			if (reset_points) {
				Genode::memset(_column, 0, sizeof(_column));
				_column_cur  = 0;
				_column_last = _column_cur;
				_column_offset = 0;

				_sliding_offset = 0;
				_sliding = false;
			}
		}

		Genode::String<8> _percent(uint64_t percent, uint64_t rest) {
			return Genode::String<8> (percent < 10 ? "  " : (percent < 100 ? " " : ""),
			                          percent, ".", rest < 10 ? "0" : "", rest, "%");
		}

		Entry * find_by_id(Subject_id const id) {
			Entry * entry = _entries.first();
			if (entry)
				entry = entry->find_by_id(id);
			return entry;
		}

	public:

		Graph(Genode::Env &env) : _env(env)
		{
			Genode::memset(_column, 0, sizeof(_column));

			_graph.sigh(_graph_handler);
			_gui.mode_sigh(_signal_mode);
			_input.sigh(_signal_input);
			_config.sigh(_config_handler);

			_handle_config();
		}

	private:

		Pixel_rgb888 * _pixel(Gui::Point const &p) {
			return _ds->local_addr<Pixel_rgb888>() + p.y() * _width + p.x(); }

		Pixel_rgb888 * _pixel(int x, int y) {
			return _ds->local_addr<Pixel_rgb888>() + y * _width + x; }

		void hline(unsigned const y, Genode::Color const &color)
		{
			Pixel_rgb888 * pixel = _pixel(0, y);

			for (unsigned i = 0; i < _width; i++)
				*(pixel + i) = Pixel_rgb888(color.r, color.g, color.b, color.a);
		}

		void hline(Gui::Point const &point, int len,
		           Genode::Color const &color, bool half = false)
		{
			Pixel_rgb888 * pixel = _pixel(point);

			for (int i = -len; i <= (half ? 0 : len); i++)
				*(pixel + i) = Pixel_rgb888(color.r, color.g, color.b, color.a);
		}

		void hline_dotted(Gui::Point const point, unsigned const len,
		                  Genode::Color const &color, unsigned const step)
		{
			Pixel_rgb888 * pixel = _pixel(point);

			for (unsigned i = 0; i < len; i += step)
				*(pixel + i) = Pixel_rgb888(color.r, color.g, color.b, color.a);
		}

		void vline(unsigned const x, Genode::Color const &color)
		{
			Pixel_rgb888 * pixel = _pixel(x, 0);
			for (unsigned i = 0; i < _height; i ++)
				*(pixel + i * _width) = Pixel_rgb888(color.r, color.g,
				                                     color.b, color.a);
		}

		void _reset_column(unsigned const x1, unsigned const x2,
		                   Genode::Color const &color)
		{
			using Gui::Point;

			Pixel_rgb888 * pixel = _pixel(0, 0);
			unsigned y_dot10 = y_root() - scale_10() * scale_e();

			for (unsigned y = 0; y < _height; y ++) {
				for (unsigned x = x1; x <= x2; x ++) {
					*(pixel + x + y * _width) = Pixel_rgb888(color.r, color.g,
					                                         color.b, color.a);
				}

				unsigned x = _x_root + ((x1 - _x_root) / _step_dot) * _step_dot;
				if (x < x1) x += _step_dot;

				if (y == y_dot10 - scale_5()) {
					Point point_5(x, y);
					hline_dotted(point_5, x2 - x + 1, _white, _step_dot);
				}

				if (y == y_dot10) {
					Point point(x, y_dot10);
					hline_dotted(point, x2 - x + 1, _white, _step_dot);

					if (y_dot10 < y_root()) y_dot10 += scale_10();
				}

				if (y == y_root())
					hline(Point(x1, y), x2 - x1 + 1, _white);
			}
		}

		void _text(char const * const text, Text_painter::Position const pos,
		           Genode::Color const &color)
		{
			Genode::Surface_base::Area const size { _width, height_mode() };
			Genode::Surface<Pixel_rgb888> surface { _ds->local_addr<Pixel_rgb888>(), size };
			Text_painter::paint(surface, pos, _font, color, text);
		}

		void _hover_entry(unsigned const hover_line, Genode::Color const &color)
		{
			bool const split_hover = _sliding && (hover_line == _sliding_size());
			unsigned const x = _x_root + (1 + hover_line) * _step_width;
			unsigned const x1 = x - _line_half;
			unsigned const x2 = split_hover ? _x_root + _line_half : x + _line_half;

			vline(x1, color);
			vline(x2, color);

			_gui.framebuffer()->refresh(x1, 0, 1, _height);
			_gui.framebuffer()->refresh(x2, 0, 1, _height);
		}

		void marker(Gui::Point const point, int len,
		            Genode::Color const &color)
		{
			Pixel_rgb888 * pixel = _ds->local_addr<Pixel_rgb888>()
			                       + point.y() * _width + point.x();

			Pixel_rgb888 const dot(color.r, color.g, color.b, color.a);
/*
			for (int y = -len; y <= len; y++) {
				*(pixel + y + y * _width) = dot;
				*(pixel - y + y * _width) = dot;
			}
*/
			for (int y = -len; y <= len; y++) {
				*(pixel + y) = dot;
			}
		}

		void marker(Gui::Point const fr, Gui::Point const to,
		            Genode::Color const &color)
		{
			Pixel_rgb888 *p_f = _pixel(fr);

			Pixel_rgb888 const dot(color.r, color.g, color.b, color.a);

			int w = to.x() - fr.x() + 1;
			if (w <= 0) {
				w = to.x() - _x_root;
				p_f = _pixel(_x_root + 1, fr.y());
			}

			int const height = to.y() - fr.y();
			int const h = (height < 0) ? height - 1 : height + 1;
			int const start = (height < 0) ? h : 0;
			int const end = (height < 0) ? 0 : h;
			int const f = h / w;

			if (height == 0) {
				for (int x = 0; x < w; x++)
					*(p_f + x) = dot;
			} else if (f == 0) {
				int const b = w / h;
				int const r = w % h;
				for (int y = start; y < end; y++) {
					for (int x = 0; x < w; x++) {
						int const o = r * y / h;

						if (y == (x - o) / b) {
							/* int() required when y negative - wrong calculation otherwise */
							*(p_f + x + y * int(_width)) = dot;
						}
					}
				}
			} else {
				int const r = (h % w);
				for (int y = start; y < end; y++) {
					for (int x = 0; x < w; x++)
					{
						int const o = r * x / w;
						int s, e;

						if (height < 0) {
							s = o + f * (x + 1) + 1;
							e = o + f * x;
							if (x == w - 1) s = start + 1;
						} else {
							s = o + f * x;
							e = o + f * (x + 1) - 1;
							if (x == w - 1) e = end - 1;
						}

						if (s <= y && y <= e) {
							/* int() required when y negative - wrong calculation otherwise */
							*(p_f + x + y * int(_width)) = dot;
						}
					}
				}
			}
		}

		Gui::Point _data(unsigned const time, unsigned const graph) const {
			return _column[time]._points[graph]; }

		uint64_t _value(unsigned const time, unsigned const graph) const {
			return _column[time]._values[graph]; }

		Subject_id _subject_id(unsigned const time, unsigned const graph) const {
			return _column[time]._id[graph]; }

		void _replay_data();
		bool _apply_data(Subject_id, uint64_t);

		Gui::Point _apply_data_point(uint64_t value, unsigned element);

		unsigned _graph_pos(unsigned element) const
		{
			return (_column_offset + element) % (_sliding_size() + 1);
		}

		void _slide();

		Checkpoint &prev_entry()
		{
			unsigned const prev = _column_cur ? _column_cur - 1
			                                  : _column_max - 1;
			return _column[prev];
		}

		void _advance_element_column(uint64_t const time) {

			if (_column[_column_cur]._used) {
				_column[_column_cur]._done = true;

				if (_column_cur) {
					Checkpoint &data_prev = prev_entry();
					Checkpoint &data = _column[_column_cur];

					/* end marker for entries which are not continued */
					for (unsigned i = 0; i < MAX_GRAPHS; i++) {
						if (data_prev.unused(i)) continue;

						bool end_marker = true;
						for (unsigned j = 0; j < MAX_GRAPHS; j++) {
							if (data.unused(j)) continue;
							if (data_prev._id[i] == data._id[j]) {
								end_marker = false;
								break;
							}
						}
						if (end_marker) {
							Genode::Color const color = _color(i);
							marker(data_prev._points[i], _marker_half, color);
						}
					}
				}

				/* next column */
				_column_cur = (_column_cur + 1) % _column_max;
				Genode::memset(&_column[_column_cur], 0, sizeof(_column[0])); /* XXX better use constructor ? */

				if (_column_cur == 0) {
					_column_warp ++;
					if (_verbose)
						Genode::log(_column_warp, ". column warp");
					_column_offset += _column_max % (_sliding_size() + 1);
				}

				if (!_sliding && _column_cur >= _sliding_size()) {
					if (_verbose)
						Genode::log("sliding starts ", _column_cur);
					_sliding = true;
					_sliding_offset = 0;
				}

				if (_sliding && _graph_pos(_column_cur) == 0) {
					if (_verbose)
						Genode::log("graph wrap ", _column_cur);
					_sliding_offset = 0;
				}

				if (_sliding) _slide();

				/* reset screen check - XXX scrolling would be nice */
#if 0
				Gui::Point point = _apply_data_point(0, _column_cur);
				if (!point.x() && !point.y()) {
					/* wrap to next column */
					_column_cur = 0;
					_scale_e = 2;

					for (; Entry * entry = _entries.first(); ) {
					    _entries.remove(entry);
					    Genode::destroy(_heap, entry);
					}

					Genode::memset(_ds->local_addr<void>(), 0, _width * _height * sizeof(Pixel_rgb888));
					_init_screen();
					_gui.framebuffer()->refresh(0, 0, _width, _height);
				}
#endif
			}

			_column[_column_cur]._time = time;
			_column[_column_cur]._done = false;
		}

		uint64_t _time(unsigned const pos) const {
			return _column[pos]._time; }

public:

		bool advance_column_by_storage(uint64_t const time);

		uint64_t time() const { return _column[_column_cur]._time; }

		/*
		 * Used also by storage layer
		 */

		bool new_data(unsigned long long, unsigned const, unsigned long long);

		bool id_available(Subject_id const id)
		{
			Entry * entry = _entries.first();
			if (entry)
				entry = entry->find_by_id(id);
			return entry != nullptr;
		}

		void add_entry(Subject_id const id, Genode::Session_label const &label,
		               Genode::Trace::Thread_name const &thread,
		               Genode::String<12> const &cpu)
		{
			if (id_available(id)) return;

			_entries.insert(new (_heap) Entry(id, thread, label, cpu));
		}
};

void Graph::_handle_config()
{
	_config.update();

	if (!_config.valid()) return;

	bool const store = _config.xml().attribute_value("store", false);
	_verbose = _config.xml().attribute_value("verbose", _verbose);
	_freq_khz = _config.xml().attribute_value("freq_khz", 2000000ULL);
	if (!_freq_khz) _freq_khz = 1;

	Genode::log("config: freq_khz=", _freq_khz,
	            store ? " storage" : "",
	            _verbose ? " verbose" : "");

	if (store && !_storage.constructed())
		_storage.construct(_env, *this);
	if (!store && _storage.constructed())
		_storage.destruct();
}

void Graph::_handle_mode()
{
	Framebuffer::Mode const mode = _gui.mode();

	if (mode.area.w() == _width && mode.area.h() == _height)
		return;

	if (mode.area.w() < 100 || mode.area.h() < 100)
		return;

	if (mode.area.w() * mode.area.h() > _max_width * _max_height) {
		unsigned diff = (mode.area.w() * mode.area.h() -
		                 _max_width * _max_height) * sizeof(Pixel_rgb888);
		if (diff > _env.pd().avail_ram().value + 0x2000) {
			Genode::warning("no memory left for mode change - ",
			                _width, "x", _height, " -> ",
			                mode.area.w(), "x", mode.area.h(), " - ",
			                _env.pd().avail_ram(), " (available) < ",
			                diff, " (required)");
			return;
		}
		_max_width  = _width;
		_max_height = _height;
	}

	_width = mode.area.w();
	_height = mode.area.h();

	if (!_ds.constructed())
		return;

	_ds.destruct();
	_ds.construct(_env.rm(), _setup(_width, _height));

	Genode::memset(_ds->local_addr<void>(), 0, _width * _height * sizeof(Pixel_rgb888));
	/* XXX - column calculation in hovered line is off when not reseting -> _column_offset ? */
	_init_screen(_sliding);
	_replay_data();
	_gui.framebuffer()->refresh(0, 0, _width, _height);
}

void Graph::_slide()
{
	if (!_sliding) return;

//	if (_sliding_offset > 1) return;

	/* clear old graphic content */
	unsigned const x = _apply_data_point(10 /* does not matter value */, _graph_pos(_column_cur)).x();
	if (x < _step_width)
		Genode::error("x < _step_width ", x);
	else {
//		Genode::log("reset ", x - _step_width + 1, "->", Genode::min(x + _step_width - _line_half - 1, _width - 1));
		_reset_column(x - _step_width + 1,
		              Genode::min(x + _step_width - _line_half - 1, _width - 1),
		              _black);
	}

	using Gui::Rect;
	using Gui::Area;
	using Gui::Point;

	Point const p_view2(-_x_root - 1 + (_sliding_size() - _sliding_offset - 1) * _step_width, 0);
	_gui.enqueue<Gui::Session::Command::Offset>(_view_2, p_view2);
	_gui.enqueue<Gui::Session::Command::To_front>(_view_2, _view_all);

	/* XXX - w/o marker_half on replay the last element is only half the value */
	/* XXX   w   marker_half the connecting lines are distracted -> marker code would need adjustments */
	Rect const r_view(Point(_x_root + 1, 0), Area((_sliding_size() - _sliding_offset) * _step_width /* + _marker_half */, _height));
	_gui.enqueue<Gui::Session::Command::Geometry>(_view, r_view);

	_sliding_offset ++;

	_gui.enqueue<Gui::Session::Command::Offset>(_view, Point(-_x_root - 1 - _sliding_offset * _step_width, 0));
	_gui.enqueue<Gui::Session::Command::To_front>(_view, _view_all);
	_gui.enqueue<Gui::Session::Command::To_front>(_view, _view_2);
	_gui.enqueue<Gui::Session::Command::To_front>(_view_scale, _view_2);

	_gui.execute();

	/* refresh will be triggered by handle_data */
}

Gui::Point Graph::_apply_data_point(uint64_t const value, unsigned element)
{
	/* use all possible pixels by applying adaptive factor */
	unsigned factor = 1;
	if ((scale_5() / 5) > 1)
		factor = scale_5() / 5;

	unsigned percent = unsigned(value / 100 * factor);
	if (percent > (scale_e() * 10) * factor)
		percent = (scale_e() * 10) * factor + _y_detract / 3; /* show bit above upper line */

	unsigned const f10 = 10 * factor;
	unsigned const f5  =  5 * factor;

	/* 10 base */
	unsigned y = scale_10() * (percent / f10);
	/* 5 base */
	y += scale_5() * ((percent % f10) / f5);
	/* rest */
	y += (scale_5() / f5) * (percent % f5);

	unsigned x1 = _x_root + (((element + 1)*_step_width) % (_width - _x_root));
	unsigned x2 = _x_root + (((element + 0)*_step_width) % (_width - _x_root));
	if (x2 > x1) {
		Genode::warning("x2 > x1 point ... ?! XXX");
		return Gui::Point(_x_root, 0);
	}
	return Gui::Point(x1, y_root() - y);
}

void Graph::_replay_data()
{
	for (unsigned i = 0; i <= (_sliding ? _sliding_size() : _column_cur); i++)
	{
		unsigned pos = i;
		if (_sliding) {
			if (_column_cur > _sliding_size())
				pos = _column_cur - _sliding_size() + i;
			else
				pos = (_column_max - _sliding_size() + _column_cur + i) % _column_max;
		}

		for (unsigned graph = 0; graph < MAX_GRAPHS; graph++)
		{
			if (_column[pos].unused(graph)) continue;

			Genode::Color color = _color(graph);
			unsigned const element = _graph_pos(pos);
			Gui::Point point = _apply_data_point(_value(pos, graph), element);

			marker(point, _marker_half, color);

			_column[pos]._points[graph] = point;
		}
	}
}

bool Graph::_apply_data(Subject_id const id, uint64_t const value)
{
	unsigned const element = _graph_pos(_column_cur);
	Gui::Point const point = _apply_data_point(value, element);
	Checkpoint &data = _column[_column_cur];

	unsigned entry = MAX_GRAPHS;
	unsigned same = MAX_GRAPHS;
	Checkpoint &data_prev = prev_entry();

	{
		unsigned free = 0;
		bool fix = false;

		/* find free entry and lookup if used previously */
		for (unsigned i = 0; i < MAX_GRAPHS; i++) {
			if (data_prev._id[i] == id) same = i;

			/* may happen if scale is re-created in while loop and data is tried to apply again */
			if (!data.unused(i) && data._id[i] == id)
				return false;

			if (data.unused(i)) {
				free ++;
				if (data_prev._id[i] == id) { entry = i; fix = true; }
				if (entry >= MAX_GRAPHS) { entry = i; }
				if (!fix && data_prev.unused(i)) { entry = i; fix = true; }
			} else {
				/* may happen due to reading from rom and from storage */
				if (data._id[i] == id) { entry = i; fix = true; }
			}
		}

		if ((entry < MAX_GRAPHS) && (same < MAX_GRAPHS) && (entry != same)) {
			/* try to get same position, if enough free entries */
			if (free > 1 && !fix) {
				if (_verbose)
					Genode::log("move ", free, " same=", same, " entry=", entry);
				for (unsigned i = 0; i < MAX_GRAPHS; i++) {
					if (!data.unused(i)) continue;
					if (i == entry) continue;

					data._points[i] = data._points[same];
					data._values[i] = data._values[same];
					data._id[i] = data._id[same];

					entry = same;
					break;
				}
			}
		}
	}

	if (entry >= MAX_GRAPHS) return false;

	data._points[entry] = point;
	data._values[entry] = value;
	data._id[entry] = id;
	data._used ++;

	{
		Genode::Color const color = _color(entry);

		if (same < MAX_GRAPHS)
			marker(data_prev._points[same], point, color);
		else
			marker(point, _marker_half, color);
	}

	return true;
}

bool Graph::advance_column_by_storage(uint64_t const time)
{
	if (time > this->time()) {
//		if (_sliding_offset >= 2) return true;
		_advance_element_column(time);
	}

	if (time == _time_storage_wait_for) {
		/* read enough from storage file -> trigger graphical update */
		_handle_data();
		return false;
	}

	return true;
}

bool Graph::new_data(uint64_t const value, unsigned const id, uint64_t const tsc)
{
	if (tsc < time()) return false;

	if (tsc > time())
		_advance_element_column(tsc);

	if (id == Top::Storage<Graph>::INVALID_ID) return false;
	if (_column[_column_cur]._done) return false;

	Subject_id const subject {id};
	return _apply_data(subject, value);
}

void Graph::_handle_graph()
{
#if 0
	static bool done = false;

	Genode::warning("_handle_graph ", _column_cur, " ", _sliding_offset);

	if (done) return;

	if (_sliding) {
		Genode::memset(_ds->local_addr<void>(), 0, _width * _height * sizeof(Pixel_rgb888));
		_init_screen(false);
		_replay_data();
		_gui.framebuffer()->refresh(0, 0, _width, _height);
		done = true;
		//return;
	}
#endif

	_graph.update();
	if (!_graph.valid())
		return;

	/* no values - no graph view */
	if (!_graph.xml().num_sub_nodes()) {
		/* destruct current ds until new data arrives */
		if (_ds.constructed()) {
			_ds.destruct();
			_setup(0, 0);
		}
		return;
	}

	/* new data means new graph if not setup currently */
	if (!_ds.constructed()) {
		_ds.construct(_env.rm(), _setup(_width, _height));
		_init_screen();
		_gui.framebuffer()->refresh(0, 0, _width, _height);
	}

	if (_storage.constructed()) {
		Genode::Xml_node node = _graph.xml().sub_node("entry");
		unsigned long long const tsc = node.attribute_value("tsc", 0ULL);

		if (time() < tsc) {
			_time_storage_wait_for = tsc;

//			Genode::error(Genode::Hex(time()), " < ", Genode::Hex(tsc));
			_storage->ping();
			return;
		}
	}

	_handle_data();
}

void Graph::_handle_data()
{
//	if (_sliding_offset >= 2) return;

//	Genode::warning("_handle_data ", _column_last, "->", _column_cur);

	bool scale_update = false;
	bool refresh_all = false;

	do {
		if (scale_update) {
			Genode::memset(_ds->local_addr<void>(), 0, _width * _height * sizeof(Pixel_rgb888));
			_init_screen(false);
			_replay_data();
			refresh_all = true;
		}

		scale_update = false;
		unsigned data_cnt = 0;
		unsigned scale_above = 0;

		_graph.xml().for_each_sub_node("entry", [&](Genode::Xml_node &node){
			/* stop processing if we get too many entries we can't consume */
			if (data_cnt >= MAX_GRAPHS)
				return;

			unsigned value = 0;
			unsigned id = 0;
			uint64_t tsc = 0;

			node.attribute("value").value(value);
			node.attribute("id").value(id);
			node.attribute("tsc").value(tsc);

			{
				Entry * entry = find_by_id(id);
				/* XXX - read out from storage if available */
				if (!entry) {
					Genode::String<12> cpu;
					Genode::String<64> session_label;
					Genode::Trace::Thread_name thread_name;

					node.attribute("cpu").value(cpu);
					node.attribute("label").value(session_label);
					node.attribute("thread").value(thread_name);

					Genode::Session_label const label(session_label);
					add_entry(id, label, thread_name, cpu);
				}
			}

			if (new_data(value, id, tsc))
				data_cnt ++;

			/* XXX heuristic when do re-create scale */
			if (value / 100 > _scale_e * 10) {
				if (value / 100 - _scale_e * 10 <= 15) {
					_scale_e = Genode::min(10U, value / 1000 + 1);
					scale_update = true;
				} else
					scale_above ++;

				if (scale_above > 1) {
					_scale_e = Genode::min(10U, value / 1000 + 1);
					scale_update = true;
				}
			}
		});
	} while (scale_update);

	/* better a function call XXX */
	_column[_column_cur]._done = true;

	unsigned const graph_last = _graph_pos(_column_last);
	unsigned const graph_cur  = _graph_pos(_column_cur);

	_column_last = _column_cur;

	if (graph_last != graph_cur) {
		if (graph_last > graph_cur)
			refresh_all = true;
		/* XXX optimized only for 1 step update */
		if (graph_last < graph_cur && (graph_cur - graph_last > 1))
			refresh_all = true;
	}

	if (refresh_all) {
		_gui.framebuffer()->refresh(0, 0, _width, _height);
		return;
	}

	/* XXX optimize not only for 1 step width update */
	unsigned xpos_s = _apply_data_point(10 /* does not matter value */, graph_last).x();
	unsigned xpos_e = _apply_data_point(10 /* does not matter value */, graph_cur).x();

	_gui.framebuffer()->refresh(xpos_s - _marker_half, 0,
	                            xpos_e - xpos_s + 2 * _marker_half + 1, _height);
}

void Graph::_handle_input()
{
	bool     hovered       = false;
	unsigned hovered_vline = ~0U;
	unsigned hovered_old   = _hovered_vline;
	unsigned last_y        = 0;

	_input.for_each_event([&] (Input::Event const &ev) {
		ev.handle_absolute_motion([&] (int x, int y) {

			/* consume events but drop it if we have no data to show */
			if (!_ds.constructed())
				return;

			last_y = y;

			/* skip */
			if (x < int(_x_root + _step_width - _line_half)) {
				hovered = false;
				return;
			}

			x -= _x_root + _step_width - _line_half;

			int vline = x / _step_width;
			if (x > vline * int(_step_width + 2*_line_half)) {
				hovered = false;
				return;
			}

			hovered_vline = vline;
			hovered = _sliding || (vline <= _column_cur);
		});
	});

	if (hovered) {
		if (_sliding && (hovered_vline + _sliding_offset > _sliding_size()))
			_hovered_vline = hovered_vline + _sliding_offset - _sliding_size() - 1;
		else
			_hovered_vline = hovered_vline + _sliding_offset;
	} else
		_hovered_vline = ~0U;

	if (!hovered && hovered_old == ~0U) return;

	unsigned column = hovered_vline;
	if (_sliding) {
		column = (_column_cur - (_sliding_size() - hovered_vline)) % _column_max;
	}

#if 0
	if (hovered_vline != ~0U && _sliding_offset)
		Genode::error("column=", column, " hovered=", hovered,
		              " sliding_offset=", _sliding_offset,
		              " _hovered_vline=", _hovered_vline,
		              " sliding_size=", _sliding_size());
#endif

	if (!hovered) {
		_hover_entry(hovered_old, _black);

		/* hiding a view would be nice - destroy and re-create */
		using namespace Gui;

		/* HACK - wm does not work properly for destroy_view ... nor to_back XXX */
		Point point(0, _height);
		Rect geometry_text(point, Area { 1, 1 });
		_gui.enqueue<Session::Command::Geometry>(_view_text, geometry_text);
		_gui.execute();
		return;
	}

	if (_hovered_vline == hovered_old) return;

	if (hovered_old != ~0U) {
		_hover_entry(hovered_old, _black);
	}

	unsigned const x = _x_root + (1 + _hovered_vline) * _step_width;

	_hover_entry(_hovered_vline, _white);

	/*
	 * Show details of column, threads etc.
	 */

	/* reset old content */
	memset(_pixel(0,0) + _height * _width, 0,
	       (height_mode() - _height) * _width * sizeof(Pixel_rgb888));

	unsigned text_count = 0;
	unsigned max_len    = 0;
	unsigned skipped    = 0;

	unsigned drop = 0;

	/* show info about timestamp */
	{
		uint64_t const ms = _time(column) / _freq_khz;
		uint64_t const s = ms / 1000;
		uint64_t const m = s / 60;
		Genode::String<16> str_m(m, "min ");
		Genode::String<48> string(m ? str_m : "", s % 60, "s ",
		                          ms % 1000, "ms col=",
		                          column, "/", _column_cur, "/", _column_max, " ",
		                          "slide=", _sliding_offset, "/", _sliding_size());
		max_len = unsigned(Genode::max(string.length(), max_len));

		int const ypos = _height + 5;
		if (ypos + (_font.height() + 5) < 0U + height_mode())
			_text(string.string(), Text_painter::Position(0, ypos), _white);
		else
			drop++;

		text_count ++;
	}

	/* show infos about threads */
	for (unsigned i = 0; i < MAX_GRAPHS; i++) {
		if (_column[column].unused(i)) {
			skipped ++;
			continue;
		}

		Entry const * entry = find_by_id(_subject_id(column, i));
		if (!entry) {
			entry = &_entry_unknown;
			if (_verbose)
				Genode::log("unknown id ", _subject_id(column, i).id);
		}

		unsigned const cmp = entry->cpu().length() > 6 ? 8 : 5;
		Genode::String<128> string(_percent(_value(column, i) / 100,
		                                    _value(column, i) % 100), " ",
		                           entry->cpu().length() < cmp ? " " : "",
		                           entry->cpu(), " ",
		                           entry->thread_name(), ", ",
		                           entry->session_label());

		max_len = unsigned(Genode::max(string.length(), max_len));

		int const ypos = _height + 5 + (i + 1 - skipped) * (_font.height() + 5);
		if (ypos + (_font.height() + 5) < 0U + height_mode())
			_text(string.string(), Text_painter::Position(0, ypos), _color(i));
		else
			drop++;

		text_count ++;
	}

	unsigned const width = Genode::min(_width, (_font.bounding_box().w() - 1) * max_len);
	unsigned const height = Genode::min(height_mode() - _height, (_font.height() + 5) * text_count);

	using namespace Gui;

	Area area_text { width, height };

	unsigned xpos = x + _step_width;
	if ((xpos + width > _width)) {
		if (xpos - _step_width >= width)
			xpos -= _step_width + width;
		else
		if (_width - xpos > xpos)
			xpos = _width - width;
		else
			xpos = 0;
	}
	int ypos = last_y;
	if (last_y + area_text.h() >= _height)
		ypos = _height - area_text.h();

	Point point(xpos, ypos);
	Rect geometry_text(point, area_text);

	_gui.enqueue<Session::Command::Offset>(_view_text, Point(0, -_height));
	_gui.enqueue<Session::Command::Geometry>(_view_text, geometry_text);
	_gui.enqueue<Session::Command::To_front>(_view_text, Gui::Session::View_handle());
	_gui.execute();
}

void Component::construct(Genode::Env &env) { static Graph component(env); }
