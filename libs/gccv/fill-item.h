// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * canvas/fill-item.h 
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

#ifndef GCCV_FILL_ITEM_H
#define GCCV_FILL_ITEM_H

#include "line-item.h"

namespace gccv {

class FillItem: public LineItem {
public:
	FillItem (Canvas *canvas);
	FillItem (Group *parent, ItemClient *client = NULL);
	virtual ~FillItem ();

GCCV_ITEM_PROP (GOColor, FillColor)
};

}

#endif	//	GCCV_FILL_ITEM_H
