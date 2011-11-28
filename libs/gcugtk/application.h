// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gcugtk/application.h
 *
 * Copyright (C) 2005-2011 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCU_GTK_APPLICATION_H
#define GCU_GTK_APPLICATION_H

#include <gcu/application.h>

namespace gcu {
class Object;
}

/*!\file*/
namespace gcugtk {

class Object;
class CmdContextGtk;

typedef struct {
	std::string name;
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

	virtual void ReceiveTargets (GtkClipboard *, GtkSelectionData *) {;}
	static void OnReceiveTargets (GtkClipboard *clipboard, GtkSelectionData *selection_data, Application *App)
		{
			App->ReceiveTargets (clipboard, selection_data);
		}

protected:

/*!
This method is called by the framework when all the documents have been removed from
the set of opened documents. The default behavior is to call gtk_main_quit and exit
the program. Derived class might overide this method to change this.
*/
	virtual void NoMoreDocsEvent () {gtk_main_quit ();}

	void CreateDefaultCmdContext ();
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
