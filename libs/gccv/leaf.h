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

/*!\file*/

#include "fill-item.h"
#include "structs.h"

namespace gccv {

/*!
@brief A drop or leaf item.

The Leaf item looks like a drop or some type of tree leaf:
@image html gccvleaf.png
*/
	class Leaf: public FillItem
{
public:
/*!
@param canvas a Canvas.
@param x the leaf origin horizontal position.
@param y the leaf origin vertical position.
@param radius the leaf radius.

Creates a new Leaf and sets it as a child of the root Group of \a canvas. The
origin is the angular point, and the radius the distance between the origin and
the opposite point.
*/
	Leaf (Canvas *canvas, double x, double y, double radius);
/*!
@param parent the Group to which the new Leaf will be added.
@param x the leaf origin horizontal position.
@param y the leaf origin vertical position.
@param radius the leaf radius.
@param client the ItemClient for the new Leaf if any.

Creates a new Leaf inside \a parent and sets \a client as its associated
ItemClient. The origin is the angular point, and the radius the distance
between the origin and the opposite point.
*/
	Leaf (Group *parent, double x, double y, double radius, ItemClient *client = NULL);
/*!
The destructor.
*/
	virtual ~Leaf ();

/*!
@param x the new leaf origin horizontal position.
@param y the new leaf origin vertical position.

Sets the position of the leaf origin.
*/
	void SetPosition (double x, double y);
/*!
@param x where to store the leaf originhorizontal position.
@param y where to store the leaf origin vertical position.

Retrieves the position of the leaf origin.
*/
	void GetPosition (double &x, double &y);
/*!
@param factor the new width factor.

Sets the width of the leaf relative to its radius. Actually, the width is
0.8 * factor * radius.
*/
	void SetWidthFactor (double factor);

	// virtual methods
/*!
@param x horizontal position
@param y vertical position
@param item where to store the Item.

Implementation of Item::Distance() for the Leaf class. Sets \a item to \a this.
*/
	double Distance (double x, double y, Item **item) const;
/*!
@param cr a cairo_t.
@param is_vector whether the cairo_t is a vectorial context.

Draws the Leaf to \a cr.
*/
	void Draw (cairo_t *cr, bool is_vector) const;
/*!
@param x the horizontal deplacement
@param y the vertical deplacement

Moves the Leaf.
*/
	void Move (double x, double y);

protected:
/*!
Evaluates the Leaf bounds.
*/
	void UpdateBounds ();

private:
	double m_x, m_y;
	gccv::Point m_Controls[11];

/*!\fn SetRadius(double radius)
@param radius the new leaf radius.

Sets the radius for the leaf. The radius is defined as the distance
between the origin and the opposite point.
*/
/*!\fn GetRadius()
@return the current Leaf radius.
*/
GCCV_ITEM_POS_PROP (double, Radius)
/*!\fn SetRotation(double rotation)
@param rotation the new orientation in radians.

Sets the orientation relative to the up vertical direction, using the
trigonometric convention.
*/
/*!\fn GetRotation()
@return the current Leaf orientation.
*/
GCCV_ITEM_POS_PROP (double, Rotation)
/*!\fn GetWidthFactor()
@return the width factor for the Leaf. Actually, the width is
0.8 * factor * radius.
*/
GCU_RO_PROP (double, WidthFactor)
};

}	//	namespace gccv

#endif	//	GCCV_LEAF_H
