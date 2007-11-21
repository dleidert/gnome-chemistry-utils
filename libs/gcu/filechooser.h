// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * filechooser.h 
 *
 * Copyright (C) 2006-2007 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCU_FILECHOOSER_H
#define GCU_FILECHOOSER_H

#include <gtk/gtkstock.h>
#include <gtk/gtkfilechooser.h>
#include <list>
#include <string>

namespace gcu
{

class Application;
class Document;

/*!\class FileChooser gcu/filechooser.h
This class is used each time a file name must be choosen. It embeds a GtkFileChooserDialog
window.
*/
class FileChooser
{
public:
/*!
@param App the Application instance owning the dialog.
@param Save tells if the requested file is to be saved or loaded.
@param mime_types a std::list of supported mime types.
@param pDoc the document to save, the parameter is optional when loading.
Default value is NULL.
@param title an optional title for the dialog box. Default value is NULL.
@param extra_widget an optional widget to add to the GtkFileChooserDialog
window. Default value is NULL.

The constructor shows the GtkFileChooserDialog window, retrieve it's response and calls
Application::FileProcess if needed. The dialog is modal (nothing else can be done while
it is opened).
*/
	FileChooser (Application *App, bool Save, std::list<std::string> mime_types, Document *pDoc = NULL, char const *title = NULL, GtkWidget *extra_widget = NULL);
	
private:
	GtkFileChooser* dialog;
	bool m_bSave;
	Application* m_pApp;
	Document* m_pDoc;
};

}

#endif	// GCU_FILECHOOSER_H
