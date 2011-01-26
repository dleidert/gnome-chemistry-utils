// -*- C++ -*-

/* 
 * GChemPaint library
 * brackets.h 
 *
 * Copyright (C) 2011 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCCV_BRACKETS_H
#define GCCV_BRACKETS_H

#include "item.h"
#include <string>

namespace gccv {

typedef struct _BracketsMetrics BracketsMetrics;

class Brackets: public Item
{
public:
	Brackets (Canvas *canvas, char const *fontdesc, double x0, double y0, double x1, double y1);
	Brackets (Group *parent, char const *fontdesc, double x0, double y0, double x1, double y1, ItemClient *client = NULL);
	~Brackets ();

/*!
@param x0 the new brackets top left inside horizontal position.
@param y0 the new brackets top left inside vertical position.
@param x1 the new brackets bottom right inside horizontal position.
@param y1 the new brackets bottom right inside vertical position.

Sets the brackets top left inside coordinates.
*/
	void SetPosition (double x0, double y0, double x1, double y1);
/*!
@param x0 where to store the brackets top left inside horizontal position.
@param y0 where to store the brackets top left inside vertical position.
@param x1 where to store the brackets bottom right inside horizontal position.
@param y1 where to store the brackets bottom right inside vertical position.

Retrieves the brackets top left inside coordinates.
*/
	void GetPosition (double &x0, double &y0, double &x1, double &y1);

	// virtual methods
/*!
@param x horizontal position
@param y vertical position
@param item where to store the Item.

Implementation of Item::Distance() for the Brackets class. Sets \a item to \a this.
*/
	double Distance (double x, double y, Item **item) const;
/*!
@param cr a cairo_t.
@param is_vector whether the cairo_t is a vectorial context.

Draws the Brackets to \a cr.
*/
	void Draw (cairo_t *cr, bool is_vector) const;
/*!
@param x the horizontal deplacement
@param y the vertical deplacement

Moves the Brackets.
*/
	void Move (double x, double y);

protected:
/*!
Evaluates the Brackets bounds.
*/
	void UpdateBounds ();

private:
		
private:
	BracketsMetrics const *m_Metrics;
//	std::string m_FontDesc;
	double m_x0, m_y0, m_x1, m_y1;

/*!\fn SetSize(double size)
@param size the new brackets size.

Sets the brackets size.
*/
/*!\fn GetSize()
@return the brackets size.
*/
GCCV_ITEM_POS_PROP (double, Size)
GCCV_ITEM_POS_PROP (std::string, FontDesc)
/*!\fn SetColor(GOColor color)
@param color the new brackets color.

Sets the Brackets color.
*/
/*!\fn GetColor()
@return the brackets color.
*/
GCCV_ITEM_PROP (GOColor, Color)
};

}

#endif	//	GCCV_BRACKETS_H
