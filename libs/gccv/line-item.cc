// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gccv/fill-item.cc
 *
 * Copyright (C) 2008-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "line-item.h"
#include "canvas.h"
#include <cstring>

namespace gccv {

LineItem::LineItem (Canvas *canvas):
	Item (canvas),
	m_Dashes (NULL),
	m_DashesNb (0),
	m_DashOffset (0.),
	m_LineWidth (1.),
	m_LineColor (0),
	m_AutoColor (false)
{
}

LineItem::LineItem (Group *parent, ItemClient *client):
	Item (parent, client),
	m_Dashes (NULL),
	m_DashesNb (0),
	m_DashOffset (0.),
	m_LineWidth (1.),
	m_LineColor (0),
	m_AutoColor (false)
{
}

LineItem::~LineItem ()
{
}

GOColor LineItem::GetEffectiveLineColor () const
{
	return m_AutoColor? GetCanvas ()->GetColor (): m_LineColor;
}

void LineItem::ApplyLine (cairo_t *cr) const
{
	cairo_save (cr);
	cairo_set_line_width (cr, m_LineWidth);
	cairo_set_source_rgba (cr, GO_COLOR_TO_CAIRO (m_AutoColor? GetCanvas ()->GetColor (): m_LineColor));
	if (m_Dashes)
		cairo_set_dash (cr, m_Dashes, m_DashesNb, m_DashOffset);
}

void LineItem::SetDashes (double const *dashes, int num_dashes, double offset)
{
	m_Dashes = new double [num_dashes];
	memcpy (m_Dashes, dashes, num_dashes * sizeof (double));
	m_DashesNb = num_dashes;
	m_DashOffset = offset;
	Invalidate ();
}

}
