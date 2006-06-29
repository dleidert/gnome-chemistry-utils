// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * programs/3d/application.h 
 *
 * Copyright (C) 2006 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GC3D_APPLICATION_H
#define GC3D_APPLICATION_H

#include <gcu/application.h>

using namespace gcu;

class gc3dDocument;

class gc3dApplication: public Application
{
public:
	gc3dApplication ();
	~gc3dApplication ();

	gc3dDocument *OnFileNew ();
	void OnFileOpen (gc3dDocument *Doc);
	bool FileProcess (const gchar* filename, const gchar* mime_type, bool bSave, GtkWindow *window, Document *pDoc = NULL);
	void OnQuit ();
};

#endif	//	GC3D_APPLICATION_H