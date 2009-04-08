// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gccv/text.cc 
 *
 * Copyright (C) 2008-2009 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "text-client.h"
#include "text-tag.h"
#include <pango/pangocairo.h>
#include <cairo-pdf.h>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <cstring>

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

void TextPrivate::OnCommit (G_GNUC_UNUSED GtkIMContext *context, const gchar *str, Text *text)
{
	std::string s = str;
	unsigned start, length;
	if (text->m_CurPos > text->m_StartSel) {
		start = text->m_StartSel;
		length = text->m_CurPos - text->m_StartSel;
	} else {
		start = text->m_CurPos;
		length = text->m_StartSel - text->m_CurPos;
	}
	text->ReplaceText (s, start, length);
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

struct draw_attrs
{
	unsigned index, length;
	PangoAttrList *attrs;
};

static gboolean filter_attrs (PangoAttribute *attr, gpointer _data)
{
	struct draw_attrs *data = static_cast <struct draw_attrs *> (_data);
	if (attr->start_index <= data->index && attr->end_index > data->index) {
		PangoAttribute *new_attr = pango_attribute_copy (attr);
		new_attr->start_index = 0;
		new_attr->end_index = data->length;
		pango_attr_list_insert (data->attrs, new_attr);
	}
	return false;
}

void TextRun::Draw (cairo_t *cr)
{
	// first get the pango iter at first character
	PangoLayoutIter* iter = pango_layout_get_iter (m_Layout), *local;
	PangoAttrList *l = pango_layout_get_attributes (m_Layout);
	char const *text = pango_layout_get_text (m_Layout);
	char const *next;
	double curx, ascent = (double) pango_layout_iter_get_baseline (iter) / PANGO_SCALE;
	unsigned index = 0;
	PangoLayout *pl = pango_cairo_create_layout (cr);
	pango_layout_set_font_description (pl, pango_layout_get_font_description (m_Layout));
	PangoRectangle rect;
	cairo_set_source_rgba (cr, 0., 0., 0., 1.);
	struct draw_attrs data;
	while (*text) {
		pango_layout_iter_get_char_extents (iter, &rect);
		curx = (double) rect.x / PANGO_SCALE;
		next = g_utf8_find_next_char (text, NULL);
		data.attrs = pango_attr_list_new ();
		data.index = index;
		data.length = next - text;
		index += data.length;
		pango_layout_set_text (pl, text, data.length);
		text = next;
		if (l) {
			pango_attr_list_filter (l, filter_attrs, &data);
			pango_layout_set_attributes (pl, data.attrs);
			pango_attr_list_unref (data.attrs);
		}
		local = pango_layout_get_iter (pl);
		pango_layout_iter_get_char_extents (local, &rect);
		cairo_save (cr);
		cairo_translate (cr, m_X + curx, m_Y + ascent - (double) pango_layout_iter_get_baseline (local) / PANGO_SCALE);
		pango_cairo_show_layout (cr, pl);
		cairo_restore (cr);
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
	m_CurTags (NULL),
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
	m_CurTags (new TextTagList ()),
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
	if (m_CurTags)
		delete m_CurTags;
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
	// evaluate the starting position
	double startx, starty;
	unsigned start_sel, end_sel;
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
	if (m_CurPos >= m_StartSel) {
		start_sel = m_StartSel;
		end_sel = m_CurPos;
	} else {
		end_sel = m_StartSel;
		start_sel = m_CurPos;
	}
	// now, draw the glyphs in each run
	std::list <TextRun *>::const_iterator i, end = m_Runs.end ();
	for (i = m_Runs.begin (); i != end; i++) {
		// display the selection rectangle if needed
		if (m_BlinkSignal) {
			if (start_sel < (*i)->m_Index + (*i)->m_Length && end_sel > (*i)->m_Index) {
				PangoRectangle rect;
				unsigned s = MAX (start_sel, (*i)->m_Index) - (*i)->m_Index,
						e = MIN (end_sel, (*i)->m_Index + (*i)->m_Length) - (*i)->m_Index;
				double x, y, w, h;
				cairo_set_source_rgb (cr, 0.9, 0.9, 0.9);
				pango_layout_get_cursor_pos ((*i)->m_Layout, s, &rect, NULL);
				x = (double) rect.x / PANGO_SCALE;
				h = (double) rect.height / PANGO_SCALE;
				y = starty + (*i)->m_Y + (double) rect.y / PANGO_SCALE;
				pango_layout_get_cursor_pos ((*i)->m_Layout, e, &rect, NULL);
				w = (double) rect.x / PANGO_SCALE - x;
				x += startx + (*i)->m_X;
				cairo_rectangle (cr, x, y, w, h);
				cairo_fill (cr);
			}
		}
		// draw the text
		cairo_save (cr);
		cairo_translate (cr, startx + (*i)->m_X, starty + (*i)->m_Y);
		(*i)->Draw (cr);
		cairo_restore (cr);
		// draw the cursor if needed
		if (m_CursorVisible && m_CurPos > (*i)->m_Index && m_CurPos <= (*i)->m_Index + (*i)->m_Length) {
			PangoRectangle rect;
			pango_layout_get_cursor_pos ((*i)->m_Layout, m_CurPos - (*i)->m_Index, &rect, NULL);
			cairo_set_line_width (cr, 1.);
			cairo_move_to (cr, floor (startx + (*i)->m_X + (double) rect.x / PANGO_SCALE) + .5, floor (starty + (*i)->m_Y + (double) rect.y / PANGO_SCALE) + .5);
			cairo_rel_line_to (cr, 0, rect.height / PANGO_SCALE);
			cairo_set_source_rgb (cr, 0., 0., 0.);
			cairo_stroke (cr);
		}
	}
	if (m_CursorVisible && m_CurPos == 0) {
		PangoRectangle rect;
		pango_layout_get_cursor_pos (m_Runs.front ()->m_Layout, m_CurPos, &rect, NULL); // FIXME: might be wrong if we allow above-under characters
		cairo_set_line_width (cr, 1.);
		cairo_new_path (cr);
		cairo_move_to (cr, floor (startx + (double) rect.x / PANGO_SCALE) + .5, floor (starty + (double) rect.y / PANGO_SCALE) + .5);
		cairo_rel_line_to (cr, 0, rect.height / PANGO_SCALE);
		cairo_set_source_rgb (cr, 0., 0., 0.);
		cairo_stroke (cr);
	}
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
	m_Runs.front ()->m_Length = strlen (text);
	SetPosition (m_x, m_y);
}

void Text::SetText (std::string const &text)
{
	m_Text = text;
	pango_layout_set_text (m_Runs.front ()->m_Layout, text.c_str (), -1); // FIXME: parse for line breaks
	m_Runs.front ()->m_Length = strlen (text.c_str ());
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
	m_FontDesc = desc;
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
	// FIXME: we need to filter tags to avoid duplicates
	// now, insert the new tag
	if (tag->GetPriority () == TagPriorityFirst)
		m_Tags.push_front (tag);
	else
		m_Tags.push_back (tag);
	// now, rebuild pango attributes lists for modified runs.
	std::list <TextRun *>::iterator run, end_run = m_Runs.end ();
	for (run = m_Runs.begin (); run != end_run; run++) {
		if ((*run)->m_Index < tag->GetEndIndex () && (*run)->m_Index + (*run)->m_Length > tag->GetStartIndex ()) {
			PangoAttrList *l = pango_layout_get_attributes ((*run)->m_Layout);
			if (l == NULL)
				l = pango_attr_list_new ();
			unsigned start = (tag->GetStartIndex () > (*run)->m_Index)? tag->GetStartIndex () - (*run)->m_Index: 0;
			unsigned end = (tag->GetEndIndex () < (*run)->m_Index + (*run)->m_Length)? tag->GetEndIndex () - (*run)->m_Index: (*run)->m_Length;
			tag->Filter (l, start, end);
			pango_layout_set_attributes ((*run)->m_Layout, l);	
		}
	}
	// force reposition and redraw
	SetPosition (m_x, m_y);
}

void Text::SetCurTagList (TextTagList *l)
{
	if (m_CurTags)
		delete m_CurTags;
	m_CurTags = l;
}

void Text::ApplyTagsToSelection (TextTagList const *l)
{
	if (m_CurPos == m_StartSel)
		return;
	TextTagList::iterator i, iend = m_Tags.end ();
	TextTagList::const_iterator j, jend = l->end ();
	unsigned start, end;
	if (m_CurPos > m_StartSel) {
		start = m_StartSel;
		end = m_CurPos;
	} else {
		start = m_CurPos;
		end = m_StartSel;
	}
	TextTagList extra_tags;
	// store tags to potentially insert into a vector
	std::vector <TextTag *> new_tags (MaxTag);
	// fist set all vectors to NULL
	for (int n = 0; n < MaxTag; n++) 
		new_tags[n] = NULL;
	// set the tags, unuseful ones will be reset to NULL later
	for (j = l->begin (); j != jend; j++)
		new_tags[(*j)->GetTag ()] = *j;
	for (i = m_Tags.begin (); i != iend; i++) {
		if ((*i)->GetStartIndex () > end || (*i)->GetEndIndex () < start)
			continue;
		for (j = l->begin (); j != jend; j++) {
			if ((*i)->GetTag() == (*j)->GetTag ()) {
				if (*(*i) == *(*j)) {
					// check if we need to change the tag limits
					if ((*i)->GetStartIndex () > start)
						(*i)->SetStartIndex (start);
					if ((*i)->GetEndIndex () < end)
						(*i)->SetEndIndex (end);
					// remove *j from new_tags
					new_tags[(*j)->GetTag ()] = NULL;
				} else if ((*i)->GetStartIndex () < start) {
					// we must change the tag limits since they overlap, and possibly split it
					if ((*i)->GetEndIndex () > end) {
						// we must split the tag
						TextTag *tag = (*i)->Duplicate ();
						tag->SetEndIndex ((*i)->GetEndIndex ());
						tag->SetStartIndex (end);
						extra_tags.push_front (tag);
					}
					(*i)->SetEndIndex (start);
				} else // ditto
					(*i)->SetStartIndex (end);
			}
		}
	}
	// Add new tags
	for (int n = 0; n < MaxTag; n++) 
		if (new_tags[n]) {
			TextTag *tag = new_tags[n]->Duplicate ();
			tag->SetStartIndex (start);
			tag->SetEndIndex (end);
			if (tag->GetPriority () == TagPriorityFirst)
				m_Tags.push_front (tag);
			else
				m_Tags.push_back (tag);
		}
	
	// Add extra tags
	iend = extra_tags.end ();
	for (i = extra_tags.begin (); i != iend; i++)
		if ((*i)->GetPriority () == TagPriorityFirst)
			m_Tags.push_front (*i);
		else
			m_Tags.push_back (*i);
	extra_tags.clear (); // avoid destroying the tags
	// Force redraw
	RebuildAttributes ();
	SetPosition (m_x, m_y);
}

void Text::ReplaceText (std::string &str, int pos, unsigned length)
{
	unsigned l = m_Text.length (), nl = str.length ();
	if (pos == -1)
		pos = m_CurPos;
	else if (static_cast <unsigned> (pos) > l)
		pos = l;
	if (length > l - pos)
		length = l - pos;
	TextTagList::iterator i, iend = m_Tags.end ();
	TextTagList new_tags, extra_tags;
	if (length > 0) {
		m_Text.erase (pos, length);
		std::vector <TextTag *> borders (MaxTag);
		for (int n = 0; n < MaxTag; n++) 
			borders[n] = NULL;
		//TODO: update runs
		for (i = m_Tags.begin (); i != iend; i++) {
			unsigned end = (*i)->GetEndIndex (), start = (*i)->GetStartIndex ();
			if (end < static_cast <unsigned> (pos))
				continue;
			if (end == static_cast <unsigned> (pos)) {
				if (borders[(*i)->GetTag ()] != NULL) {
					if (*(*i) == *borders[(*i)->GetTag ()]) {
						// merge the two tags
						(*i)->SetEndIndex (borders[(*i)->GetTag ()]->GetEndIndex ());
						extra_tags.push_front (borders[(*i)->GetTag ()]); // will be deleted
					}
				} else
					borders[(*i)->GetTag ()] = *i;
				continue;
			}
			if (end - static_cast <unsigned> (pos) > length)
				end -= length;
			else
				end = static_cast <unsigned> (pos);
			(*i)->SetEndIndex (end);
			if (start < static_cast <unsigned> (pos))
				continue;
			if (start == static_cast <unsigned> (pos)) {
				if (end == static_cast <unsigned> (pos)) {
					extra_tags.push_front (*i); // will be deleted
					continue;
				}
				if (borders[(*i)->GetTag ()] != NULL) {
					if (*(*i) == *borders[(*i)->GetTag ()]) {
						// merge the two tags
						(*i)->SetStartIndex (borders[(*i)->GetTag ()]->GetStartIndex ());
						extra_tags.push_front (borders[(*i)->GetTag ()]); // will be deleted
					}
				} else
					borders[(*i)->GetTag ()] = *i;
				continue;
			}
			// if we are there, the tag must be translated
			if (start - static_cast <unsigned> (pos) > length)
				start -= length;
			else
				start = static_cast <unsigned> (pos);
			if (start >= end) { // == should be enough
				extra_tags.push_front (*i); // will be deleted
				continue;
			}
			(*i)->SetStartIndex (start);
			if (start == static_cast <unsigned> (pos) || end == static_cast <unsigned> (pos)) {
				if (borders[(*i)->GetTag ()] != NULL) {
					if (*(*i) == *borders[(*i)->GetTag ()]) {
						// merge the two tags
						if (start == static_cast <unsigned> (pos))
							(*i)->SetStartIndex (borders[(*i)->GetTag ()]->GetStartIndex ());
						else
							(*i)->SetEndIndex (borders[(*i)->GetTag ()]->GetStartIndex ());
						extra_tags.push_front (borders[(*i)->GetTag ()]); // will be deleted
					}
				} else
					borders[(*i)->GetTag ()] = *i;
			}
		}
		// now delete extra tags
		iend = extra_tags.end ();
		for (i = extra_tags.begin (); i != iend; i++) {
			m_Tags.remove (*i);
			delete (*i);
		}
		extra_tags.clear ();
		iend = m_Tags.end ();
	}
	if (nl == 0) {
		pango_layout_set_text (m_Runs.front ()->m_Layout, m_Text.c_str (), -1); // FIXME: parse for line breaks and update runs
		m_Runs.front ()->m_Length = m_Text.length ();
		RebuildAttributes ();
		m_CurPos = m_StartSel = pos;
		SetPosition (m_x, m_y);
		//TODO: update attribute list
		return;
	}
	m_Text.insert (pos, str);
	TextTagList::iterator j, jend = m_CurTags->end ();
	for (j = m_CurTags->begin (); j != jend; j++)
		extra_tags.push_front (*j);
	for (i = m_Tags.begin (); i != iend; i++) {
		if ((*i)->GetEndIndex () < static_cast <unsigned> (pos))
			continue;
		if ((*i)->GetStartIndex () > static_cast <unsigned> (pos)) {
			(*i)->SetStartIndex ((*i)->GetStartIndex () + nl);
			(*i)->SetEndIndex ((*i)->GetEndIndex () + nl);
		}
		for (j = m_CurTags->begin (); j != jend; j++) {
			if ((*i)->GetTag () != (*j)->GetTag ())
				continue;
			// now let's see if it is the same tag or not
			if (*(*i) == *(*j)) {
				// just merge them
				(*i)->SetEndIndex ((*i)->GetEndIndex () + nl);
				extra_tags.remove (*j);
			} else if ((*i)->GetStartIndex () < static_cast <unsigned> (pos) && (*i)->GetEndIndex () > static_cast <unsigned> (pos)) {
				TextTag *tag = (*i)->Duplicate ();
				tag->SetStartIndex (pos + nl);
				tag->SetEndIndex ((*i)->GetEndIndex () + nl);
				(*i)->SetEndIndex (pos);
				new_tags.push_front (tag);
			} else if ((*i)->GetStartIndex () == static_cast <unsigned> (pos)) {
				(*i)->SetStartIndex ((*i)->GetStartIndex () + nl);
				(*i)->SetEndIndex ((*i)->GetEndIndex () + nl);
			}
			break; // m_CurTags should not have more than one tag of each category
		}
	}
	// now add all new tags to the list
	iend = new_tags.end ();
	for (i = new_tags.begin (); i != iend; i++)
		if ((*i)->GetPriority () == TagPriorityFirst)
			m_Tags.push_front (*i);
		else
			m_Tags.push_back (*i);
	new_tags.clear (); // avoid destroying the tags
	
	iend = extra_tags.end ();
	for (i = extra_tags.begin (); i != iend; i++) {
		TextTag *tag = (*i)->Duplicate ();
		tag->SetStartIndex (pos);
		tag->SetEndIndex (pos + nl);
		if (tag->GetPriority () == TagPriorityFirst)
			m_Tags.push_front (tag);
		else
			m_Tags.push_back (tag);
	}
	extra_tags.clear (); // avoid destroying the current tags
	pango_layout_set_text (m_Runs.front ()->m_Layout, m_Text.c_str (), -1); // FIXME: parse for line breaks and update runs
	m_Runs.front ()->m_Length = m_Text.length ();
	RebuildAttributes ();
	m_CurPos = m_StartSel = pos + str.length ();
	SetPosition (m_x, m_y);
}

static std::string empty_st = "";

bool Text::OnKeyPressed (GdkEventKey *event)
{
	TextClient *client = dynamic_cast <TextClient *> (GetClient ());
	if (gtk_im_context_filter_keypress (m_ImContext, event)) {
		if (client)
			client->TextChanged (m_CurPos);
		return true;
	}

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
		if (client)
			client->TextChanged (m_CurPos);
		break;

	/* MOVEMENT */
	case GDK_Right:
		if (m_CurPos == m_Text.length ())
			break;
		if (event->state & GDK_CONTROL_MASK) {
			/* move to end of word */
			char const* s = m_Text.c_str ();
			char *p = g_utf8_next_char (s + m_CurPos);
			while (*p && (!g_unichar_isgraph (g_utf8_get_char(p)) || g_unichar_ispunct (g_utf8_get_char(p))))
				p = g_utf8_next_char (p);
			while (g_unichar_isgraph (g_utf8_get_char(p)) && !g_unichar_ispunct (g_utf8_get_char(p)))
				p = g_utf8_next_char (p);
			m_CurPos = p - s;
			if (!(event->state & GDK_SHIFT_MASK))
				m_StartSel = m_CurPos;
			Invalidate ();
		} else {
			char const* s = m_Text.c_str ();
			char *p = g_utf8_next_char (s + m_CurPos);
			if (!p)
				break;
			m_CurPos = p - s;
			if (!(event->state & GDK_SHIFT_MASK))
				m_StartSel = m_CurPos;
			Invalidate ();
		}
		if (client)
			client->SelectionChanged (m_StartSel, m_CurPos);
		break;
	case GDK_Left:
		if (m_CurPos == 0)
			break;
		if (event->state & GDK_CONTROL_MASK) {
			/* move to start of word */
			char const* s = m_Text.c_str ();
			char *p = g_utf8_prev_char (s + m_CurPos);
			while (p != s && (!g_unichar_isgraph (g_utf8_get_char(p)) || g_unichar_ispunct (g_utf8_get_char (p))))
				p = g_utf8_prev_char (p);
			while (p != s && g_unichar_isgraph (g_utf8_get_char(p)) && !g_unichar_ispunct (g_utf8_get_char(p)))
				p = g_utf8_prev_char (p);
			if (!g_unichar_isgraph (g_utf8_get_char(p))) // don't go to the previous word end
				p = g_utf8_next_char (p);
			m_CurPos = p - s;
			if (!(event->state & GDK_SHIFT_MASK))
				m_StartSel = m_CurPos;
			Invalidate ();
		} else {
			char const* s = m_Text.c_str ();
			char *p = g_utf8_prev_char (s + m_CurPos);
			m_CurPos = p - s;
			if (!(event->state & GDK_SHIFT_MASK))
				m_StartSel = m_CurPos;
			Invalidate ();
		}
		if (client)
			client->SelectionChanged (m_StartSel, m_CurPos);
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
		if (m_CurPos != m_StartSel) {
			ReplaceText (empty_st, MIN (m_CurPos, m_StartSel), abs (m_CurPos - m_StartSel));
			if (client)
				client->TextChanged (m_CurPos);
			break;
		}
		if (m_CurPos == m_Text.length ())
			break;
		char const* s = m_Text.c_str ();
		char *p = g_utf8_next_char (s + m_CurPos);
		int new_pos = p - s;
		ReplaceText (empty_st, m_CurPos, new_pos - m_CurPos);
		if (client)
			client->TextChanged (m_CurPos);
		break;
	}
	case GDK_d:
		/* TODO: write this code */
		break;
	case GDK_BackSpace: {
		if (m_CurPos != m_StartSel) {
			ReplaceText (empty_st, MIN (m_CurPos, m_StartSel), abs (m_CurPos - m_StartSel));
			if (client)
				client->TextChanged (m_CurPos);
			break;
		}
		if (m_CurPos == 0)
			break;
		char const* s = m_Text.c_str ();
		char *p = g_utf8_prev_char (s + m_CurPos);
		int new_pos = p - s;
		ReplaceText (empty_st, new_pos, m_CurPos - new_pos);
		if (client)
			client->TextChanged (m_CurPos);
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

void Text::RebuildAttributes ()
{
	std::list <TextRun *>::iterator run, end_run = m_Runs.end ();
	TextTagList::iterator tag, end_tag = m_Tags.end ();
	for (run = m_Runs.begin (); run != end_run; run++) {
		PangoAttrList *l = pango_attr_list_new ();
		for (tag = m_Tags.begin (); tag != end_tag; tag++) {
			if ((*tag)->GetEndIndex () <= (*run)->m_Index || (*tag)->GetStartIndex () >= (*run)->m_Index + (*run)->m_Length)
				continue;
			unsigned start = ((*tag)->GetStartIndex () > (*run)->m_Index)? (*tag)->GetStartIndex () - (*run)->m_Index: 0;
			unsigned end = ((*tag)->GetEndIndex () < (*run)->m_Index + (*run)->m_Length)? (*tag)->GetEndIndex () - (*run)->m_Index: (*run)->m_Length;
			(*tag)->Filter (l, start, end);
		}
		pango_layout_set_attributes ((*run)->m_Layout, l);
		pango_attr_list_unref (l);
	}
	// force reposition and redraw
	SetPosition (m_x, m_y);
}

void Text::OnButtonPressed (double x, double y)
{
	x -= m_x0;
	y -= m_y0;
	int index, trailing;
	std::list <TextRun *>::iterator run, end_run = m_Runs.end ();
	for (run = m_Runs.begin (); run != end_run; run++)
		if (pango_layout_xy_to_index ((*run)->m_Layout, x * PANGO_SCALE, y * PANGO_SCALE, &index, &trailing)) {
			m_CurPos = m_StartSel = index + trailing + (*run)->m_Index;
			TextClient *client = dynamic_cast <TextClient *> (GetClient ());
			if (client)
				client->SelectionChanged (m_StartSel, m_CurPos);
			return;
		}
}

void Text::OnDrag (double x, double y)
{
	int index, trailing;
	x -= m_x0;
	y -= m_y0;
	std::list <TextRun *>::iterator run, end_run = m_Runs.end ();
	for (run = m_Runs.begin (); run != end_run; run++)
		if (pango_layout_xy_to_index ((*run)->m_Layout, x * PANGO_SCALE, y * PANGO_SCALE, &index, &trailing)) {
			m_CurPos = index + trailing + (*run)->m_Index;
			Invalidate ();
			TextClient *client = dynamic_cast <TextClient *> (GetClient ());
			if (client)
				client->SelectionChanged (m_StartSel, m_CurPos);
			return;
		}
}

void Text::GetSelectionBounds (unsigned &start, unsigned &end)
{
	start = m_StartSel;
	end = m_CurPos;
}

void Text::SetSelectionBounds (unsigned start, unsigned end)
{
	GetText (); // force update of m_Text
	m_StartSel = (start > m_Text.length ())? m_Text.length (): start;
	m_CurPos = (end > m_Text.length ())? m_Text.length (): end;
	TextClient *client = dynamic_cast <TextClient *> (GetClient ());
	if (client)
		client->SelectionChanged (m_StartSel, m_CurPos);
	Invalidate ();
}

}
