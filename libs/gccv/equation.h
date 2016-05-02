// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gccv/equation.h
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

#ifndef GCCV_EQUATION_H
#define GCCV_EQUATION_H

#include "rectangle.h"
#include "structs.h"
#include "text.h"
#include <lsmdom.h>

/*!\file*/

namespace gccv {

class Equation: public Rectangle
{
public:
/*!
@param canvas a Canvas.
@param x the horizontal position.
@param y the vertical position.

Creates a new Equation, sets it as a child of the root Group of \a canvas.
Interpretation of the values \a x and \a y relies on the anchor.
*/
	Equation (Canvas *canvas, double x, double y);
/*!
@param parent the Group to which the new Equation will be added.
@param x the top left corner horizontal position.
@param y the top left corner vertical position.
@param client the ItemClient for the new Text if any.

Creates a new Equation inside \a parent and sets \a client as its associated
ItemClient. Interpretation of the values \a x and \a y relies on the anchor.
*/
	Equation (Group *parent, double x, double y, ItemClient *client = NULL);
/*!
The destructor.
*/
	virtual ~Equation ();

	/*!
@param cr a cairo_t.
@param is_vector whether the cairo_t is a vectorial context.

Draws the Equation to \a cr.
*/
	void Draw (cairo_t *cr, bool is_vector) const;

/*!
@param x the top left corner new horizontal position.
@param y the top left corner new vertical position.

Sets the position of the Equation. Interpretation of the values \a x and \a y relies
on the equation anchor.
*/
	void SetPosition (double x, double y);

private:
	double m_x, m_y;
	LsmDomView *m_View;

GCCV_TEXT_PROP (LsmDomDocument const *, Math)
/*!\fn SetAnchor(Anchor anchor)
@param anchor the new Anchor.

Sets the Anchor foir the Text.
*/
/*!\fn Anchor GetAnchor()
@return the Equation Anchor.
*/
GCCV_TEXT_PROP (Anchor, Anchor)
/*!\fn SetAutoTextColor(bool auto)
@param auto whether to use a color from the Gtk+ theme.

if \a auto is true, the color used to draw the line will be retrieved from
the Gtk+ theme instead of using the default color for equation.
*/
/*!\fn bool GetTextAutoColor()
@return the equation color mode, true if automatic, false otherwise.
*/
GCCV_ITEM_PROP (bool, AutoTextColor)
/*!\fn SetAutFont(bool auto)
@param auto whether to use a font from the Gtk+ theme.

if \a auto is true, the font used to draw the equation will be retrieved from
the Gtk+ theme instead of using the default font for equation.
*/
/*!\fn bool GetAutoFont()
@return the equation font mode, true if automatic, false otherwise.
*/
GCCV_ITEM_PROP (bool, AutoFont)
};

}   // namespace gccv

#endif	//	GCCV_FILL_ITEM_H
