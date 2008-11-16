// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gccv/hash.h 
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

#ifndef GCCV_HASH_H
#define GCCV_HASH_H

#include "wedge.h"

namespace gccv {

class Hash: public Wedge
{
public:
	Hash (Canvas *canvas, double x0, double y0, double x1, double y1, double width);
	Hash (Group *parent, double x0, double y0, double x1, double y1, double width, ItemClient *client = NULL);
	virtual ~Hash ();

	// virtual methods
	void Draw (cairo_t *cr, bool is_vector) const;

GCCV_ITEM_POS_PROP (double, LineWidth)
GCCV_ITEM_POS_PROP (double, LineDist)
};

}

#endif	//	 GCCV_HASH_H
