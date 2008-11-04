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
	Rectangle (canvas, x, y, 0., 0.),
	m_x (x), m_y (y), m_w (0.), m_h (0.),
	m_Padding (0.),
	m_Anchor (AnchorLine),
	m_LineOffset (0.)
{
	m_Layout = pango_layout_new (const_cast <PangoContext *> (Ctx.GetContext ()));
}

Text::Text (Group *parent, double x, double y, ItemClient *client):
	Rectangle (parent, x, y, 0., 0., client),
	m_x (x), m_y (y), m_w (0.), m_h (0.),
	m_Padding (0.),
	m_Anchor (AnchorLine),
	m_LineOffset (0.)
{
	m_Layout = pango_layout_new (const_cast <PangoContext *> (Ctx.GetContext ()));
}

Text::~Text ()
{
	g_object_unref (m_Layout);
}

void Text::SetPosition (double x, double y)
{
	double xr, yr, w, h;
	PangoRectangle r;
	pango_layout_get_extents (m_Layout, &r, NULL); // FIXME: might be wrong if we allow above-under characters
	m_x = x;
	m_y = y;
	m_w = (double) r.width / PANGO_SCALE;
	m_h = (double) r.height / PANGO_SCALE;
	w = m_w + 2 * m_Padding;
	h = m_h + 2 * m_Padding;
	// Horizontal position
	switch (m_Anchor) {
	default:
	case AnchorNorth:
	case AnchorLine:
	case AnchorCenter:
	case AnchorSouth:
		xr = m_x - w / 2.;
		break;
	case AnchorNorthWest:
	case AnchorLineWest:
	case AnchorWest:
	case AnchorSouthWest:
		xr = m_x - w + m_Padding;
		break;
	case AnchorNorthEast:
	case AnchorLineEast:
	case AnchorEast:
	case AnchorSouthEast:
		xr = m_x - m_Padding;
		break;
	}
	// Vertical position
	switch (m_Anchor) {
	default:
	case AnchorLine:
	case AnchorLineWest:
	case AnchorLineEast: {
		PangoLayoutIter* iter = pango_layout_get_iter (m_Layout);
		yr = m_y - (double) pango_layout_iter_get_baseline (iter) / PANGO_SCALE + m_LineOffset;
		pango_layout_iter_free (iter);
		break;
	}
	case AnchorCenter:
	case AnchorWest:
	case AnchorEast:
		yr = m_y - h / 2.;
		break;
	case AnchorNorth:
	case AnchorNorthWest:
	case AnchorNorthEast:
		yr = m_y - m_Padding;
		break;
	case AnchorSouth:
	case AnchorSouthWest:
	case AnchorSouthEast:
		yr = m_y - m_h + m_Padding;
		break;
	}
	Rectangle::SetPosition (xr, yr, w, h);
}

void Text::Draw (cairo_t *cr, bool is_vector) const
{
	Rectangle::Draw (cr, is_vector);
}

PangoContext *Text::GetContext ()
{
	return Ctx.GetContext ();
}

void Text::SetText (char const *text)
{
	pango_layout_set_text (m_Layout, text, -1);
	SetPosition (m_x, m_y);
}

void Text::SetFontDescription (PangoFontDescription *desc)
{
	pango_layout_set_font_description (m_Layout, desc);
	SetPosition (m_x, m_y);
}

}
