// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gcu/window.h
 *
 * Copyright (C) 2008 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCU_WINDOW_H
#define GCU_WINDOW_H

#include "macros.h"
#include "ui-builder.h"

/*!\file*/
namespace gcu {
class UIManager;

/*!\class Window gcu/window.h
Base class for windows. Just a placeholder for now.
*/
class Window
{
public:
/*!
The default constructor.
*/
	Window ();
/*!
The destructor.
*/
	virtual ~Window ();

/*!
Destroys the window. This method should be overloaded in derived classes since
default imlementation does not do anything.
*/
	virtual void Destroy ();
/*!
Raises the window and makes it active. This method should be overloaded in
derived classes since default imlementation does not do anything.
*/
	virtual void Show ();
/*!
@param path the path to activate.
@param activate whether to activate or deactivate.

Activates or deactivates the menu item corresponding to \a path according
to the value of \a activate.

To deactivate the "Paste" menu item, use:
\code
ActivateActionWidget ("/MainMenu/EditMenu/Paste", false);
\endcode
*/
	void ActivateActionWidget (char const *path, bool activate);

protected:
/*!
The gcugtk::UIManager associated with the window.
*/
	UIManager* m_UIManager;
GCU_PROT_POINTER_PROP (UIBuilder, Builder)
};

}

#endif	//	GCU_WINDOW_H
