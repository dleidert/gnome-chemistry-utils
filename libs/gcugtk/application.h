// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gcugtk/application.h
 *
 * Copyright (C) 2005-2012 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCU_GTK_APPLICATION_H
#define GCU_GTK_APPLICATION_H

#include <gcu/application.h>

namespace gcu {
class Object;
}

/*!\file*/
/*!\namespace gcugtk
\brief The Gtk using base classes.

The namespace used for C++ base classes usin Gtk+. This namespace implements
various virtual classes defined in namespace gcu.
*/
namespace gcugtk {

class Object;
class CmdContextGtk;

/*!\struct Database gcugtk/application.h
A simple struture storing a database name and the URI used to access it.
*/
typedef struct {
/*!
The database name as it will appear in the user interface
*/
	std::string name;
/*!
The URI of the database. %I will be replaced by the molecule InChI,
%K by the InChiKey and %S by the SMILES of the target molecule.
*/
	std::string uri;
} Database;

/*!
Window states
*/
typedef enum {
/*!
Normal window.
*/
	NormalWindowState,
/*!
Maximized window.
*/
	MaximizedWindowState,
/*!
Minimized window.
*/
	MinimizedWindowState,
/*!
Full screen window.
*/
	FullScreenWindowState
} WindowState;

/*!\class Application gcugtk/application.h
This class is a base class for applications. It provides some basic services.
*/
class Application: public gcu::Application
{
friend class ApplicationPrivate;
public:
/*!
@param name the name of the application.
@param datadir where data for the application are stored.
@param help_name the name to use for the help file (with .xml extension).
If NULL, the name parameter is used.
@param icon_name the name to use for the default icon of all windows. If NULL,
the help_name or name parameters will be used.
@param cc the associated CmdContextGtk.

The datadir variable is used to build the full path to the help file:
"file://"+datadir+"/gnome/help/"+name+"/"+LANG+"/"+name".xml".
*/
	Application (std::string name, std::string datadir = DATADIR, char const *help_name = NULL, char const *icon_name = NULL, CmdContextGtk *cc = NULL);
/*!
The destructor.
*/
	virtual ~Application ();

/*!
@return the default WindowState for the application. New window should use this setting.
*/
	static WindowState GetDefaultWindowState () {return DefaultWindowState;}

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
@param clipboard a GtkClipboard
@param selection_data the current GtkSelectionData

The virtual member called by OnReceiveTargets(). The defaullt implementation
does nothing. This method should be overriden for derived classes supporting
clipboard operations.
*/
	virtual void ReceiveTargets (G_GNUC_UNUSED GtkClipboard *clipboard, G_GNUC_UNUSED GtkSelectionData *selection_data) {;}

/*!
@param clipboard a GtkClipboard
@param selection_data the current GtkSelectionData
@param App the Application target

Static callback to pass as third argument to gtk_clipboard_request_contents().
\a App must be used as fourth argument (user_data).
*/
	static void OnReceiveTargets (GtkClipboard *clipboard, GtkSelectionData *selection_data, Application *App)
		{
			App->ReceiveTargets (clipboard, selection_data);
		}

/*!
@param screen the screen wher the uri should be displayed.
@param uri the uri to display.

Displays uri in the default web browser if any.
*/
	void ShowURI (GdkScreen *screen, std::string& uri);

/*!
@param screen the screen where the uri should be displayed.
@param uri the uri to the package bugs database. Default value is PACKAGE_BUGREPORT.

	 Opens the bugs web page in the default browser if any.
*/
	void OnBug (GdkScreen *screen, char const *uri = PACKAGE_BUGREPORT)
		{std::string s (uri); ShowURI (screen, s);}

/*!
@param screen the screen where the uri should be displayed.
@param uri the uri to the main web page of the program. Default value is
"http://gchemutils.nongnu.org/".
*/
	void OnWeb (GdkScreen *screen, char const *uri = "http://gchemutils.nongnu.org/")
		{std::string s (uri); ShowURI (screen, s);}

/*!
@param screen the screen where the uri should be displayed.
@param MailAddress the mail adress to which a message will be sent. Defaults to the
		Gnome Chemistry Utils main list.

Creates a new empty message using the default mail if any.
*/
	void OnMail (GdkScreen *screen, char const *MailAddress = "mailto:gchemutils-main@nongnu.org");

/*!
@param screen the screen where the IRC window should be displayed.
Attempts to open the \#gchemutils channel at irc.gimp.net.
*/
	void OnLiveAssistance (GdkScreen *screen);

/*!
@return a dummy Application instance which might be used when there is no other
Application available.
*/
	static Application *GetDefaultApplication ();

protected:

/*!
	 This method is called by the framework when all the documents have been removed from
the set of opened documents. The default behavior is to call gtk_main_quit and exit
the program. Derived class might overide this method to change this.
*/
	virtual void NoMoreDocsEvent () {gtk_main_quit ();}

/*!
Creates a default GtkCmdContext instance for the application.
*/
	void CreateDefaultCmdContext ();

/*!
@return true if the main loop is running.
*/
	bool LoopRunning () {return (gtk_main_level ());}

private:
	static WindowState DefaultWindowState;

/*!\fn GetRecentManager()
@return the GtkRecentFileManager attached to the application.
*/
GCU_RO_PROP (GtkRecentManager*, RecentManager)
};

}	// namespace gcugtk

#endif // GCU_GTK_APPLICATION_H
