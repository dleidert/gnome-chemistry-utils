// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gccv/group.h 
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

#ifndef GCCV_GROUP_H
#define GCCV_GROUP_H

#include "item.h"
#include <list>

namespace gccv {

class Group: public Item
{
public:
	Group (Canvas *canvas);
	Group (Canvas *canvas, double x, double y);
	Group (Group *parent, ItemClient *client = NULL);
	Group (Group *parent, double x, double y, ItemClient *client = NULL);
	virtual ~Group();

	void AddChild (Item *item);
	void RemoveChild (Item *item);
	void MoveToFront (Item *item);
	void MoveToBack (Item *item);

	Item *GetFirstChild (std::list<Item *>::iterator &it);
	Item *GetNextChild (std::list<Item *>::iterator &it);

	void AdjustBounds (double &x0, double &y0, double &x1, double &y1) const;
	void SetPosition (double x, double y);

	// virtual methods
	double Distance (double x, double y, Item **item) const;
	bool Draw (cairo_t *cr, double x0, double y0, double x1, double y1, bool is_vector) const;
	void Move (double x, double y);

protected:
	void UpdateBounds ();

private:
	std::list<Item *> m_Children;
	double m_x, m_y;	// translation offset
};

}

#endif	//	 GCCV_GROUP_H
