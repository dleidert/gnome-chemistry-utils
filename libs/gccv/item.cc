// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gccv/item.cc 
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
#include "canvas.h"
#include "group.h"
#include "item.h"
#include "item-client.h"

namespace gccv {

Item::Item (Canvas *canvas):
	m_Canvas (canvas),
	m_CachedBounds (false),
	m_Client (NULL),
	m_Parent (canvas->GetRoot ()),
	m_Visible (true)
{
	if (m_Parent)
		m_Parent->AddChild (this);
}

Item::Item (Group *parent, ItemClient *client):
	m_CachedBounds (false),
	m_Client (client),
	m_Parent (parent),
	m_Visible (true)
{
	if (parent)
		parent->AddChild (this);
	m_Canvas = (parent)? parent->m_Canvas: NULL;
}

Item::~Item()
{
	Invalidate ();
	if (m_Parent)
		m_Parent->RemoveChild (this);
	if (m_Client && m_Client->GetItem () == this) // this might not be the top item for this client
		m_Client->m_Item = NULL;
}

void Item::GetBounds (double &x0, double &y0, double &x1, double &y1) const
{
	if (!m_CachedBounds)
		const_cast <Item *> (this)->UpdateBounds ();
	x0 = m_x0;
	y0 = m_y0;
	x1 = m_x1;
	y1 = m_y1;
}

void Item::BoundsChanged ()
{
	m_CachedBounds = false;
	if (m_Parent)
		m_Parent->BoundsChanged ();
}

void Item::UpdateBounds ()
{
	m_CachedBounds = true;
}

double Item::Distance (double x, double y, Item **item) const
{
	if (item)
		*item = NULL;
	return G_MAXDOUBLE;
}

void Item::Draw (cairo_t *cr, bool is_vector) const
{
}

bool Item::Draw (cairo_t *cr, double x0, double y0, double x1, double y1, bool is_vector) const
{
	return false;
}

void Item::Invalidate () const
{
	if (!m_CachedBounds)
		const_cast <Item *> (this)->UpdateBounds ();
	Group const *parent = m_Parent;
	double x0 = m_x0, y0 = m_y0, x1 = m_x1, y1 = m_y1;
	while (parent) {
		parent->AdjustBounds (x0, y0, x1, y1);
		parent = parent->m_Parent;
	}
printf("invalidating %g;%g -> %g;%g\n",x0, y0, x1, y1);
	m_Canvas->Invalidate (x0, y0, x1, y1);
}

void Item::Move (double x, double y)
{
}

void Item::SetVisible (bool visible)
{
	if (visible != m_Visible) {
		m_Visible = visible;
		Group const *parent = m_Parent;
		double x0 = m_x0, y0 = m_y0, x1 = m_x1, y1 = m_y1;
		while (parent) {
			parent->AdjustBounds (x0, y0, x1, y1);
			parent = parent->m_Parent;
		}
		m_Canvas->Invalidate (x0, y0, x1, y1);
	}
}

}
