// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gccv/brackets.cc 
 *
 * Copyright (C) 2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "brackets.h"
#include "text.h"
#include <map>

#include <cstring>

namespace gccv {

/******************************************************************************/
/* Brackets metrics code                                                      */
/******************************************************************************/

struct _BracketsMetrics
{
	//round brackets metrics
	double rnheight, rnyoffset, rnwidth, rntheight, rntyoffset, rnbheight, rnmheight, rnmwidth;
	//square brackets metrics
	double sqheight, sqyoffset, sqwidth, sqtheight, sqtyoffset, sqbheight, sqmheight, sqmwidth;
	//curly brackets metrics
	double cyheight, cyyoffset, cywidth, cytheight, cytyoffset, cybheight, cymheight, cymwidth, cyeheight;
};

static std::map <std::string, BracketsMetrics> BracketsMetricsMap;
static BracketsMetrics const *GetBracketsMetrics (std::string &fontdesc)
{
	std::map <std::string, BracketsMetrics>::iterator it = BracketsMetricsMap.find (fontdesc);
	if (it == BracketsMetricsMap.end ()) {
		BracketsMetrics bm;
		// populate the struct
		PangoContext *context = gccv::Text::GetContext ();
		PangoLayout *layout = pango_layout_new (context);
		PangoFontDescription *desc = pango_font_description_from_string (fontdesc.c_str ());
		pango_layout_set_font_description (layout, desc);
		pango_font_description_free (desc);
		PangoRectangle rect, log;
		pango_layout_set_text (layout, "[", 1);
		pango_layout_get_extents (layout, &rect, &log);
		bm.sqheight = static_cast <double> (rect.height) / PANGO_SCALE;
		bm.sqyoffset = static_cast <double> (rect.y) / PANGO_SCALE;
		bm.sqwidth = static_cast <double> (log.width) / PANGO_SCALE;
		pango_layout_set_text (layout, "⎡", -1);
		pango_layout_get_extents (layout, &rect, NULL);
		bm.sqtheight = static_cast <double> (rect.height) / PANGO_SCALE;
		bm.sqtyoffset = static_cast <double> (rect.y) / PANGO_SCALE;
		pango_layout_set_text (layout, "⎢", -1);
		pango_layout_get_extents (layout, &rect, &log);
		bm.sqmheight = static_cast <double> (rect.height) / PANGO_SCALE;
		bm.sqmwidth = static_cast <double> (log.width) / PANGO_SCALE;
		pango_layout_set_text (layout, "⎣", -1);
		pango_layout_get_extents (layout, &rect, NULL);
		bm.sqbheight = static_cast <double> (rect.height) / PANGO_SCALE;
		pango_layout_set_text (layout, "(", -1);
		pango_layout_get_extents (layout, &rect, &log);
		bm.rnheight = static_cast <double> (rect.height) / PANGO_SCALE;
		bm.rnyoffset = static_cast <double> (rect.y) / PANGO_SCALE;
		bm.rnwidth = static_cast <double> (log.width) / PANGO_SCALE;
		pango_layout_set_text (layout, "⎛", -1);
		pango_layout_get_extents (layout, &rect, NULL);
		bm.rntheight = static_cast <double> (rect.height) / PANGO_SCALE;
		bm.rntyoffset = static_cast <double> (rect.y) / PANGO_SCALE;
		pango_layout_set_text (layout, "⎜", -1);
		pango_layout_get_extents (layout, &rect, &log);
		bm.rnmheight = static_cast <double> (rect.height) / PANGO_SCALE;
		bm.rnmwidth = static_cast <double> (log.width) / PANGO_SCALE;
		pango_layout_set_text (layout, "⎝", -1);
		pango_layout_get_extents (layout, &rect, NULL);
		bm.rnbheight = static_cast <double> (rect.height) / PANGO_SCALE;
		pango_layout_set_text (layout, "{", -1);
		pango_layout_get_extents (layout, &rect, &log);
		bm.cyheight = static_cast <double> (rect.height) / PANGO_SCALE;
		bm.cyyoffset = static_cast <double> (rect.y) / PANGO_SCALE;
		bm.cywidth = static_cast <double> (log.width) / PANGO_SCALE;
		pango_layout_set_text (layout, "⎧", -1);
		pango_layout_get_extents (layout, &rect, NULL);
		bm.cytheight = static_cast <double> (rect.height) / PANGO_SCALE;
		bm.cytyoffset = static_cast <double> (rect.y) / PANGO_SCALE;
		pango_layout_set_text (layout, "⎨", -1);
		pango_layout_get_extents (layout, &rect, &log);
		bm.cymheight = static_cast <double> (rect.height) / PANGO_SCALE;
		bm.cymwidth = static_cast <double> (log.width) / PANGO_SCALE;
		pango_layout_set_text (layout, "⎩", -1);
		pango_layout_get_extents (layout, &rect, NULL);
		bm.cybheight = static_cast <double> (rect.height) / PANGO_SCALE;
		pango_layout_set_text (layout, "⎪", -1);
		pango_layout_get_extents (layout, &rect, NULL);
		bm.cyeheight = static_cast <double> (rect.height) / PANGO_SCALE;
		g_object_unref (layout);
		BracketsMetricsMap[fontdesc] = bm;
		it = BracketsMetricsMap.find (fontdesc);
	}
	return &(*it).second;
}

/******************************************************************************/
/* Brackets class implementation                                              */
/******************************************************************************/
Brackets::Brackets (Canvas *canvas, BracketsTypes type, BracketsUses used, char const *fontdesc, double x0, double y0, double x1, double y1):
	Item (canvas)
{
	m_FontDesc = fontdesc;
	m_x0 = x0;
	m_y0 = y0;
	m_x1 = x1;
	m_y1 = y1;
	SetType (type);
	m_Used = used;
	m_Color = GO_COLOR_BLACK;
	BoundsChanged ();
	Invalidate ();
}

Brackets::Brackets (Group *parent, BracketsTypes type, BracketsUses used, char const *fontdesc, double x0, double y0, double x1, double y1, ItemClient *client):
	Item (parent, client)
{
	m_FontDesc = fontdesc;
	m_x0 = x0;
	m_y0 = y0;
	m_x1 = x1;
	m_y1 = y1;
	SetType (type);
	m_Used = used;
	m_Color = GO_COLOR_BLACK;
	BoundsChanged ();
	Invalidate ();
}

Brackets::~Brackets ()
{
	m_Elems.clear ();	// not sure it is needed
}

void Brackets::SetPosition (double x0, double y0, double x1, double y1)
{
	Invalidate ();
	m_x0 = x0;
	m_y0 = y0;
	m_x1 = x1;
	m_y1 = y1;
	BoundsChanged ();
	Invalidate ();
}

void Brackets::GetPosition (double &x0, double &y0, double &x1, double &y1)
{
	x0 = m_x0;
	y0 = m_y0;
	x1 = m_x1;
	y1 = m_y1;
}

double Brackets::Distance (double x, double y, Item **item) const
{
	double x_min, x_max;
	if (item)
		*item = const_cast < Brackets * > (this);
	if (m_Used == BracketsBoth) {
		if (x - m_x0 < (m_x1 - m_x0) / 2.) {
			// nearest bracket is opening bracket
			x_min = Item::m_x0;
			x_max = m_x0;
		} else {
			x_min = m_x1;
			x_max = Item::m_x1;
		}
	} else {
		x_min = Item::m_x0;
		x_max = Item::m_x1;
	}
	if (x < x_min) {
		if (y < Item::m_y0)
			y -= Item::m_y0;
		else if (y > Item::m_y1)
			y -= Item::m_y1;
		else
			return x_min - x;
		x -= x_min;
		return sqrt (x * x + y * y);
	} else if (x > x_max) {
		if (y < Item::m_y0)
			y -= Item::m_y0;
		else if (y > Item::m_y1)
			y -= Item::m_y1;
		else
			return x - x_max;
		x -= x_max;
		return sqrt (x * x + y * y);
	} else {
		if (y < Item::m_y0)
			return Item::m_y0 - y;
		else if (y > Item::m_y1)
			return y - Item::m_y1;
		else return 0.;
	}
	return G_MAXDOUBLE;
}

void Brackets::Draw (cairo_t *cr, G_GNUC_UNUSED bool is_vector) const
{
	PangoLayout *layout = pango_layout_new (gccv::Text::GetContext ());
	PangoFontDescription *desc = pango_font_description_from_string (m_FontDesc.c_str ());
	pango_layout_set_font_description (layout, desc);
	pango_font_description_free (desc);
	cairo_save (cr);
	cairo_set_source_rgba (cr, GO_COLOR_TO_CAIRO (m_Color));
	std::list < BracketElem >::const_iterator it, end = m_Elems.end ();
	for (it = m_Elems.begin (); it != end; it++) {
		pango_layout_set_text (layout, (*it).ch, -1);
		cairo_move_to (cr, (*it).x, (*it).y);
		if ((*it).needs_clip) {
			cairo_save (cr);
			cairo_rel_move_to (cr, 0., (*it).offset);
			cairo_rel_line_to (cr, (*it).w, 0.);
			cairo_rel_line_to (cr, 0., (*it).h);
			cairo_rel_line_to (cr, -(*it).w, 0.);
			cairo_rel_line_to (cr, 0., -(*it).h);
			cairo_close_path (cr);
			cairo_clip (cr);
			cairo_move_to (cr, (*it).x, (*it).y);
			pango_cairo_show_layout (cr, layout);
			cairo_restore (cr);
		} else
			pango_cairo_show_layout (cr, layout);
	}
	g_object_unref (layout);
	cairo_restore (cr);
}

void Brackets::Move (double x, double y)
{
	Invalidate ();
	m_x0 += x;
	m_y0 += y;
	m_x1 += x;
	m_y1 += y;
	BoundsChanged ();
	Invalidate ();
}

void Brackets::UpdateBounds ()
{
	BracketElem elem;
	elem.needs_clip = false;
	elem.offset = 0.;
	m_Elems.clear ();
	m_Metrics = GetBracketsMetrics (m_FontDesc);
	double height = m_y1 - m_y0;
	switch (m_Type) {
	case BracketsTypeNormal:
		if (height <  m_Metrics->rnheight) {
			Item::m_y0 = m_y0 - (m_Metrics->rnheight - height) / 2. - m_Metrics->rnyoffset;
			Item::m_y1 = Item::m_y0 + m_Metrics->rnheight + 2 * m_Metrics->rnyoffset; // a bit larger than really needed
			if (m_Used & BracketsOpening) {
				Item::m_x0 = m_x0 - m_Metrics->rnwidth;
				elem.x = Item::m_x0;
				elem.y = Item::m_y0;
				elem.ch = "(";
				m_Elems.push_back (elem);
			} else
				Item::m_x0 = m_x1;
			if (m_Used & BracketsClosing) {
				Item::m_x1 = m_x1 + m_Metrics->rnwidth;
				elem.x = m_x1;
				elem.y = Item::m_y0;
				elem.ch = ")";
				m_Elems.push_back (elem);
			} else
				Item::m_x1 = m_x0;
		} else if (height < m_Metrics->rntheight + m_Metrics->rnbheight) {
			Item::m_y0 = m_y0 - (m_Metrics->rntheight + m_Metrics->rnbheight - height) / 2. - m_Metrics->rntyoffset;
			Item::m_y1 = Item::m_y0 + m_Metrics->rntheight + m_Metrics->rnbheight + 2 * m_Metrics->rntyoffset; // a bit larger than really needed
			if (m_Used & BracketsOpening) {
				Item::m_x0 = m_x0 - m_Metrics->rnmwidth;
				elem.x = m_x0 - m_Metrics->rnmwidth;
				elem.y = Item::m_y0;
				elem.ch = "⎛";
				m_Elems.push_back (elem);
				elem.y += m_Metrics->rntheight + m_Metrics->rntyoffset;
				elem.ch = "⎝";
				m_Elems.push_back (elem);
			} else
				Item::m_x0 = m_x1;
			if (m_Used & BracketsClosing) {
				Item::m_x1 = m_x1 + m_Metrics->rnmwidth;
				elem.x = m_x1;
				elem.y = Item::m_y0;
				elem.ch = "⎞";
				m_Elems.push_back (elem);
				elem.y += m_Metrics->rntheight + m_Metrics->rntyoffset;
				elem.ch = "⎠";
				m_Elems.push_back (elem);
			} else
				Item::m_x1 = m_x0;
		} else {
			unsigned i, elems = ceil ((height - m_Metrics->rntheight - m_Metrics->rnbheight) / m_Metrics->rnmheight);
			Item::m_y0 = m_y0;
			Item::m_y1 = m_y1;
			elem.w = m_Metrics->rnmwidth;
			if (m_Used & BracketsOpening) {
				Item::m_x0 = m_x0 - m_Metrics->rnmwidth;
				elem.x = m_x0 - m_Metrics->rnmwidth;
				elem.y = m_y0 - m_Metrics->rntyoffset;
				elem.ch = "⎛";
				m_Elems.push_back (elem);
				elem.y = m_y1 - m_Metrics->rnbheight;
				elem.ch = "⎝";
				m_Elems.push_back (elem);
				elem.y = m_y0 + m_Metrics->rntheight;
				elem.ch = "⎜";
				for (i = 1; i < elems; i++) {
					m_Elems.push_back (elem);
					elem.y += m_Metrics->rnmheight;
				}
				elem.h = m_y1 - elem.y - m_Metrics->rntheight;
				if (elem.h > 0.) {
					elem.needs_clip = true;
					m_Elems.push_back (elem);
					elem.needs_clip = false;
				}
			} else
				Item::m_x0 = m_x1;
			if (m_Used & BracketsClosing) {
				Item::m_x1 = m_x1 + m_Metrics->rnmwidth;
				elem.x = m_x1;
				elem.y = m_y0 - m_Metrics->rntyoffset;
				elem.ch = "⎞";
				m_Elems.push_back (elem);
				elem.y = m_y1 - m_Metrics->rnbheight;
				elem.ch = "⎠";
				m_Elems.push_back (elem);
				elem.y = m_y0 + m_Metrics->rntheight;
				elem.ch = "⎟";
				for (i = 1; i < elems; i++) {
					m_Elems.push_back (elem);
					elem.y += m_Metrics->rnmheight;
				}
				elem.h = m_y1 - elem.y - m_Metrics->rntheight;
				if (elem.h > 0.) {
					elem.needs_clip = true;
					m_Elems.push_back (elem);
				}
			} else
				Item::m_x1 = m_x0;
		}
		break;
	case BracketsTypeSquare:
		if (height <  m_Metrics->sqheight) {
			Item::m_y0 = m_y0 - (m_Metrics->sqheight - height) / 2. - m_Metrics->sqyoffset;
			Item::m_y1 = Item::m_y0 + m_Metrics->sqheight + 2 * m_Metrics->sqyoffset; // a bit larger than really needed
			if (m_Used & BracketsOpening) {
				Item::m_x0 = m_x0 - m_Metrics->sqwidth;
				elem.x = Item::m_x0;
				elem.y = Item::m_y0;
				elem.ch = "[";
				m_Elems.push_back (elem);
			} else
				Item::m_x0 = m_x1;
			if (m_Used & BracketsClosing) {
				Item::m_x1 = m_x1 + m_Metrics->sqwidth;
				elem.x = m_x1;
				elem.y = Item::m_y0;
				elem.ch = "]";
				m_Elems.push_back (elem);
			} else
				Item::m_x1 = m_x0;
		} else if (height < m_Metrics->sqtheight + m_Metrics->sqbheight) {
			// as they are square brackets, they can be easily clipped
			double clip_height = (m_Metrics->sqtheight + m_Metrics->sqbheight - height) / 2;
			Item::m_y0 = m_y0;
			Item::m_y1 = m_y1;
			elem.needs_clip = true;
			elem.w = m_Metrics->sqmwidth;
			if (m_Used & BracketsOpening) {
				Item::m_x0 = m_x0 - m_Metrics->sqmwidth;
				elem.x = m_x0 - m_Metrics->sqmwidth;
				elem.y = Item::m_y0;
				elem.h = m_Metrics->sqtheight - clip_height;
				elem.ch = "⎡";
				m_Elems.push_back (elem);
				elem.y = m_y1 - m_Metrics->sqbheight;
				elem.h = m_Metrics->sqbheight - clip_height;
				elem.offset = clip_height;
				elem.ch = "⎣";
				m_Elems.push_back (elem);
				elem.offset = 0.;
			} else
				Item::m_x0 = m_x1;
			if (m_Used & BracketsClosing) {
				Item::m_x1 = m_x1 + m_Metrics->sqmwidth;
				elem.x = m_x1;
				elem.y = Item::m_y0;
				elem.h = m_Metrics->sqtheight - clip_height;
				elem.ch = "⎤";
				m_Elems.push_back (elem);
				elem.y = m_y1 - m_Metrics->sqbheight;
				elem.h = m_Metrics->sqbheight - clip_height;
				elem.offset = clip_height;
				elem.ch = "⎦";
				m_Elems.push_back (elem);
			} else
				Item::m_x1 = m_x0;
		} else {
			unsigned i, elems = ceil ((height - m_Metrics->sqtheight - m_Metrics->sqbheight) / m_Metrics->sqmheight);
			Item::m_y0 = m_y0;
			Item::m_y1 = m_y1;
			elem.w = m_Metrics->sqmwidth;
			if (m_Used & BracketsOpening) {
				Item::m_x0 = m_x0 - m_Metrics->sqmwidth;
				elem.x = m_x0 - m_Metrics->sqmwidth;
				elem.y = m_y0 - m_Metrics->sqtyoffset;
				elem.ch = "⎡";
				m_Elems.push_back (elem);
				elem.y = m_y1 - m_Metrics->sqbheight;
				elem.ch = "⎣";
				m_Elems.push_back (elem);
				elem.y = m_y0 + m_Metrics->sqtheight;
				elem.ch = "⎢";
				for (i = 1; i < elems; i++) {
					m_Elems.push_back (elem);
					elem.y += m_Metrics->sqmheight;
				}
				elem.h = m_y1 - elem.y - m_Metrics->sqtheight;
				if (elem.h > 0.) {
					elem.needs_clip = true;
					m_Elems.push_back (elem);
					elem.needs_clip = false;
				}
			} else
				Item::m_x0 = m_x1;
			if (m_Used & BracketsClosing) {
				Item::m_x1 = m_x1 + m_Metrics->sqmwidth;
				elem.x = m_x1;
				elem.y = m_y0 - m_Metrics->sqtyoffset;
				elem.ch = "⎤";
				m_Elems.push_back (elem);
				elem.y = m_y1 - m_Metrics->sqbheight;
				elem.ch = "⎦";
				m_Elems.push_back (elem);
				elem.y = m_y0 + m_Metrics->sqtheight;
				elem.ch = "⎥";
				for (i = 1; i < elems; i++) {
					m_Elems.push_back (elem);
					elem.y += m_Metrics->sqmheight;
				}
				elem.h = m_y1 - elem.y - m_Metrics->sqtheight;
				if (elem.h > 0.) {
					elem.needs_clip = true;
					m_Elems.push_back (elem);
				}
			} else
				Item::m_x1 = m_x0;
		}
		break;
	case BracketsTypeCurly:
		if (height <  m_Metrics->cyheight) {
			Item::m_y0 = m_y0 - (m_Metrics->cyheight - height) / 2. - m_Metrics->cyyoffset;
			Item::m_y1 = Item::m_y0 + m_Metrics->cyheight + 2 * m_Metrics->cyyoffset; // a bit larger than really needed
			if (m_Used & BracketsOpening) {
				Item::m_x0 = m_x0 - m_Metrics->cywidth;
				elem.x = Item::m_x0;
				elem.y = Item::m_y0;
				elem.ch = "{";
				m_Elems.push_back (elem);
			} else
				Item::m_x0 = m_x1;
			if (m_Used & BracketsClosing) {
				Item::m_x1 = m_x1 + m_Metrics->cywidth;
				elem.x = m_x1;
				elem.y = Item::m_y0;
				elem.ch = "}";
				m_Elems.push_back (elem);
			} else
				Item::m_x1 = m_x0;
		} else if (height < m_Metrics->cytheight + m_Metrics->cybheight + m_Metrics->cymheight) {
			Item::m_y0 = m_y0 - (m_Metrics->cytheight + m_Metrics->cybheight + m_Metrics->cymheight - height) / 2. - m_Metrics->cytyoffset;
			Item::m_y1 = Item::m_y0 + m_Metrics->cytheight + m_Metrics->cybheight + m_Metrics->cymheight + 2 * m_Metrics->cytyoffset; // a bit larger than really needed
			if (m_Used & BracketsOpening) {
				Item::m_x0 = m_x0 - m_Metrics->cymwidth;
				elem.x = m_x0 - m_Metrics->cymwidth;
				elem.y = Item::m_y0;
				elem.ch = "⎧";
				m_Elems.push_back (elem);
				elem.y += m_Metrics->cytheight + m_Metrics->cytyoffset;
				elem.ch = "⎨";
				m_Elems.push_back (elem);
				elem.y += m_Metrics->cymheight;
				elem.ch = "⎩";
				m_Elems.push_back (elem);
			} else
				Item::m_x0 = m_x1;
			if (m_Used & BracketsClosing) {
				Item::m_x1 = m_x1 + m_Metrics->cymwidth;
				elem.x = m_x1;
				elem.y = Item::m_y0;
				elem.ch = "⎫";
				m_Elems.push_back (elem);
				elem.y += m_Metrics->cytheight + m_Metrics->cytyoffset;
				elem.ch = "⎬";
				m_Elems.push_back (elem);
				elem.y += m_Metrics->cymheight;
				elem.ch = "⎭";
				m_Elems.push_back (elem);
			} else
				Item::m_x1 = m_x0;
		} else {
			unsigned i, elems = ceil ((height - m_Metrics->cytheight - m_Metrics->cybheight - m_Metrics->cymheight)/ 2. / m_Metrics->cyeheight);
			Item::m_y0 = m_y0;
			Item::m_y1 = m_y1;
			elem.w = m_Metrics->cymwidth;
			if (m_Used & BracketsOpening) {
				Item::m_x0 = m_x0 - m_Metrics->cymwidth;
				elem.x = m_x0 - m_Metrics->cymwidth;
				elem.y = m_y0 - m_Metrics->cytyoffset;
				elem.ch = "⎧";
				m_Elems.push_back (elem);
				elem.y = m_y1 - m_Metrics->cybheight;
				elem.ch = "⎩";
				m_Elems.push_back (elem);
				elem.y = (m_y0 + m_y1 - m_Metrics->cymheight) /2.;
				elem.ch = "⎨";
				m_Elems.push_back (elem);
				elem.ch = "⎪";
				double cury =  m_y0 + m_Metrics->cytheight, offset = (height + m_Metrics->cymheight)/ 2. - m_Metrics->cytheight;
				for (i = 1; i < elems; i++) {
					elem.y = cury;
					m_Elems.push_back (elem);
					elem.y = cury + offset;
					m_Elems.push_back (elem);
					cury += m_Metrics->cyeheight;
				}
				elem.h = m_y1 - cury - offset - m_Metrics->sqtheight;
				if (elem.h > 0.) {
					elem.needs_clip = true;
					elem.y = cury;
					m_Elems.push_back (elem);
					elem.y = cury + offset;
					m_Elems.push_back (elem);
					elem.needs_clip = false;
				}
			} else
				Item::m_x0 = m_x1;
			if (m_Used & BracketsClosing) {
				Item::m_x1 = m_x1 + m_Metrics->cymwidth;
				elem.x = m_x1;
				elem.y = m_y0 - m_Metrics->cytyoffset;
				elem.ch = "⎫";
				m_Elems.push_back (elem);
				elem.y = m_y1 - m_Metrics->cybheight;
				elem.ch = "⎭";
				m_Elems.push_back (elem);
				elem.y = (m_y0 + m_y1 - m_Metrics->cymheight) /2.;
				elem.ch = "⎬";
				m_Elems.push_back (elem);
				elem.ch = "⎪";
				double cury =  m_y0 + m_Metrics->cytheight, offset = (height + m_Metrics->cymheight)/ 2. - m_Metrics->cytheight;
				for (i = 1; i < elems; i++) {
					elem.y = cury;
					m_Elems.push_back (elem);
					elem.y = cury + offset;
					m_Elems.push_back (elem);
					cury += m_Metrics->cyeheight;
				}
				elem.h = m_y1 - cury - offset - m_Metrics->sqtheight;
				if (elem.h > 0.) {
					elem.needs_clip = true;
					elem.y = cury;
					m_Elems.push_back (elem);
					elem.y = cury + offset;
					m_Elems.push_back (elem);
					elem.needs_clip = false;
				}
			} else
				Item::m_x1 = m_x0;
		}
		break;
	}
	Item::UpdateBounds ();
}

}
