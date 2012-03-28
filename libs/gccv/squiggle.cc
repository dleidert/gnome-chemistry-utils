// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gccv/squiggle.h
 *
 * Copyright (C) 2008-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "canvas.h"
#include "group.h"
#include "squiggle.h"
#include <cmath>

namespace gccv {

Squiggle::Squiggle (Canvas *canvas, double xstart, double ystart, double xend, double yend):
	LineItem (canvas), m_xstart (0.), m_ystart (0.), m_xend (0.), m_yend (0.)
{
	SetPosition (xstart, ystart, xend, yend);
}

Squiggle::Squiggle (Group *parent, double xstart, double ystart, double xend, double yend, ItemClient *client):
	LineItem (parent, client), m_xstart (0.), m_ystart (0.), m_xend (0.), m_yend (0.)
{
	SetPosition (xstart, ystart, xend, yend);
}

Squiggle::~Squiggle ()
{
}

void Squiggle::SetPosition (double xstart, double ystart, double xend, double yend)
{
	Invalidate ();
	m_xstart = xstart;
	m_ystart = ystart;
	m_xend = xend;
	m_yend = yend;
	BoundsChanged ();
	Invalidate ();
}

double Squiggle::Distance (double x, double y, Item **item) const
{
	double d1, d2;
	d1 = (m_xend - m_xstart) * (x - m_xstart) + (m_yend - m_ystart) * (y - m_ystart);
	d2 = (m_xend - m_xstart) * (x - m_xend) + (m_yend - m_ystart) * (y - m_yend);
	if (item)
		*item = const_cast <Squiggle *> (this);
	if (d1 >= 0. && d2 >= 0.) {
		x -= m_xend;
		y -= m_yend;
		return sqrt (x * x + y * y);
	}
	if (d1 <= 0. && d2 <= 0.) {
		x -= m_xstart;
		y -= m_ystart;
		return sqrt (x * x + y * y);
	}
	x -= m_xstart;
	y -= m_ystart;
	d1 = m_xend - m_xstart;
	d2 = m_yend - m_ystart;
	return fabs (d1 * y - d2 * x) / sqrt (d1 * d1 + d2 * d2) - m_Width / 2.;
}

void Squiggle::Draw (cairo_t *cr, G_GNUC_UNUSED bool is_vector) const
{
	GOColor color = GetEffectiveLineColor ();
	if (color != 0) {
		double dx = m_xend - m_xstart, dy = m_yend - m_ystart, length = sqrt (dx * dx + dy * dy);
		dx /= length;
		dy /= length; // we now have a unit vector.
		// evaluate the number of waves
		int n = floor (length / m_Step);
		// now the steps between consecutive control points on the same size
		length /= n;
		double xstep = length * dx, ystep = length * dy, x = xstep / 1.5, y = ystep / 1.5;
		// evaluate the first left and right control points.
		double width = m_Width / 2. - GetLineWidth () / 2.;
		double x0 = m_xstart + dy * width + xstep / 2.,
			   y0 = m_ystart - dx * width + ystep / 2.,
			   x1 = m_xstart - dy * width + 1.5 * xstep,
			   y1 = m_ystart + dx * width + 1.5 * ystep, t;
		cairo_set_line_width (cr, GetLineWidth ());
		cairo_set_line_cap (cr, CAIRO_LINE_CAP_BUTT);
		cairo_move_to (cr, m_xstart, m_ystart);
		cairo_curve_to (cr, m_xstart + x, m_ystart + y, x0 - x, y0 - y, x0, y0);
		for (int i = 1; i < n; i++) {
			cairo_curve_to (cr, x0 + x, y0 + y, x1 - x, y1 - y, x1, y1);
			t = x0;
			x0 = x1;
			x1 = t + 2 * xstep;
			t = y0;
			y0 = y1;
			y1 = t + 2 * ystep;
		}
		cairo_curve_to (cr, x0 + x, y0 + y, m_xend - x, m_yend - y, m_xend, m_yend);
		cairo_set_source_rgba (cr, GO_COLOR_TO_CAIRO (color));
		cairo_stroke (cr);
	}
}

void Squiggle::UpdateBounds ()
{
	double lw = m_Width / 2., lh;
	double angle = atan2 (m_yend - m_ystart, m_xend - m_xstart);
	// TODO: take line cap into account
	lh = fabs (lw * cos (angle));
	lw = fabs (lw * sin (angle));
	if (m_xstart < m_xend) {
		m_x0 = m_xstart - lw;
		m_x1 = m_xend + lw;
	} else {
		m_x0 = m_xend - lw;
		m_x1 = m_xstart + lw;
	}
	if (m_ystart < m_yend) {
		m_y0 = m_ystart - lh;
		m_y1 = m_yend + lh;
	} else {
		m_y0 = m_yend - lh;
		m_y1 = m_ystart + lh;
	}
	Item::UpdateBounds ();
}

void Squiggle::Move (double x, double y)
{
	Invalidate ();
	m_xstart += x;
	m_ystart += y;
	m_xend += x;
	m_yend += y;
	BoundsChanged ();
	Invalidate ();
}

}
