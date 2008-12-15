// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gccv/text.cc 
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
#include <cmath>

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

class TextPrivate
{
public:
	static bool OnBlink (Text *text);
};

#define PREBLINK_TIME 300
#define CURSOR_ON_TIME 800
#define CURSOR_OFF_TIME 400

static gint on_blink (gpointer data)
{
	return TextPrivate::OnBlink (reinterpret_cast <Text *> (data));
}

bool TextPrivate::OnBlink (Text *text)
{
	text->m_BlinkSignal = g_timeout_add (((text->m_CursorVisible)? CURSOR_OFF_TIME: CURSOR_ON_TIME), on_blink, text);

	text->m_CursorVisible = !text->m_CursorVisible;
	text->Invalidate (); // FIXME: just invalidate the cursor rectangle
	/* Remove ourself */
	return false;
}

////////////////////////////////////////////////////////////////////////////////
//  gccv::TextRun class implementation

class TextRun
{
public:
	TextRun ();
	~TextRun ();

	PangoLayout *m_Layout;
};

TextRun::TextRun ()
{
	m_Layout = pango_layout_new (const_cast <PangoContext *> (Ctx.GetContext ()));
}

TextRun::~TextRun ()
{
	g_object_unref (m_Layout);
}

////////////////////////////////////////////////////////////////////////////////
//  gccv::Text class implementation

Text::Text (Canvas *canvas, double x, double y):
	Rectangle (canvas, x, y, 0., 0.),
	m_x (x), m_y (y),
	m_BlinkSignal (0),
	m_CursorVisible (false),
	m_CurPos (0),
	m_Padding (0.),
	m_Anchor (AnchorLine),
	m_LineOffset (0.), m_Width (0.), m_Height (0.)
{
	m_Layout = pango_layout_new (const_cast <PangoContext *> (Ctx.GetContext ()));
}

Text::Text (Group *parent, double x, double y, ItemClient *client):
	Rectangle (parent, x, y, 0., 0., client),
	m_x (x), m_y (y),
	m_BlinkSignal (0),
	m_CursorVisible (false),
	m_CurPos (0),
	m_Padding (0.),
	m_Anchor (AnchorLine),
	m_LineOffset (0.), m_Width (0.), m_Height (0.)
{
	m_Layout = pango_layout_new (const_cast <PangoContext *> (Ctx.GetContext ()));
}

Text::~Text ()
{
	g_object_unref (m_Layout);
	while (!m_Runs.empty ()) {
		delete m_Runs.front ();
		m_Runs.pop_front ();
	}
}

void Text::SetPosition (double x, double y)
{
	double xr, yr, w, h;
	PangoRectangle r;
	if (m_BlinkSignal)
		pango_layout_get_extents (m_Layout, NULL, &r); // FIXME: might be wrong if we allow above-under characters
	else 
		pango_layout_get_extents (m_Layout, &r, NULL); // FIXME: might be wrong if we allow above-under characters
	m_x = x;
	m_y = y;
	m_Y = (double) r.y / PANGO_SCALE;
	m_Width = (double) (r.width + 2 * r.x) / PANGO_SCALE; // FIXME: this might be wrong for multiline texts, and any way looks weird
	m_Height = (double) r.height / PANGO_SCALE;
	w = m_Width + 2 * m_Padding;
	h = m_Height + 2 * m_Padding;
	PangoLayoutIter* iter = pango_layout_get_iter (m_Layout);
	m_Ascent = (double) pango_layout_iter_get_baseline (iter) / PANGO_SCALE;
	pango_layout_iter_free (iter);
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
	case AnchorLineEast:
		yr = m_y - m_Ascent + m_LineOffset + m_Y - m_Padding;
		break;
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
		yr = m_y - m_Height + m_Padding;
		break;
	}
	Rectangle::SetPosition (xr, yr, w, h);
}

