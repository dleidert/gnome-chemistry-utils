// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gccv/poly-line.h 
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

#ifndef GCCV_POLY_LINE_H
#define GCCV_POLY_LINE_H

#include "line-item.h"
#include "structs.h"
#include <list>

/*!\file*/

namespace gccv {

/*!
@brief Multi-segments line.

Line item with several linear segments.
*/
class PolyLine: public LineItem
{
public:
/*!
@param canvas a Canvas.
@param points the vertices positions.

Creates a new PolyLine sets it as a child of the root Group of \a canvas.
*/
	PolyLine (Canvas *canvas, std::list <Point> &points);
/*!
@param parent the Group to which the new Arrow will be added.
@param points the vertices positions.
@param client the ItemClient for the new Arrow if any.

Creates a new PolyLine inside \a parent and sets \a client as its associated
ItemClient.
*/
	PolyLine (Group *parent, std::list <Point> &points, ItemClient *client = NULL);
/*!
The destructor.
*/
	virtual ~PolyLine ();

/*!
@param points the new vertices positions.

Sets the vertices for the PolyLine instance.
*/
	void SetPoints (std::list <Point> &points);

	// virtual methods
/*!
@param x horizontal position
@param y vertical position
@param item where to store the Item.

Implementation of Item::Distance() for the PolyLine class. Sets \a item to \a this.
*/
	double Distance (double x, double y, Item **item) const;
/*!
@param cr a cairo_t.
@param is_vector whether the cairo_t is a vectorial context.

Draws the PolyLine to \a cr.
*/
	void Draw (cairo_t *cr, bool is_vector) const;
/*!
@param x the horizontal deplacement
@param y the vertical deplacement

Moves the PolyLine.
*/
	void Move (double x, double y);

protected:
/*!
Evaluates the PolyLine bounds.
*/
	void UpdateBounds ();

protected:
/*!
The PolyLine vertices.
*/
	std::list <Point> m_Points;
};

}

#endif	//	GCCV_POLY_LINE_H
