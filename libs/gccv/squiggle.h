// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gccv/squiggle.h 
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

#ifndef GCCV_SQUIGGLE_H
#define GCCV_SQUIGGLE_H

#include "line-item.h"

namespace gccv {

class Squiggle: public LineItem
{
public:
	Squiggle (Canvas *canvas, double xstart, double ystart, double xend, double yend);
	Squiggle (Group *parent, double xstart, double ystart, double xend, double yend, ItemClient *client = NULL);
	virtual ~Squiggle ();

	void SetPosition (double xstart, double ystart, double xend, double yend);

	// virtual methods
	double Distance (double x, double y, Item **item) const;
	void Draw (cairo_t *cr, bool is_vector) const;
	void UpdateBounds ();
	void Move (double x, double y);

private:
	double m_xstart;
	double m_ystart;
	double m_xend;
	double m_yend;
	
GCCV_ITEM_POS_PROP (double, Width)
GCCV_ITEM_POS_PROP (double, Step)
};

}

#endif	//	 GCCV_SQUIGGLE_H
