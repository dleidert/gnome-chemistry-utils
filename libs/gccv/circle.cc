// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gccv/circle.cc 
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
#include "circle.h"
#include "canvas.h"
#include <cmath>

namespace gccv {

Circle::Circle (Canvas *canvas, double x, double y, double radius):
	FillItem (canvas)
{
	SetPosition (x, y);
	SetRadius (radius);
}

Circle::Circle (Group *parent, double x, double y, double radius, ItemClient *client):
	FillItem (parent, client)
{
	SetPosition (x, y);
	SetRadius (radius);
}

Circle::~Circle ()
{
}

void Circle::SetPosition (double x, double y)
{
	Invalidate ();
	m_x = x;
	m_y = y;
	BoundsChanged ();
	Invalidate ();
}

void Circle::GetPosition (double &x, double &y)
{
	x = m_x;
	y = m_y;
}

double Circle::Distance (double x, double y, Item **item) const
{
	double result;
	x -= m_x;
	y -= m_y;
	result = sqrt (x * x + y * y);
	if (item)
		*item = const_cast <Circle *> (this);
	if ((GetFillColor () & 0xff) && result < m_Radius + GetLineWidth () / 2.)
		return 0.;
	return fabs (result - m_Radius) - GetLineWidth () / 2.;
}

void Circle::Draw (cairo_t *cr, bool is_vector) const
{
	GOColor color = GetFillColor ();
	cairo_set_line_width (cr, GetLineWidth ());
	cairo_arc (cr, m_x, m_y, m_Radius, 0., 2 * M_PI);
	if (color != 0) {
		cairo_set_source_rgba (cr, GO_COLOR_TO_CAIRO (color));
		color = GetLineColor ();
		if (color != 0)
			cairo_fill_preserve (cr);
		else
			cairo_fill (cr);
	} else
		color = GetLineColor ();
	if (color != 0) {
		cairo_set_source_rgba (cr, GO_COLOR_TO_CAIRO (color));
		cairo_stroke (cr);
	}
}

void Circle::UpdateBounds ()
{
	double lw = GetLineWidth () / 2.;
	m_x0 = m_x - m_Radius - lw;
	m_x1 = m_x + m_Radius + lw;
	m_y0 = m_y - m_Radius - lw;
	m_y1 = m_y + m_Radius + lw;
	Item::UpdateBounds ();
}

void Circle::Move (double x, double y)
{
	Invalidate ();
	m_x += x;
	m_y += y;
	BoundsChanged ();
	Invalidate ();
}

}
