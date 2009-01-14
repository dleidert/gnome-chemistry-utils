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
#include "text-tag.h"
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
	g_type_init (); // we need it as we use a static instance which might be created before the others needing gobject types
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
	static void OnCommit (GtkIMContext *context, const gchar *str, Text *text);
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

void TextPrivate::OnCommit (GtkIMContext *context, const gchar *str, Text *text)
{
	std::string s = str;
	// FIXME: erase selection
	text->ReplaceText (s, -1, 0);
}

////////////////////////////////////////////////////////////////////////////////
//  gccv::TextRun class implementation

class TextRun
{
public:
	TextRun ();
	~TextRun ();

	void Draw (cairo_t *cr);

	PangoLayout *m_Layout;
	double m_X, m_Y;
	unsigned m_Index, m_Length;
};

TextRun::TextRun ()
{
	m_Layout = pango_layout_new (const_cast <PangoContext *> (Ctx.GetContext ()));
	m_X = m_Y = 0.;
	m_Index = m_Length = 0;
}

TextRun::~TextRun ()
{
	g_object_unref (m_Layout);
}

void TextRun::Draw (cairo_t *cr)
{
	// first get the pango iter at first character
	PangoLayoutIter* iter = pango_layout_get_iter (m_Layout);
	char const *text = pango_layout_get_text (m_Layout);
	char const *next;
	double curx;
	PangoLayout *pl = pango_cairo_create_layout (cr);
	pango_layout_set_font_description (pl, pango_layout_get_font_description (m_Layout));
	PangoRectangle rect;
	cairo_set_source_rgba (cr, 0., 0., 0., 1.); // FIXME, use text color if any
	// FIXME: use text attributes
	while (*text) {
		pango_layout_iter_get_char_extents (iter, &rect);
		curx = rect.x / PANGO_SCALE;
		cairo_move_to (cr, m_X + curx, m_Y + (double) pango_layout_iter_get_baseline (iter) / PANGO_SCALE);
		next = g_utf8_find_next_char (text, NULL);
		pango_layout_set_text (pl, text, next - text);
		text = next;
		pango_cairo_show_layout (cr, pl);
		pango_layout_iter_next_char (iter);
	}
	// free the iterator
	pango_layout_iter_free (iter);
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
	TextRun *run = new TextRun ();
	m_Runs.push_front (run);
	m_ImContext = gtk_im_multicontext_new ();
	g_signal_connect (G_OBJECT (m_ImContext), "commit",
		G_CALLBACK (TextPrivate::OnCommit), this);
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
	TextRun *run = new TextRun ();
	m_Runs.push_front (run);
	m_ImContext = gtk_im_multicontext_new ();
	g_signal_connect (G_OBJECT (m_ImContext), "commit",
		G_CALLBACK (TextPrivate::OnCommit), this);
}

Text::~Text ()
{
	while (!m_Runs.empty ()) {
		delete m_Runs.front ();
		m_Runs.pop_front ();
	}
	while (!m_Tags.empty ()) {
		delete m_Tags.front ();
		m_Tags.pop_front ();
	}
}

