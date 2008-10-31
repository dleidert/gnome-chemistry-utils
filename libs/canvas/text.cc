// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * canvas/text.cc 
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
#include "text.h"
#include <pango/pangocairo.h>
#include <cairo-pdf.h>

namespace gccv {

class Context
{
public:
	Context ();
	~Context ();

GCU_POINTER_PROP (PangoContext, Context);
};

static Context Ctx;

Context::Context ()
{
	cairo_surface_t *s = cairo_pdf_surface_create ("/tmp/foo", 100., 100.);
	cairo_t *cr = cairo_create (s);
	cairo_surface_destroy (s);
	PangoLayout *layout = pango_cairo_create_layout (cr);
	m_Context = reinterpret_cast <PangoContext *> (g_object_ref (pango_layout_get_context (layout)));
	g_object_unref (layout);
	cairo_destroy (cr);
}

Context::~Context ()
{
	g_object_unref (m_Context);
}

Text::Text (Canvas *canvas, double x, double y):
		Rectangle (canvas, x, y, 0., 0.)
{
}

Text::Text (Group *parent, double x, double y, ItemClient *client):
		Rectangle (parent, x, y, 0., 0., client)
{
	m_Layout = pango_layout_new (const_cast <PangoContext *> (Ctx.GetContext ()));
}

Text::~Text ()
{
	g_object_unref (m_Layout);
}

void Text::SetPosition (double x, double y)
{
}

void Text::Draw (cairo_t *cr, bool is_vector) const
{
}

PangoContext *Text::GetContext ()
{
	return Ctx.GetContext ();
}

void Text::SetText (char const *text)
{
	pango_layout_set_text (m_Layout, text, -1);
	UpdateBounds ();
}

void Text::SetFontDescription (PangoFontDescription *desc)
{
	pango_layout_set_font_description (m_Layout, desc);
	UpdateBounds ();
}

void Text::UpdateBounds ()
{
	// Update the rectangle bounds according to the layout size
	// TODO: write this code
}

}
