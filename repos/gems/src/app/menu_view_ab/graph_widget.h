/*
 * \brief  Widget that shows a simple graph
 * \author Alexander Boettcher
 * \date   2019-12-09
 */

/*
 * Copyright (C) 2019-2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _GRAPH_WIDGET_H_
#define _GRAPH_WIDGET_H_

/* os includes */
#include <nitpicker_gfx/box_painter.h>

/* gems include */
#include <polygon_gfx/line_painter.h>

/* local includes */
#include "widget.h"

namespace Menu_view { struct Graph_widget; }


struct Menu_view::Graph_widget : Widget
{
	Color _color      { 0, 0, 0 };
	Color _color_text { 0, 255, 0 };
	Area  _size       { 16, 16 };

	static uint8_t constexpr _entries = 20;

	uint8_t  _px[_entries];
	uint8_t  _px_c { 0 };

	uint64_t _id { 0 };

	Text_painter::Font const *_font { nullptr };

	typedef String<8> Text;
	Text _text { };

	Color _update_color_bar(Xml_node node)
	{
		return node.attribute_value("color", _color);
	}

	Color _update_color_text(Xml_node node)
	{
		if (!node.has_attribute("textcolor")) {
			_font = nullptr;
			return _color_text;
		}

		return node.attribute_value("textcolor", _color_text);
	}

	Graph_widget(Widget_factory &factory, Xml_node node, Unique_id unique_id)
	: Widget(factory, node, unique_id) { }

	void update(Xml_node node) override
	{
		_font       = _factory.styles.font(node);

		_color      = _update_color_bar(node);
		_color_text = _update_color_text(node);

		_text = node.attribute_value("text", Text(""));

		unsigned w = node.attribute_value("width", 0U);
		unsigned h = node.attribute_value("height", 0U);
		uint64_t id = node.attribute_value("id", 0ULL);

		if (!id || (id != _id)) {
			unsigned percent = node.attribute_value("percent", 101U);

			if (percent > 100)
				_px[_px_c] = _px[(_px_c + _entries - 1) % _entries];
			else
				_px[_px_c] = uint8_t(percent);
			_px_c = uint8_t((_px_c + 1) % _entries);

			_id = id;
		}

		if (_font && !h) h = _font->height();
		if (!w) w = _size.w();
		if (!h) h = _size.h();

		_size   = Area(w, h);
	}

	Area min_size() const override
	{
		return _size;
	}

	void draw(Surface<Pixel_rgb888> &pixel_surface,
	          Surface<Pixel_alpha8> &alpha_surface,
	          Point at) const override
	{
		if (_font) {

			Area const text_size(_font->string_width(_text.string()).decimal(),
			                     _font->height());

#if 0
			int const dx = (int)geometry().w() - text_size.w(),
			          dy = (int)geometry().h() - text_size.h();
#endif

			Point const centered = at; // + Point(dx/2, dy/2);

			Text_painter::paint(pixel_surface,
			                    Text_painter::Position(centered.x(), centered.y()),
			                    *_font, _color_text, _text.string());

			Text_painter::paint(alpha_surface,
			                    Text_painter::Position(centered.x(), centered.y()),
			                    *_font, Color(255, 255, 255), _text.string());
		}

		Line_painter line;

		for (uint8_t i = _entries - 1; i > 1; i--) {
			uint8_t prev = uint8_t((_px_c + i - 1) % _entries);
			uint8_t curr = uint8_t((_px_c + i) % _entries);
			Point f { at.x() + int(5 + (i + 0) * (geometry().w() - 10) / _entries),
			          at.y() + int(geometry().h() - 5 - (geometry().h() - 10) * _px[prev] / 100) };
			Point t { at.x() + int(5 + (i + 1) * (geometry().w() - 10) / _entries),
			          at.y() + int(geometry().h() - 5 - (geometry().h() - 10) * _px[curr] / 100) };

			line.paint(pixel_surface, f, t, _color_text);
		}
	}

	private:

		/**
		 * Noncopyable
		 */
		Graph_widget(Graph_widget const &);
		Graph_widget &operator = (Graph_widget const &);
};

#endif /* _GRAPH_WIDGET_H_ */
