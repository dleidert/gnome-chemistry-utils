// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gccv/line.h
 *
 * Copyright (C) 2008-2010 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCCV_LINE_H
#define GCCV_LINE_H

/*!\file*/

#include "line-item.h"

namespace gccv {

/*!
@brief Straight lines.

Implements one segment straight lines.
*/
	class Line: public LineItem
{
public:
/*!
@param canvas a Canvas.
@param xstart the horizontal start position.
@param ystart the vertical start position.
@param xend the horizontal end position.
@param yend the vertical end position.

Creates a new Line sets it as a child of the root Group of \a canvas.
*/
	Line (Canvas *canvas, double xstart, double ystart, double xend, double yend);
/*!
@param parent the Group to which the new Line will be added.
@param xstart the horizontal start position.
@param ystart the vertical start position.
@param xend the horizontal end position.
@param yend the vertical end position.
@param client the ItemClient for the new Line if any.

Creates a new Line inside \a parent and sets \a client as its associated
ItemClient.
*/
	Line (Group *parent, double xstart, double ystart, double xend, double yend, ItemClient *client = NULL);
/*!
The destructor.
*/
	virtual ~Line ();
/*!
@param xstart the new horizontal start position.
@param ystart the new vertical start position.
@param xend the new horizontal end position.
@param yend the new vertical end position.

Sets the Line position.
*/
	void SetPosition (double xstart, double ystart, double xend, double yend);

	// virtual methods
/*!
@param x horizontal position
@param y vertical position
@param item where to store the Item.

Implementation of Item::Distance() for the Line class. Sets \a item to \a this.
*/
	double Distance (double x, double y, Item **item) const;
/*!
@param cr a cairo_t.
@param is_vector whether the cairo_t is a vectorial context.

Draws the Line to \a cr.
*/
	void Draw (cairo_t *cr, bool is_vector) const;
/*!
@param x the horizontal deplacement
@param y the vertical deplacement

Moves the Line.
*/
	void Move (double x, double y);

protected:
/*!
Evaluates the Line bounds.
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
The horizontal end position.
*/
	double m_xend;
/*!
The vertical end position.
*/
	double m_yend;
};

}

#endif	//	GCCV_LINE_H
