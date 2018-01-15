/*
 * \brief  Widget that simply shows a progress bar
 * \author Josef Soentgen
 * \date   2017-11-22
 */

/*
 * Copyright (C) 2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _BAR_WIDGET_H_
#define _BAR_WIDGET_H_

/* os includes */
#include <nitpicker_gfx/box_painter.h>

/* local includes */
#include "widget.h"

namespace Menu_view { struct Bar_widget; }


struct Menu_view::Bar_widget : Widget
{
	unsigned _length     { 0 };

	Color    _color      { 0, 0, 0 };
	Color    _color_text { 0, 255, 0 };
	Area     _size       { 16, 16 };

	Text_painter::Font const *_font { nullptr };

	typedef String<32> Text;
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

	Bar_widget(Widget_factory &factory, Xml_node node, Unique_id unique_id)
	: Widget(factory, node, unique_id) { }

	void update(Xml_node node) override
	{
		_font       = _factory.styles.font(node);

		_color      = _update_color_bar(node);
		_color_text = _update_color_text(node);

		_text = node.attribute_value("text", Text(""));

		unsigned int percent = node.attribute_value("percent", 100u);
		if (percent > 100) { percent = 100; }

		unsigned int w = node.attribute_value("width", 0U);
		unsigned int h = node.attribute_value("height", 0U);

		if (_font && !h) h = _font->height();
		if (!w) w = _size.w();
		if (!h) h = _size.h();

		_size   = Area(w, h);
		_length = percent ? _size.w() / (100u / percent) : 0;
	}

	Area min_size() const override
	{
		return _size;
	}

	void draw(Surface<Pixel_rgb888> &pixel_surface,
	          Surface<Pixel_alpha8> &alpha_surface,
	          Point at) const override
	{
		Rect const rect(at, Area(_length, _size.h()));

		Box_painter::paint(pixel_surface, rect, _color);
		Box_painter::paint(alpha_surface, rect, _color);

		if (!_font)
			return;

		Area const text_size(_font->string_width(_text.string()).decimal(),
		                     _font->height());

		int const dx = (int)geometry().w() - text_size.w(),
		          dy = (int)geometry().h() - text_size.h();

		Point const centered = at + Point(dx/2, dy/2);

		Text_painter::paint(pixel_surface,
		                    Text_painter::Position(centered.x(), centered.y()),
		                    *_font, _color_text, _text.string());

		Text_painter::paint(alpha_surface,
		                    Text_painter::Position(centered.x(), centered.y()),
		                    *_font, Color(255, 255, 255), _text.string());
	}

	private:

		/**
		 * Noncopyable
		 */
		Bar_widget(Bar_widget const &);
		Bar_widget &operator = (Bar_widget const &);
};

#endif /* _BAR_WIDGET_H_ */
