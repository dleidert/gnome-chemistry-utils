/*
 * GCrystal library
 * window.h
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
 * USA
 */

#ifndef GCR_WINDOW_H
#define GCR_WINDOW_H

#include <gcu/macros.h>
#include <gcugtk/window.h>

/*!\file*/
namespace gcu {
class Application;
}

namespace gcr {

class Application;
class Document;
class View;

/*!\class Window gcr/window.h
\brief Window class.

This class wraps the window used to display a view.
*/
class Window: public gcugtk::Window
{
friend class WindowPrivate;
public:
/*!
@param app the application.
@param doc the document.
@param extra_ui a string describing the menu items to add.

Contructs the new window. Default menus do not have a File/Quit item or a
Windows menu. \a extra_ui can be used to add them. GCrystal uses:
\code
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu action='FileMenu'>"
"	   <placeholder name='file1'>"
"        <menuitem action='Quit'/>"
"	   </placeholder>"
"    </menu>"
"	 <placeholder name='menu1'>"
"      <menu action='WindowsMenu'>"
"        <menuitem action='NewView'/>"
"        <menuitem action='CloseView'/>"
"      </menu>"
"	 </placeholder>"
"  </menubar>"
"</ui>";
\endcode
*/
	Window (gcu::Application *app, Document *doc, char const *extra_ui = NULL);
/*!
The destructor.
*/
	virtual ~Window ();

/*!
Destroys the window.
*/
	virtual void Destroy ();

/*!
Clears the status bar message.
*/
	void ClearStatus ();

/*!
@param text the text to display in the status bar.

Sets the status bar message.
*/
	void SetStatusText (const char* text);

protected:
/*!
Saves the document.
*/
	virtual void OnSave ();

private:
	GtkWidget* m_Bar;	//GtkStatusBar
	unsigned m_statusId;
	unsigned m_MessageId; //currently displayed message in the status bar

/*!\var m_Document
The gcr::Document displayed in this window.
*/
/*!\fn GetApplication()
@return the gcr::Document displayed in this window.
*/
GCU_PROT_PROP (Document*, Document)
/*!\var m_Application
The gcu::Application owning this window.
*/
/*!\fn GetApplication()
@return the gcu::Application owning this window.
*/
GCU_PROT_PROP (gcr::Application*, Application)
/*!\fn GetView()
@return the View displayed by this window.
*/
GCU_RO_PROP (View *, View)
};

}

#endif	//	GCR_WINDOW_H
