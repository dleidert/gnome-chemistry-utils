// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gccv/rectangle.cc 
 *
 * Copyright (C) 2008 Jean Bréfort <jean.brefort@normalesup.org>
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include "config.h"
#include "rectangle.h"
#include "canvas.h"
#include <cmath>

namespace gccv {

Rectangle::Rectangle (Canvas *canvas, double x, double y, double width, double height):
	FillItem (canvas), m_x (0.), m_y (0.), m_w (0.), m_h (0.)
{
	SetPosition (x, y, width, height);
}

Rectangle::Rectangle (Group *parent, double x, double y, double width, double height, ItemClient *client):
	FillItem (parent, client), m_x (0.), m_y (0.), m_w (0.), m_h (0.)
{
	SetPosition (x, y, width, height);
}

Rectangle::~Rectangle ()
{
}

void Rectangle::SetPosition (double x, double y, double width, double height)
{
	Invalidate ();
	if (width > 0) {
		m_x = x;
		m_w = width;
	} else {
		m_x = x + width;
		m_w = -width;
	}
	if (height > 0) {
		m_y = y;
		m_h = height;
	} else {
		m_y = y + height;
		m_h = -height;
	}
	BoundsChanged ();
	Invalidate ();
}

void Rectangle::GetPosition (double &x, double &y) const
{
	x = m_x;
	y = m_y;
}

void Rectangle::GetPosition (double &x, double &y, double &width, double &height) const
{
	x = m_x;
	y = m_y;
	width = m_w;
	height = m_h;
}

double Rectangle::Distance (double x, double y, Item **item) const
{
	double result;
	if (x < m_x0) {
		if (y < m_y0) {
			x -= m_x0;
			y -= m_y0;
			result = sqrt (x * x + y * y);
		} else if (y < m_y1) {
			result = m_x0 - x;
		} else {
			x -= m_x0;
			y -= m_y1;
			result = sqrt (x * x + y * y);
		}
	} else if (x < m_x1) {
		if (y < m_y0)
			result = m_y0 - y;
		else if (y < m_y1)
			result = 0.;
		else
			result = y - m_y1;
	} else {
		if (y < m_y0) {
			x -= m_x1;
			y -= m_y0;
			result = sqrt (x * x + y * y);
		} else if (y < m_y1) {
			result =  x - m_x1;
		} else {
			x -= m_x1;
			y -= m_y1;
			result = sqrt (x * x + y * y);
		}
	}
	if (item)
		*item = const_cast <Rectangle *> (this);
	return result;
}

void Rectangle::Draw (cairo_t *cr, bool is_vector) const
{
	GOColor color = GetFillColor ();
	cairo_set_line_width (cr, GetLineWidth ());
	cairo_set_line_join (cr, CAIRO_LINE_JOIN_MITER);
	cairo_set_miter_limit (cr, 10.);
	cairo_rectangle (cr, m_x, m_y, m_w, m_h);
	if (color != 0) {
		cairo_set_source_rgba (cr, DOUBLE_RGBA_R (color), DOUBLE_RGBA_G (color), DOUBLE_RGBA_B (color), DOUBLE_RGBA_A (color));
		color = GetLineColor ();
		if (color != 0)
			cairo_fill_preserve (cr);
		else
			cairo_fill (cr);
	} else
		color = GetLineColor ();
	if (color != 0) {
		cairo_set_source_rgba (cr, DOUBLE_RGBA_R (color), DOUBLE_RGBA_G (color), DOUBLE_RGBA_B (color), DOUBLE_RGBA_A (color));
		cairo_stroke (cr);
	}
}

void Rectangle::UpdateBounds ()
{
	double lw = GetLineWidth () / 2.;
	m_x0 = m_x - lw;
	m_x1 = m_x + m_w + lw;
	m_y0 = m_y - lw;
	m_y1 = m_y + m_h + lw;
	Item::UpdateBounds ();
}

void Rectangle::Move (double x, double y)
{
	Invalidate ();
	m_x += x;
	m_y += y;
	BoundsChanged ();
	Invalidate ();
}

}
