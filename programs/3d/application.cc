// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * programs/3d/application.cc
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

#include "config.h"
#include "application.h"
#include "document.h"
#include "view.h"
#include "window.h"
#include <glib/gi18n.h>
#include <cstring>

using namespace std;

gc3dApplication::gc3dApplication (gcu::Display3DMode display3d, char const *bg):
	gcugtk::Chem3dApplication (display3d, bg)
{
}

gc3dApplication::~gc3dApplication ()
{
}

gcugtk::Chem3dDoc *gc3dApplication::OnFileNew ()
{
	gc3dDocument* Doc = new gc3dDocument (this);
	Doc->SetTitle (_("GChem3D Viewer"));
	Doc->SetDisplay3D (m_Display3D);
	gcu::GLView *view = Doc->GetView ();
	view->SetRed (m_Red);
	view->SetGreen (m_Green);
	view->SetBlue (m_Blue);
	gc3dWindow *w = new gc3dWindow (this, Doc);
	w->SetTitle (_("GChem3D Viewer"));
	return Doc;
}
