// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * programs/spectra/window.h
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

#ifndef GSV_WINDOW_H
#define GSV_WINDOW_H

#include <gtk/gtkwindow.h>

class gsvApplication;
class gsvDocument;
class gsvView;

class gsvWindow
{
public:
	gsvWindow (gsvApplication *App, gsvDocument *Doc);
	virtual ~gsvWindow ();

	void OnFileOpen ();
	void OnFilePrint ();
	void OnFileClose ();
	void OnCopy ();
	void SetTitle (string const &title);

private:
	GtkWindow* m_Window;

GCU_RO_PROP (gsvApplication *, App);
GCU_RO_PROP (gsvView *, View);
GCU_RO_PROP (gsvDocument *, Doc);
};

#endif	// GSV_WINDOW_H
