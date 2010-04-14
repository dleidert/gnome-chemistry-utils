// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gccv/structs.h 
 *
 * Copyright (C) 2008-2009 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCCV_STRUCTS_H
#define GCCV_STRUCTS_H

/*!\file*/

namespace gccv {

/*!
Describes a rectangle for which (x0,y0) are the top left coordinates
and (x1,y1) the bottom right coordinates.
*/
typedef struct {
/*!the top left horizontal position*/
	double x0;
/*!the bottom right horizontal position*/
	double x1;
/*!the top left vertical position*/
	double y0;
/*!the bottom right vertical position*/
	double y1;
} Rect;

/*!
Describes a point with (x,y) as coordinates.
*/
typedef struct {
/*!the horizontal position*/
	double x;
/*!the vertical position*/
	double y;
} Point;

/*!Anchor
Text anchoring modes.
*/
typedef enum {
/*!Anchor at top left.*/
	AnchorNorthWest,
/*!Anchor at top center.*/
	AnchorNorth,
/*!Anchor at top right.*/
	AnchorNorthEast,
/*!Anchor at left of base of the first line.*/
	AnchorLineWest,
/*!Anchor at center of base of the first line.*/
	AnchorLine,
/*!Anchor at right of base of the first line.*/
	AnchorLineEast,
/*!Anchor at left and vertical center.*/
	AnchorWest,
/*!Anchor at center, both vertically and horizontally.*/
	AnchorCenter,
/*!Anchor at right and vertical center.*/
	AnchorEast,
/*!Anchor at bottom left.*/
	AnchorSouthWest,
/*!Anchor at bottom center.*/
	AnchorSouth,
/*!Anchor at bottom right.*/
	AnchorSouthEast 
} Anchor;

/*!
Arrow heads of a line.
*/
typedef enum {
/*!No arrow head.*/
	ArrowHeadNone,
/*!Full arrow head*/
	ArrowHeadFull,
/*!Half left arrow hea*/
	ArrowHeadLeft,
/*!Half right arrow head*/
	ArrowHeadRight,
} ArrowHeads;

/*!
Describes the position of the scripts relative to the base line.
*/
typedef enum {
/*!Text on base line*/
	Normalscript,
/*!Text below line*/
	Subscript,
/*!Text above line*/
	Superscript
} TextPosition;

/*!
Can be used for underline, overline or strikethrough.
*/
typedef enum {
/*!No line.*/
	TextDecorationNone,
/*!High for underline, low for overline and medium for strikethrough*/
	TextDecorationDefault,
/*!Line at the hightest position.*/
	TextDecorationHigh,
/*!Line at intermediate position.*/
	TextDecorationMedium,
/*!Line at the lowest position.*/
	TextDecorationLow,
/*!Two lines, one at high and the other at low position.*/
	TextDecorationDouble,
/*!Line oscillating between highest and lowest position.*/
	TextDecorationSquiggle
} TextDecoration;

}

#endif	//	 GCCV_STRUCTS_H
