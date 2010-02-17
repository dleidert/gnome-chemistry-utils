// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gccv/group.cc 
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
#include "group.h"

#include <cstdio>

using namespace std;

namespace gccv {

Group::Group (Canvas *canvas):
	Item (canvas),
	m_x (0.),
	m_y (0.)
{
}

Group::Group (Canvas *canvas, double x, double y):
	Item (canvas),
	m_x (x),
	m_y (y)
{
}

Group::Group (Group *parent, ItemClient *client):
	Item (parent, client),
	m_x (0.),
	m_y (0.)
{
}

Group::Group (Group *parent, double x, double y, ItemClient *client):
	Item (parent, client),
	m_x (x),
	m_y (y)
{
}

Group::~Group()
{
	while (!m_Children.empty ())
		delete (*(m_Children.begin ()));
}

void Group::UpdateBounds ()
{
	if (m_Children.empty ()) {
		m_x0 = m_y0 = G_MAXDOUBLE;
		m_x1 = m_y1 = -G_MAXDOUBLE;
	} else {
		list <Item *>::const_iterator i = m_Children.begin (), end = m_Children.end ();
		double x0, y0, x1, y1;
		(*i)->GetBounds (m_x0, m_y0, m_x1, m_y1);
		for (i++; i != end; i++) {
			(*i)->GetBounds (x0, y0, x1, y1);
			if (x0 < m_x0)
				m_x0 = x0;
			if (y0 < m_y0)
				m_y0 = y0;
			if (x1 > m_x1)
				m_x1 = x1;
			if (y1 > m_y1)
				m_y1 = y1;
		}
	}
	m_x0 += m_x;
	m_y0 += m_y;
	m_x1 += m_x;
	m_y1 += m_y;
	Item::UpdateBounds ();
}

double Group::Distance (double x, double y, Item **item) const
{
	if (m_Children.empty ())
		return Item::Distance (x, y, item);
	x -= m_x;
	y -= m_y;
	double d = G_MAXDOUBLE, di;
	Item *nearest = NULL, *cur;
	list <Item *>::const_iterator i, end = m_Children.end ();
	for (i = m_Children.begin (); i != end; i++) {
		cur = NULL;
		di = (*i)->Distance (x, y, &cur);
		if (di < d) {
			d = di;
			nearest = cur? cur: *i;
		}
	}
	if (item)
		*item = nearest;
	return d;
}

bool Group::Draw (cairo_t *cr, double x0, double y0, double x1, double y1, bool is_vector) const
{
	if (m_Children.empty ())
		return true;
	cairo_save (cr);
	cairo_translate (cr, m_x, m_y);	
	x0 -= m_x;
	y0 -= m_y;
	x1 -= m_x;
	y1 -= m_y;
	list <Item *>::const_iterator i, end = m_Children.end ();
	for (i = m_Children.begin (); i != end; i++) {
		double x, y, x_, y_;
		if (!(*i)->GetVisible ())
			continue;
		(*i)->GetBounds (x, y, x_, y_);
		if (x <= x1 && x_ >= x0 && y <= y1 && y_ >= y0) {
			cairo_set_operator (cr, (*i)->GetOperator ());
			if (!(*i)->Draw (cr, x0, y0, x1, y1, is_vector))
				(*i)->Draw (cr, is_vector);
		}
	}
	cairo_restore (cr);
	return true;
}

void Group::AddChild (Item *item)
{
	m_Children.push_front (item);
	BoundsChanged ();
}

void Group::RemoveChild (Item *item)
{
	m_Children.remove (item);
	BoundsChanged ();
}

Item *Group::GetFirstChild (std::list<Item *>::iterator &it)
{
	it = m_Children.begin ();
	return (it != m_Children.end ())? *it: NULL;
}

Item *Group::GetNextChild (std::list<Item *>::iterator &it)
{
	it++;
	return (it != m_Children.end ())? *it: NULL;
}

void Group::Move (double x, double y)
{
	Invalidate ();
	m_x += x;
	m_y += y;
	BoundsChanged ();
	Invalidate ();
}

void Group::AdjustBounds (double &x0, double &y0, double &x1, double &y1) const
{
	x0 += m_x;
	y0 += m_y;
	x1 += m_x;
	y1 += m_y;
}

void Group::SetPosition (double x, double y)
{
	Invalidate ();
	m_x = x;
	m_y = y;
	BoundsChanged ();
	Invalidate ();
}

void Group::MoveToFront (Item *item)
{
	list <Item *>::iterator i, end = m_Children.end ();
	for (i = m_Children.begin (); i != end; i++)
		if (*i == item)
		break;
	if (i != end) {
		m_Children.erase (i);
		m_Children.push_back (item); // back items are displayed at last
	}
}

void Group::MoveToBack (Item *item)
{
	list <Item *>::iterator i, end = m_Children.end ();
	for (i = m_Children.begin (); i != end; i++)
		if (*i == item)
		break;
	if (i != end) {
		m_Children.erase (i);
		m_Children.push_front (item); // front items are displayed at first
	}
}

}
