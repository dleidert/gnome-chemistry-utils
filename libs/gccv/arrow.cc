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
	m_EndHead (ArrowHeadFull),
	m_A (6.),
	m_B (8.),
	m_C (4.)
{
}

Arrow::Arrow (Group *parent, double xstart, double ystart, double xend, double yend, ItemClient *client):
	Line (parent, xstart, ystart, xend, yend, client),
	m_StartHead (ArrowHeadNone),
	m_EndHead (ArrowHeadFull),
	m_A (6.),
	m_B (8.),
	m_C (4.)
{
}

Arrow::~Arrow ()
{
}

double Arrow::Distance (double x, double y, Item **item) const
{
	double dx, dy, dx1, dy1, length;
	dx = m_xend - m_xstart;
	dy = m_yend - m_ystart;
	dx1 = x - m_xstart;
	dy1 = y - m_ystart;
	length = sqrt (dx * dx  + dy * dy);
	if (item)
		*item = const_cast <Arrow *> (this);
	if (length == 0.)
		return sqrt (dx1 * dx1 + dy1 * dy1);
	// project the (dx1, dy1) vector on the wedge axis.
	double xx = (dx1 * dx + dy1 * dy) / length;
	double yy = (dx1 * dy - dy1 * dx) / length;
	double lw = GetLineWidth () / 2.;
	if (xx < 0.) {
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
		return sqrt (dx1 * dx1 + dy1 * dy1);
	}
	if (xx > length) {
		dx1 = xx -length;
		switch (m_EndHead) {
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
/*		if (yy > m_Width / 2.)
			yy -= m_Width / 2.;
		else if (yy < -m_Width / 2.)
			yy += m_Width / 2.;
		else
			return fabs (dx1);*/
		return sqrt (dx1 * dx1 + yy * yy);
	}
	if (fabs (yy) < lw)
		return 0.; // this might be wrong near the head, but still acceptable
	yy += (yy > 0)? lw: -lw;
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
	return fabs (yy);
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
		cairo_set_source_rgba (cr,  GO_COLOR_TO_CAIRO (color));
		cairo_stroke (cr);
		cairo_set_line_width (cr, 0.);
		switch (m_StartHead) {
		default:
		case ArrowHeadNone:
			break;
		case ArrowHeadFull:
			cairo_move_to (cr, m_A, GetLineWidth () / 2.);
			cairo_line_to (cr, m_B, GetLineWidth () / 2. + m_C);
			cairo_line_to (cr, 0., 0.);
			cairo_line_to (cr, m_B, -GetLineWidth () / 2. - m_C);
			cairo_line_to (cr, m_A, -GetLineWidth () / 2.);
			cairo_close_path (cr);
			cairo_fill (cr);
			break;
		case ArrowHeadLeft:
			cairo_move_to (cr, m_A, -GetLineWidth () / 2.);
			cairo_line_to (cr, m_B, -GetLineWidth () / 2. - m_C);
			cairo_line_to (cr, 0., GetLineWidth () / 2.);
			cairo_line_to (cr, m_A, GetLineWidth () / 2.);
			cairo_close_path (cr);
			cairo_fill (cr);
			break;
		case ArrowHeadRight:
			cairo_move_to (cr, m_A, GetLineWidth () / 2.);
			cairo_line_to (cr, m_B, GetLineWidth () / 2. + m_C);
			cairo_line_to (cr, 0., -GetLineWidth () / 2.);
			cairo_line_to (cr, m_A, -GetLineWidth () / 2.);
			cairo_close_path (cr);
			cairo_fill (cr);
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
			cairo_move_to (cr, length - m_A, -GetLineWidth () / 2.);
			cairo_line_to (cr, length - m_B, -GetLineWidth () / 2. - m_C);
			cairo_line_to (cr, length, GetLineWidth () / 2.);
			cairo_line_to (cr, length - m_A, GetLineWidth () / 2.);
			cairo_close_path (cr);
			cairo_fill (cr);
			break;
		case ArrowHeadRight:
			cairo_move_to (cr, length - m_A, GetLineWidth () / 2.);
			cairo_line_to (cr, length - m_B, GetLineWidth () / 2. + m_C);
			cairo_line_to (cr, length, -GetLineWidth () / 2.);
			cairo_line_to (cr, length - m_A, -GetLineWidth () / 2.);
			cairo_close_path (cr);
			cairo_fill (cr);
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

}
