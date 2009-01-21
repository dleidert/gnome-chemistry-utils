// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gccv/bezier-arrow.cc 
 *
 * Copyright (C) 2009 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "bezier-arrow.h"

namespace gccv {

BezierArrow::BezierArrow (Canvas *canvas):
	LineItem (canvas),
	m_Head (ArrowHeadFull),
	m_A (6.),
	m_B (8.),
	m_C (4.)
{
	for (int i = 0; i < 4; i++)
		m_Controls[i].x = m_Controls[i].y = 0.;
}

BezierArrow::BezierArrow (Group *parent, ItemClient *client):
	LineItem (parent, client),
	m_Head (ArrowHeadFull),
	m_A (6.),
	m_B (8.),
	m_C (4.)
{
	for (int i = 0; i < 4; i++)
		m_Controls[i].x = m_Controls[i].y = 0.;
}

BezierArrow::~BezierArrow ()
{
}

double BezierArrow::Distance (double x, double y, Item **item) const
{
	return G_MAXDOUBLE; //FIXME
}

void BezierArrow::Draw (cairo_t *cr, bool is_vector) const
{
}

void BezierArrow::UpdateBounds ()
{
}

}   //  namespace gccv