// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gcugtk/chem3dwindow.h
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

#ifndef GCU_GTK_CHEM3D_WINDOW_H
#define GCU_GTK_CHEM3D_WINDOW_H

#include "window.h"

/*!\file*/
namespace gcugtk {

class Application;
class Chem3dDoc;
class Chem3dView;
class Molecule;

/*!
\class Chem3dWindow gcugtk/chem3dwindow.h

Window class for a molecule.
*/
class Chem3dWindow: public Window
{
friend class Chem3dWindowPrivate;
public:
/*!
@param app the application owning the window.
@param doc the document for the window.
@param extra_ui an xml string representing menus and tool buttons to add to the
window.

The constructor.
*/
	Chem3dWindow (Application *app, Chem3dDoc *doc, char const *extra_ui = NULL);

/*!
Default destructor.
*/
	virtual ~Chem3dWindow ();

/*!
@param mol the displayed molecule.

Adds menus relative to the displayed molecule, such as database access, InChI
generation and so on.
*/
	void AddMoleculeMenus (Molecule *mol);

protected:
/*
Virtual method to save the displayed molecule if needed. Default implementation
does nothing.
*/
	virtual void Save ();

/*!\var m_Application
The Application owning the window.
*/
/*!\fn GetApplication()
@return the Application owning the window.
*/
	GCU_PROT_POINTER_PROP (Application, Application);
/*!\var m_Document
The document displayed in the window.
*/
/*!\fn GetDocument()
@return the document displayed in the window.
*/
	GCU_PROT_POINTER_PROP (Chem3dDoc, Document);
/*!\var m_View
The view associated with the window.
*/
/*!\fn GetView()
@return view associated with the window.
*/
GCU_PROT_POINTER_PROP (Chem3dView, View);
};

}	// namespace gcugtk

#endif	//	GCU_GTK_CHEM3D_VIEW_H
