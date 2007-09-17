// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * programs/spectra/application.h
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


#ifndef GSV_APPLICATION_H
#define GSV_APPLICATION_H

#include <gcu/application.h>

class gsvDocument;

class gsvApplication: public gcu::Application
{
public:
	gsvApplication ();
	~gsvApplication ();

	gsvDocument *OnFileNew ();
	void OnFileOpen (gsvDocument *Doc);
	void OnSaveAsImage (gsvDocument *Doc);
	bool FileProcess (const gchar* filename, const gchar* mime_type, bool bSave, GtkWindow *window, gcu::Document *pDoc = NULL);
	void OnQuit ();
};

#endif	//	GSV_APPLICATION_H
