// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gccv/arc.cc
 *
 * Copyright (C) 2009-2010 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "arc.h"

namespace gccv {

Arc::Arc (Canvas *canvas, double xc, double yc, double radius, double start, double end):
	LineItem (canvas),
	m_X (xc),
	m_Y (yc),
	m_Radius (radius),
	m_Start (start),
	m_End (end),
	m_Head (ArrowHeadNone),
	m_A (6.),
	m_B (8.),
	m_C (4.)
{
}

Arc::Arc (Group *parent, double xc, double yc, double radius, double start, double end, ItemClient *client):
	LineItem (parent, client),
	m_X (xc),
	m_Y (yc),
	m_Radius (radius),
	m_Start (start),
	m_End (end),
	m_Head (ArrowHeadNone),
	m_A (6.),
	m_B (8.),
	m_C (4.)
{
}

Arc::~Arc ()
{
}

double Arc::Distance (double x, double y, Item **item) const
{
	// use cairo_in_stroke to detect if the point is inside
	// size for the surface is probably unimportant
	if (item)
		*item = const_cast <Arc *> (this);
	cairo_surface_t *surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 1, 1);
	cairo_t *cr = cairo_create (surface);
	cairo_surface_destroy (surface);
	cairo_set_line_width (cr, GetLineWidth () + 1.); // add 1. to the width to help for selection
	ToCairo (cr);
	// FIXME, take colors into account
	if (cairo_in_stroke (cr, x, y)) {
		cairo_destroy (cr);
		return 0.;
	}
	cairo_destroy (cr);
	return G_MAXDOUBLE; // FIXME
}

void Arc::Draw (cairo_t *cr, G_GNUC_UNUSED bool is_vector) const
{
	if (ApplyLine (cr)) {
		ToCairo (cr);
		cairo_stroke (cr);
	}
	if (m_Head != ArrowHeadNone) {
	}
	cairo_restore (cr);
}

void Arc::UpdateBounds ()
{
	cairo_surface_t *surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 1, 1);
	cairo_t *cr = cairo_create (surface);
	cairo_surface_destroy (surface);
	cairo_set_line_width (cr, GetLineWidth ());
	ToCairo (cr);
	cairo_stroke_extents (cr, &m_x0, &m_y0, &m_x1, &m_y1);
	cairo_destroy (cr);
	Item::UpdateBounds ();
}

void Arc::ToCairo (cairo_t *cr) const
{
	if (m_Head == ArrowHeadNone) {
		if (m_Start < m_End)
			cairo_arc (cr, m_X, m_Y, m_Radius, m_Start, m_End);
		else
			cairo_arc_negative (cr, m_X, m_Y, m_Radius, m_Start, m_End);
	} else {
		double end = m_End + ((m_End > m_Start)? -1.: 1.) * m_A / m_Radius;
		if (m_Start < m_End)
			cairo_arc (cr, m_X, m_Y, m_Radius, m_Start, end);
		else
			cairo_arc_negative (cr, m_X, m_Y, m_Radius, m_Start, end);
		cairo_stroke (cr);
		// now draw the head
		double x0, y0, x1, y1, rot;
		x0 = m_X + m_Radius * cos (end);
		y0 = m_Y + m_Radius * sin (end);
		x1 = m_X + m_Radius * cos (m_End);
		y1 = m_Y + m_Radius * sin (m_End);
		rot = atan2 (y1 - y0, x1 - x0);
		cairo_save (cr);
		cairo_translate (cr, x0, y0);
		cairo_rotate (cr, rot);
		switch (m_Head) {
		case ArrowHeadRight:
			// FIXME
				break;
		case ArrowHeadLeft:
			// FIXME
			break;
		default:
			cairo_move_to (cr, 0., GetLineWidth () / 2.);
			cairo_line_to (cr, m_A - m_B, GetLineWidth () / 2. + m_C);
			cairo_line_to (cr, m_A, 0.);
			cairo_line_to (cr, m_A -  m_B, -GetLineWidth () / 2. - m_C);
			cairo_line_to (cr, 0., -GetLineWidth () / 2.);
			break;
		}
		cairo_close_path (cr);
		cairo_fill (cr);
		cairo_restore (cr);
	}
}

}	//	namespace gccv
