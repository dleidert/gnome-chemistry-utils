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

namespace gccv {

class BezierArrow: public LineItem {
public:
	BezierArrow (Canvas *canvas);
	BezierArrow (Group *parent, ItemClient *client = NULL);
	virtual ~BezierArrow ();

	// virtual methods
	double Distance (double x, double y, Item **item) const;
	void Draw (cairo_t *cr, bool is_vector) const;
	void UpdateBounds ();
	void SetControlPoints (double x0, double y0, double x1, double y1, double x2, double y2, double x3, double y3);

GCCV_ITEM_POS_PROP (ArrowHeads, Head)
GCCV_ITEM_POS_PROP (double, A)
GCCV_ITEM_POS_PROP (double, B)
GCCV_ITEM_POS_PROP (double, C)
GCCV_ITEM_POS_PROP (bool, ShowControls)

private:
	Point m_Controls[4];
};

}


#endif  //  GCCV_BEZIER_ARROW_H
