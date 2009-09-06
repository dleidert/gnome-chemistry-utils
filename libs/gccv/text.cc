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
#include "canvas.h"
#include "group.h"
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
//  gccv::TextRun class declaration

class TextRun
{
public:
	TextRun ();
	~TextRun ();

	void Draw (cairo_t *cr);

	PangoLayout *m_Layout;
	double m_X, m_Y, m_Width, m_Height, m_BaseLine, m_CharOffset;
	unsigned m_Index, m_Length, m_NbGlyphs;
	bool m_Stacked, m_NewLine;
};

////////////////////////////////////////////////////////////////////////////////
//  gccv::TextLine class implementation

class TextLine
{
public:
	TextLine ();
	~TextLine ();

	void DrawDecorations (cairo_t *cr, bool is_vector);

	double m_Width, m_Height, m_BaseLine;
	double m_Y;
	std::list <TextRun *> m_Runs;
	std::list <TextTag *> m_Decorations;
	unsigned m_Index, m_End;
};

TextLine::TextLine ()
{
	m_Width = m_Height = m_BaseLine = m_Y = 0.;
}

TextLine::~TextLine ()
{
}

void TextLine::DrawDecorations (cairo_t *cr, bool is_vector)
{
	unsigned start, end;
	double xstart, xend, y;
	std::list <TextTag *>::iterator tag, end_tag = m_Decorations.end ();
	std::list <TextRun *>::iterator run, end_run = m_Runs.end ();
	Tag type;
	TextDecoration dec;
	GOColor color = 0xff;
	for (tag = m_Decorations.begin (); tag != end_tag; tag++) {
		type = (*tag)->GetTag ();
		switch (type) {
		case Underline:
			dec = static_cast <UnderlineTextTag *> (*tag)->GetUnderline ();
			color = static_cast <UnderlineTextTag *> (*tag)->GetColor ();
			y = m_Y + m_BaseLine + (m_Height - m_BaseLine) * 3. / 4.;
			if (dec == TextDecorationDefault)
					dec = TextDecorationHigh;
			break;
		case Overline:
			dec = static_cast <OverlineTextTag *> (*tag)->GetOverline ();
			color = static_cast <OverlineTextTag *> (*tag)->GetColor ();
			y = m_Y + (m_Height - m_BaseLine) / 4.;
			if (dec == TextDecorationDefault)
					dec = TextDecorationLow;
			break;
		case Strikethrough:
			dec = static_cast <StrikethroughTextTag *> (*tag)->GetStrikethrough ();
			color = static_cast <StrikethroughTextTag *> (*tag)->GetColor ();
			y = m_Y + m_BaseLine * 3. / 4.;
			if (dec == TextDecorationDefault)
					dec = TextDecorationMedium;
			break;
		default:
			dec = TextDecorationNone;
			break;
		}
		if (dec == TextDecorationNone || color == 0)
			continue;
		double width = (m_Height - m_BaseLine) / 4.;
		// find the limits as indexes
		start = MAX (m_Index, (*tag)->GetStartIndex ());
		end = MIN (m_End, (*tag)->GetEndIndex ());
		// now convert to x coordinates
		xstart = xend = 0.;
		for (run = m_Runs.begin (); run != end_run; run++) {
			if (start > (*run)->m_Index + (*run)->m_Length)
				continue;
			if (start < (*run)->m_Index)
				xstart = (*run)->m_X;
			else {
				PangoRectangle rect;
				pango_layout_get_cursor_pos ((*run)->m_Layout, start - (*run)->m_Index, &rect, NULL);
				xstart = static_cast <double> (rect.x) / PANGO_SCALE;
			}
			break;
		}
		for (; run != end_run; run++) {
			if (end > (*run)->m_Index + (*run)->m_Length)
				continue;
			if (end < (*run)->m_Index)
				xend = (*run)->m_X;
			else {
				PangoRectangle rect;
				pango_layout_get_cursor_pos ((*run)->m_Layout, end - (*run)->m_Index, &rect, NULL);
				xend = static_cast <double> (rect.x) / PANGO_SCALE;
			}
		}
		if (xstart > xend) {
			double buf = xstart;
			xstart = xend;
			xend = buf;
		}
		if (!is_vector) {
			double scalex = 1., scale = 1.;
			cairo_user_to_device_distance (cr, &scalex, &scale);
			width = round (width * scale) / scale;
			y = round ((y + width / 2.) * scale) / scale - width / 2.;
			xstart = round (xstart * scale) / scale;
			xend = round (xend * scale) / scale;
		}
		switch (dec) {
		case TextDecorationHigh:
			y -= width;
			cairo_move_to (cr, xstart, y);
			cairo_line_to (cr, xend, y);
			break;	
		case TextDecorationLow:
			y += width;
			cairo_move_to (cr, xstart, y);
			cairo_line_to (cr, xend, y);
			break;	
		case TextDecorationMedium:
			cairo_move_to (cr, xstart, y);
			cairo_line_to (cr, xend, y);
			break;	
		case TextDecorationDouble:
			y += width;
			cairo_move_to (cr, xstart, y);
			cairo_line_to (cr, xend, y);
			y -= 2 * width;
			cairo_move_to (cr, xstart, y);
			cairo_line_to (cr, xend, y);
			break;	
		default:
			break;	
		}
		cairo_set_source_rgba (cr, GO_COLOR_TO_CAIRO (color));
		cairo_set_line_width (cr, width);
		cairo_set_line_cap (cr, CAIRO_LINE_CAP_BUTT);
		cairo_stroke (cr);
	}
}

