// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gccv/bezier-arrow.cc 
 *
 * Copyright (C) 2009 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "bezier-arrow.h"

namespace gccv {

BezierArrow::BezierArrow (Canvas *canvas):
	LineItem (canvas),
	m_Head (ArrowHeadFull),
	m_A (6.),
	m_B (8.),
	m_C (4.),
	m_ShowControls (false)
{
	for (int i = 0; i < 4; i++)
		m_Controls[i].x = m_Controls[i].y = 0.;
}

BezierArrow::BezierArrow (Group *parent, ItemClient *client):
	LineItem (parent, client),
	m_Head (ArrowHeadFull),
	m_A (6.),
	m_B (8.),
	m_C (4.),
	m_ShowControls (false)
{
	for (int i = 0; i < 4; i++)
		m_Controls[i].x = m_Controls[i].y = 0.;
}

BezierArrow::~BezierArrow ()
{
}

double BezierArrow::Distance (double x, double y, Item **item) const
{
	return G_MAXDOUBLE; //FIXME
}

void BezierArrow::Draw (cairo_t *cr, bool is_vector) const
{
	double dx, dy, x, y, l;
	dx = m_Controls[3].x - m_Controls[2].x;
	dy = m_Controls[3].y - m_Controls[2].y;
	l = hypot (dx, dy);
	if (l == 0.)
		return;
	dx /= l;
	dy /= l;
	x = m_Controls[3].x - dx * m_A;
	y = m_Controls[3].y - dy * m_A;
	cairo_save (cr);
	cairo_set_line_width (cr, GetLineWidth ());
	cairo_set_line_cap (cr, CAIRO_LINE_CAP_BUTT);
	GOColor color = GetLineColor ();
	cairo_set_source_rgba (cr, DOUBLE_RGBA_R (color), DOUBLE_RGBA_G (color), DOUBLE_RGBA_B (color), DOUBLE_RGBA_A (color));
	cairo_move_to (cr, m_Controls[0].x, m_Controls[0].y);
	cairo_curve_to (cr, m_Controls[1].x, m_Controls[1].y, m_Controls[2].x, m_Controls[2].y, x, y);
	cairo_stroke (cr);
	cairo_set_line_width (cr, 0.);	
	// show control points if needed
	if (m_ShowControls) {
		double half_width = GetLineWidth () * 2.5;
		for (int i = 0; i < 4; i++) {
			cairo_rectangle (cr, m_Controls[i].x - half_width, m_Controls[i].y - half_width, 2 * half_width, 2 * half_width);
			cairo_fill (cr);
		}
	}
	// draw the arrow head
	cairo_translate (cr, x, y);
	cairo_rotate (cr, atan2 (dy, dx));
	switch (m_Head) {
	default:
	case ArrowHeadNone: // this should not happen
		break;
	case ArrowHeadFull:
		cairo_move_to (cr, 0., GetLineWidth () / 2.);
		cairo_line_to (cr, m_A - m_B, GetLineWidth () / 2. + m_C);
		cairo_line_to (cr, m_A, 0.);
		cairo_line_to (cr, m_A - m_B, -GetLineWidth () / 2. - m_C);
		cairo_line_to (cr, 0., -GetLineWidth () / 2.);
		cairo_close_path (cr);
		cairo_fill (cr);
		break;
	case ArrowHeadLeft:
		cairo_move_to (cr, 0., -GetLineWidth () / 2.);
		cairo_line_to (cr, m_A - m_B, -GetLineWidth () / 2. - m_C);
		cairo_line_to (cr, m_A, GetLineWidth () / 2.);
		cairo_line_to (cr, 0., GetLineWidth () / 2.);
		cairo_close_path (cr);
		cairo_fill (cr);
		break;
	case ArrowHeadRight:
		cairo_move_to (cr, 0., GetLineWidth () / 2.);
		cairo_line_to (cr, m_A - m_B, GetLineWidth () / 2. + m_C);
		cairo_line_to (cr, m_A, -GetLineWidth () / 2.);
		cairo_line_to (cr, 0., -GetLineWidth () / 2.);
		cairo_close_path (cr);
		cairo_fill (cr);
		break;
	}
	cairo_restore (cr);
}

void BezierArrow::UpdateBounds ()
{
	// ensure that the two last control points are at least 2 * m_A away from
	// each other, and otherwise, move Control[2]
	double dx, dy, l;
	dx = m_Controls[3].x - m_Controls[2].x;
	dy = m_Controls[3].y - m_Controls[2].y;
	l = hypot (dx, dy);
	if (l == 0.)
		return;
	if (l < 2 * m_A) {
		l = 2 * m_A / l;
		m_Controls[2].x = m_Controls[3].x - dx * l;
		m_Controls[2].y = m_Controls[3].y - dy * l;
	}

	m_x0 = m_x1 = m_Controls[0].x;
	m_y0 = m_y1 = m_Controls[0].y;
	for (int i = 1; i < 4; i++) {
		if (m_Controls[i].x < m_x0)
			m_x0 = m_Controls[i].x;
		else if (m_Controls[i].x > m_x1)
			m_x1 = m_Controls[i].x;
		if (m_Controls[i].y < m_y0)
			m_y0 = m_Controls[i].y;
		else if (m_Controls[i].y > m_y1)
			m_y1 = m_Controls[i].y;
	}
	// TODO: take arrow head and line width into account
}

void BezierArrow::SetControlPoints (double x0, double y0, double x1, double y1, double x2, double y2, double x3, double y3)
{
	m_Controls[0].x = x0;
	m_Controls[0].y = y0;
	m_Controls[1].x = x1;
	m_Controls[1].y = y1;
	m_Controls[2].x = x2;
	m_Controls[2].y = y2;
	m_Controls[3].x = x3;
	m_Controls[3].y = y3;
}

}   //  namespace gccv