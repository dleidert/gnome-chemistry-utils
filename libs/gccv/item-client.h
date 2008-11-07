// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * canvas/item-client.h 
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

#ifndef GCCV_ITEM_CLIENT_H
#define GCCV_ITEM_CLIENT_H

#include <gcu/macros.h>

namespace gccv {

class Item;

class ItemClient {
friend class Item;
public:
	ItemClient ();
	virtual ~ItemClient ();

/*!
Used to add a representation of the Object in the view. This method might be overloaded for displayable Object classes
unless the application uses another mechanism.
*/
	virtual void AddItem ();
/*!
Used to update the representation of the Object in the view. This method might be overloaded for displayable Object classes
unless the application uses another mechanism.
*/
	virtual void UpdateItem ();
/*!
@param state the selection state of the Object.

Used to set the selection state of the Object inside the widget. The values of state are application dependant and have no
default value.
*/
	virtual void SetSelected (int state);

GCU_PROT_POINTER_PROP (Item, Item)
};

}

#endif	//	GCCV_CLIENT_H
