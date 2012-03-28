// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * programs/3d/document.h
 *
 * Copyright (C) 2006-2011 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GC3D_DOCUMENT_H
#define GC3D_DOCUMENT_H

#include <gcu/macros.h>
#include <gcugtk/chem3ddoc.h>

namespace gcu {
class Window;
}

class gc3dApplication;

class gc3dDocument: public gcugtk::Chem3dDoc
{
public:
	gc3dDocument (gc3dApplication *App);
	virtual ~gc3dDocument ();

	void Load (char const *uri, char const *mime_type);
	gcu::Window *GetWindow ();
};

#endif	//	GC3D_DOCUMENT_H
