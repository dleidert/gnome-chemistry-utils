// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gccv/polgon.h
 *
 * Copyright (C) 2010 Jean Bréfort <jean.brefort@normalesup.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the
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

#ifndef GCCV_POLYGON_H
#define GCCV_POLYGON_H

#include "fill-item.h"
#include "structs.h"
#include <list>

/*!\file*/

namespace gccv {

/*!
@brief Polygon.

Polygon item.
*/
class Polygon: public FillItem
{
public:
/*!
@param canvas a Canvas.
@param points the vertices positions.

Creates a new Polygon and sets it as a child of the root Group of \a canvas.
*/
	Polygon (Canvas *canvas, std::list <Point> &points);
/*!
@param parent the Group to which the new PolyLine will be added.
@param points the vertices positions.
@param client the ItemClient for the new PolyLine if any.

Creates a new Polygon inside \a parent and sets \a client as its associated
ItemClient.
*/
	Polygon (Group *parent, std::list <Point> &points, ItemClient *client = NULL);
/*!
The destructor.
*/
	virtual ~Polygon ();

/*!
@param points the new vertices positions.

Sets the vertices for the Polygon instance.
*/
	void SetPoints (std::list <Point> &points);

	// virtual methods
/*!
@param x horizontal position of the new vertex.
@param y vertical position of the new vertex.

Apends a new vrtex to the vertices list.
*/
	void AddPoint (double x, double y);
/*!
@param x horizontal position
@param y vertical position
@param item where to store the Item.

Implementation of Item::Distance() for the Polygon class. Sets \a item to \a this.
*/
	double Distance (double x, double y, Item **item) const;
/*!
@param cr a cairo_t.
@param is_vector whether the cairo_t is a vectorial context.

Draws the Polygon to \a cr.
*/
	void Draw (cairo_t *cr, bool is_vector) const;
/*!
@param cr a cairo_t.

Builds the cairo path corresponding to the Polygon bounds. Don't draw anything.
*/
	virtual void BuildPath (cairo_t *cr) const;
/*!
@param x the horizontal deplacement
@param y the vertical deplacement

Moves the Polygon.
*/
	void Move (double x, double y);

protected:
/*!
Evaluates the Polygon bounds.
*/
	void UpdateBounds ();

protected:
/*!
The Polygon vertices.
*/
	std::list <Point> m_Points;
};

}

#endif	//	GCCV_POLYGON_H
