/*
 * Gnome Chemistry Utils
 * gcugtk/ui-manager.h
 *
 * Copyright (C) 2011 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCU_GTK_UI_MANAGER_H
#define GCU_GTK_UI_MANAGER_H

#include <gcu/macros.h>
#include <gcu/ui-manager.h>
#include <gtk/gtk.h>

/*!\file */
namespace gcugtk {

/*!\class UIManager
@brief A GtkUIManager wrapper.

Implements gcu::UIManager using an associated GtkUIManager.
*/
class UIManager: public gcu::UIManager
{
public:
/*!
@param ui A GtkUIManager.

The constructor. Associates \a ui with the new instance.
*/
	UIManager (GtkUIManager *ui);
/*!
The destructor.
*/
	virtual ~UIManager ();

/*!
@param path a path describing a menu or tool item.
@param activate whether to activate the item.

Make the item associated to \a path active or inactive according to \a activate.
*/
	void ActivateActionWidget (char const *path, bool activate);

/*!\fn GetUIManager()
@return the associated GtkUIManager.
*/
GCU_RO_PROP (GtkUIManager *, UIManager)
};

}	//	namespace gcu

#endif	//	GCU_GTK_UI_MANAGER_H
