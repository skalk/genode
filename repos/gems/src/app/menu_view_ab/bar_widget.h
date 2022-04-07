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
	unsigned _length { 0 };

	Color    _color { 0, 0, 0 };
	Area     _size  { 16, 16 };

	Color _update_color(Xml_node node)
	{
		return node.attribute_value("color", _color);
	}

	Bar_widget(Widget_factory &factory, Xml_node node, Unique_id unique_id)
	: Widget(factory, node, unique_id) { }

	void update(Xml_node node) override
	{
		_color = _update_color(node);

		unsigned int percent = node.attribute_value("percent", 100u);
		if (percent > 100) { percent = 100; }

		unsigned int const w = node.attribute_value("width", _size.w());
		unsigned int const h = node.attribute_value("height", _size.h());

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
	}

	private:

		/**
		 * Noncopyable
		 */
		Bar_widget(Bar_widget const &);
		Bar_widget &operator = (Bar_widget const &);
};

#endif /* _BAR_WIDGET_H_ */