void Text::SetPosition (double x, double y)
{
	double xr, yr, w, h;
	PangoRectangle r;
	std::list <TextRun *>::iterator i, end = m_Runs.end ();
	int x0, y0, x1, y1;
	if (m_BlinkSignal) {
		i = m_Runs.begin ();
		pango_layout_get_extents ((*i)->m_Layout, NULL, &r);
		x0 = r.x;
		y0 = r.y;
		x1 = x0 + r.width;
		y1 = y0 + r.height;
		for (i++; i != end; i++) {
			pango_layout_get_extents ((*i)->m_Layout, NULL, &r);
			if (r.x < x0)
				x0 = r.x;
			if (r.y < y0)
				y0 = r.y;
			r.x += r.width;
			r.y += r.height;
			if (r.x > x1)
				x1 = r.x;
			if (r.y > y1)
				y1 = r.y;
		}
	} else {
		i = m_Runs.begin ();
		pango_layout_get_extents ((*i)->m_Layout, &r, NULL);
		x0 = r.x;
		y0 = r.y;
		x1 = x0 + r.width;
		y1 = y0 + r.height;
		for (i++; i != end; i++) {
			pango_layout_get_extents ((*i)->m_Layout, &r, NULL);
			if (r.x < x0)
				x0 = r.x;
			if (r.y < y0)
				y0 = r.y;
			r.x += r.width;
			r.y += r.height;
			if (r.x > x1)
				x1 = r.x;
			if (r.y > y1)
				y1 = r.y;
		}
	}
	m_x = x;
	m_y = y;
	m_Y = (double) y0 / PANGO_SCALE;
	m_Width = (double) (x0 + x1) / PANGO_SCALE;
	m_Height = (double) (y1 - y0) / PANGO_SCALE;
	w = m_Width + 2 * m_Padding;
	h = m_Height + 2 * m_Padding;
	PangoLayoutIter* iter = pango_layout_get_iter (m_Runs.front ()->m_Layout);
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
		xr = m_x - m_Padding;
		break;
	case AnchorNorthEast:
	case AnchorLineEast:
	case AnchorEast:
	case AnchorSouthEast:
		xr = m_x - w + m_Padding;
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
//	PangoLayoutIter* iter = pango_layout_get_iter (m_Layout);
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
		startx = m_x;
		break;
	case AnchorNorthEast:
	case AnchorLineEast:
	case AnchorEast:
	case AnchorSouthEast:
		startx = m_x - m_Width;
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
	// now, draw the glyphs in each run
	std::list <TextRun *>::const_iterator i, end = m_Runs.end ();
	for (i = m_Runs.begin (); i != end; i++) {
		cairo_save (cr);
		cairo_translate (cr, startx + (*i)->m_X, starty + (*i)->m_Y - m_Ascent);
		(*i)->Draw (cr);
		cairo_restore (cr);
		if (m_CursorVisible && m_CurPos > (*i)->m_Index && m_CurPos <= (*i)->m_Index + (*i)->m_Length) {
			PangoRectangle rect;
			pango_layout_get_cursor_pos ((*i)->m_Layout, m_CurPos - (*i)->m_Index, &rect, NULL);
			cairo_set_line_width (cr, 1.);
			cairo_new_path (cr);
			cairo_move_to (cr, floor (startx + (double) rect.x / PANGO_SCALE) + .5, floor (starty + (double) rect.y / PANGO_SCALE) + .5);
			cairo_rel_line_to (cr, 0, rect.height / PANGO_SCALE);
			cairo_set_source_rgb (cr, 0., 0., 0.);
			cairo_stroke (cr);
		}
	}
/*	char const *text = pango_layout_get_text (m_Layout);
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
	}*/
	if (m_CursorVisible) {
		PangoRectangle rect;
		pango_layout_get_cursor_pos (m_Runs.front ()->m_Layout, m_CurPos, &rect, NULL); // FIXME: might be wrong if we allow above-under characters
		cairo_set_line_width (cr, 1.);
		cairo_new_path (cr);
		cairo_move_to (cr, floor (startx + (double) rect.x / PANGO_SCALE) + .5, floor (starty + (double) rect.y / PANGO_SCALE) + .5);
		cairo_rel_line_to (cr, 0, rect.height / PANGO_SCALE);
		cairo_set_source_rgb (cr, 0., 0., 0.);
		cairo_stroke (cr);
	}
	// free the iterator
	/*pango_layout_iter_free (iter);*/
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
	m_Text = text;
	pango_layout_set_text (m_Runs.front ()->m_Layout, text, -1); // FIXME: parse for line breaks
	m_Runs.front ()->m_Length = g_utf8_strlen (text, -1);
	SetPosition (m_x, m_y);
}

char const *Text::GetText ()
{
	std::list <TextRun *>::iterator i = m_Runs.begin (), end = m_Runs.end ();
	m_Text = pango_layout_get_text ((*i)->m_Layout);
	for (i++; i != end; i++)
		m_Text += pango_layout_get_text ((*i)->m_Layout);
	return m_Text.c_str ();
}

void Text::SetFontDescription (PangoFontDescription *desc)
{
	std::list <TextRun *>::iterator i, end = m_Runs.end ();
	for (i = m_Runs.begin (); i != end; i++) {
		pango_layout_set_font_description ((*i)->m_Layout, desc);
	}
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
	int x0, x1, x2, x3, y0, y1, y2, y3;
	std::list <TextRun *>::iterator it = m_Runs.begin (), end = m_Runs.end ();
	pango_layout_get_extents ((*it)->m_Layout, &i, &l);
	x0 = i.x;
	y0 = i.y;
	x1 = x0 + i.width;
	y1 = y0 + i.height;
	x2 = l.x;
	y2 = l.y;
	x3 = x2 + l.width;
	y3 = y2 + l.height;
	for (it++; it != end; it++) {
		pango_layout_get_extents ((*it)->m_Layout, &i, &l);
		if (i.x < x0)
			x0 = i.x;
		if (i.y < y0)
			y0 = i.y;
		i.x += i.width;
		i.y += i.height;
		if (i.x > x1)
			x1 = i.x;
		if (i.y > y1)
			y1 = i.y;
		if (l.x < x2)
			x2 = l.x;
		if (l.y < y2)
			y2 = l.y;
		l.x += l.width;
		l.y += l.height;
		if (l.x > x3)
			x3 = l.x;
		if (l.y > y3)
			y3 = l.y;
	}
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
		startx = m_x;
		break;
	case AnchorNorthEast:
	case AnchorLineEast:
	case AnchorEast:
	case AnchorSouthEast:
		startx = m_x - m_Width;
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
		ink->x0 = startx + (double) x0 / PANGO_SCALE;
		ink->y0 = starty + (double) y0 / PANGO_SCALE;
		ink->x1 = ink->x0 + (double) (x1 - x0) / PANGO_SCALE;
		ink->y1 = ink->y0 + (double) (y1 - y0) / PANGO_SCALE;
	}
	if (logical) {
		logical->x0 = startx + (double) x2 / PANGO_SCALE;
		logical->y0 = starty + (double) y2 / PANGO_SCALE;
		logical->x1 = logical->x0 + (double) (x3 - x2) / PANGO_SCALE;
		logical->y1 = logical->y0 + (double) (y3 - y2) / PANGO_SCALE;
	}
}

