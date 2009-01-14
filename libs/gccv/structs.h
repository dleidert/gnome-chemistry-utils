// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gccv/structs.h 
 *
 * Copyright (C) 2008 Jean Bréfort <jean.brefort@normalesup.org>
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

namespace gccv {

typedef struct {
	double x0, x1, y0, y1;
} Rect;

typedef struct {
	double x, y;
} Point;

typedef struct {
	int start, cur;
} TextSelBounds;

typedef enum {
	AnchorNorthWest, AnchorNorth, AnchorNorthEast,
	AnchorLineWest, AnchorLine, AnchorLineEast,
	AnchorWest, AnchorCenter, AnchorEast,
	AnchorSouthWest, AnchorSouth, AnchorSouthEast 
} Anchor;

}

#endif	//	 GCCV_RECTANGLE_H
