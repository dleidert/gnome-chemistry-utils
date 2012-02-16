// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gcugtk/chem3dview.h
 *
 * Copyright (C) 2011-2012 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCU_GTK_CHEM3D_VIEW_H
#define GCU_GTK_CHEM3D_VIEW_H

#include "glview.h"

/*!\file*/
namespace gcugtk {

class Chem3dDoc;
class Chem3dWindow;

/*!
\class Chem3dView gcugtk/chem3dview.h

View class for a molecule.
*/
class Chem3dView: public GLView
{
public:
/*!
@param doc the document for the view.

Default constructor
*/
	Chem3dView (Chem3dDoc *doc);

	/*!
Default destructor
*/
	virtual ~Chem3dView ();

// Properties
/*!\fn SetWindow()
@param val the new window for the view.

Sets the view window.
*/
/*!\fn GetWindow()
@return the view window.
*/
GCU_POINTER_PROP (Chem3dWindow, Window);
};

}	// namespace gcugtk

#endif	//	GCU_GTK_CHEM3D_VIEW_H
