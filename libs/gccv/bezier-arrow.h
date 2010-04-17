// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gccv/bezier-arrow.h 
 *
 * Copyright (C) 2009 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCCV_BEZIER_ARROW_H
#define GCCV_BEZIER_ARROW_H

#include "line-item.h"
#include "structs.h"

/*!\file*/

namespace gccv {

/*!
@brief Curved arrows.

Arrow composed of a Bezier cubic curve and an arrow head.
*/
class BezierArrow: public LineItem {
public:
/*!
@param canvas a Canvas.

Creates a new BezierArrow with a full head and sets
it as a child of the root Group of \a canvas.
*/
	BezierArrow (Canvas *canvas);
/*!
@param parent the Group to which the new Arrow will be added.
@param client the ItemClient for the new Arrow if any.

Creates a new BezierArrow with a full head inside
\a parent and sets \a client as its associated ItemClient.
*/
	BezierArrow (Group *parent, ItemClient *client = NULL);
/*!
The destructor.
*/
	virtual ~BezierArrow ();

	// virtual methods
/*!
@param x horizontal position
@param y vertical position
@param item where to store the Item.

Implementation of Item::Distance() for the BezierArrow class. Sets \a item to \a this.
*/
	double Distance (double x, double y, Item **item) const;
/*!
@param cr a cairo_t.
@param is_vector whether the cairo_t is a vectorial context.

Draws the BezierArrow to \a cr.
*/
	void Draw (cairo_t *cr, bool is_vector) const;
/*!
@param x the horizontal deplacement
@param y the vertical deplacement

Moves the BezierArrow.
*/
	void Move (double x, double y);
/*!
Evaluates the BezierArrow bounds.
*/
	void UpdateBounds ();
/*!
@param x0 where to store the horizontal coordinate of the first control point.
@param y0 where to store the horizontal coordinate of the first control point.
@param x1 where to store the horizontal coordinate of the second control point.
@param y1 where to store the horizontal coordinate of the second control point.
@param x2 where to store the horizontal coordinate of the third control point.
@param y2 where to store the horizontal coordinate of the third control point.
@param x3 where to store the horizontal coordinate of the fourth control point.
@param y3 where to store the horizontal coordinate of the fourth control point.

Retreives the coordinates of the four control points used to build the Bezier
cubic curve.
*/
	void GetControlPoints (double &x0, double &y0, double &x1, double &y1, double &x2, double &y2, double &x3, double &y3);
/*!
@param x0 the new horizontal coordinate of the first control point.
@param y0 the new horizontal coordinate of the first control point.
@param x1 the new horizontal coordinate of the second control point.
@param y1 the new horizontal coordinate of the second control point.
@param x2 the new horizontal coordinate of the third control point.
@param y2 the new horizontal coordinate of the third control point.
@param x3 the new horizontal coordinate of the fourth control point.
@param y3 the new horizontal coordinate of the fourth control point.

Sets the coordinates of the four control points used to build the Bezier
cubic curve.
*/
	void SetControlPoints (double x0, double y0, double x1, double y1, double x2, double y2, double x3, double y3);

/*!\fn SetHead(ArrowHeads Head)
@param Head the ArrowHeads for the end position of the arrow.

Sets the arrow head type at the segment end position.
*/
/*!\fn GetHead()
@return the ArrowHeads for the end position of the arrow.
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
/*!\fn SetShowControls(bool ShowControls)
@param ShowControls whether to show control points.

If set to true, the control points will be displayed as squares. Lhe size of
these squares is five times the line witdth.
*/
/*!\fn GetShowControls()
@return whether the controlpoints are currently displayed (if the arrow is
visible, see Item::SetVisible()).
*/
GCCV_ITEM_POS_PROP (bool, ShowControls)

private:
	Point m_Controls[4];
};

}


#endif  //  GCCV_BEZIER_ARROW_H
