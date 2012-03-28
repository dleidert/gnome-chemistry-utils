// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * programs/3d/window.h
 *
 * Copyright (C) 2006 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GC3D_WINDOW_H
#define GC3D_WINDOW_H

#include <gcugtk/chem3dwindow.h>

namespace gcugtk
{
	class Molecule;
}

class gc3dApplication;
class gc3dDocument;
class gc3dView;

class gc3dWindow: public gcugtk::Chem3dWindow
{
public:
	gc3dWindow (gc3dApplication *App, gc3dDocument *Doc);
	virtual ~gc3dWindow ();

	void SetTitle (char const *title);
	void DoPrint (GtkPrintOperation *print, GtkPrintContext *context);
};

#endif	// GC3D_WINDOW_H
