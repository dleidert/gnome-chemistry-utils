// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gccv/line.h 
 *
 * Copyright (C) 2008-2010 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCCV_LINE_H
#define GCCV_LINE_H

/*!\file*/

#include "line-item.h"

namespace gccv {

/*!
@brief Straight lines.

Implements one segment straight lines.
*/
class Line: public LineItem
{
public:
	Line (Canvas *canvas, double xstart, double ystart, double xend, double yend);
	Line (Group *parent, double xstart, double ystart, double xend, double yend, ItemClient *client = NULL);
	virtual ~Line ();

	void SetPosition (double xstart, double ystart, double xend, double yend);

	// virtual methods
	double Distance (double x, double y, Item **item) const;
	void Draw (cairo_t *cr, bool is_vector) const;
	void UpdateBounds ();
	void Move (double x, double y);

protected:
	double m_xstart;
	double m_ystart;
	double m_xend;
	double m_yend;
};

}

#endif	//	GCCV_LINE_H
