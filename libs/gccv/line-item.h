// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gccv/fill-item.h 
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

#ifndef GCCV_LINE_ITEM_H
#define GCCV_LINE_ITEM_H

/*!\file*/

#include "item.h"
#include <gcu/macros.h>

namespace gccv {

/*!
@brief Base class for line items, whether straight or curved.

This base class implements the common properties of all lines.
*/
class LineItem: public Item {
public:
/*!
@param canvas a Canvas.

Creates a new LineItem and sets it as a child of the root Group of \a canvas.
*/
	LineItem (Canvas *canvas);
/*!
@param parent the Group to which the new LineItem will be added.
@param client the ItemClient for the new LineItem if any.

Creates a new LineItem inside \a parent and sets \a client as its associated
ItemClient.
*/
	LineItem (Group *parent, ItemClient *client = NULL);
/*!
The destructor.
*/
	virtual ~LineItem ();

/*!\fn SetLineWidth(double width)
@param width the new line width.

Sets the LineItem line width.
*/
/*!\fn GetLineWidth()
@return the line width
*/
GCCV_ITEM_PROP (double, LineWidth)
/*!\fn SetLineColor(GOColor color)
@param color the new line color.

Sets the LineItem color.
*/
/*!\fn GetLineColor()
@return the line color.
*/
GCCV_ITEM_PROP (GOColor, LineColor)
};

}

#endif	//	GCCV_LINE_ITEM_H
