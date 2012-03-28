// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gccv/bezier-arrow.cc
 *
 * Copyright (C) 2009-2011 Jean Bréfort <jean.brefort@normalesup.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the
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

static double newton (double t, double d5, double d4, double d3, double d2, double d1, double d0)
{
	int i;
	double u;
	// limit to 10 iterations to avoid an improbable infinite loop
	for (i = 0; i < 10; i++) {
		u = t;
		t -= (((((d5 * t + d4) * t + d3) * t + d2) * t + d1) * t + d0) / ((((5 * d5 * t + 4 * d4) * t + 3 * d3) * t + 2 * d2) * t + d1);
		if (fabs (1 - u / t) < 1.e-12)
			break;
	}
	return t;
}

double BezierArrow::Distance (double x, double y, Item **item) const
{
	double a, b, c, d, e, f, g, h, r, s, t, u;
	if (item)
		*item = const_cast <BezierArrow *> (this);
	if (x < m_x0 - 10. || x > m_x1 + 10 || y < m_y0 - 10 || y > m_y1 + 10)
		return G_MAXDOUBLE; // don't care we are far from the arrow
	if (m_ShowControls) {
		if (x >= m_x0 && x <= m_x1 && y >= m_y0 && y <= m_y1)
			return 0.;
	}
	a = m_Controls[3].x - 3 * (m_Controls[2].x - m_Controls[1].x) - m_Controls[0].x;
	b = 3 * (m_Controls[2].x - 2 * m_Controls[1].x + m_Controls[0].x);
	c = 3 * (m_Controls[1].x - m_Controls[0].x);
	d = m_Controls[0].x - x;
	e = m_Controls[3].y - 3 * (m_Controls[2].y - m_Controls[1].y) - m_Controls[0].y;
	f = 3 * (m_Controls[2].y - 2 * m_Controls[1].y + m_Controls[0].y);
	g = 3 * (m_Controls[1].y - m_Controls[0].y);
	h = m_Controls[0].y - y;
	// evaluate the distance using the Newton method starting from each end and
	// from the center, and take the lowest found value (undemonstrated validity).
	// evaluate the derivative at 0:
	double d5, d4, d3, d2, d1, d0;
	d5 = 3 * (a * a + e * e);
	d4 = 5 * (a * b + e * f);
	d3 = 4 * (a * c + e * g) + 2 * (b * b + f * f);
	d2 = 3 * (b * c + a * d + f * g + e * h);
	d1 = c * c + g * g + 2 * (b * d + f * h);
	d0 = c * d + g * h;
	if (d0 < 0.) {
		// the curve moves towards the target at x0, y0
		// find where the derivative becomes 0
		t = newton (0, d5, d4, d3, d2, d1, d0);
		u = ((a * t + b) * t + c) * t + d;
		r = u * u;
		u = ((e * t + f) * t + g) * t + h;
		r += u * u;
	} else
		r = hypot (x - m_Controls[0].x, y - m_Controls[0].y);
	// reiterate from the other end
	if (d5 + d4 + d3 + d2 + d1 + d0 > 0.) {
		t = newton (1, d5, d4, d3, d2, d1, d0);
		u = ((a * t + b) * t + c) * t + d;
		s = u * u;
		u = ((e * t + f) * t + g) * t + h;
		s += u * u;
	} else
		s = hypot (x - m_Controls[3].x, y - m_Controls[3].y);
	if (s < r)
		r = s;
	// now start from t = .5
	t = newton (0.5, d5, d4, d3, d2, d1, d0);
	u = ((a * t + b) * t + c) * t + d;
	s = u * u;
	u = ((e * t + f) * t + g) * t + h;
	s += u * u;
	if (s < r)
		r = s;
	// FIXME: take arrow head into account
	return r;
}

void BezierArrow::Draw (cairo_t *cr, G_GNUC_UNUSED bool is_vector) const
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
	GOColor color = GetEffectiveLineColor ();
	cairo_set_source_rgba (cr, GO_COLOR_TO_CAIRO (color));
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
	double dx, dy, l, x, y, c;
	dx = m_Controls[3].x - m_Controls[2].x;
	dy = m_Controls[3].y - m_Controls[2].y;
	l = hypot (dx, dy);
	if (l == 0.)
		return;
	if (l < 2 * m_A) {
		double d = 2 * m_A / l;
		m_Controls[2].x = m_Controls[3].x - dx * d;
		m_Controls[2].y = m_Controls[3].y - dy * d;
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
	dx /= l;
	dy /= l;
	c = m_C + GetLineWidth () / 2.;
	x = m_Controls[3].x - m_B * dx - c * dy;
	y = m_Controls[3].y - m_B * dy + c * dx;
	if (x < m_x0)
		m_x0 = x;
	else if (x > m_x1)
		m_x1 = x;
	if (y < m_y0)
		m_y0 = y;
	else if (y > m_y1)
		m_y1 = y;
	x = m_Controls[3].x - m_B * dx + c * dy;
	y = m_Controls[3].y - m_B * dy - c * dx;
	if (x < m_x0)
		m_x0 = x;
	else if (x > m_x1)
		m_x1 = x;
	if (y < m_y0)
		m_y0 = y;
	else if (y > m_y1)
		m_y1 = y;
	double half_width = GetLineWidth () * (m_ShowControls? 2.5: .5);
	m_x0 -= half_width;
	m_y0 -= half_width;
	m_x1 += half_width;
	m_y1 += half_width;
	Item::UpdateBounds ();
}

void BezierArrow::SetControlPoints (double x0, double y0, double x1, double y1, double x2, double y2, double x3, double y3)
{
	Invalidate ();
	m_Controls[0].x = x0;
	m_Controls[0].y = y0;
	m_Controls[1].x = x1;
	m_Controls[1].y = y1;
	m_Controls[2].x = x2;
	m_Controls[2].y = y2;
	m_Controls[3].x = x3;
	m_Controls[3].y = y3;
	BoundsChanged ();
	Invalidate ();
}

void BezierArrow::GetControlPoints (double &x0, double &y0, double &x1, double &y1, double &x2, double &y2, double &x3, double &y3)
{
	x0 = m_Controls[0].x;
	y0 = m_Controls[0].y;
	x1 = m_Controls[1].x;
	y1 = m_Controls[1].y;
	x2 = m_Controls[2].x;
	y2 = m_Controls[2].y;
	x3 = m_Controls[3].x;
	y3 = m_Controls[3].y;
}

void BezierArrow::Move (double x, double y)
{
	Invalidate ();
	m_Controls[0].x += x;
	m_Controls[0].y += y;
	m_Controls[1].x += x;
	m_Controls[1].y += y;
	m_Controls[2].x += x;
	m_Controls[2].y += y;
	m_Controls[3].x += x;
	m_Controls[3].y += y;
	BoundsChanged ();
	Invalidate ();
}

}   //  namespace gccv