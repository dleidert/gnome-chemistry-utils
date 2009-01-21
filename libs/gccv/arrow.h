// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gccv/arrow.h 
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

#ifndef GCCV_ARROW_H
#define GCCV_ARROW_H

#include "line.h"
#include "structs.h"

namespace gccv {

class Arrow: public Line
{
public:
	Arrow (Canvas *canvas, double xstart, double ystart, double xend, double yend);
	Arrow (Group *parent, double xstart, double ystart, double xend, double yend, ItemClient *client = NULL);
	virtual ~Arrow ();

	// virtual methods
	double Distance (double x, double y, Item **item) const;
	void Draw (cairo_t *cr, bool is_vector) const;
	void UpdateBounds ();

private:
	
GCCV_ITEM_POS_PROP (ArrowHeads, StartHead)
GCCV_ITEM_POS_PROP (ArrowHeads, EndHead)
GCCV_ITEM_POS_PROP (double, A)
GCCV_ITEM_POS_PROP (double, B)
GCCV_ITEM_POS_PROP (double, C)
};

}

#endif	//	 GCCV_SQUIGGLE_H
