// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * canvas/line.h 
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

#ifndef GCCV_TEXT_H
#define GCCV_TEXT_H

#include "rectangle.h"
#include <gtk/gtk.h>
#include <pango/pango.h>

namespace gccv {

class Text: public Rectangle
{
public:
	Text (Canvas *canvas, double x, double y);
	Text (Group *parent, double x, double y, ItemClient *client = NULL);
	virtual ~Text ();

	void SetPosition (double x, double y);
	void SetText (char const *text);
	void SetFontDescription (PangoFontDescription *desc);

	// virtual methods
	void Draw (cairo_t *cr, bool is_vector) const;

protected:
	void UpdateBounds ();

	// static methods
public:
	static PangoContext *GetContext ();

GCU_RO_POINTER_PROP (PangoLayout, Layout);
GCU_PROP (double, Padding);
GCU_PROP (GtkAnchorType, Anchor);
};

}

#endif	//	GCCV_TEXT_H
