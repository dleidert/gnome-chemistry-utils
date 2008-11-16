// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gccv/wedge.h 
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

#ifndef GCCV_WEDGE_H
#define GCCV_WEDGE_H

#include "fill-item.h"
#include <goffice/utils/go-color.h>

namespace gccv {

class Wedge: public Item
{
public:
	Wedge (Canvas *canvas, double x0, double y0, double x1, double y1, double width);
	Wedge (Group *parent, double x0, double y0, double x1, double y1, double width, ItemClient *client = NULL);
	virtual ~Wedge ();

	void SetPosition (double x0, double y0, double x1, double y1);

	// virtual methods
	double Distance (double x, double y, Item **item) const;
	void Draw (cairo_t *cr, bool is_vector) const;
	void UpdateBounds ();
	void Move (double x, double y);

protected:
	double m_xstart, m_ystart, m_xend, m_yend;
	double m_xe1, m_ye1, m_xe2, m_ye2;

GCCV_ITEM_POS_PROP (double, Width)
GCCV_ITEM_PROP (GOColor, FillColor)
};

}

#endif	//	 GCCV_WEDGE_H
