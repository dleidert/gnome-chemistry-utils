// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gcugtk/chem3ddoc.h
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

#ifndef GCU_GTK_CHEM3D_DOCUMENT_H
#define GCU_GTK_CHEM3D_DOCUMENT_H

#include <gcu/chem3ddoc.h>

namespace gcu {
	class GLView;
}
/*!\file*/
namespace gcugtk {

class Application;
class GLView;

/*!
\class Chem3dDoc gcugtk/chem3ddoc.h

Document class for a molecule.
*/
class Chem3dDoc: public gcu::Chem3dDoc
{
public:
/*!
Default constructor
*/
	Chem3dDoc ();
/*!
@param App the application.
@param View: an optional already existing GLView instance.
*/
	Chem3dDoc (Application *App, GLView *View);
/*!
Default destructor
*/
	virtual ~Chem3dDoc ();

	virtual gcu::GLView *CreateView ();
};

}	// namespace gcugtk

#endif	//	GCU_GTK_CHEM3D_DOCUMENT_H
