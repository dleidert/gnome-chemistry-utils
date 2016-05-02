// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gccv/fill-item.h
 *
 * Copyright (C) 2008-2014 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCCV_FILL_ITEM_H
#define GCCV_FILL_ITEM_H

/*!\file*/

#include "line-item.h"

namespace gccv {

/*!
@brief Base class for filled items.

This class has just one important member: the fill color. Although it is
not a virtual class, only derived objects should be used.
*/
class FillItem: public LineItem {
public:
/*!
@param canvas a Canvas.

Creates a new FillItem and sets it as a child of the root Group of \a canvas.
*/
	FillItem (Canvas *canvas);
/*!
@param parent the Group to which the new FillItem will be added.
@param client the ItemClient for the new FillItem if any.

Creates a new FillItem inside \a parent and sets \a client as its associated
ItemClient.
*/
	FillItem (Group *parent, ItemClient *client = NULL);
/*!
The destructor.
*/
	virtual ~FillItem ();

/*!
@return the fill color whether it is an automatic color or not.
*/
	GOColor GetEffectiveFillColor () const;

/*!\fn SetFillColor(GOColor color)
@param color the new fill color.

Sets the fill color for the item.
*/
/*!\fn GetFillColor()
@return the fill color for the item.
*/
GCCV_ITEM_PROP (GOColor, FillColor)
/*!\fn SetAutoFillColor(bool auto)
@param auto whether to use a color from the theme.

if \a auto is true, the color used to fill the shape whill be retrieved from
the Gtk+ theme instead of using the FillColor member.
*/
/*!\fn bool GetAutoFillColor()
@return the line color mode, true if automatic, false otherwise.
*/
GCCV_ITEM_PROP (bool, AutoFillColor)
};

}

#endif	//	GCCV_FILL_ITEM_H
