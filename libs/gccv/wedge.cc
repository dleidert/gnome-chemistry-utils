// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gccv/wedge.h
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
#include "wedge.h"
#include <cmath>

namespace gccv {

Wedge::Wedge (Canvas *canvas, double x0, double y0, double x1, double y1, double width):
	Item (canvas), m_xstart (0.), m_ystart (0.), m_xend (0.), m_yend (0.)
{
	m_Width = width;
	m_FillColor = GO_COLOR_BLACK;
	SetPosition (x0, y0, x1, y1);
}

Wedge::Wedge (Group *parent, double x0, double y0, double x1, double y1, double width, ItemClient *client):
	Item (parent, client), m_xstart (0.), m_ystart (0.), m_xend (0.), m_yend (0.)
{
	m_Width = width;
	m_FillColor = GO_COLOR_BLACK;
	SetPosition (x0, y0, x1, y1);
}

Wedge::~Wedge ()
{
}

void Wedge::SetPosition (double x0, double y0, double x1, double y1)
{
	Invalidate ();
	m_xstart = x0;
	m_ystart = y0;
	m_xend = x1;
	m_yend = y1;
	BoundsChanged ();
	Invalidate ();
}

double Wedge::Distance (double x, double y, Item **item) const
{
	double dx, dy, dx1, dy1, length;
	dx = m_xend - m_xstart;
	dy = m_yend - m_ystart;
	dx1 = x - m_xstart;
	dy1 = y - m_ystart;
	length = sqrt (dx * dx  + dy * dy);
	if (item)
		*item = const_cast <Wedge *> (this);
	if (length == 0.)
		return sqrt (dx1 * dx1 + dy1 * dy1);
	// project the (dx1, dy1) vector on the wedge axis.
	double xx = (dx1 * dx + dy1 * dy) / length;
	if (xx < 0.)
		return sqrt (dx1 * dx1 + dy1 * dy1);
	double yy = (dx1 * dy - dy1 * dx) / length;
	if (xx > length) {
		dx1 = xx -length;
		if (yy > m_Width / 2.)
			yy -= m_Width / 2.;
		else if (yy < -m_Width / 2.)
			yy += m_Width / 2.;
		else
			return fabs (dx1);
		return sqrt (dx1 * dx1 + yy * yy);
	}
	if (fabs (yy) < m_Width / 2. * fabs (xx) / length)
		return 0.;
	yy = fabs (yy) - m_Width / 2. * fabs (xx) / length;
	return yy;
}

void Wedge::Draw (cairo_t *cr, G_GNUC_UNUSED bool is_vector) const
{
	if (m_FillColor == 0)
		return;
	cairo_move_to (cr, m_xstart, m_ystart);
	cairo_line_to (cr, m_xe1, m_ye1);
	cairo_line_to (cr, m_xe2, m_ye2);
	cairo_close_path (cr);
	cairo_set_source_rgba (cr, GO_COLOR_TO_CAIRO (m_FillColor));
	cairo_fill (cr);
}

void Wedge::UpdateBounds ()
{
	double dx, dy, length;
	dx = m_xend - m_xstart;
	dy = m_yend - m_ystart;
	length = sqrt (dx * dx  + dy * dy);
	if (length == 0.) {
		m_xe1 = m_xe2 = m_xend;
		m_ye1 = m_ye2 = m_yend;
		return;
	}
	dx = (m_ystart - m_yend) / length * m_Width / 2.;
	dy = (m_xend - m_xstart) / length * m_Width / 2.;
	m_x0 = m_x1 = m_xstart;
	m_y0 = m_y1 = m_ystart;
	m_xe1 = m_xend + dx;
	if (m_xe1 < m_x0)
		m_x0 = m_xe1;
	else if (m_xe1 > m_x1)
		m_x1 = m_xe1;
	m_ye1 = m_yend + dy;
	if (m_ye1 < m_y0)
		m_y0 = m_ye1;
	else if (m_ye1 > m_y1)
		m_y1 = m_ye1;
	m_xe2 = m_xend - dx;
	if (m_xe2 < m_x0)
		m_x0 = m_xe2;
	else if (m_xe2 > m_x1)
		m_x1 = m_xe2;
	m_ye2 = m_yend - dy;
	if (m_ye2 < m_y0)
		m_y0 = m_ye2;
	else if (m_ye2 > m_y1)
		m_y1 = m_ye2;
	Item::UpdateBounds ();
}

void Wedge::Move (double x, double y)
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
