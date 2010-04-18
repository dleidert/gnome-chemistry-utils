// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gccv/arrow.h 
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

#ifndef GCCV_ARROW_H
#define GCCV_ARROW_H

#include "line.h"
#include "structs.h"

/*!\file*/

namespace gccv {

/*!
@brief Simple arrows class.

Item class for arrows composed of a straight line with one or both ends decorated
with an arrow head.
*/
class Arrow: public Line
{
public:
/*!
@param canvas a Canvas.
@param xstart the horizontal start position.
@param ystart the vertical start position.
@param xend the horizontal end position.
@param yend the vertical end position.

Creates a new Arrow with a full head at end and no head at start and sets
it as a child of the root Group of \a canvas.
*/
	Arrow (Canvas *canvas, double xstart, double ystart, double xend, double yend);
/*!
@param parent the Group to which the new Arrow will be added.
@param xstart the horizontal start position.
@param ystart the vertical start position.
@param xend the horizontal end position.
@param yend the vertical end position.
@param client the ItemClient for the new Arrow if any.

Creates a new Arrow with a full head at end and no head at start inside
\a parent and sets \a client as its associated ItemClient.
*/
	Arrow (Group *parent, double xstart, double ystart, double xend, double yend, ItemClient *client = NULL);
/*!
The destructor.
*/
	virtual ~Arrow ();

	// virtual methods
/*!
@param x horizontal position
@param y vertical position
@param item where to store the Item.

Implementation of Item::Distance() for the Arrow class. Sets \a item to \a this.
*/
	double Distance (double x, double y, Item **item) const;
/*!
@param cr a cairo_t.
@param is_vector whether the cairo_t is a vectorial context.

Draws the Arrow to \a cr.
*/
	void Draw (cairo_t *cr, bool is_vector) const;

protected:
/*!
Evaluates the Arrow bounds.
*/
	void UpdateBounds ();

private:

/*!\fn SetStartHead(ArrowHeads StartHead)
@param StartHead the ArrowHeads for the start position of the arrow.

Sets the arrow head type at the segment start position.
*/
/*!\fn GetStartHead()
@return the ArrowHeads for the start position of the arrow.
*/
GCCV_ITEM_POS_PROP (ArrowHeads, StartHead)
/*!\fn SetEndHead(ArrowHeads EndHead)
@param EndHead the ArrowHeads for the end position of the arrow.

Sets the arrow head type at the segment end position.
*/
/*!\fn GetEndHead()
@return the ArrowHeads for the end position of the arrow.
*/
		GCCV_ITEM_POS_PROP (ArrowHeads, EndHead)
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

}

#endif	//	 GCCV_SQUIGGLE_H
