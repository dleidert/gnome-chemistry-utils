// -*- C++ -*-

/*
 * GCrystal library
 * application.h
 *
 * Copyright (C) 2010-2011 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCR_APPLICATION_H
#define GCR_APPLICATION_H

#include <gcugtk/application.h>
#include <goffice/goffice.h>

namespace gcr {

class Document;
class View;
class Window;

class Application: public gcugtk::Application {
public:
	Application ();
	virtual ~Application ();

	virtual gcr::Document *OnFileNew () = 0;
	void OnFileOpen ();
	void OnFileSave ();
	void OnFileSaveAs ();
	bool OnFileClose ();
	void OnSaveAsImage ();
	bool OnQuit ();
	void SetActiveDocument (Document *doc) {m_pActiveDoc = doc;}
	virtual Window *CreateNewWindow (Document *doc) = 0;
	bool FileProcess (const gchar* filename, const gchar* mime_type, bool bSave, GtkWindow *window, gcu::Document *pDoc = NULL);
	char const *GetFirstSupportedMimeType (std::list<std::string>::iterator &it);
	char const *GetNextSupportedMimeType (std::list<std::string>::iterator &it);
	Document* GetDocument (const char* filename);
	bool IsEmpty() {return m_Views.empty();}
	void SetOpening() {m_bFileOpening = true;}

private:
	void AddMimeType (std::list<std::string> &l, std::string const& mime_type);

private:
	Document* m_pActiveDoc;
	std::list<std::string> m_SupportedMimeTypes;
	std::list<std::string> m_WriteableMimeTypes;
	std::list <View*> m_Views;
	GtkUIManager* m_UIManager;
	unsigned m_statusId;
	bool m_bFileOpening;
	guint m_NotificationId;

GCU_RO_PROP (GOConfNode *, ConfNode)
};

}	//	namespace gcr

#endif	//	GCR_APPLICATION_H