void Text::InsertTextTag (TextTag *tag)
{
	if (tag->GetPriority () == TagPriorityFirst)
		m_Tags.push_front (tag);
	else
		m_Tags.push_back (tag);
}

void Text::ReplaceText (std::string &str, int pos, unsigned length)
{
	unsigned l = m_Text.length ();
	if (pos == -1)
		pos = m_CurPos;
	else if (static_cast <unsigned> (pos) > l)
		pos = l;
	if (length > l - pos)
		length = l - pos;
	if (length > 0) {
		m_Text.erase (pos, length);
		//TODO: manage atributes
	}
	m_Text.insert (pos, str);
	//TODO: manage atributes
	pango_layout_set_text (m_Runs.front ()->m_Layout, m_Text.c_str (), -1); // FIXME: parse for line breaks
	m_Runs.front ()->m_Length = g_utf8_strlen (m_Text.c_str (), -1);
	m_CurPos = pos + str.length ();
	SetPosition (m_x, m_y);
}

bool Text::OnKeyPressed (GdkEventKey *event)
{
	if (gtk_im_context_filter_keypress (m_ImContext, event))
		return true;

	switch (event->keyval) {
	case GDK_Control_L:
	case GDK_Control_R:
		return false;
	case GDK_Return:
	case GDK_KP_Enter:
		/* TODO: write this code */
		break;

	case GDK_Tab:
		TextPrivate::OnCommit (m_ImContext, "\t", this);
		break;

	/* MOVEMENT */
	case GDK_Right:
		/* TODO: write this code */
		if (event->state & GDK_CONTROL_MASK) {
			/* move to start of word */
			/* TODO: write this code */
		} else {
			char const* s = m_Text.c_str ();
			char *p = g_utf8_next_char (s + m_CurPos);
			if (!p)
				break;
			m_CurPos = p - s;
			Invalidate ();
		}
			break;
	case GDK_Left:
		if (m_CurPos == 0)
			break;
		if (event->state & GDK_CONTROL_MASK) {
			/* move to start of word */
			/* TODO: write this code */
		} else {
			char const* s = m_Text.c_str ();
			char *p = g_utf8_prev_char (s + m_CurPos);
			m_CurPos = p - s;
			Invalidate ();
		}
		break;
	case GDK_f:
		/* TODO: write this code */
		break;
	case GDK_b:
		/* TODO: write this code */
		break;
	case GDK_p:
		if (!(event->state & GDK_CONTROL_MASK))
			break;
	case GDK_Up:
		/* TODO: write this code */
		break;
	case GDK_n:
		if (!(event->state & GDK_CONTROL_MASK))
			break;
	case GDK_Down:
		/* TODO: write this code */
		break;
	case GDK_Home:
		/* TODO: write this code */
		break;
	case GDK_End:
		/* TODO: write this code */
		break;
	case GDK_a:
		/* TODO: write this code */
		break;
	case GDK_e:
		/* TODO: write this code */
		if (event->state & GDK_CONTROL_MASK) {
		}
		break;

	/* DELETING TEXT */
	case GDK_Delete:
	case GDK_KP_Delete: {
		/* TODO: write this code */
	}
	case GDK_d:
		/* TODO: write this code */
		break;
	case GDK_BackSpace: {
		/* TODO: write this code */
		break;
	}
	case GDK_k:
		if (event->state & GDK_CONTROL_MASK) {
			/* delete from cursor to end of paragraph */
			/* TODO: write this code */
		}
		break;
	case GDK_u:
		if (event->state & GDK_CONTROL_MASK) {
			/* delete whole paragraph */
			/* TODO: write this code */
		}
		break;
	case GDK_backslash:
		if (event->state & GDK_MOD1_MASK) {
			/* delete all white spaces around the cursor */
			/* TODO: write this code */
		}
		break;
	default:
		break;
	}
	return true;
}

}
