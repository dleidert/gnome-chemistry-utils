/*
 * Gnome Chemistry Utils
 * gcu/ui-manager.h
 *
 * Copyright (C) 2011-2012 Jean Bréfort <jean.brefort@normalesup.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the
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

/*!\file*/

#ifndef GCU_UI_BUILDER_H
#define GCU_UI_BUILDER_H

namespace gcu {

/*!\class UIBuilder gcu/ui-builder.h
@brief base class for a user interface manager
*/
class UIBuilder
{
public:
/*!
The default constructor.
*/
	UIBuilder ();

/*!
The destructor.
*/
	virtual ~UIBuilder ();

/*!
@param path a path describing a menu or tool.
@param activate whether to activate the item.

Make the item associated to \a path active or inactive according to \a activate.
*/
	virtual void ActivateActionWidget (char const *path, bool activate);
	// TODO: add as many virtual functions as needed

};

}	//	namespace gcu

#endif	//	GCU_UI_BUILDER_H
