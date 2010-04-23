// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gccv/rectangle.h 
 *
 * Copyright (C) 2008-2010 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCCV_RECTANGLE_H
#define GCCV_RECTANGLE_H

/*!\file */

#include "fill-item.h"

namespace gccv {
/*!
@brief Rectangles.

Implements rectangular, optionally filled, items.
*/

class Rectangle: public FillItem
{
public:
/*!
@param canvas a Canvas.
@param x the top left corner horizontal position.
@param y the top left corner vertical position.
@param width the rectangle width.
@param height the rectangle height.

Creates a new Rectangle sets it as a child of the root Group of \a canvas.
*/
	Rectangle (Canvas *canvas, double x, double y, double width, double height);
/*!
@param parent the Group to which the new Text will be added.
@param x the top left corner horizontal position.
@param y the top left corner vertical position.
@param width the rectangle width.
@param height the rectangle height.
@param client the ItemClient for the new Text if any.

Creates a new Rectangle inside \a parent and sets \a client as its associated
ItemClient.
*/
	Rectangle (Group *parent, double x, double y, double width, double height, ItemClient *client = NULL);
/*!
The destructor.
*/
	virtual ~Rectangle ();

/*!
@param x the top left corner new horizontal position.
@param y the top left corner new vertical position.
@param width the rectangle new width.
@param height the rectangle new height.

Sets the position and size of the Rectangle.
*/
	void SetPosition (double x, double y, double width, double height);
/*!
@param x where to store the top left corner horizontal position.
@param y where to store the top left corner vertical position.

Retrieves the position the Rectangle.
*/
	void GetPosition (double &x, double &y) const;
/*!
@param x where to store the top left corner horizontal position.
@param y where to store the top left corner vertical position.
@param width where to store the rectangle width.
@param height where to store the rectangle height.

Retrieves the position and size of the Rectangle.
*/
	void GetPosition (double &x, double &y, double &width, double &height) const;

	// virtual methods
/*!
@param x horizontal position
@param y vertical position
@param item where to store the Item.

Implementation of Item::Distance() for the Rectangle class. Sets \a item to \a this.
*/
	double Distance (double x, double y, Item **item) const;
/*!
@param cr a cairo_t.
@param is_vector whether the cairo_t is a vectorial context.

Draws the Rectangle to \a cr.
*/
	void Draw (cairo_t *cr, bool is_vector) const;
/*!
@param x the horizontal deplacement
@param y the vertical deplacement

Moves the Rectangle.
*/
	void Move (double x, double y);

protected:
/*!
Evaluates the Rectangle bounds.
*/
	void UpdateBounds ();

private:
	double m_x, m_y, m_w, m_h;
};

}

#endif	//	 GCCV_RECTANGLE_H
