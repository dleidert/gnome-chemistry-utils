// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gccv/lemniscate.cc 
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
#include "lemniscate.h"

namespace gccv {

Lemniscate::Lemniscate (Canvas *canvas, double x, double y, double radius):
	LineItem (canvas)
{
	SetPosition (x, y);
	SetRadius (radius);
}

Lemniscate::Lemniscate (Group *parent, double x, double y, double radius, ItemClient *client):
	LineItem (parent, client)
{
	SetPosition (x, y);
	SetRadius (radius);
}

Lemniscate::~Lemniscate ()
{
}

void Lemniscate::SetPosition (double x, double y)
{
	Invalidate ();
	m_x = x;
	m_y = y;
	BoundsChanged ();
	Invalidate ();
}

void Lemniscate::GetPosition (double &x, double &y)
{
	x = m_x;
	y = m_y;
}

double Lemniscate::Distance (double x, double y, Item **item) const
{
	return G_MAXDOUBLE; // FIXME
}

void Lemniscate::Draw (cairo_t *cr, G_GNUC_UNUSED bool is_vector) const
{
}

void Lemniscate::UpdateBounds ()
{
	double lw = GetLineWidth () / 2.;
	m_x0 = m_x - m_Radius - lw;
	m_x1 = m_x + m_Radius + lw;
	m_y0 = m_y - m_Radius - lw;
	m_y1 = m_y + m_Radius + lw;
	Item::UpdateBounds ();
}

void Lemniscate::Move (double x, double y)
{
	Invalidate ();
	m_x += x;
	m_y += y;
	BoundsChanged ();
	Invalidate ();
}

}	//	namespace gccv
