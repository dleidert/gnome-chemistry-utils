// -*- C++ -*-

/* 
 * Gnome Crystal
 * application.h 
 *
 * Copyright (C) 2001-2006 Jean Bréfort <jean.brefort@normalesup.org>
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

#include <list>
#include "config.h"
#include "document.h"
#include "view.h"
#include <gcu/application.h>

using namespace gcu;

class gcApplication: public Application
{
public:
	gcApplication();
	~gcApplication();

//Callbacks methods
	gcDocument *OnFileNew();
	void OnFileOpen();
	void OnFileSave();
	void OnFileSaveAs();
	bool OnFileClose();
	void OnFilePrint();
	void OnSaveAsImage ();
	bool IsEmpty() {return m_Views.empty();}
	gcDocument* GetDoc (const char* filename);
	void SetOpening() {m_bFileOpening = true;}
	bool FileProcess (const gchar* filename, const gchar* mime_type, bool bSave, GtkWindow *window, Document *pDoc = NULL);
	void SetActiveDocument (gcDocument *doc) {m_pActiveDoc = doc;}
	void AddDocument (gcDocument *pDoc) {m_Docs.push_front (pDoc);}
	void RemoveDocument (gcDocument *pDoc);
	bool OnQuit ();
	
private:
	list<gcView*>m_Views;
	list<gcDocument*> m_Docs;
	gcDocument* m_pActiveDoc;
	GtkUIManager* m_UIManager;
	unsigned m_statusId;
	bool m_bFileOpening;
};

#endif //GCRYSTAL_APPLICATION_H
