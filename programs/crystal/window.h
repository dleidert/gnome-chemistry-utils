// -*- C++ -*-

/*
 * Gnome Crystal
 * window.h
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
 * USA
 */

#ifndef GCRYSTAL_WINDOW_H
#define GCRYSTAL_WINDOW_H

#include <gcu/macros.h>
#include <gtk/gtk.h>

class gcApplication;
class gcDocument;
class gcView;

class gcWindow
{
public:
	gcWindow (gcApplication *App, gcDocument *Doc);
	~gcWindow ();

	gcApplication *GetApplication () {return m_App;}
	void ClearStatus ();
	void SetStatusText (const char* text);
	bool TryClose ();
	void Destroy ();
	void ActivateActionWidget (char const *path, bool activate);

private:
	GtkUIManager* m_UIManager;
	GtkWidget* m_Bar;	//GtkStatusBar
	unsigned m_statusId;
	unsigned m_MessageId; //currently displayed message in the status bar

GCU_RO_PROP (gcApplication *, App);
GCU_RO_PROP (gcView *, View);
GCU_RO_PROP (gcDocument *, Doc);
GCU_RO_PROP (GtkWindow *, Window);
};

#endif	//	GCRYSTAL_WINDOW_H
