/* 
 * GChemPaint library
 * window.h
 *
 * Copyright (C) 2006-2008 Jean Bréfort <jean.brefort@normalesup.org>
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


#ifndef GCP_WINDOW_H
#define GCP_WINDOW_H

#include "target.h"
#include <gtk/gtkwidget.h>
#include <gtk/gtkuimanager.h>

/*!\file*/
namespace gcp {

class Application;
class Document;

/*!\class Window gcp/window.h
The top level window class for GChemPaint. Each document is
associated with a view which might be itself associated with a window.
*/
class Window: public Target
{
public:
/*!
@param app the gcp::Application instance.
@param theme the name of the theme used by the document.
@param extra_ui a string describing the menu items to add.

Contructs the new window. Default menus do not have a File/Quit item.
\a extra_ui can be used to add it. GChemPaint uses:
\code
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu action='FileMenu'>"
"	   <placeholder name='file1'>"
"        <menuitem action='Quit'/>"
"	   </placeholder>"
"    </menu>"
"  </menubar>"
"</ui>";
\endcode
*/
	Window (gcp::Application *app, char const *theme = NULL, char const *extra_ui = NULL);
	virtual ~Window ();

/*!
Method called by the framework when the File/New File menu command is fired.
*/
	void OnFileNew ();
/*!
Method called by the framework when the File/Open menu command is fired.
*/
	void OnFileOpen ();
/*!
Method called by the framework when the File/Properties menu command is fired.
*/
	void OnProperties ();
/*!
Method called by the framework when the File/Save menu command is fired.
*/
	void SetActive (gcp::Document* pDoc, GtkWidget* w);
/*!
Method called by the framework when the Edit/Undo menu command is fired.
*/
	void OnUndo ();
/*!
Method called by the framework when the Edit/Redo menu command is fired.
*/
	void OnRedo ();
/*!
Method called by the framework when the Edit/Select All menu command is fired.
*/
	void OnSelectAll ();
/*!
Method called by the framework when the Edit/Paste menu command is fired.
*/
	void OnPasteSelection ();
/*!
Method called by the framework when the Edit/Cut menu command is fired.
*/
	void OnCutSelection ();
/*!
Method called by the framework when the Edit/Copy menu command is fired.
*/
	void OnCopySelection ();
/*!
Method called by the framework when the Edit/Delete menu command is fired.
*/
	void OnDeleteSelection ();
/*!
Method called by the framework when the Edit/Preferences menu command is fired.
*/
	void OnPreferences ();
/*!
@param zoom the new zoom level.

Sets the zoom level.
*/
	void Zoom (double zoom);
/*!
Clears the status bar message.
*/
	void ClearStatus ();
/*!
@param text the text to display in the status bar.

Sets the status bar message.
*/
	void SetStatusText (const char* text);
/*!
Raises the window and makes it active.
*/
	void Show ();
/*!
Called by the framework on a "key-press-event" event.
*/
	bool OnKeyPressed (GtkWidget* widget, GdkEventKey* ev);
/*!
Called by the framework on a "key-release-event" event.

*/
	bool OnKeyReleased (GtkWidget* widget, GdkEventKey* ev);
/*!
Checks if the document must be saved (see VerifySaved()) and safely closes the window.
*/
	bool Close ();

/*!
Method called by the framework when the File/Save menu command is fired.
*/
	virtual void OnSave ();
/*!
@return the default window title used when the document does not have a title.
*/
	virtual char const *GetDefaultTitle ();

/*!
Destroys the window.
*/
	void Destroy ();
/*!
@param title the new window title.

Sets the window title.
*/
	void SetTitle (char const *title);
/*!
@param path the path to activate.
@param activate whether to activate or deactivate.

Activates or deactivates the menu item corresponding to \a path according
to the value of \a activate. 

To deactivate the "Paste" menu item, use:
\code
ActivateActionWidget ("/MainMenu/EditMenu/Paste", false);
\endcode
*/
	void ActivateActionWidget (char const *path, bool activate); 
/*!
If the document has been modified since last saving, pops-up a dialog box to
ask the user if he wants to save the document, to drop the changes or to abort
closing.

@return true if the document can be closed, false otherwise.
*/
	bool VerifySaved ();
/*!
Method called by the framework when the File/Page setup menu command is fired.
*/
	void OnPageSetup ();

protected:
/*!
The GtkUIManager associated with the window.
*/
	GtkUIManager* m_UIManager;

private:
	GtkWidget* m_Bar;	//GtkStatusBar
	unsigned m_statusId;
	unsigned m_MessageId; //currently displayed message in the status bar
};

}	// namespace gcp

#endif	//	GCP_WINDOW_H
