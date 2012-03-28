// -*- C++ -*-

/*
 * GChemPaint library
 * brackets.h
 *
 * Copyright (C) 2011-2012 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCCV_BRACKETS_H
#define GCCV_BRACKETS_H

#include "line-item.h"
#include <list>
#include <string>

namespace gccv {

typedef struct _BracketsMetrics BracketsMetrics;

/*!\enum BracketsTypes
Enumeration of the known brackets types.
*/
typedef enum
{
/*!
Parenthesis.
*/
	BracketsTypeNormal,
/*!
Square brackets.
*/
	BracketsTypeSquare,
/*!
Curly brackets.
*/
	BracketsTypeCurly
} BracketsTypes;

/*!\enum BracketsUses
Enumeration of the brackets use modes.
*/
typedef enum
{
/*!
Use only an opening bracket.
*/
	BracketsOpening = 1,
/*!
Use only a closing bracket.
*/
	BracketsClosing,
/*!
Use both an opening and a closing bracket.
*/
	BracketsBoth
} BracketsUses;

class Brackets: public LineItem
{
public:
/*!
@param canvas a Canvas.
@param type the new brackets type.
@param the used brackets, opening, closing or both.
@param font desc the font to use (see Pango documentation).
@param x0 the top left horizontal coordinate of the rectangle to enclose.
@param y0 the top left vertical coordinate of the rectangle to enclose.
@param x1 the bottom right horizontal coordinate of the rectangle to enclose.
@param y1 the bottom right vertical coordinate of the rectangle to enclose.

Creates a new Bracket object and sets
it as a child of the root Group of \a canvas.
*/
	Brackets (Canvas *canvas, BracketsTypes type, BracketsUses use, char const *fontdesc, double x0, double y0, double x1, double y1);
/*!
@param parent the Group to which the new Bracket will be added.
@param type the new brackets type.
@param the used brackets, opening, closing or both.
@param font desc the font to use (see Pango documentation).
@param x0 the top left horizontal coordinate of the rectangle to enclose.
@param y0 the top left vertical coordinate of the rectangle to enclose.
@param x1 the bottom right horizontal coordinate of the rectangle to enclose.
@param y1 the bottom right vertical coordinate of the rectangle to enclose.
@param client the ItemClient for the new Bracket if any.

Creates a new Bracket inside \a parent and sets \a client as its associated
ItemClient.
*/
	Brackets (Group *parent, BracketsTypes type, BracketsUses use, char const *fontdesc, double x0, double y0, double x1, double y1, ItemClient *client = NULL);
/*!
The destructor.
*/
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

	typedef struct
	{
		char const *ch;
		double x, y, w, h, offset;
		bool needs_clip;
	} BracketElem;
	std::list < BracketElem > m_Elems;

/*!\fn SetSize(double size)
@param size the new brackets size.

Sets the brackets size.
*/
/*!\fn GetSize()
@return the brackets size.
*/
GCCV_ITEM_POS_PROP (double, Size)

/*!\fn SetFontDesc(std::string desc)
@param desc the new brackets font.

Sets the brackets font.
*/
/*!\fn GetFontDesc()
@return the brackets font.
*/
GCCV_ITEM_POS_PROP (std::string, FontDesc)

/*!\fn SetType(BracketsTypes type)
@param size the new brackets type.

Sets the brackets type.
*/
/*!\fn GetType()
@return the brackets type.
*/
GCCV_ITEM_POS_PROP (BracketsTypes, Type)

/*!\fn SetUsed(BracketsUses used)
@param used the brackets to use.

Sets the brackets to use, opening, closing or both.
*/
/*!\fn GetUsed()
@return the used brackets size, opening, closing or both.
*/
GCCV_ITEM_POS_PROP (BracketsUses, Used)
};

}

#endif	//	GCCV_BRACKETS_H