void Text::Draw (cairo_t *cr, bool is_vector) const
{
	Rectangle::Draw (cr, is_vector);
	// now drawing text
	// first get the pango iter at first character
	PangoLayoutIter* iter = pango_layout_get_iter (m_Layout);
	// evaluate the starting position
	double startx, starty, curx;
	// Horizontal position
	switch (m_Anchor) {
	default:
	case AnchorNorth:
	case AnchorLine:
	case AnchorCenter:
	case AnchorSouth:
		startx = m_x - m_Width / 2.;
		break;
	case AnchorNorthWest:
	case AnchorLineWest:
	case AnchorWest:
	case AnchorSouthWest:
		startx = m_x - m_Width;
		break;
	case AnchorNorthEast:
	case AnchorLineEast:
	case AnchorEast:
	case AnchorSouthEast:
		startx = m_x;
		break;
	}
	// Vertical position
	switch (m_Anchor) {
	default:
	case AnchorLine:
	case AnchorLineWest:
	case AnchorLineEast: {
		starty = m_y - m_Ascent + m_LineOffset;
		break;
	}
	case AnchorCenter:
	case AnchorWest:
	case AnchorEast:
		starty = m_y - m_Height / 2. - m_LineOffset;
		break;
	case AnchorNorth:
	case AnchorNorthWest:
	case AnchorNorthEast:
		starty = m_y - m_LineOffset;
		break;
	case AnchorSouth:
	case AnchorSouthWest:
	case AnchorSouthEast:
		starty = m_y - m_Height - m_LineOffset;
		break;
	}
	// now, draw the glyphs
	char const *text = pango_layout_get_text (m_Layout);
	char const *next;
	PangoLayout *pl = pango_cairo_create_layout (cr);
	pango_layout_set_font_description (pl, pango_layout_get_font_description (m_Layout));
	PangoRectangle rect;
	cairo_set_source_rgba (cr, 0., 0., 0., 1.); // FIXME, use text color if any
	// FIXME: use text attributes
	while (*text) {
		pango_layout_iter_get_char_extents (iter, &rect);
		curx = rect.x / PANGO_SCALE;
		cairo_move_to (cr, startx + curx, starty  - m_Ascent + (double) pango_layout_iter_get_baseline (iter) / PANGO_SCALE);
		next = g_utf8_find_next_char (text, NULL);
		pango_layout_set_text (pl, text, next - text);
		text = next;
		pango_cairo_show_layout (cr, pl);
		pango_layout_iter_next_char (iter);
	}
	if (m_CursorVisible) {
		PangoRectangle rect;
		pango_layout_get_cursor_pos (m_Layout, m_CurPos, &rect, NULL); // FIXME: might be wrong if we allow above-under characters
		cairo_set_line_width (cr, 1.);
		cairo_new_path (cr);
		cairo_move_to (cr, floor (startx + (double) rect.x / PANGO_SCALE) + .5, floor (starty + (double) rect.y / PANGO_SCALE) + .5);
		cairo_rel_line_to (cr, 0, rect.height / PANGO_SCALE);
		cairo_set_source_rgb (cr, 0., 0., 0.);
		cairo_stroke (cr);
	}
	// free the iterator
	pango_layout_iter_free (iter);
}

void Text::Move (double x, double y)
{
	SetPosition (m_x + x, m_y + y);
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

void Text::SetEditing (bool editing)
{
	if (editing) {
		if (m_BlinkSignal != 0)
			return;
		m_BlinkSignal = g_timeout_add (CURSOR_ON_TIME, on_blink, this);
		m_CursorVisible = true;
	} else {
		if (m_BlinkSignal == 0)
			return;
		g_source_remove (m_BlinkSignal);
		m_BlinkSignal = 0;
		m_CursorVisible = false;
	}
	SetPosition (m_x, m_y);
}

void Text::GetBounds (Rect *ink, Rect *logical)
{
	PangoRectangle i, l;
	pango_layout_get_extents (m_Layout, &i, &l); // FIXME: use runs.
	double startx, starty;
	// Horizontal position
	switch (m_Anchor) {
	default:
	case AnchorNorth:
	case AnchorLine:
	case AnchorCenter:
	case AnchorSouth:
		startx = m_x - m_Width / 2.;
		break;
	case AnchorNorthWest:
	case AnchorLineWest:
	case AnchorWest:
	case AnchorSouthWest:
		startx = m_x - m_Width;
		break;
	case AnchorNorthEast:
	case AnchorLineEast:
	case AnchorEast:
	case AnchorSouthEast:
		startx = m_x;
		break;
	}
	// Vertical position
	switch (m_Anchor) {
	default:
	case AnchorLine:
	case AnchorLineWest:
	case AnchorLineEast: {
		starty = m_y - m_Ascent + m_LineOffset;
		break;
	}
	case AnchorCenter:
	case AnchorWest:
	case AnchorEast:
		starty = m_y - m_Height / 2. - m_LineOffset;
		break;
	case AnchorNorth:
	case AnchorNorthWest:
	case AnchorNorthEast:
		starty = m_y - m_LineOffset;
		break;
	case AnchorSouth:
	case AnchorSouthWest:
	case AnchorSouthEast:
		starty = m_y - m_Height - m_LineOffset;
		break;
	}
	if (ink) {
		ink->x0 = startx + (double) i.x / PANGO_SCALE;
		ink->y0 = starty + (double) i.y / PANGO_SCALE;
		ink->x1 = ink->x0 + (double) i.width / PANGO_SCALE;
		ink->y1 = ink->y0 + (double) i.height / PANGO_SCALE;
	}
	if (logical) {
		logical->x0 = startx + (double) l.x / PANGO_SCALE;
		logical->y0 = starty + (double) l.y / PANGO_SCALE;
		logical->x1 = logical->x0 + (double) l.width / PANGO_SCALE;
		logical->y1 = logical->y0 + (double) l.height / PANGO_SCALE;
	}
}

}
