// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gccv/path.h
 *
 * Copyright (C) 2014 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCCV_PATH_H
#define GCCV_PATH_H

/*!\file*/

#include "fill-item.h"
#include "structs.h"
#include <goffice/goffice.h>

namespace gccv {

/*!
@brief A drop or leaf item.

The Path item is made of line and curved segments.
*/
	class Path: public FillItem
{
public:
/*!
@param canvas a Canvas.
@param path: the path to display.

Creates a new Path and sets it as a child of the root Group of \a canvas.
It absorbs a reference for %path.
*/
	Path (Canvas *canvas, GOPath *path);
/*!
@param parent the Group to which the new Leaf will be added.
@param path: the path to display.
@param client the ItemClient for the new Path if any.

Creates a new Path inside \a parent and sets \a client as its associated
ItemClient. It absorbs a reference for %path.
*/
	Path (Group *parent, GOPath *path, ItemClient *client = NULL);
/*!
The destructor.
*/
	virtual ~Path ();

/*!
@param x horizontal position
@param y vertical position
@param item where to store the Item.

Implementation of Item::Distance() for the Path class. Sets \a item to \a this.
*/
	double Distance (double x, double y, Item **item) const;
/*!
@param cr a cairo_t.
@param is_vector whether the cairo_t is a vectorial context.

Draws the Path to \a cr.
*/
	void Draw (cairo_t *cr, bool is_vector) const;

protected:
/*!
Evaluates the Path bounds.
*/
	void UpdateBounds ();

private:
	GOPath *m_Path;

};

}	//	namespace gccv

#endif	//	GCCV_PATH_H
