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

#ifndef GCCV_LINE_ITEM_H
#define GCCV_LINE_ITEM_H

#include "item.h"
#include <gcu/macros.h>
#include <goffice/utils/go-color.h>

namespace gccv {

class LineItem: public Item {
public:
	LineItem (Canvas *canvas);
	LineItem (Group *parent, ItemClient *client = NULL);
	virtual ~LineItem ();

GCCV_ITEM_PROP (double, LineWidth)
GCCV_ITEM_PROP (GOColor, LineColor)
};

}

#endif	//	GCCV_LINE_ITEM_H
