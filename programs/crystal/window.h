// -*- C++ -*-

/*
 * Gnome Crystal
 * window.h
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
 * USA
 */

#ifndef GCRYSTAL_WINDOW_H
#define GCRYSTAL_WINDOW_H

#include <gcu/macros.h>
#include <gcr/window.h>
#include <gtk/gtk.h>

class gcApplication;
class gcDocument;
class gcView;

class gcWindow: public gcr::Window
{
public:
	gcWindow (gcApplication *App, gcDocument *Doc);
	~gcWindow ();

	gcApplication *GetApplication () {return GetApplication ();}
	bool TryClose ();
	void Destroy ();
};

#endif	//	GCRYSTAL_WINDOW_H
