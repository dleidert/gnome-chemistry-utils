// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gccv/arrow.cc
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
#include "canvas.h"
#include "group.h"
#include "arrow.h"
#include <cmath>

namespace gccv {

Arrow::Arrow (Canvas *canvas, double xstart, double ystart, double xend, double yend):
	Line (canvas, xstart, ystart, xend, yend),
	m_StartHead (ArrowHeadNone),
	m_EndHead (ArrowHeadFull)
{
}

Arrow::Arrow (Group *parent, double xstart, double ystart, double xend, double yend, ItemClient *client):
	Line (parent, xstart, ystart, xend, yend, client),
	m_StartHead (ArrowHeadNone),
	m_EndHead (ArrowHeadFull)
{
}

Arrow::~Arrow ()
{
}

double Arrow::Distance (double x, double y, Item **item) const
{
	return G_MAXDOUBLE; //FIXME
}

void Arrow::Draw (cairo_t *cr, bool is_vector) const
{
	double theta = atan2 (m_yend - m_ystart, m_xend -m_xstart),
		   length = sqrt ((m_yend - m_ystart) * (m_yend - m_ystart) + (m_xend -m_xstart) * (m_xend -m_xstart));
	GOColor color = GetLineColor ();
	if (color != 0) {
		cairo_save (cr);
		cairo_translate (cr, m_xstart, m_ystart);
		cairo_rotate (cr, theta);
		cairo_move_to (cr, ((m_StartHead == ArrowHeadNone)? 0.: m_A), 0.);
		cairo_line_to (cr, ((m_EndHead == ArrowHeadNone)? length: length - m_A), 0.);
		cairo_set_line_width (cr, GetLineWidth ());
		cairo_set_source_rgba (cr, DOUBLE_RGBA_R (color), DOUBLE_RGBA_G (color), DOUBLE_RGBA_B (color), DOUBLE_RGBA_A (color));
		cairo_stroke (cr);
		cairo_set_line_width (cr, 0.);
		switch (m_StartHead) {
		default:
		case ArrowHeadNone:
			break;
		case ArrowHeadFull:
			break;
		case ArrowHeadLeft:
			break;
		case ArrowHeadRight:
			break;
		}
		switch (m_EndHead) {
		default:
		case ArrowHeadNone:
			break;
		case ArrowHeadFull:
			cairo_move_to (cr, length - m_A, GetLineWidth () / 2.);
			cairo_line_to (cr, length - m_B, GetLineWidth () / 2. + m_C);
			cairo_line_to (cr, length, 0.);
			cairo_line_to (cr, length - m_B, -GetLineWidth () / 2. - m_C);
			cairo_line_to (cr, length - m_A, -GetLineWidth () / 2.);
			cairo_close_path (cr);
			cairo_fill (cr);
			break;
		case ArrowHeadLeft:
			break;
		case ArrowHeadRight:
			break;
		}
	}
	cairo_restore (cr);
}

void Arrow::UpdateBounds ()
{
	// FIXME: take arrow heads into account
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
	// FIXME:  really take arrow heads into account
	m_y0 -= m_C;
	m_y1 += m_C;
	m_x0 -= m_C;
	m_x1 += m_C;
}

void Arrow::Move (double x, double y)
{
}

}
