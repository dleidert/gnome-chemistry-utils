// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gccv/circle.h 
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

#ifndef GCCV_CIRCLE_H
#define GCCV_CIRCLE_H

#include "fill-item.h"

/*!\file*/

namespace gccv {

/*!
@brief Circles.

Circle item.
*/
class Circle: public FillItem
{
public:
/*!
@param canvas a Canvas.
@param x the circle center horizontal position.
@param y the circle center vertical position.
@param radius the cricle radius.

Creates a new Circle and sets it as a child of the root Group of \a canvas.
*/
	Circle (Canvas *canvas, double x, double y, double radius);
/*!
@param parent the Group to which the new Circle will be added.
@param x the circle center horizontal position.
@param y the circle center vertical position.
@param radius the cricle radius.
@param client the ItemClient for the new Circle if any.

Creates a new Circle insideb\a parent and sets \a client as its associated
ItemClient.
*/
	Circle (Group *parent, double x, double y, double radius, ItemClient *client = NULL);
/*!
The destructor.
*/
	virtual ~Circle ();

/*!
@param x the new circle center horizontal position.
@param y the new circle center vertical position.

Sets the position of the circle center.
*/
	void SetPosition (double x, double y);
/*!
@param x where to store the circle center horizontal position.
@param y where to store the circle center vertical position.

Retreives the position of the circle center.
*/
	void GetPosition (double &x, double &y);

	// virtual methods
/*!
@param x horizontal position
@param y vertical position
@param item where to store the Item.

Implementation of Item::Distance() for the Circle class. Sets \a item to \a this.
*/
	double Distance (double x, double y, Item **item) const;
/*!
@param cr a cairo_t.
@param is_vector whether the cairo_t is a vectorial context.

Draws the Circle to \a cr.
*/
	void Draw (cairo_t *cr, bool is_vector) const;
/*!
@param x the horizontal deplacement
@param y the vertical deplacement

Moves the Circle.
*/
	void Move (double x, double y);

protected:
/*!
Evaluates the Circle bounds.
*/
	void UpdateBounds ();

private:
	double m_x, m_y;

/*!\fn SetRadius(double radius)
@param radius the new circle radius.

Sets the circle radius.
*/
/*!\fn GetRadius()
@return the circle radius.
*/
GCCV_ITEM_POS_PROP (double, Radius)
};

}

#endif	//	 GCCV_CIRCLE_H
