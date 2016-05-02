// -*- C++ -*-

/*
 * GChemPaint library
 * application.h
 *
 * Copyright (C) 2004-2012 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_APPLICATION_H
#define GCHEMPAINT_APPLICATION_H

#include <gcugtk/application.h>
#include <gcu/macros.h>
#include <libxml/parser.h>
#include <set>
#include <string>
#include <map>
#include <stdexcept>

namespace gccv {
	class Canvas;
}

namespace gcu {
	class Dialog;
	class Object;
}

/*!\file*/
/*!\namespace gcp
\brief GChemPaint specific C++ classes

The namespace used for the C++ classes used by GChemPaint.
*/

namespace gcp {

/*!\struct ToolDesc
Structure to use as button descriptors for tools.
See gcp::Application::AddTools() for information about its use.
*/
typedef struct
{
/*!
The tool name.
*/
	char const *name;
/*!
The tool tip.
*/
	char const *tip;
/*!
The tool bar number
*/
	unsigned bar;
/*!
The tool group inside the toolbar. This field helps merging toolbars with tools
from different plugins.
*/
	unsigned group;
/*!
The name of the icon to add to the button.
*/
	char const *icon_name;
/*!
The widget to add to the tool button, if NULL the icon with %icon_name will
be used. If both are NULL, the tool button will not be added.
*/
	GtkWidget *widget;
} ToolDesc;

// standard toolbars
enum {
	SelectionToolbar,
	AtomToolbar,
	BondToolbar,
	RingToolbar,
	ArrowToolbar,
	MaxToolbar
};

class Target;
class NewFileDlg;
class Tool;
class Document;
typedef void (*BuildMenuCb) (gcu::UIManager *UIManager);

/*!
@brief Cursors.

Enumerates known cursors.
*/
typedef enum {
/*!Cursor used when a click would have no effect.*/
	CursorUnallowed,
/*!Cursor used when a click would start drawing operation.*/
	CursorPencil,
/*!Maximum value, does not correspond to a valid cursor.*/
	CursorMax
} CursorId;

/*!\class Application gcp/application.h
\brief GChemPaint application base class.

This class is used to represent a GChemPaint application.
It is a virtual class since at least one method is pure virtual (gcp::Application::GetWindow)
*/
class Application: public gcugtk::Application
{
friend class ApplicationPrivate;
public:
/*!
The default constructor.
*/
	Application (gcugtk::CmdContextGtk *cc = NULL);
/*!
The destructor.
*/
	virtual ~Application ();

/*!
@param toolname the name of the tool.
@param activate whether to activate or deactivate.

Activates or deactivates a tool in the GChempaint tool box.

To activate the selection tool:
\code
		 ActivateTool ("Select", true);
\endcode
*/
	void ActivateTool (const std::string& toolname, bool activate);

/*!
@param path the path to activate.
@param activate whether to activate or deactivate.

Activates or deactivates the menu item corresponding to \a path according
to the value of \a activate.

To deactivate the "Paste" menu item, use:
\code
		 ActivateWindowsActionWidget ("/MainMenu/EditMenu/Paste", false);
\endcode

Calls gcp::Window::ActivateActionWidget.
*/
	void ActivateWindowsActionWidget (const char *path, bool activate);
/*!
Clears the message in the status bar.
*/
	virtual void ClearStatus ();
/*!
@param text a text to display

Displays \a text in the status bar.
*/
	virtual void SetStatusText (const char* text);
/*!
This pure virtual method must be overloaded by derived classes.
@return the current active top level window if any, or NULL.
*/
	virtual GtkWindow* GetWindow () = 0;
/*!
@return the active tool.
*/
	Tool* GetActiveTool () {return m_pActiveTool;}
/*!
@return the active document.
*/
	gcp::Document* GetActiveDocument () {return m_pActiveDoc;}
/*!
@param pDoc the document becoming active.

Sets the new active document.
*/
	void SetActiveDocument (gcp::Document* pDoc) {m_pActiveDoc = pDoc;}
/*!
@param name the name of a tool
@return the Tool corresponding to \a name.
*/
	Tool* GetTool (const std::string& name) {return m_Tools[name];}
/*!
@param toolname the name of a new tool.
@param tool the new Tool.

Adds a new tool to the tools box. This method is called from the Tool
constructor.
*/
	void SetTool (const std::string& toolname, Tool* tool) {m_Tools[toolname] = tool;}
/*!
@param name the name of a tool
@return the GtkWidget corresponding to the Tool named \a name.
*/
	GtkWidget* GetToolItem(const std::string& name) {return ToolItems[name];}
/*!
@param name the name of a new tool.
@param w a GtkWidget.

Associates \a w to the Tool named \a name. SetTool() will return this widget
when its argument is \a name.
*/
	void SetToolItem (const std::string& name, GtkWidget* w) {ToolItems[name] = w;}
/*!
@param Z the new current atomic number.

Sets the new current atomic number. This number is used for new atoms.
*/
	void SetCurZ (int Z) {m_CurZ = Z;}
/*!
@return the current atomic number.
*/
	int GetCurZ () {return m_CurZ;}
/*!
Open the file save as dialog to save the current document with a new name.
*/
	void OnSaveAs ();
/*!
@param filename the URI of the file to save or open.
@param mime_type the mime type.
@param bSave true when saving and false when opening.
@param window a parent GtkWindow which is used for messabe boxes if any.
@param pDoc a document (might be NULL when loading.

Callback called when the user clicks on the Save or Open button in the file
chooser to process the file.
@return false on success, true otherwise.
*/
	bool FileProcess (char const *filename, char const *mime_type, bool bSave, GtkWindow *window, gcu::Document *pDoc = NULL);
/*!
@param filename the URI of the file to save.
@param pDoc the document to save.

Saves the active document in the native GChemPaint format.
*/
	void SaveGcp (std::string const &filename, gcp::Document* pDoc);
/*!
@param filename the URI of the file to load.
@param pDoc a document or NULL.

Loads a GChemPaint document.
*/
	void OpenGcp (std::string const &filename, gcp::Document* pDoc);
/*!
@return a xmlDocPtr used for some undo/redo related operations. The text
tools use it.
*/
	xmlDocPtr GetXmlDoc () {return XmlDoc;}
/*!
Saves the active view as an image.
*/
	void OnSaveAsImage ();
/*!
@return true if InChIs can be evaluated for molecules.
*/
	bool HaveInChI () {return m_Have_InChI;}
/*!
@return the number of opened documents.
*/
	int GetDocsNumber () {return m_Docs.size ();}
/*!
@param zoom the new zoom level.

Sets the zoom level for the active document window.
*/
	void Zoom (double zoom);
/*!
@param tools an array with the new tools descriptions, last one having its name set to #NULL.

Adds new tools, typically from a plugin.
*/
	void AddTools (ToolDesc const *tools);
/*!
@param name the name of the toolbar.
@param index the rank of the toolbar in the toolbox.

Adds a new toolbar to the tools box. See the documentation of
gcp::Application::AddActions() for a case use.
*/
	void RegisterToolbar (char const *name, int index);
/*!
@param new_tool_name the activated tool name.

Call by the framework when the active tool changed.
*/
	void OnToolChanged (char const *new_tool_name);
/*!
@param target the Target to add.

Adds a Target to the list of known Targets and displays the tools box next to
the Target.
*/
	void AddTarget (Target *target);
/*!
@param target the Target to delete.

Deletes a Target from the list of known Targets. The tools box will be hidden
if no Target remains active.
*/
	void DeleteTarget (Target *target);
/*!
@param iconified whether the currently active Target is iconified or not.

If \a iconified is true, the tools box will be hidden if no Target remains
active, otherwise it will be displayed next to the active Target.
*/
	void NotifyIconification (bool iconified);
/*!
@param has_focus whether the Target has focus or not.
@param target the Target for which the event occured.

Shows the tools box next to \a target if \a has_focus is true and if \a target
is not NULL.
*/
	void NotifyFocus (bool has_focus, Target *target = NULL);
/*!
Closes all open documents and ends the application.
*/
	void CloseAll ();
/*!
@return a list of supported mime types.
*/
	std::list<std::string> &GetSupportedMimeTypes () {return m_SupportedMimeTypes;}
/*!
@param node the GONode which changed.
@param name the name of the key.

Called by the framework when the configuration entry has changed to update a
running application preferences if the system allows such callbacks.
*/
	void OnConfigChanged (GOConfNode *node, char const *name);
/*!
@param mime_type a mime type.
@return the list of file name extensions corresponding to the mime type.
*/
	std::list<std::string> &GetExtensions(std::string &mime_type);

/*!
Called by the framework after a change of a theme name. Ensure evrything is
correctly updated.
*/
	void OnThemeNamesChanged ();

/*!
@param cb: the BuildMenuCb callback to call when building the menu.

adds a callback for adding entries to the windows menus.
*/
	void AddMenuCallback (BuildMenuCb cb);

/*!
@param manager: the gcu::UIManager to populate.

Populates the user interface by calling all callbacks registered
with AddMenuCallback.
*/
	void BuildMenu (gcu::UIManager *manager);

/*!
Creates a new document using the default theme.
@return the newly created document.
*/
	gcu::Document *CreateNewDocument ();

