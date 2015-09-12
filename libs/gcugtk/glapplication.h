// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gcugtk/glapplication.h
 *
 * Copyright (C) 2015 Jean Bréfort <jean.brefort@normalesup.org>
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

#include "application.h"

#ifndef GCU_GTK_GL_APPLICATION_H
#define GCU_GTK_GL_APPLICATION_H

namespace gcugtk {

/*!
\class GLApplication gcu/GLApplication.h
View class based on OpenGL for rendering. Used to display 3d chemical structures
such as molecules or crystals cells.
*/
class GLApplication: public Application
{
friend class GLApplicationPrivate;
public:
//!Constructor.
/*!
@param name the name of the application.
@param datadir where data for the application are stored.
@param help_name the name to use for the help file (with .xml extension).
If NULL, the name parameter is used.
@param icon_name the name to use for the default icon of all windows. If NULL,
the help_name or name parameters will be used.
@param cc the associated CmdContextGtk.

The datadir variable is used to build the full path to the help file:
"file://"+datadir+"/gnome/help/"+name+"/"+LANG+"/"+name".xml".
*/
	GLApplication (std::string name, std::string datadir = DATADIR, char const *help_name = NULL, char const *icon_name = NULL, CmdContextGtk *cc = NULL);
//!Destructor.
/*!
The destructor of GLApplication.
*/
	virtual ~GLApplication ();

	 
private:
	GOConfNode *m_ConfNode;
	unsigned m_NotificationId;

/*!GetRenderDirect()
@return whether to use direct rendering when drawing to a pixbuf.
*/
GCU_PROT_PROP (bool, RenderDirect)
};

}	//	namespace gcugtk

#endif	//	GCU_GTK_GL_APPLICATION_H