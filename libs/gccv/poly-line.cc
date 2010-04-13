// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gccv/poly-line.cc 
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
#include "poly-line.h"
#include <cmath>

using namespace std;

namespace gccv {

PolyLine::PolyLine (Canvas *canvas, list <Point> &points):
	LineItem (canvas)
{
	SetPoints (points);
}

PolyLine::PolyLine (Group *parent, list <Point> &points, ItemClient *client):
	LineItem (parent, client)
{
	SetPoints (points);
}

PolyLine::~PolyLine ()
{
}

void PolyLine::SetPoints (list <Point> &points)
{
	Invalidate ();
	BoundsChanged ();
	Invalidate ();
	m_Points = points;
}

double PolyLine::Distance (double x, double y, Item **item) const
{
	list <Point>::const_iterator i = m_Points.begin (), end = m_Points.end ();
	double x0 = (*i).x, y0 = (*i).y, x1, y1;
	double lw = GetLineWidth () / 2.;
	double result = G_MAXDOUBLE, d, dx, dy, dx1, dy1, xx, yy, length;
	// we do not take miter limits into account
	if (item)
		*item = const_cast <PolyLine *> (this);
	for (i++; i != end; i++) {
		x1 = (*i).x;
		y1 = (*i).y;
		dx = x1 - x0;
		dy = y1 - y0;
		dx1 = x - x0;
		dy1 = y - y0;
		length = sqrt (dx * dx  + dy * dy);
		if (length == 0.)
			d = sqrt (dx1 * dx1 + dy1 * dy1);
		else {
			xx = (dx1 * dx + dy1 * dy) / length;
			yy = (dx1 * dy - dy1 * dx) / length;
			if (xx < 0.) {
				if (fabs (yy) < lw)
					d = fabs (xx);
				else {
					yy = fabs (yy) - lw;
					d = sqrt (xx * xx + yy * yy);
				}
			} else if (xx > length) {
				xx -= length;
				if (fabs (yy) < lw)
					d = fabs (xx);
				else {
					yy = fabs (yy) - lw;
					d = sqrt (xx * xx + yy * yy);
				}
			} else {
				if (fabs (yy) <= lw)
					return 0;
				else
					d = fabs (yy) - lw;
			}
		}
		if (d < result)
			result = d;
		x0 = x1;
		y0 = y1;
	}
	return result;
}

void PolyLine::Draw (cairo_t *cr, G_GNUC_UNUSED bool is_vector) const
{
	GOColor color = GetLineColor ();
	if (color != 0) {
		cairo_set_line_width (cr, GetLineWidth ());
		list <Point>::const_iterator i = m_Points.begin (), end = m_Points.end ();
		cairo_move_to (cr, (*i).x, (*i).y);
		for (i++; i != end; i++)
			cairo_line_to (cr, (*i).x, (*i).y);
		cairo_set_line_cap (cr, CAIRO_LINE_CAP_BUTT);
		cairo_set_line_join (cr, CAIRO_LINE_JOIN_MITER);
		cairo_set_miter_limit (cr, 10.);
		cairo_set_source_rgba (cr, GO_COLOR_TO_CAIRO (color));
		cairo_stroke (cr);
	}
}

void PolyLine::UpdateBounds ()
{
	// This code might be off by a little thing because of miter limits.
	list <Point>::const_iterator i = m_Points.begin (), end = m_Points.end ();
	m_x0 = m_x1 = (*i).x;
	m_y0 = m_y1 = (*i).y;
	for (i++; i != end; i++) {
		if ((*i).x < m_x0)
			m_x0 = (*i).x;
		else if ((*i).x > m_x1)
			m_x1 = (*i).x;
		if ((*i).y < m_y0)
			m_y0 = (*i).y;
		else if ((*i).y > m_y1)
			m_y1 = (*i).y;
	}
	double lw = GetLineWidth () / 2.;
	m_x0 -= lw;
	m_x1 += lw;
	m_y0 -= lw;
	m_y1 += lw;
}

void PolyLine::Move (double x, double y)
{
	Invalidate ();
	list <Point>::iterator i, end = m_Points.end ();
	for (i = m_Points.begin (); i != end; i++) {
		(*i).x += x;
		(*i).y += y;
	}
	BoundsChanged ();
	Invalidate ();
}

}
