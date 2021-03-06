// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gccv/line.cc
 *
 * Copyright (C) 2008 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "line.h"
#include <cmath>

namespace gccv {

Line::Line (Canvas *canvas, double xstart, double ystart, double xend, double yend):
	LineItem (canvas), m_xstart (0.), m_ystart (0.), m_xend (0.), m_yend (0.)
{
	SetPosition (xstart, ystart, xend, yend);
}

Line::Line (Group *parent, double xstart, double ystart, double xend, double yend, ItemClient *client):
	LineItem (parent, client), m_xstart (0.), m_ystart (0.), m_xend (0.), m_yend (0.)
{
	SetPosition (xstart, ystart, xend, yend);
}

Line::~Line ()
{
}

void Line::SetPosition (double xstart, double ystart, double xend, double yend)
{
	Invalidate ();
	m_xstart = xstart;
	m_ystart = ystart;
	m_xend = xend;
	m_yend = yend;
	BoundsChanged ();
	Invalidate ();
}

double Line::Distance (double x, double y, Item **item) const
{
	double d1, d2;
	d1 = (m_xend - m_xstart) * (x - m_xstart) + (m_yend - m_ystart) * (y - m_ystart);
	d2 = (m_xend - m_xstart) * (x - m_xend) + (m_yend - m_ystart) * (y - m_yend);
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
	if (item)
		*item = const_cast <Line *> (this);
	return fabs (d1 * y - d2 * x) / sqrt (d1 * d1 + d2 * d2) - GetLineWidth () / 2.;
}

void Line::Draw (cairo_t *cr, G_GNUC_UNUSED bool is_vector) const
{
	cairo_operator_t op = GetOperator ();
	ApplyLine (cr);
	cairo_set_line_cap (cr, CAIRO_LINE_CAP_BUTT);
	cairo_move_to (cr, m_xstart, m_ystart);
	cairo_line_to (cr, m_xend, m_yend);
	GOColor color = GetEffectiveLineColor ();
	if (op == CAIRO_OPERATOR_CLEAR || op == CAIRO_OPERATOR_SOURCE) {
		cairo_content_t content = cairo_surface_get_content (cairo_get_target (cr));
		if (!(content & CAIRO_CONTENT_ALPHA)) {
			color = GetCanvas ()->GetBackgroundColor ();
			if (color == 0) // FIXME: this is a hack, may be the canvas should have a fallback or access to a GtkStyleContext?
				color = GO_COLOR_WHITE;
		}
	}
	cairo_set_source_rgba (cr, GO_COLOR_TO_CAIRO (color));
	cairo_stroke (cr);
	cairo_restore (cr);
}

void Line::UpdateBounds ()
{
	double lw = GetLineWidth () / 2., lh;
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

void Line::Move (double x, double y)
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
