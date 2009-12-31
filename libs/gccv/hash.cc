// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gccv/hash.cc 
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
#include "hash.h"
#include "cmath"

namespace gccv {

Hash::Hash (Canvas *canvas, double x0, double y0, double x1, double y1, double width):
	Wedge (canvas, x0, y0, x1, y1, width)
{
}

Hash::Hash (Group *parent, double x0, double y0, double x1, double y1, double width, ItemClient *client):
	Wedge (parent, x0, y0, x1, y1, width, client)
{
}

Hash::~Hash ()
{
}

void Hash::Draw (cairo_t *cr, bool is_vector) const
{
	if (GetFillColor () == 0)
		return;
	double dx = m_xend - m_xstart, dy = m_yend - m_ystart, length = sqrt (dx * dx + dy * dy);
	if (length == 0.)
		return;
	double xstep = (m_LineWidth + m_LineDist), ystep;
	int n = floor (length / xstep);
	ystep = xstep * dy / length;
	xstep *= dx / length;
	double x1 = m_xe1 - m_LineWidth / 2. * dx / length,
		   y1 = m_ye1 - m_LineWidth / 2. * dy / length,
		   x2 = m_xe2 - m_LineWidth / 2. * dx / length,
		   y2 = m_ye2 - m_LineWidth / 2. * dy / length;
	cairo_save (cr);
	cairo_move_to (cr, m_xstart, m_ystart);
	cairo_line_to (cr, m_xe1, m_ye1);
	cairo_line_to (cr, m_xe2, m_ye2);
	cairo_close_path (cr);
	cairo_clip (cr);
	cairo_set_line_width (cr, m_LineWidth);
	cairo_set_line_cap (cr, CAIRO_LINE_CAP_BUTT);
	cairo_set_source_rgba (cr, GO_COLOR_TO_CAIRO (GetFillColor ()));
	for (int i = 0; i < n; i++) {
		cairo_move_to (cr, x1, y1);
		cairo_line_to (cr, x2, y2);

		x1 -= xstep;
		y1 -= ystep;
		x2 -= xstep;
		y2 -= ystep;
	}
	cairo_stroke (cr);
	cairo_restore (cr);
}

}

