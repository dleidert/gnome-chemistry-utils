// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gcu/application.h
 *
 * Copyright (C) 2005-2006 Jean Bréfort <jean.brefort@normalesup.org>
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

#include <set>
#include <string>
#include <gtk/gtkmain.h>
#include <gtk/gtkwindow.h>
#include <gtk/gtkrecentmanager.h>
#include <gcu/macros.h>

using namespace std;

namespace gcu {

class Document;

/*!\class Application gcu/application.h
This class is a base class for applications. It provides some basic services.
*/
class Application
{
public:
/*!
@param name: the name of the application.
@param datadir: where data for the application are stored.
@param help_name: the name to use for the help file (with .xml extension). 
If NULL, the name strng is used.
@param icon_name: the name to use for the default icon of all windows. If NULL,
the help_name or name parameters will be used.

The datadir variable is used to build the full path to the help file:
datadir+"/gnome/help/"+name+"/"+LANG+"/"+name".xml".
*/
	Application (string name, string datadir = DATADIR, char const *help_name = NULL, char const *icon_name = NULL);
	virtual ~Application ();

/*!
@param s: an optional tag in the help file.
Displays the help file using the file browser retrieved from GConf using the
"/desktop/gnome/applications/help_viewer/exec" key. If a tag is given, it will
be added to the help uri.
For an application named "myapp" and a tag "mytag", the uri will be:
helpfilename#myapp-mytag.
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
@param filename: the uri of the file.
@param mime-type: the mime type of the file if known.
@param bSave: true if saving, and false if loading.
@param window: the current top level window.
@param pDoc: an optional document.

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
@param dir: the path to the new current directory.
*/
	void SetCurDir (char const* dir);
	void ShowURI (string& uri);
	void OnBug (char *uri = PACKAGE_BUGREPORT)
		{string s (uri); ShowURI (s);}
	void OnWeb (char *uri = "http://gchemutils.nongnu.org/")
		{string s (uri); ShowURI (s);}
	void OnMail (char *MailAddress = "mailto:gchemutils-main@nongnu.org");
	bool HasMailAgent () {return MailAgent.length () > 0;}
	bool HasWebBrowser () {return WebBrowser.length () > 0;}
	void AddDocument (Document *Doc) {m_Docs.insert (Doc);}
	void RemoveDocument (Document *Doc) {m_Docs.erase (Doc); if (m_Docs.size () == 0) NoMoreDocsEvent ();}
	virtual void NoMoreDocsEvent () {gtk_main_quit ();}
	
private:
	string Name;
	string HelpBrowser;
	string HelpFilename;
	char *CurDir;

protected:
	string WebBrowser;
	string MailAgent;
	set <Document*> m_Docs;

GCU_RO_PROP (unsigned, ScreenResolution);
GCU_RO_PROP (GtkRecentManager*, RecentManager);
};

}	// namespace gcu

#endif // GCU_APPLICATION_H
