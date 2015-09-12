// -*- C++ -*-

/*
 * GCrystal library
 * application.h
 *
 * Copyright (C) 2010-2012 Jean Bréfort <jean.brefort@normalesup.org>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef GCR_APPLICATION_H
#define GCR_APPLICATION_H

#include <gcugtk/glapplication.h>
#include <goffice/goffice.h>

/*!\file*/

/*!\namespace gcr
\brief The crystal structures related classes

The namespace used for C++ classes related to crystal structures.
*/
namespace gcr {

class Document;
class View;
class Window;

/*!\class Application gcr/application.h
\brief GCrystal application base class.

This class is used to represent a GCrystal application.
It is a virtual class since at least one method is pure virtual (gcp::Application::OnFileNew)
*/
class Application: public gcugtk::GLApplication {
public:
/*!
The default constructor.
*/
	Application ();
/*!
The destructor.
*/
	virtual ~Application ();

/*!
Creates a new document. This method is pure virtual and must be implemented
in derived classes.
*/
	virtual gcr::Document *OnFileNew () = 0;

/*!
Open the file open dialog to select documents to open.
*/
	void OnFileOpen ();

/*!
Saves the current document. If it has no file name, the file save as dialog
will pop up.
*/
	void OnFileSave ();

/*!
Open the file save as dialog to save the current document with a new name.
*/
	void OnFileSaveAs ();

/*!
Closes the current document. If the document has been modified, the user will be
asked if he wants to save the modifications or ignore them.
*/
	bool OnFileClose ();

/*!
Open the file save as dialog to save the current view as an image.
*/
	void OnSaveAsImage ();

/*!
Quits the application. If a document has been modified, the user will be
asked if he wants to save the modifications or ignore them. 
*/
	bool OnQuit ();

/*!
@param doc the document becoming active.

Sets the new active document.
*/
	void SetActiveDocument (Document *doc) {m_pActiveDoc = doc;}

/*!
@param doc a document.

Creates a new window for the document.
@return the newly allocated window.
*/
	virtual Window *CreateNewWindow (Document *doc);

/*!
@param filename the uri of the file.
@param mime_type the mime type of the file if known.
@param bSave true if saving, and false if loading.
@param window the current top level window.
@param pDoc an optional document.

Called by the FileChooser when a file name has been selected. This method loads
a new file or saves \a pDoc according to \a bSave. When loading, a new document
is created unless \a pDoc is not NULL and is empty.

@return true if no error occured.
*/
	bool FileProcess (char const *filename, char const *mime_type, bool bSave, GtkWindow *window, gcu::Document *pDoc = NULL);

/*!
@param it a mime type iterator.

Initializes the iterator and returns the first registered mime type.
@return the first registered mime type.
*/
	char const *GetFirstSupportedMimeType (std::list<std::string>::iterator &it);

/*!
@param it a mime type iterator initialized during GetFirstSupportedMimeType()
call.

@return the next registered mime type.
*/
	char const *GetNextSupportedMimeType (std::list<std::string>::iterator &it);

/*!
@param filename a file name

@return the document associated with \a filename if opened or NULL.
*/
	Document* GetDocument (const char* filename);

/*!
@return whether there is no opened view.
*/
	bool IsEmpty() {return m_Views.empty();}

/*!
Tells the application that a document is loading.
*/
	void SetOpening() {m_bFileOpening = true;}

private:
	void AddMimeType (std::list<std::string> &l, std::string const& mime_type);

protected:
/*!
The active document.
*/
	Document* m_pActiveDoc;

private:
	std::list<std::string> m_SupportedMimeTypes;
	std::list<std::string> m_WriteableMimeTypes;
	std::list <View*> m_Views;
	GtkUIManager* m_UIManager;
	unsigned m_statusId;
	bool m_bFileOpening;
	unsigned m_NotificationId;

/*!\fn GetConfNode()
@return the GOConfNode used for the default configuration.
*/
GCU_RO_PROP (GOConfNode *, ConfNode)
};

}	//	namespace gcr

#endif	//	GCR_APPLICATION_H