// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gcugtk/chem3dapplication.h
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

#ifndef GCU_GTK_CHEM3D_APPLICATION_H
#define GCU_GTK_CHEM3D_APPLICATION_H

#include "application.h"
#include <gcu/chem3ddoc.h>

/*!\file*/
namespace gcugtk {

class Chem3dDoc;

/*!
\class Chem3dApplication gcugtk/chem3dapplication.h

Application class for the molecule 3d viewer.
*/
class Chem3dApplication: public Application
{
public:
/*!
@param display3d the default display mode for the application.
@param bg the default background color name, accetpted values are "white", "black",
and "#rrggbb". Defaults to 'black'.

The constructor.
*/
	Chem3dApplication (gcu::Display3DMode display3d = gcu::BALL_AND_STICK, char const *bg = "black");

	/*!
Default destructor.
*/
	virtual ~Chem3dApplication ();

/*!
Pure virtual function used to create a new document of the appropriate type.

@return the newly created document.
*/
	virtual Chem3dDoc *OnFileNew () = 0;

/*!
@param doc a Chem3dDoc

Displays a file open dialog and when a file is selected, loads it's contents
inside \a doc if it's empty or inside a nex document.
*/
	void OnFileOpen (Chem3dDoc *doc);

/*!
@param doc the document to save as an image.

Displays a file selctor and saves the image insode the selected file if any. The
framework will ask the user to overwrite or not if the file already exists.
*/
	void OnSaveAsImage (Chem3dDoc *doc);

/*!
@param filename the uri of the file.
@param mime_type the mime type of the file if known.
@param bSave true if saving, and false if loading.
@param window the current top level window.
@param pDoc an optional document.

Called by the FileChooser when a file name has been selected. Saves or loads the
document according to \a bSave.

@return true if no error occured.
*/
	bool FileProcess (char const *filename, char const *mime_type, bool bSave, GtkWindow *window, gcu::Document *pDoc = NULL);

/*!
Called when the user selects the 'Quit' menu item.
*/
	void OnQuit ();

/*!\var m_Display3D
The current default display mode.
*/
/*!\fn GetDisplay3D()
@return the current default display mode.
*/
GCU_PROT_PROP (gcu::Display3DMode, Display3D)
/*!\var m_Red
The current red component of the default background color.
*/
/*!\fn GetRed()
@return the current red component of the default background color.
*/
GCU_PROT_PROP (float, Red);
/*!\var m_Green
The current green component of the default background color.
*/
/*!\fn GetGreen()
@return the current green component of the default background color.
*/
GCU_PROT_PROP (float, Green);
/*!\var m_Blue
The current blue component of the default background color.
*/
/*!\fn GetBlue()
@return the current blue component of the default background color.
*/
GCU_PROT_PROP (float, Blue);
};

}	// namespace gcugtk

#endif	//	GCU_GTK_CHEM3D_APPLICATION_H
