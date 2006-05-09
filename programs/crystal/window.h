// -*- C++ -*-

/* 
 * Gnome Crystal
 * window.h 
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#ifndef GCRYSTAL_WINDOW_H
#define GCRYSTAL_WINDOW_H

#include <gtk/gtkwidget.h>
#include <gtk/gtkwindow.h>
#include <gtk/gtkuimanager.h>

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
	gcDocument *GetDocument () {return m_Doc;}
	bool TryClose ();
	gcView *GetView () {return m_View;}
	void Destroy ();

private:
	gcApplication *m_App;
	gcDocument *m_Doc;
	gcView *m_View;
	GtkUIManager* m_UIManager;
	GtkWindow* m_Window;
	GtkWidget* m_Bar;	//GtkStatusBar
	unsigned m_statusId;
	unsigned m_MessageId; //currently displayed message in the status bar
};

#endif	//	GCRYSTAL_WINDOW_H
