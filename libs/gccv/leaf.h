// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gccv/leaf.h 
 *
 * Copyright (C) 2009-2010 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCCV_LEAF_H
#define GCCV_LEAF_H

#include "fill-item.h"
#include "structs.h"

namespace gccv {

class Leaf: public FillItem
{
public:
	Leaf (Canvas *canvas, double x, double y, double radius);
	Leaf (Group *parent, double x, double y, double radius, ItemClient *client = NULL);
	virtual ~Leaf ();

	void SetPosition (double x, double y);
	void GetPosition (double &x, double &y);

	// virtual methods
	double Distance (double x, double y, Item **item) const;
	void Draw (cairo_t *cr, bool is_vector) const;
	void UpdateBounds ();
	void Move (double x, double y);
	void SetWidthFactor (double factor);

private:
	double m_x, m_y;
	gccv::Point m_Controls[11];

GCCV_ITEM_POS_PROP (double, Radius)
GCCV_ITEM_POS_PROP (double, Rotation)
GCU_RO_PROP (double, WidthFactor)
};

}	//	namespace gccv

#endif	//	GCCV_LEAF_H