	// virtual menus actions:
/*!
@param Theme a gcp::Theme or NULL.

Creates a new document using the given theme or the default theme if
\a Theme is NULL. This method must be overloaded by derived classes since
it is pure virtual.
*/
	virtual void OnFileNew (char const *Theme = NULL) = 0;

/*!
@param id a CursorId

@return the corresponding GdkCursor if any.
*/
	GdkCursor *GetCursor (CursorId id)  {return m_Cursors[id];}

/*!
@return true if an appropriate 3D viewer is available.
*/
	bool Have3DSupport () {return m_HaveGhemical | m_HaveGChem3D | m_HaveAvogadro;}
/*!
@param clipboard a clipboard.
@param selection_data the available data

Used as callback as gtk_clipboard_request_contents().
*/
	void ReceiveTargets (GtkClipboard *clipboard, GtkSelectionData *selection_data);
/*!
@return the list of the registered tools descriptions.
*/
	std::list < ToolDesc const * > const &GetToolDescriptions () const {return m_ToolDescriptions;}

protected:
/*!
Initialize the tools box so that the selection tool is active. This method is
called only once aafter startup by the framework.
*/
	void InitTools();
/*!
Builds the tools box. This method is
called only once after startup by the framework.
*/
	void BuildTools () throw (std::runtime_error);
/*!
@param visible whether the tools box should be visible or not

Shows or hides the tools box.
*/
	void ShowTools (bool visible);

private:
	void TestSupportedType (char const *mime_type, char const* babel_type = NULL, bool writeable = false);
	void AddMimeType (std::list<std::string> &l, std::string const& mime_type);
	void UpdateAllTargets ();

protected:
/*!
The active document.
*/
	gcp::Document *m_pActiveDoc;
/*!
The active target.
*/
	Target *m_pActiveTarget;
/*!
Used to add a number to new files default names.
*/
	unsigned m_NumWindow; //used for new files (Untitled%d)

private:
	int m_CurZ;
	std::map <std::string, GtkWidget*> ToolItems;
	std::map <std::string, GtkWidget*> Toolbars;
	std::map <std::string, Tool*> m_Tools;
	Tool* m_pActiveTool;
	static bool m_bInit;
	static bool m_Have_InChI;
	xmlDocPtr XmlDoc;
	std::map<int, std::string> ToolbarNames;
	unsigned m_NumDoc; //used to build the name of the action associated with the menu
	std::set<Target*> m_Targets;
	int visible_windows;
	std::list<std::string> m_SupportedMimeTypes;
	std::list<std::string> m_WriteableMimeTypes;
	GOConfNode *m_ConfNode;
	unsigned m_NotificationId;
	gcu::Object *m_Dummy;
	std::list<BuildMenuCb> m_MenuCbs;
	GdkCursor *m_Cursors[CursorMax];
	std::list < ToolDesc const * > m_ToolDescriptions;

/*!\fn GetHaveGhemical
@return true if ghemical is usable on startup.
*/
	GCU_RO_STATIC_PROP (bool, HaveGhemical)
/*!\fn GetHaveGChem3D
@return true if gchem3d is usable on startup.
*/
	GCU_RO_STATIC_PROP (bool, HaveGChem3D)
/*!\fn GetHaveAvogadro
@return true if avogadro is usable on startup.
*/
	GCU_RO_STATIC_PROP (bool, HaveAvogadro)
/*!\fn GetUseAtomColors
@return true if atomic symbols are displayed using the element symbolic color.
*/
	GCU_RO_PROP (bool, UseAtomColors)
};

}	// namespace gcp

#endif //GCHEMPAINT_APPLICATION_H
