// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gcu/application.h
 *
 * Copyright (C) 2005-2007 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCU_APPLICATION_H
#define GCU_APPLICATION_H

#include "dialog-owner.h"
#include <map>
#include <set>
#include <string>
#include <gtk/gtkmain.h>
#include <gtk/gtkwindow.h>
#include <gtk/gtkrecentmanager.h>
#include <gcu/macros.h>

using namespace std;

namespace gcu {

class Document;
class Dialog;

/*!\class Application gcu/application.h
This class is a base class for applications. It provides some basic services.
*/
class Application: public DialogOwner
{
friend class Document;
friend class Dialog;
public:
/*!
@param name the name of the application.
@param datadir where data for the application are stored.
@param help_name the name to use for the help file (with .xml extension). 
If NULL, the name parameter is used.
@param icon_name the name to use for the default icon of all windows. If NULL,
the help_name or name parameters will be used.

The datadir variable is used to build the full path to the help file:
"file://"+datadir+"/gnome/help/"+name+"/"+LANG+"/"+name".xml".
*/
	Application (string name, string datadir = DATADIR, char const *help_name = NULL, char const *icon_name = NULL);
	virtual ~Application ();

/*!
@param s an optional tag in the help file.
Displays the help file using the file browser retrieved from GConf using the
"/desktop/gnome/applications/help_viewer/exec" key. If a tag is given, it will
be added to the help uri.
For an application named "myapp" and a tag "mytag", the uri will be:
helpfilename\#myapp-mytag.
*/
	void OnHelp (string s = "");
/*!
@return true if both a help browser and a help file are available, false otherwise.
*/
	bool HasHelp ();
/*!
@return the name of the application. Mostly useful to set the icon name of a new window.
*/
	string &GetName () {return Name;}

/*!
@return a GtkWindow if any. Should be overloaded by children classes.
*/
	virtual GtkWindow * GetWindow () {return NULL;}

/*!
@param filename the uri of the file.
@param mime_type the mime type of the file if known.
@param bSave true if saving, and false if loading.
@param window the current top level window.
@param pDoc an optional document.

Called by the FileChooser when a file name has been selected. This method does
nothing in the parent class and must be implemented in children classes
if they use the FileChooser.
	
@return true if no error occured.
*/
	virtual bool FileProcess (const gchar* filename, const gchar* mime_type, bool bSave, GtkWindow *window, Document *pDoc = NULL)
		{return false;}

/*!
@return the path to the current directory.
*/
	char const* GetCurDir () {return CurDir;}

/*!
@param dir the path to the new current directory.
*/
	void SetCurDir (char const* dir);

/*!
@param uri the uri to display.

Displays uri in the default web browser if any.
*/
	void ShowURI (string& uri);

/*!
@param uri the uri to the package bugs database. Default value is PACKAGE_BUGREPORT.

Opens the bugs web page in the default browser if any.
*/
	void OnBug (char *uri = PACKAGE_BUGREPORT)
		{string s (uri); ShowURI (s);}

/*!
@param uri the uri to the main web page of the program. Default value is
"http://gchemutils.nongnu.org/".
*/
	void OnWeb (char *uri = "http://gchemutils.nongnu.org/")
		{string s (uri); ShowURI (s);}

/*!
@param MailAddress the mail adress to which a message will be sent. Defaults to the
		Gnome Chemistry Utils main list.

Creates a new empty message using the default mail if any.
*/
	void OnMail (char *MailAddress = "mailto:gchemutils-main@nongnu.org");

/*!
@return true if the default mail agent is known, false otherwise.
*/
	bool HasMailAgent () {return MailAgent.length () > 0;}

/*!
@return true if the default web browser is known, false otherwise.
*/
	bool HasWebBrowser () {return WebBrowser.length () > 0;}

/*!
@return a GtkWidget for managing pixmap resolution when saving as image. This widget is
intended to be added to a GtkFileChooserDialog.
*/
	GtkWidget *GetImageResolutionWidget ();

/*!
@return a GtkWidget for managing pixmap width and height when saving as image. This
widget is intended to be added to a GtkFileChooserDialog.
*/
	GtkWidget *GetImageSizeWidget ();

/*!
@return a std::map of the supported pixbuf formats. Keys are the mime type names.
*/
	map<string, GdkPixbufFormat*> &GetSupportedPixbufFormats () {return m_SupportedPixbufFormats;}

/*!
@param filename the file name.
@param mime_type the file mime type.

A default extension is appended to filename if none is detected.

@return the GdkPixbuf name associated to mime_type or NULL if the file type
is not supported by GdkPixbuf.
*/
	char const *GetPixbufTypeName (string& filename, char const *mime_type);

protected:

/*!
This method is called by the framework when all the documents have been removed from
the set of opened documents. The default behavior is to call gtk_main_quit and exit
the program. Derived class might overide this method to change this.
*/
	virtual void NoMoreDocsEvent () {gtk_main_quit ();}

private:
	void AddDocument (Document *Doc) {m_Docs.insert (Doc);}
	void RemoveDocument (Document *Doc) {m_Docs.erase (Doc); if (m_Docs.size () == 0) NoMoreDocsEvent ();}

private:
	string Name;
	string HelpName;
	string HelpBrowser;
	string HelpFilename;
	char *CurDir;
	string WebBrowser;
	string MailAgent;

protected:
/*!
std::map of the supported pixbuf formats. Keys are the mime type names.
*/
	map<string, GdkPixbufFormat*> m_SupportedPixbufFormats;	

/*!\var m_Docs
The currently opened documents.
*/
/*!\fn GetDocs()
@return the set of currently opened documents.
*/
GCU_PROT_PROP (set <Document*>, Docs)
/*!\fn GetScreenResolution()
@return the current screen resolution.
*/
GCU_RO_PROP (unsigned, ScreenResolution)
/*!\fn SetImageResolution(unsigned ImageResolution)
@param ImageResolution the new image resolution.
Sets the image resolution used when exporting a pixmap.
Applications can use either the image resolution or the width and height to select
an exported image size, but not both.
*/
/*!\fn GetImageResolution()
@return the current image resolution used on export.
*/
/*!\fn GetRefImageResolution()
@return the current image resolution used on export as a reference.
*/
GCU_PROP (unsigned, ImageResolution)
/*!\fn SetImageWidth(unsigned Width)
@param Width the new image width.
Sets the image width used when exporting a pixmap.
Applications can use either the image resolution or the width and height to select
an exported image size, but not both.
*/
/*!\fn GetImageWidth()
@return the current image width used on export.
*/
/*!\fn GetRefImageWidth()
@return the current image width used on export as a reference.
*/
GCU_PROP (unsigned, ImageWidth)
/*!\fn SetImageHeight(unsigned Height)
@param Height the new image height.
Sets the image height used when exporting a pixmap.
Applications can use either the image resolution or the width and height to select
an exported image size, but not both.
*/
/*!\fn GetImageHeight()
@return the current image height used on export.
*/
/*!\fn GetRefImageHeight()
@return the current image height used on export as a reference.
*/
GCU_PROP (unsigned, ImageHeight)
/*!\fn GetRecentManager()
@return the GtkRecentFileManager attached to the application.
*/
GCU_RO_PROP (GtkRecentManager*, RecentManager)
};

}	// namespace gcu

#endif // GCU_APPLICATION_H
