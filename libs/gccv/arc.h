// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gccv/arc.h
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

#ifndef GCCV_ARC_H
#define GCCV_ARC_H

/*!\file*/

#include "line-item.h"
#include "structs.h"

namespace gccv {

/*!
@brief An arc item.

The Arc item is a circular arc.
*/
class Arc: public LineItem
{
public:
/*!
@param canvas a Canvas.
@param xc: the arc center horizontal position.
@param yc: the arc center vertical position.
@param radius: the arc radius.
@param start: the arc start angle.
@param end: the arc end angle.

Creates a new Arc and sets it as a child of the root Group of \a canvas.
*/
	Arc (Canvas *canvas, double xc, double yc, double radius, double start, double end);
/*!
@param parent the Group to which the new Leaf will be added.
@param xc: the arc center horizontal position.
@param yc: the arc center vertical position.
@param radius: the arc radius.
@param start: the arc start angle.
@param end: the arc end angle.
@param client the ItemClient for the new Arc if any.

Creates a new Arc inside \a parent and sets \a client as its associated
ItemClient.
*/
	Arc (Group *parent, double xc, double yc, double radius, double start, double end, ItemClient *client = NULL);
/*!
The destructor.
*/
	virtual ~Arc ();

/*!
@param x horizontal position
@param y vertical position
@param item where to store the Item.

Implementation of Item::Distance() for the Arc class. Sets \a item to \a this.
*/
	double Distance (double x, double y, Item **item) const;
/*!
@param cr a cairo_t.
@param is_vector whether the cairo_t is a vectorial context.

Draws the Arc to \a cr.
*/
	void Draw (cairo_t *cr, bool is_vector) const;

protected:
/*!
Evaluates the Arc bounds.
*/
	void UpdateBounds ();

private:
	void ToCairo (cairo_t *cr) const;

/*!\fn SetX(double x)
@param x the new arc center horizontal position.

Sets the arc center horizontal position.
*/
/*!\fn GetX()
@return the arc center horizontal position.
*/
GCCV_ITEM_POS_PROP (double, X)

/*!\fn SetY(double y)
@param y the new arc center vertical position.

Sets the arc center vertical position.
*/
/*!\fn GetY()
@return the arc center vertical position.
*/
GCCV_ITEM_POS_PROP (double, Y)

/*!\fn SetRadius(double radius)
@param radius the new arc radius.

Sets the arc radius.
*/
/*!\fn GetRadius()
@return the arc radius.
*/
GCCV_ITEM_POS_PROP (double, Radius)

/*!\fn SetStart(double start)
@param start the new arc start angle.

Sets the arc start angle.
*/
/*!\fn GetStart()
@return the arc start angle.
*/
GCCV_ITEM_POS_PROP (double, Start)

/*!\fn SetStart(double end)
@param end the new arc end angle.

Sets the arc end angle.
*/
/*!\fn GetStart()
@return the arc end angle.
*/
GCCV_ITEM_POS_PROP (double, End)

/*!\fn SetHead(ArrowHeads Head)
@param Head the ArrowHeads for the end position of the arc.

Sets the arrow head type at the segment end position.
*/
/*!\fn GetHead()
@return the ArrowHeads for the end position of the arc.
*/
GCCV_ITEM_POS_PROP (ArrowHeads, Head)
/*!\fn SetA(double A)
@param A new arrow head size parameter.

Sets the distance from tip of arrowhead to center.
*/
/*!\fn GetA()
@return the distance from tip of arrowhead to center.
*/
GCCV_ITEM_POS_PROP (double, A)
/*!\fn SetB(double B)
@param B new arrow head size parameter.

Sets the distance from tip of arrowhead to trailing point, measured along shaft.
*/
/*!\fn GetB()
@return the distance from tip of arrowhead to trailing point, measured along
shaft.
*/
GCCV_ITEM_POS_PROP (double, B)
/*!\fn SetC(double C)
@param C new arrow head size parameter.

Sets the distance of arrowhead trailing points from outside edge of shaft.
*/
/*!\fn GetC()
@return the distance of arrowhead trailing points from outside edge of shaft.
*/
GCCV_ITEM_POS_PROP (double, C)
};

}	//	namespace gccv

#endif	//	GCCV_PATH_H
