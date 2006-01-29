// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gcu/application.h
 *
 * Copyright (C) 2005-2006
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef GCU_APPLICATION_H
#define GCU_APPLICATION_H

#include <string>

using namespace std;

namespace gcu {

/*!\class Application gcu/application.h
This class is a base class for applications. It provides some basic services.
*/
class Application
{
public:
/*!
@param name: the name to use for the default icon of all windows and the
help file (with .xml extension).
@param datadir: where data for the application are stored.

The datadir variable is used to build the full path to the help file:
datadir+"/gnome/help/"+name+"/"+LANG+"/"+name".xml".
*/
	Application (string name, string datadir = DATADIR);
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

private:
	string Name;
	string HelpBrowser;
	string HelpFilename;
};

}	// namespace gcu

#endif // GCU_APPLICATION_H
