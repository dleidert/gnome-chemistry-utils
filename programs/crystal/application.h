// -*- C++ -*-

/* 
 * Gnome Crystal
 * application.h 
 *
 * Copyright (C) 2001-2005
 *
 * Developed by Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCRYSTAL_APPLICATION_H
#define GCRYSTAL_APPLICATION_H

#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#include <list>
#include "config.h"
#include "document.h"
#include "view.h"

class gcApplication
{
public:
	gcApplication();
	~gcApplication();

//Callbacks methods
	void OnFileNew();
	void OnFileOpen();
	void OnFileSave();
	void OnFileSaveAs();
	bool OnFileClose();
	void OnFilePrint();
	void OnExportJPEG();
	void OnExportPNG();
	void OnExportVRML();
	void OnViewNew();
	bool OnViewClose(gcView *pView = NULL);
	void OnSelectView(GtkWidget* widget);
	void OnCrystalDefine(int page);
	void OnViewSettings();
	bool LoadFile(char* filename);
	bool HasDocument(gcDocument* pDoc);
	void AddView(gcView* pView);
	bool IsEmpty() {return m_Views.empty();}
	void UpdateConfig();
	gcView* GetDocView(const char* filename);
	gcView* GetView(gcDocument* pDoc);
	void SetOpening() {m_bFileOpening = true;}
	void SelectView(gcView* pView);
	void OnChangePage(int i);
	
private:
	std::list<gcView*>m_Views;
	gcView* m_pActiveView;
	GtkWidget* m_App;
	GtkNotebook* m_Notebook;
	GtkMenu* m_WindowsMenu;
	bool m_bFileOpening;
};

extern std::list<gcApplication*> Apps;
#endif //GCRYSTAL_APPLICATION_H