////////////////////////////////////////////////////////////////////////////////
//  gccv::TextRun class implementation

TextRun::TextRun ()
{
	m_Layout = pango_layout_new (const_cast <PangoContext *> (Ctx.GetContext ()));
	m_X = m_Y = 0.;
	m_Index = m_Length = 0;
	m_CharOffset = 0;
	m_Stacked = m_NewLine = false;
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
	double offset = 0; // used for justification
	while (*text) {
		pango_layout_iter_get_char_extents (iter, &rect);
		curx = (double) rect.x / PANGO_SCALE + offset;
		offset += m_CharOffset;
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
		cairo_translate (cr, curx, ascent - (double) pango_layout_iter_get_baseline (local) / PANGO_SCALE);
		pango_cairo_show_layout (cr, pl);
		cairo_restore (cr);
		pango_layout_iter_free (local);
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
	m_LineOffset (0.),
	m_Justification (GTK_JUSTIFY_LEFT),
	m_Interline (0.),
	m_Width (0.), m_Height (0.)
{
	TextRun *run = new TextRun ();
	m_Runs.push_front (run);
	m_FontDesc = pango_font_description_copy (pango_layout_get_font_description (run->m_Layout));
	m_Lines = NULL;
	m_LinesNumber = 0;
	m_Color = GO_COLOR_BLACK;
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
	m_LineOffset (0.),
	m_Justification (GTK_JUSTIFY_LEFT),
	m_Interline (0.),
	m_Width (0.), m_Height (0.)
{
	TextRun *run = new TextRun ();
	m_Runs.push_front (run);
	m_FontDesc = pango_font_description_copy (pango_layout_get_font_description (run->m_Layout));
	m_Lines = NULL;
	m_LinesNumber = 0;
	m_Color = 0;
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
	if (m_Lines)
		delete [] m_Lines;
	pango_font_description_free (m_FontDesc);
}

void Text::SetPosition (double x, double y)
{
	double xr, yr, w, h;
	PangoRectangle r;
	std::list <TextRun *>::iterator i, end = m_Runs.end ();
	double x0, y0, x1, y1, x_, y_;
	i = m_Runs.begin ();
	pango_layout_get_extents ((*i)->m_Layout, NULL, &r);
	x0 = (*i)->m_X + static_cast <double> (r.x) / PANGO_SCALE;
	y0 = (*i)->m_Y + static_cast <double> (r.y) / PANGO_SCALE;
	x1 = x0 + static_cast <double> (r.width) / PANGO_SCALE;
	y1 = y0 + static_cast <double> (r.height) / PANGO_SCALE;
	for (i++; i != end; i++) {
		pango_layout_get_extents ((*i)->m_Layout, NULL, &r);
		x_ = (*i)->m_X + static_cast <double> (r.x) / PANGO_SCALE;
		y_ = (*i)->m_Y + static_cast <double> (r.y) / PANGO_SCALE;
		if (x_ < x0)
			x0 = x_;
		if (y_ < y0)
			y0 = y_;
		x_ += static_cast <double> (r.width) / PANGO_SCALE;
		y_ += static_cast <double> (r.height) / PANGO_SCALE;
		if (x_ > x1)
			x1 = x_;
		if (y_ > y1)
			y1 = y_;
	}
	m_x = x;
	m_y = y;
	m_Y = y0;
	m_Width = x1 - x0;
	m_Height = y1 - y0;
	w = m_Width + 2 * m_Padding;
	h = m_Height + 2 * m_Padding;
	PangoLayoutIter* iter = pango_layout_get_iter (m_Runs.front ()->m_Layout);
	m_Ascent = (double) pango_layout_iter_get_baseline (iter) / PANGO_SCALE + m_Runs.front ()->m_Y;
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
	if (m_BlinkSignal && start_sel != end_sel)
		for (i = m_Runs.begin (); i != end; i++) {
			// display the selection rectangle if needed (before drawing any text)
			if (start_sel < (*i)->m_Index + (*i)->m_Length && end_sel > (*i)->m_Index) {
				PangoRectangle rect;
				unsigned s = MAX (start_sel, (*i)->m_Index) - (*i)->m_Index,
						e = MIN (end_sel, (*i)->m_Index + (*i)->m_Length) - (*i)->m_Index;
				double x, y, w, h;
				cairo_set_source_rgb (cr, 0.85, 0.85, 0.85);
				pango_layout_get_cursor_pos ((*i)->m_Layout, s, &rect, NULL);
				x = (double) rect.x / PANGO_SCALE + (s > 0? s - 1: 0) * (*i)->m_CharOffset;
				h = (double) rect.height / PANGO_SCALE;
				y = starty + (*i)->m_Y + (double) rect.y / PANGO_SCALE;
				pango_layout_get_cursor_pos ((*i)->m_Layout, e, &rect, NULL);
				w = (double) rect.x / PANGO_SCALE + (e > 0? e - 1: 0) * (*i)->m_CharOffset - x;
				x += startx + (*i)->m_X;
				cairo_rectangle (cr, x, y, w, h);
				cairo_fill (cr);
			}
		}
	// now, draw the glyphs in each run
	for (i = m_Runs.begin (); i != end; i++) {
		// draw the text
		cairo_save (cr);
		cairo_translate (cr, startx + (*i)->m_X, starty + (*i)->m_Y);
		(*i)->Draw (cr);
		cairo_restore (cr);
		// draw the cursor if needed
		if (m_CursorVisible && ((m_CurPos > (*i)->m_Index && m_CurPos <= (*i)->m_Index + (*i)->m_Length) ||
		    ((*i)->m_NewLine && m_CurPos == (*i)->m_Index))) {
			PangoRectangle rect;
			pango_layout_get_cursor_pos ((*i)->m_Layout, m_CurPos - (*i)->m_Index, &rect, NULL);
			cairo_set_line_width (cr, 1.);
			int nbskip = (m_CurPos > (*i)->m_Index)? m_CurPos - (*i)->m_Index - 1: 0;
			cairo_move_to (cr, floor (startx + (*i)->m_X + (double) rect.x / PANGO_SCALE + nbskip * (*i)->m_CharOffset) + .5, floor (starty + (*i)->m_Y + (double) rect.y / PANGO_SCALE) + .5);
			cairo_rel_line_to (cr, 0, rect.height / PANGO_SCALE);
			cairo_set_source_rgb (cr, 0., 0., 0.);
			cairo_stroke (cr);
		}
	}
	// draw decorations (underline and friends)
	cairo_save (cr);
	if (!is_vector) {
		double scalex = 1., scale = 1.;
		cairo_user_to_device_distance (cr, &scalex, &scale);
		startx = round (startx * scale) / scale;
		starty = round (starty * scale) / scale;
	}
	cairo_translate (cr, startx, starty);
	for (unsigned line = 0; line < m_LinesNumber; line++)
		m_Lines[line].DrawDecorations (cr, is_vector);
	cairo_restore (cr);
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
	RebuildAttributes ();
}

void Text::SetText (std::string const &text)
{
	m_Text = text;
	pango_layout_set_text (m_Runs.front ()->m_Layout, text.c_str (), -1); // FIXME: parse for line breaks
	m_Runs.front ()->m_Length = strlen (text.c_str ());
	RebuildAttributes ();
}

char const *Text::GetText ()
{
	return m_Text.c_str ();
}

void Text::SetFontDescription (PangoFontDescription *desc)
{
	m_FontDesc = pango_font_description_copy (desc);
	std::list <TextRun *>::iterator i, end = m_Runs.end ();
	for (i = m_Runs.begin (); i != end; i++) {
		pango_layout_set_font_description ((*i)->m_Layout, m_FontDesc);
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
	double x0, x1, x2, x3, y0, y1, y2, y3, x, y;
	std::list <TextRun *>::iterator it = m_Runs.begin (), end = m_Runs.end ();
	pango_layout_get_extents ((*it)->m_Layout, &i, &l);
	x0 = (*it)->m_X + static_cast <double> (i.x) / PANGO_SCALE;
	y0 = (*it)->m_Y + static_cast <double> (i.y) / PANGO_SCALE;
	x1 = x0 + static_cast <double> (i.width) / PANGO_SCALE;
	y1 = y0 + static_cast <double> (i.height) / PANGO_SCALE;
	x2 = (*it)->m_X + static_cast <double> (l.x) / PANGO_SCALE;
	y2 = (*it)->m_X + static_cast <double> (l.y) / PANGO_SCALE;
	x3 = x2 + static_cast <double> (l.width) / PANGO_SCALE;
	y3 = y2 + static_cast <double> (l.height) / PANGO_SCALE;
	for (it++; it != end; it++) {
		pango_layout_get_extents ((*it)->m_Layout, &i, &l);
		x = (*it)->m_X + static_cast <double> (i.x) / PANGO_SCALE;
		y = (*it)->m_Y + static_cast <double> (i.y) / PANGO_SCALE;
		if (x < x0)
			x0 = x;
		if (y < y0)
			y0 = y;
		x += static_cast <double> (i.width) / PANGO_SCALE;
		y += static_cast <double> (i.height) / PANGO_SCALE;
		if (x > x1)
			x1 = x;
		if (y > y1)
			y1 = y;
		x = (*it)->m_X + static_cast <double> (l.x) / PANGO_SCALE;
		y = (*it)->m_Y + static_cast <double> (l.y) / PANGO_SCALE;
		if (x < x2)
			x2 = x;
		if (y < y2)
			y2 = y;
		x += static_cast <double> (l.width) / PANGO_SCALE;
		y += static_cast <double> (l.height) / PANGO_SCALE;
		if (x > x3)
			x3 = x;
		if (y > y3)
			y3 = y;
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
		ink->x0 = startx + x0;
		ink->y0 = starty + y0;
		ink->x1 = ink->x0 + x1 - x0;
		ink->y1 = ink->y0 + y1 - y0;
	}
	if (logical) {
		logical->x0 = startx + x2;
		logical->y0 = starty + y2;
		logical->x1 = logical->x0 + x3 - x2;
		logical->y1 = logical->y0 + y3 - y2;
	}
}

void Text::InsertTextTag (TextTag *tag, bool rebuild_attributes)
{
	// we need to filter tags to avoid duplicates
	std::list <TextTag *> new_tags, invalid;
	TextTagList::iterator i, iend = m_Tags.end ();
	TextTag *new_tag;
	for (i = m_Tags.begin (); i != m_Tags.end (); i++) {
		new_tag = tag->Restrict (*i);
		if (new_tag)
			new_tags.push_back (new_tag);
		if ((*i)->GetStartIndex () >= (*i)->GetEndIndex ())
			invalid.push_back (*i);
	}
	// no remove invalid tags
	while (!invalid.empty ()) {
		m_Tags.remove (invalid.front ());
		invalid.pop_front ();
	}
	// add split tags
	while (!new_tags.empty ()) {
		new_tag = new_tags.front ();
		new_tags.pop_front ();
		if (new_tag->GetPriority () == TagPriorityFirst)
			m_Tags.push_front (new_tag);
		else
			m_Tags.push_back (new_tag);
	}
	// now, insert the new tag
	if (tag->GetPriority () == TagPriorityFirst)
		m_Tags.push_front (tag);
	else
		m_Tags.push_back (tag);
	// now, rebuild pango attributes lists for modified runs.
	// Note: it would be better to update only the changed portion, but how???
	if (rebuild_attributes)
		RebuildAttributes ();
}

void Text::DeleteTextTag (TextTag *tag, bool rebuild_attributes)
{
	if (!tag)
		return;
	m_Tags.remove (tag);
	delete tag;
	if (rebuild_attributes)
		RebuildAttributes ();
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
	std::vector <TextTag *> new_tags (TextTag::MaxTag);
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
	for (int n = 0; n < TextTag::MaxTag; n++) 
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
		std::vector <TextTag *> borders (TextTag::MaxTag);
		for (int n = 0; n < TextTag::MaxTag; n++) 
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
		m_CurPos = m_StartSel = pos;
		RebuildAttributes ();
		SetPosition (m_x, m_y);
		return;
	}
	m_Text.insert (pos, str);
	TextTagList::iterator j, jend = m_CurTags->end ();
	for (j = m_CurTags->begin (); j != jend; j++)
		extra_tags.push_front (*j);
	for (i = m_Tags.begin (); i != iend; i++) {
		if ((*i)->GetEndIndex () < static_cast <unsigned> (pos))
			continue;
		if ((*i)->GetStartIndex () >= static_cast <unsigned> (pos)) {
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
	m_CurPos = m_StartSel = pos + str.length ();
	RebuildAttributes ();
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
	case GDK_KP_Enter: {
		m_Text.insert (m_CurPos, "\n");
		TextTag *tag = new NewLineTextTag ();
		tag->SetStartIndex (m_CurPos++);
		tag->SetEndIndex (m_CurPos);
		m_StartSel = m_CurPos;
		m_Tags.push_front (tag);
		RebuildAttributes ();
		SetPosition (m_x, m_y);
		if (client)
			client->TextChanged (m_CurPos);
		return true;
	}
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

bool Text::OnKeyReleased (GdkEventKey *event)
{
	TextClient *client = dynamic_cast <TextClient *> (GetClient ());
	if (gtk_im_context_filter_keypress (m_ImContext, event)) {
		if (client)
			client->TextChanged (m_CurPos);
		return true;
	}
	return false;
}

void Text::RebuildAttributes ()
{
	// This might be optimized: not everything should be scanned each time a change occur
	// first update runs list
	// we need to order tags
	m_Tags.sort (gccv::TextTag::Order);
	std::list <TextRun *>::iterator run, end_run = m_Runs.end ();
	for (run = m_Runs.begin ();run != end_run; run++) // delete all runs
		delete (*run);
	m_Runs.clear ();
	// Recreate first run and first line
	TextRun *new_run = new TextRun (), *last_run;
	pango_layout_set_font_description (new_run->m_Layout, m_FontDesc);
	m_Runs.push_front (new_run);
	last_run = new_run;
	TextTagList::iterator tag, end_tag = m_Tags.end ();
	bool stacked = false;
	unsigned lines = 1;
	std::string str;
	TextTagList decorations;
	unsigned cur_end = 0;
	for (tag = m_Tags.begin (); tag != end_tag; tag++) {
		if (stacked || (*tag)->GetStacked ()) {
			// we need a new run
			if (cur_end < (*tag)->GetStartIndex () && last_run->m_Stacked) {
				new_run = new TextRun ();
				pango_layout_set_font_description (new_run->m_Layout, m_FontDesc);
				new_run ->m_Index = cur_end;
				last_run->m_Length = new_run->m_Index - last_run->m_Index;
				last_run->m_NbGlyphs = g_utf8_strlen (m_Text.c_str () + last_run->m_Index, last_run->m_Length);
				new_run->m_Stacked = false;
				m_Runs.push_back (new_run);
				last_run = new_run;
			}
			cur_end = (*tag)->GetEndIndex ();
			new_run = new TextRun ();
			pango_layout_set_font_description (new_run->m_Layout, m_FontDesc);
			new_run ->m_Index = (*tag)->GetStartIndex ();
			last_run->m_Length = new_run->m_Index - last_run->m_Index;
			last_run->m_NbGlyphs = g_utf8_strlen (m_Text.c_str () + last_run->m_Index, last_run->m_Length);
			stacked = new_run->m_Stacked = (*tag)->GetStacked ();
			m_Runs.push_back (new_run);
			last_run = new_run;
		} else if ((*tag)->GetNewLine ()) {
			lines++;
			stacked = false;
			// we need a new run
			new_run = new TextRun ();
			pango_layout_set_font_description (new_run->m_Layout, m_FontDesc);
			new_run ->m_Index = (*tag)->GetStartIndex ();
			last_run->m_Length = new_run->m_Index - last_run->m_Index;
			last_run->m_NbGlyphs = g_utf8_strlen (m_Text.c_str () + last_run->m_Index, last_run->m_Length);
			new_run->m_Index++; // skip the new line character
			new_run->m_NewLine = true;
			m_Runs.push_back (new_run);
			last_run = new_run;
		} else if ((*tag)->GetTag () == Underline || (*tag)->GetTag () == Strikethrough || (*tag)->GetTag () == Overline)
			decorations.push_back (*tag);
	}
	last_run->m_Length = m_Text.length () - last_run->m_Index;
	last_run->m_NbGlyphs = g_utf8_strlen (m_Text.c_str () + last_run->m_Index, last_run->m_Length);
	// now update attributes for each run
	end_run = m_Runs.end ();
	PangoLayoutIter *iter;
	for (run = m_Runs.begin (); run != end_run; run++) {
		str = m_Text.substr ((*run)->m_Index, (*run)->m_Length);
		pango_layout_set_text ((*run)->m_Layout, str.c_str (), -1);
		PangoAttrList *l = pango_attr_list_new ();
		// set the default text color
		if (m_Color) {
			PangoAttribute *attr = pango_attr_foreground_new (GO_COLOR_UINT_R (m_Color) * 0x101, GO_COLOR_UINT_G (m_Color) * 0x101, GO_COLOR_UINT_B (m_Color) * 0x101);
			attr->start_index = 0;
			attr->end_index = (*run)->m_Length;
			pango_attr_list_insert (l, attr);
		}
		for (tag = m_Tags.begin (); tag != end_tag; tag++) {
			if ((*tag)->GetEndIndex () <= (*run)->m_Index || (*tag)->GetStartIndex () >= (*run)->m_Index + (*run)->m_Length)
				continue;
			unsigned start = ((*tag)->GetStartIndex () > (*run)->m_Index)? (*tag)->GetStartIndex () - (*run)->m_Index: 0;
			unsigned end = ((*tag)->GetEndIndex () < (*run)->m_Index + (*run)->m_Length)? (*tag)->GetEndIndex () - (*run)->m_Index: (*run)->m_Length;
			(*tag)->Filter (l, start, end);
		}
		pango_layout_set_attributes ((*run)->m_Layout, l);
		pango_attr_list_unref (l);
		PangoRectangle rect;
		pango_layout_get_extents ((*run)->m_Layout, NULL, &rect);
		(*run)->m_Width = (double) rect.width / PANGO_SCALE;
		(*run)->m_Height = (double) rect.height / PANGO_SCALE;
		iter = pango_layout_get_iter ((*run)->m_Layout);
		(*run)->m_BaseLine = (double) pango_layout_iter_get_baseline (iter) / PANGO_SCALE;
		pango_layout_iter_free (iter);
	}
	if (m_Lines)
		delete [] m_Lines;
	m_Lines = new TextLine[lines];
	m_LinesNumber = lines;
	// we now need to rebuild runs positions
	// FIXME: support several lines
	double curx = 0., curw = 0., cury = 0;
	unsigned cur_line = 0;
	m_Lines[0].m_Index = 0;
	double width = 0.;
	for (run = m_Runs.begin (); run != end_run; run++) {
		if ((*run)->m_Stacked) {
			if ((*run)->m_Width > curw)
				curw = (*run)->m_Width;
			(*run)->m_X = curx;
		} else if ((*run)->m_NewLine) {
			curw += curx;
			m_Lines[cur_line].m_Width = curw;
			m_Lines[cur_line].m_End = (*run)->m_Index;
			if (width < curw)
				width = curw;
			curw = 0.;
			curx = (*run)->m_Width;
			cur_line++;
			m_Lines[cur_line].m_Index = (*run)->m_Index - 1; // -1 because of the hidden \n
		} else {
			(*run)->m_X = curx + curw;
			curx += curw + (*run)->m_Width;
			curw = 0.;
		}
		m_Lines[cur_line].m_Runs.push_back (*run);
		if (m_Lines[cur_line].m_BaseLine < (*run)->m_BaseLine)
			m_Lines[cur_line].m_BaseLine = (*run)->m_BaseLine;
		if (m_Lines[cur_line].m_Height < (*run)->m_Height)
			m_Lines[cur_line].m_Height = (*run)->m_Height;
	}
	curw += curx;
	m_Lines[cur_line].m_Width = curw;
	run --;
	m_Lines[cur_line].m_End = (*run)->m_Index + (*run)->m_Length;
	if (width < curw)
		width = curw;
	end_tag = decorations.end ();
	for (cur_line = 0; cur_line < lines; cur_line++) {
		m_Lines[cur_line].m_Y = cury;
		end_run = m_Lines[cur_line].m_Runs.end ();
		for (run = m_Lines[cur_line].m_Runs.begin (); run != end_run; run++)
			(*run)->m_Y = cury + m_Lines[cur_line].m_BaseLine - (*run)->m_BaseLine;
		cury += m_Lines[cur_line].m_Height + m_Interline;
		// now manage justification
		if (m_Justification != GTK_JUSTIFY_LEFT && m_Lines[cur_line].m_Width < width) {
			switch (m_Justification) {
			case GTK_JUSTIFY_RIGHT:
				for (run = m_Lines[cur_line].m_Runs.begin (); run != end_run; run++)
					(*run)->m_X += width - m_Lines[cur_line].m_Width;
				break;
			case GTK_JUSTIFY_CENTER:
				for (run = m_Lines[cur_line].m_Runs.begin (); run != end_run; run++)
					(*run)->m_X += (width - m_Lines[cur_line].m_Width) / 2.;
				break;
			case GTK_JUSTIFY_FILL: {
				// this is a bit more difficult, current code might be suboptimal if there are stacked runs
				// evaluate how many characters are in the line
				unsigned nbchars = 0;
				stacked = false;
				unsigned *nc = new unsigned[m_Lines[cur_line].m_Runs.size ()]; // this is more than needed if we have stacked runs
				int cur, max = 0;
				nc[0] = 0;	// fortunately, we have at least one run
				for (run = m_Lines[cur_line].m_Runs.begin (); run != end_run; run++) {
					if ((*run)->m_Stacked) {
						stacked = true;
						if (nc[max] < (*run)->m_NbGlyphs)
							nc[max] = (*run)->m_NbGlyphs;
					} else {
						if (stacked)
							max++;
						nc[max++] = (*run)->m_NbGlyphs;
						nc[max] = 0;
						stacked = false;
					}
				}
				if (stacked)
					max++;
				for (cur = 0; cur < max; cur++)
					nbchars += nc[cur];
				// calculate the increment to add between two glyphs
				double incr = (width - m_Lines[cur_line].m_Width) / (nbchars - 1), curoffs = 0.;
				// update the runs accordingly
				cur = 0;
				for (run = m_Lines[cur_line].m_Runs.begin (); run != end_run; run++) {
					(*run)->m_X += curoffs;
					(*run)->m_CharOffset = incr;
					if ((*run)->m_Stacked)
						stacked = true;
					else {
						if (stacked) {
							stacked = false;
							curoffs += nc[cur] * incr;
							cur++;
						}
						curoffs += nc[cur] * incr;
						cur++;
					}
				}
				delete [] nc;
				break;
			}
			default:
				break;
			}
		}
		// now find which decorations apply to the line
		for (tag = decorations.begin (); tag != end_tag; tag++)
			if ((*tag)->GetStartIndex () < m_Lines[cur_line].m_End && (*tag)->GetEndIndex () > m_Lines[cur_line].m_Index)
				m_Lines[cur_line].m_Decorations.push_back (*tag);
	}
	decorations.clear ();
	// force reposition and redraw
	SetPosition (m_x, m_y);
}

void Text::OnButtonPressed (double x, double y)
{
	double x0 = m_x0, y0 = m_y0, x1 = 0., y1 = 0.;
	GetParent ()->AdjustBounds (x0, y0, x1, y1);
	x -= x0;
	y -= y0;
	unsigned index = GetIndexAt (x, y);
	if (index < G_MAXUINT) {
		m_CurPos = index;
		if ((GetCanvas ()->GetLastEventState () & GDK_SHIFT_MASK) == 0)
			m_StartSel = m_CurPos;
		TextClient *client = dynamic_cast <TextClient *> (GetClient ());
		if (client)
			client->SelectionChanged (m_StartSel, m_CurPos);
	}
}

void Text::OnDrag (double x, double y)
{
	double x0 = m_x0, y0 = m_y0, x1 = 0., y1 = 0.;
	GetParent ()->AdjustBounds (x0, y0, x1, y1);
	x -= x0;
	y -= y0;
	unsigned index = GetIndexAt (x, y);
	if (index < G_MAXUINT) {
		m_CurPos = index;
		Invalidate ();
		TextClient *client = dynamic_cast <TextClient *> (GetClient ());
		if (client)
			client->SelectionChanged (m_StartSel, m_CurPos);
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

unsigned Text::GetIndexAt (double x, double y)
{
	// first, find the line
	double starty = m_Interline / 2.;
	unsigned line = 0;
	unsigned result = 0;
	double runx = 0.;
	while (line < m_LinesNumber - 1) {
		if (y < starty + m_Lines[line].m_Height)
			break;
		starty += m_Lines[line].m_Height + m_Interline;
		line++;
	}
	// now, find the run
	std::list <TextRun *>::iterator run, end_run = m_Lines[line].m_Runs.end (), srun;
	for (run = m_Lines[line].m_Runs.begin (); run != end_run; run++) {
		runx = x - (*run)->m_X;
		if (runx <= (*run)->m_Width + (*run)->m_NbGlyphs * (*run)->m_CharOffset)
			break;
	}
	if (run == end_run)
		run--;
	// stacked, choose the right run as far as possible
	if ((*run)->m_Stacked) {
		while (run != m_Lines[line].m_Runs.begin () && (*run)->m_Stacked)
			run--;
		if (!(*run)->m_Stacked)
			run++;
		// we are now at the first run in the stack
		PangoRectangle rect;
		double ymin, ymax, dy = G_MAXDOUBLE;
		while (run != end_run && (*run)->m_Stacked) {
			pango_layout_get_extents ((*run)->m_Layout, &rect, NULL);
			ymin = (*run)->m_Y + static_cast <double> (rect.y) / PANGO_SCALE;
			ymax = ymin + static_cast <double> (rect.height) / PANGO_SCALE;
			if (y < ymin) {
				if (ymin - y < dy) {
					dy = ymin - y;
					srun = run;
				}
			} else if (y < ymax) {
				srun = run;
				break;
			} else {
				if (y - ymax < dy) {
					dy = y - ymax;
					srun = run;
				}
			}
			run++;
		}
		run = srun;
	}
	PangoLayoutIter* iter = pango_layout_get_iter ((*run)->m_Layout);
	PangoRectangle rect;
	pango_layout_iter_get_char_extents (iter, &rect);
	double curx = 0.;
	while (curx < runx) {
		if ((double) rect.width / PANGO_SCALE / 2. >= x - curx)
			break;
		result++;
		curx += (double) rect.width / PANGO_SCALE + (*run)->m_CharOffset;
		if (!pango_layout_iter_next_char (iter))
			break;
		pango_layout_iter_get_char_extents (iter, &rect);
	}
	pango_layout_iter_free (iter);
	if ((result += (*run)->m_Index) > m_Text.length ())
		result = m_Text.length ();
	return result;
}

bool Text::GetPositionAtIndex (unsigned index, Rect &rect)
{
	if (index > m_Text.length ())
		return false;
	std::list <TextRun *>::iterator run, end_run = m_Runs.end ();
	for (run = m_Runs.begin (); run != end_run; run++)
		if (index <= (*run)->m_Index + (*run)->m_Length) {
			PangoRectangle r;
			pango_layout_index_to_pos ((*run)->m_Layout, index - (*run)->m_Index, &r);
			rect.x0 = (double) r.x / PANGO_SCALE + (*run)->m_X + (index - (*run)->m_Index) * (*run)->m_CharOffset;
			rect.y0 = (double) r.y / PANGO_SCALE + (*run)->m_Y;
			rect.x1 = rect.x0 + (double) r.width / PANGO_SCALE;
			rect.y1 = (double) (r.y + r.height) / PANGO_SCALE + (*run)->m_Y;
			break;
		}
	return true;
}

void Text::SetColor (GOColor color)
{
	m_Color = color;
	RebuildAttributes ();
}

void Text::SetInterline (double interline, bool emit_changed)
{
	m_Interline = interline;
	RebuildAttributes ();
	SetPosition (m_x, m_y);
	if (emit_changed) {
		TextClient *client = dynamic_cast <TextClient *> (GetClient ());
		if (client)
			client->InterlineChanged (interline);
	}
}

void Text::SetJustification (GtkJustification justification, bool emit_changed)
{
	m_Justification = justification;
	RebuildAttributes ();
	Invalidate ();
	if (emit_changed) {
		TextClient *client = dynamic_cast <TextClient *> (GetClient ());
		if (client)
			client->JustificationChanged (justification);
	}
}

}
