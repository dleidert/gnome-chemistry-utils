// -*- C++ -*-

/* 
 * GChemPaint library
 * brackets.cc 
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

namespace gccv {

/******************************************************************************/
/* Brackets metrics code                                                      */
/******************************************************************************/

struct _BracketsMetrics
{
	//round brackets metrics
	double rnheight, rnwidth, rntheight, rnbheight, rnmheight;
	//square brackets metrics
	double sqheight, sqwidth, sqtheight, sqbheight, sqmheight;
	//curly brackets metrics
	double cyheight, cywidth, cytheight, cybheight, cymheight, cyeheight;
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
		g_object_unref (desc);
		PangoRectangle rect;
		pango_layout_set_text (layout, "[", 1);
		pango_layout_get_extents (layout, &rect, NULL);
		bm.sqheight = static_cast <double> (rect.height) / PANGO_SCALE;
		bm.sqwidth = static_cast <double> (rect.width) / PANGO_SCALE;
		pango_layout_set_text (layout, "⎡", 1);
		pango_layout_get_extents (layout, &rect, NULL);
		bm.sqtheight = static_cast <double> (rect.height) / PANGO_SCALE;
		pango_layout_set_text (layout, "⎢", 1);
		pango_layout_get_extents (layout, &rect, NULL);
		bm.sqmheight = static_cast <double> (rect.height) / PANGO_SCALE;
		pango_layout_set_text (layout, "⎣", 1);
		pango_layout_get_extents (layout, &rect, NULL);
		bm.sqbheight = static_cast <double> (rect.height) / PANGO_SCALE;
		pango_layout_set_text (layout, "(", 1);
		pango_layout_get_extents (layout, &rect, NULL);
		bm.rnheight = static_cast <double> (rect.height) / PANGO_SCALE;
		bm.rnwidth = static_cast <double> (rect.width) / PANGO_SCALE;
		pango_layout_set_text (layout, "⎛", 1);
		pango_layout_get_extents (layout, &rect, NULL);
		bm.rntheight = static_cast <double> (rect.height) / PANGO_SCALE;
		pango_layout_set_text (layout, "⎜", 1);
		pango_layout_get_extents (layout, &rect, NULL);
		bm.rnmheight = static_cast <double> (rect.height) / PANGO_SCALE;
		pango_layout_set_text (layout, "⎝", 1);
		pango_layout_get_extents (layout, &rect, NULL);
		bm.rnbheight = static_cast <double> (rect.height) / PANGO_SCALE;
		pango_layout_set_text (layout, "{", 1);
		pango_layout_get_extents (layout, &rect, NULL);
		bm.cyheight = static_cast <double> (rect.height) / PANGO_SCALE;
		bm.cywidth = static_cast <double> (rect.width) / PANGO_SCALE;
		pango_layout_set_text (layout, "⎧", 1);
		pango_layout_get_extents (layout, &rect, NULL);
		bm.cytheight = static_cast <double> (rect.height) / PANGO_SCALE;
		pango_layout_set_text (layout, "⎨", 1);
		pango_layout_get_extents (layout, &rect, NULL);
		bm.cymheight = static_cast <double> (rect.height) / PANGO_SCALE;
		pango_layout_set_text (layout, "⎩", 1);
		pango_layout_get_extents (layout, &rect, NULL);
		bm.cybheight = static_cast <double> (rect.height) / PANGO_SCALE;
		pango_layout_set_text (layout, "⎪", 1);
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
Brackets::Brackets (Canvas *canvas, BracketsTypes type, char const *fontdesc, double x0, double y0, double x1, double y1):
	Item (canvas)
{
	SetFontDesc (fontdesc);
	SetPosition (x0, y0, x1, y1);
	SetType (type);
}

Brackets::Brackets (Group *parent, BracketsTypes type, char const *fontdesc, double x0, double y0, double x1, double y1, ItemClient *client):
	Item (parent, client)
{
	SetFontDesc (fontdesc);
	SetPosition (x0, y0, x1, y1);
	SetType (type);
}

Brackets::~Brackets ()
{
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
	return G_MAXDOUBLE;
}

void Brackets::Draw (cairo_t *cr, bool is_vector) const
{
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
	m_Metrics = GetBracketsMetrics (m_FontDesc);
}


}
