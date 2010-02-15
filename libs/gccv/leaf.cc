// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gccv/leaf.cc 
 *
 * Copyright (C) 2009-2010 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "leaf.h"

namespace gccv {

Leaf::Leaf (Canvas *canvas, double x, double y, double radius):
	FillItem (canvas),
	m_WidthFactor (1.)
{
	SetPosition (x, y);
	SetRadius (radius);
	SetRotation (0.);
}

Leaf::Leaf (Group *parent, double x, double y, double radius, ItemClient *client):
	FillItem (parent, client),
	m_WidthFactor (1.)
{
	SetPosition (x, y);
	SetRadius (radius);
	SetRotation (0.);
}

Leaf::~Leaf ()
{
}

void Leaf::SetPosition (double x, double y)
{
	Invalidate ();
	m_x = x;
	m_y = y;
	BoundsChanged ();
	Invalidate ();
}

void Leaf::GetPosition (double &x, double &y)
{
	x = m_x;
	y = m_y;
}

double Leaf::Distance (double x, double y, Item **item) const
{
	return G_MAXDOUBLE; // FIXME
}

// x position of first control point
#define LEAF_INCR1	.2
// y position of first control point
#define LEAF_INCR2	.2
// x position of side limit
#define LEAF_INCR3	.4
// y position of control point befre the side limit
#define LEAF_INCR4	.4
// y position of side limit
#define LEAF_INCR5	.6
// y position of control point after the side limit
#define LEAF_INCR6	.8
// x position of control point before top
#define LEAF_INCR7	.2
// y poisition at top
#define LEAF_INCR8	1.
void Leaf::Draw (cairo_t *cr, G_GNUC_UNUSED bool is_vector) const
{
	GOColor fill_color = GetFillColor (), line_color = GetLineColor ();
	cairo_set_line_width (cr, GetLineWidth ());
	cairo_move_to (cr, m_x, m_y);
	cairo_curve_to (cr,
	    			m_Controls[0].x, m_Controls[0].y,
	    			m_Controls[1].x, m_Controls[1].y,
	    			m_Controls[2].x, m_Controls[2].y);
	cairo_curve_to (cr,
	    			m_Controls[3].x, m_Controls[3].y,
	    			m_Controls[4].x, m_Controls[4].y,
	    			m_Controls[5].x, m_Controls[5].y);
	cairo_curve_to (cr,
	    			m_Controls[6].x, m_Controls[6].y,
	    			m_Controls[7].x, m_Controls[7].y,
	    			m_Controls[8].x, m_Controls[8].y);
	cairo_curve_to (cr,
	    			m_Controls[9].x, m_Controls[9].y,
	    			m_Controls[10].x, m_Controls[10].y,
	    			m_x, m_y);
	cairo_close_path (cr);
	if (fill_color != 0) {
		cairo_set_source_rgba (cr, GO_COLOR_TO_CAIRO (fill_color));
		if (line_color != 0)
			cairo_fill_preserve (cr);
		else
			cairo_fill (cr);
	}
	if (line_color) {
			cairo_set_source_rgba (cr, GO_COLOR_TO_CAIRO (line_color));
		cairo_stroke (cr);
	}
}

void Leaf::UpdateBounds ()
{
	double cosine = cos (m_Rotation),
		sine = sin (m_Rotation),
		xRad = m_Radius * m_WidthFactor;

	m_Controls[0].x = m_x - xRad * LEAF_INCR1 * cosine - m_Radius * LEAF_INCR2 * sine;
	m_Controls[0].y = m_y + xRad * LEAF_INCR1 * sine - m_Radius * LEAF_INCR2 * cosine;
	m_Controls[1].x = m_x - xRad * LEAF_INCR3 * cosine - m_Radius * LEAF_INCR4 * sine;
	m_Controls[1].y = m_y + xRad * LEAF_INCR3 * sine - m_Radius * LEAF_INCR4 * cosine;
	m_Controls[2].x = m_x - xRad * LEAF_INCR3 * cosine- m_Radius * LEAF_INCR5 * sine;
	m_Controls[2].y = m_y + xRad * LEAF_INCR3 * sine - m_Radius * LEAF_INCR5* cosine;
	m_Controls[3].x = m_x - xRad * LEAF_INCR3 * cosine - m_Radius * LEAF_INCR6 * sine;
	m_Controls[3].y = m_y + xRad * LEAF_INCR3 * sine - m_Radius * LEAF_INCR6 * cosine;
	m_Controls[4].x = m_x - xRad * LEAF_INCR7 * cosine - m_Radius * LEAF_INCR8 * sine;
	m_Controls[4].y = m_y + xRad * LEAF_INCR7 * sine - m_Radius * LEAF_INCR8 * cosine;
	m_Controls[5].x = m_x - m_Radius * LEAF_INCR8 * sine;
	m_Controls[5].y = m_y - m_Radius * LEAF_INCR8 * cosine;
	m_Controls[6].x = m_x + xRad * LEAF_INCR7 * cosine - m_Radius * LEAF_INCR8 * sine;
	m_Controls[6].y = m_y - xRad * LEAF_INCR7 * sine - m_Radius * LEAF_INCR8 * cosine;
	m_Controls[7].x = m_x + xRad * LEAF_INCR3 * cosine - m_Radius * LEAF_INCR6 * sine;
	m_Controls[7].y = m_y - xRad * LEAF_INCR3 * sine - m_Radius * LEAF_INCR6 * cosine;
	m_Controls[8].x = m_x + xRad * LEAF_INCR3 * cosine - m_Radius * LEAF_INCR5 * sine;
	m_Controls[8].y = m_y - xRad * LEAF_INCR3 * sine - m_Radius * LEAF_INCR5 * cosine;
	m_Controls[9].x = m_x + xRad * LEAF_INCR3 * cosine - m_Radius * LEAF_INCR4 * sine;
	m_Controls[9].y = m_y - xRad * LEAF_INCR3 * sine - m_Radius * LEAF_INCR4 * cosine;
	m_Controls[10].x = m_x + xRad * LEAF_INCR1 * cosine - m_Radius * LEAF_INCR2 * sine;
	m_Controls[10].y = m_y - xRad * LEAF_INCR1 * sine - m_Radius * LEAF_INCR2 * cosine;
	double lw = GetLineWidth () / 2.;
	m_x0 = m_x1 = m_x;
	m_y0 = m_y1 = m_y;
	for (int i = 0; i < 11; i++) {
		if (m_Controls[i].x < m_x0)
			m_x0 = m_Controls[i].x;
		else if (m_Controls[i].x > m_x1)
			m_x1 = m_Controls[i].x;
		if (m_Controls[i].y < m_y0)
			m_y0 = m_Controls[i].y;
		else if (m_Controls[i].y > m_y1)
			m_y1 = m_Controls[i].y;
	}
	m_x0 -= lw;
	m_x1 += lw;
	m_y0 -= lw;
	m_y1 += lw;
	Item::UpdateBounds ();
}

void Leaf::Move (double x, double y)
{
	Invalidate ();
	m_x += x;
	m_y += y;
	BoundsChanged ();
	Invalidate ();
}

void Leaf::SetWidthFactor (double factor)
{
	if (factor < 0. || factor > 1.)
		return;
	Invalidate ();
	m_WidthFactor = factor;
	UpdateBounds ();
	Invalidate ();
}

}	//	namespace gccv
