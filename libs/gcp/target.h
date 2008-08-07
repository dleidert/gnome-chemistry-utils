// -*- C++ -*-

/* 
 * GChemPaint library
 * target.h 
 *
 * Copyright (C) 2007 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCP_TARGET_H
#define GCP_TARGET_H

#include <gcu/macros.h>
#include <gcu/window.h>
#include <gtk/gtkwindow.h>

/*!\file*/
namespace gcp {

class Application;
class Document;

/*!\class Target
The base class for windows able to edit GChemPaint files. When one of these
windows is active, the tools box is displayed on the same desktop.
gcp::Window is a derived class. Another one is gcpResidueDialog implemented
in the residue plugin (but not documented like everything implemented
in plugins).
*/
class Target: public gcu::Window
{
public:
/*!
@param App the application owning the target.

Creates a target for the application.
*/
	Target (Application *App);
/*!
The destructor.
*/
	virtual ~Target ();

/*!
@param window a GtkWindow.

Sets \a window as the GtkWindow for this target.
*/
	void SetWindow (GtkWindow *window);

/*!
virtual method called when the application ends to ensure everything is
correctly closed. This is a pure virtual class, so it must be overloaded.
*/
	virtual bool Close () = 0;
/*!\var m_Application
The gcp::Application owning this target.
*/
/*!\fn GetApplication()
@return the gcp::Application owning this target.
*/
GCU_PROT_PROP (Application*, Application)
/*!\var m_Document
The gcp::Document displayed in this target window.
*/
/*!\fn GetApplication()
@return the gcp::Document displayed in this target window.
*/
GCU_PROT_PROP (Document*, Document)
};

}	// namespace gcp

#endif	//	GCP_TARGET_H
