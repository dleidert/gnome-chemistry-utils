// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gccv/wedge.h
 *
 * Copyright (C) 2008-2012 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCCV_WEDGE_H
#define GCCV_WEDGE_H

/*!\file*/

#include "fill-item.h"

namespace gccv {

/*!
@brief Equilateral triangle.

Filled equilateral triangle.
*/
class Wedge: public Item
{
public:
/*!
@param canvas a Canvas.
@param x0 the triangle top horizontal position.
@param y0 the triangle top vertical position.
@param x1 the horizontal position of the canter of the triangle base.
@param y1 the vertical position of the canter of the triangle base.
@param width the triangle base width.

Creates a new Wedge and sets it as a child of the root Group of \a canvas.
*/
	Wedge (Canvas *canvas, double x0, double y0, double x1, double y1, double width);
/*!
@param parent the Group to which the new Wedge will be added.
@param x0 the triangle top horizontal position.
@param y0 the triangle top vertical position.
@param x1 the horizontal position of the canter of the triangle base.
@param y1 the vertical position of the canter of the triangle base.
@param width the triangle base width.
@param client the ItemClient for the new Wedge if any.

Creates a new Wedge inside \a parent and sets \a client as its associated
ItemClient.
*/
	Wedge (Group *parent, double x0, double y0, double x1, double y1, double width, ItemClient *client = NULL);
/*!
The destructor.
*/
	virtual ~Wedge ();

/*!
@param x0 the triangle top horizontal position.
@param y0 the triangle top vertical position.
@param x1 the horizontal position of the canter of the triangle base.
@param y1 the vertical position of the canter of the triangle base.

Setes the new position for the Wedge instance.
*/
	void SetPosition (double x0, double y0, double x1, double y1);

	// virtual methods
/*!
@param x horizontal position
@param y vertical position
@param item where to store the Item.

Implementation of Item::Distance() for the Wedge class. Sets \a item to \a this.
*/
	double Distance (double x, double y, Item **item) const;
/*!
@param cr a cairo_t.
@param is_vector whether the cairo_t is a vectorial context.

Draws the Wedge to \a cr.
*/
	void Draw (cairo_t *cr, bool is_vector) const;
/*!
@param x the horizontal deplacement
@param y the vertical deplacement

Moves the Wedge.
*/
	void Move (double x, double y);

protected:
/*!
Evaluates the Wedge bounds.
*/
	void UpdateBounds ();

protected:
/*!
The horizontal start position.
*/
	double m_xstart;
/*!
The vertical start position.
*/
	double m_ystart;
/*!
The horizontal central end position.
*/
	double m_xend;
/*!
The vertical central end position.
*/
	double m_yend;
/*!
The horizontal position of the first corner at end position. This value is
automatically set and should not be changed by code external to this class.
*/
	double m_xe1;
/*!
The vertical position of the first corner at end position. This value is
automatically set and should not be changed by code external to this class.
*/
	double m_ye1;
/*!
The horizontal position of the second corner at end position. This value is
automatically set and should not be changed by code external to this class.
*/
	double m_xe2;
/*!
The vertical position of the second corner at end position. This value is
automatically set and should not be changed by code external to this class.
*/
	double m_ye2;

/*!\fn SetWidth(double width)
*/
/*!\fn GetWidth()
*/
GCCV_ITEM_POS_PROP (double, Width)
/*!\fn SetFillColor(GOColor color)
*/
/*!\fn GetFillColor()
*/
GCCV_ITEM_PROP (GOColor, FillColor)
};

}

#endif	//	 GCCV_WEDGE_H
