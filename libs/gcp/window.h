/* 
 * GChemPaint library
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


#ifndef GCP_WINDOW_H
#define GCP_WINDOW_H

#include "target.h"
#include <gtk/gtkwidget.h>
#include <gtk/gtkuimanager.h>

namespace gcp {

class Application;
class Document;
	
class Window: public Target
{
public:
	Window (gcp::Application *App, char const *Theme = NULL, char const *extra_ui = NULL);
	virtual ~Window ();

	void OnFileNew ();
	void OnFileOpen ();
	void OnProperties ();
	void OnPrint ();
	void SetActive (gcp::Document* pDoc, GtkWidget* w);
	void OnUndo ();
	void OnRedo ();
	void OnSelectAll ();
	void OnPasteSelection ();
	void OnCutSelection ();
	void OnCopySelection ();
	void OnDeleteSelection ();
	void OnPreferences ();
	void Zoom (double zoom);
	void ClearStatus ();
	void SetStatusText (const char* text);
	void Show ();
	bool OnKeyPressed (GtkWidget* widget, GdkEventKey* ev);
	bool OnKeyReleased (GtkWidget* widget, GdkEventKey* ev);
	bool Close ();

	virtual void OnSave ();
	virtual char const *GetDefaultTitle ();

	gcp::Application *GetApplication () {return m_App;}

	void Destroy ();
	void SetTitle (char const *title);
	void ActivateActionWidget (char const *path, bool activate); 
	bool VerifySaved ();

protected:
	gcp::Application *m_App;
	GtkUIManager* m_UIManager;

private:
	GtkWidget* m_Bar;	//GtkStatusBar
	unsigned m_statusId;
	unsigned m_MessageId; //currently displayed message in the status bar
};

}	// namespace gcp

#endif	//	GCP_WINDOW_H
