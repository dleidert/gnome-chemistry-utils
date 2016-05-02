// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gccv/path.cc
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
#include "path.h"

namespace gccv {

Path::Path (Canvas *canvas, GOPath *path):
	FillItem (canvas),
	m_Path (path)
{
}

Path::Path (Group *parent, GOPath *path, ItemClient *client):
	FillItem (parent, client),
	m_Path (path)
{
}

Path::~Path ()
{
	go_path_free (m_Path);
}

double Path::Distance (double x, double y, Item **item) const
{
	// use cairo_in_fill to detect if the point is inside
	// size for the surface is probably unimportant
	if (item)
		*item = const_cast <Path *> (this);
	cairo_surface_t *surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 1, 1);
	cairo_t *cr = cairo_create (surface);
	cairo_surface_destroy (surface);
	cairo_set_line_width (cr, GetLineWidth ());
	go_path_to_cairo (m_Path, GO_PATH_DIRECTION_FORWARD, cr);
	// FIXME, take colors into account
	if (cairo_in_fill (cr, x, y)) {
		cairo_destroy (cr);
		return 0.;
	}
	if (cairo_in_stroke (cr, x, y)) {
		cairo_destroy (cr);
		return 0.;
	}
	cairo_destroy (cr);
	return G_MAXDOUBLE; // FIXME
}

void Path::Draw (cairo_t *cr, G_GNUC_UNUSED bool is_vector) const
{
	GOColor fill_color = GetFillColor (), line_color = GetEffectiveLineColor ();
	go_path_to_cairo (m_Path, GO_PATH_DIRECTION_FORWARD, cr);
	if (fill_color != 0) {
		cairo_set_source_rgba (cr, GO_COLOR_TO_CAIRO (fill_color));
		if (line_color != 0)
			cairo_fill_preserve (cr);
		else
			cairo_fill (cr);
	}
	if (ApplyLine (cr))
		cairo_stroke (cr);
	cairo_restore (cr);
}

void Path::UpdateBounds ()
{
	cairo_surface_t *surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 1, 1);
	cairo_t *cr = cairo_create (surface);
	cairo_surface_destroy (surface);
	cairo_set_line_width (cr, GetLineWidth ());
	go_path_to_cairo (m_Path, GO_PATH_DIRECTION_FORWARD, cr);
	cairo_stroke_extents (cr, &m_x0, &m_y0, &m_x1, &m_y1);
	cairo_destroy (cr);
	Item::UpdateBounds ();
}

}	//	namespace gccv
