// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gccv/hash.h
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

#ifndef GCCV_HASH_H
#define GCCV_HASH_H

/*!\file*/

#include "wedge.h"

namespace gccv {

/*!
@brief Equilateral hashed triangle.

Equilateral triangle filled with regularly spaced hashes parallel to the
triangle base.
*/
class Hash: public Wedge
{
public:
/*!
@param canvas a Canvas.
@param x0 the triangle top horizontal position.
@param y0 the triangle top vertical position.
@param x1 the horizontal position of the canter of the triangle base.
@param y1 the vertical position of the canter of the triangle base.
@param width the triangle base width.

Creates a new Hash and sets it as a child of the root Group of \a canvas.
*/
	Hash (Canvas *canvas, double x0, double y0, double x1, double y1, double width);
/*!
@param parent the Group to which the new Hash will be added.
@param x0 the triangle top horizontal position.
@param y0 the triangle top vertical position.
@param x1 the horizontal position of the canter of the triangle base.
@param y1 the vertical position of the canter of the triangle base.
@param width the triangle base width.
@param client the ItemClient for the new Hash if any.

Creates a new Hash inside \a parent and sets \a client as its associated
ItemClient.
*/
	Hash (Group *parent, double x0, double y0, double x1, double y1, double width, ItemClient *client = NULL);
/*!
The destructor.
*/
	virtual ~Hash ();

	// virtual methods
/*!
@param cr a cairo_t.
@param is_vector whether the cairo_t is a vectorial context.

Draws the Hash to \a cr.
*/
	void Draw (cairo_t *cr, bool is_vector) const;

/*!\fn SetLineWidth(double width)
@param width the new hash width.

Sets the width of the hashes.
*/
/*!\fn GetLineWidth()
@return the width of the hashes.
*/
GCCV_ITEM_POS_PROP (double, LineWidth)
/*!\fn SetLineDist(double dist)
@param dist the new hash distance.

Sets the distance between two consecutive hashes. The periodicity is
actually the sum of this distance and the hash width.
*/
/*!\fn GetLineDist()
@return the distance between two consecutive hashes.
*/
GCCV_ITEM_POS_PROP (double, LineDist)
};

}

#endif	//	 GCCV_HASH_H
