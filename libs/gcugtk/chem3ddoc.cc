// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gcugtk/chem3ddoc.cc
 *
 * Copyright (C) 2006-2011 Jean Bréfort <jean.brefort@normalesup.org>
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

#include "config.h"
#include "application.h"
#include "chem3ddoc.h"
#include "glview.h"

namespace gcugtk
{

Chem3dDoc::Chem3dDoc (): gcu::Chem3dDoc ()
{
}

Chem3dDoc::Chem3dDoc (Application *App, GLView *View): gcu::Chem3dDoc (App, View)
{
}

Chem3dDoc::~Chem3dDoc ()
{
}

gcu::GLView *Chem3dDoc::CreateView ()
{
	return new GLView (this);
}

}	//	namespace gcugtk
