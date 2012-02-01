// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gcugtk/chem3dapplication.h
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

#ifndef GCU_GTK_CHEM3D_APPLICATION_H
#define GCU_GTK_CHEM3D_APPLICATION_H

#include "application.h"
#include <gcu/chem3ddoc.h>

/*!\file*/
namespace gcugtk {

class Chem3dDoc;

/*!
\class Chem3dView gcugtk/chem3dapplication.h

Application class for the molecule 3d viewer.
*/
class Chem3dApplication: public Application
{
public:
/*!
@param doc the document for the view.

Default constructor
*/
	Chem3dApplication (gcu::Display3DMode display3d = gcu::BALL_AND_STICK, char const *bg = "black");

	/*!
Default destructor
*/
	virtual ~Chem3dApplication ();

/*!
Pure virtual function used to create a new document of the appropriate type.

@return the newly created document.
*/
	virtual Chem3dDoc *OnFileNew () = 0;

/*!

*/
	void OnFileOpen (Chem3dDoc *doc);

/*!

*/
	void OnSaveAsImage (Chem3dDoc *Doc);

/*!

*/
	bool FileProcess (const gchar* filename, const gchar* mime_type, bool bSave, GtkWindow *window, gcu::Document *pDoc = NULL);

/*!

*/
	void OnQuit ();

GCU_PROT_PROP (gcu::Display3DMode, Display3D)
GCU_PROT_PROP (float, Red);
GCU_PROT_PROP (float, Green);
GCU_PROT_PROP (float, Blue);
};

}	// namespace gcugtk

#endif	//	GCU_GTK_CHEM3D_APPLICATION_H
