// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gcugtk/window.h
 *
 * Copyright (C) 2011 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCU_GTK_WINDOW_H
#define GCU_GTK_WINDOW_H

#include <gcu/macros.h>
#include <gcu/window.h>
#include <gtk/gtk.h>

/*!\file*/
namespace gcugtk {

class Window: public gcu::Window
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

/*!\var m_Window
The GtkWindow instance corresponding to the Window.
*/
/*!\fn GetWindow()
@return the GtkWindow correspondig to this Window instance.
*/
GCU_PROT_PROP (GtkWindow*, Window);
};

}	//	namespace gcu

#endif	//	GCU_GTK_WINDOW_H
