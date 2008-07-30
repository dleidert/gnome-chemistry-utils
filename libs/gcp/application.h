// -*- C++ -*-

/* 
 * GChemPaint library
 * application.h 
 *
 * Copyright (C) 2004-2008 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_APPLICATION_H
#define GCHEMPAINT_APPLICATION_H

#include <gcu/application.h>
#include <gcu/dialog.h>
#include <gcu/object.h>
#ifdef HAVE_GO_CONF_SYNC
#include <goffice/app/go-conf.h>
#else
#include <gconf/gconf-client.h>
#endif
#include <set>
#include <string>
#include <map>
#include <list>

/*!\file*/
/*!\namespace gcp
\brief GChemPaint specific C++ classes

The namespace used for the C++ classes used by GChemPaint.
*/

namespace gcp {

typedef struct
{
	char const *name;
	unsigned char const *data_24;
} IconDesc;

class Target;
class NewFileDlg;
class Tool;
class Document;
struct option_data;
typedef void (*BuildMenuCb) (GtkUIManager *UIManager);

/*!\class Application gcp/application.h
\brief GChemPaint application base class.

This class is used to represent a GChemPaint application.
It is a virtual class since at least one method is pure virtual (gcp::Application::GetWindow)
*/
class Application: public gcu::Application
{
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
@param toolname the name of the tool.
@param activate whether to activate or deactivate.

Activates or deactivates a tool in the GChempaint tool box.

To activate the selection tool:
\code
		 ActivateTool ("Select", true);
\encode
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
\encode
			 
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

*/
	virtual GtkWindow* GetWindow () = 0;
/*!

*/
	Tool* GetActiveTool () {return m_pActiveTool;}
/*!

*/
	gcp::Document* GetActiveDocument () {return m_pActiveDoc;}
/*!

*/
	void SetActiveDocument (gcp::Document* pDoc) {m_pActiveDoc = pDoc;}
/*!

*/
	Tool* GetTool (const std::string& name) {return m_Tools[name];}
/*!

*/
	void SetTool (const std::string& toolname, Tool* tool) {m_Tools[toolname] = tool;}
/*!

*/
	GtkWidget* GetToolItem(const std::string& name) {return ToolItems[name];}
/*!

*/
	void SetToolItem (const std::string& name, GtkWidget* w) {ToolItems[name] = w;}
/*!

*/
	void SetCurZ (int Z) {m_CurZ = Z;}
/*!

*/
	int GetCurZ () {return m_CurZ;}
/*!

*/
	void OnSaveAs ();
/*!

*/
	bool FileProcess (const gchar* filename, const gchar* mime_type, bool bSave, GtkWindow *window, gcu::Document *pDoc = NULL);
/*!

*/
	void SaveWithBabel (std::string const &filename, const gchar *mime_type, gcp::Document* pDoc);
/*!

*/
	void OpenWithBabel (std::string const &filename, const gchar *mime_type, gcp::Document* pDoc);
/*!

*/
	void SaveGcp (std::string const &filename, gcp::Document* pDoc);
/*!

*/
	void OpenGcp (std::string const &filename, gcp::Document* pDoc);
/*!

*/
	xmlDocPtr GetXmlDoc () {return XmlDoc;}
/*!

*/
	void OnSaveAsImage ();
/*!

*/
	bool HaveGhemical () {return m_Have_Ghemical;}
/*!

*/
	bool HaveInChI () {return m_Have_InChI;}
/*!

*/
	int GetDocsNumber () {return m_Docs.size ();}
/*!

*/
	void Zoom (double zoom);
/*!

*/
	void AddActions (GtkRadioActionEntry const *entries, int nb, char const *ui_description, IconDesc const *icons);
/*!

*/
	void RegisterToolbar (char const *name, int index);
/*!

*/
	void OnToolChanged (GtkAction *current);
/*!

*/
	void AddTarget (Target *target);
/*!

*/
	void DeleteTarget (Target *target);
/*!

*/
	void NotifyIconification (bool iconified);
/*!

*/
	void NotifyFocus (bool has_focus, Target *target = NULL);
/*!

*/
	void CheckFocus ();
/*!

*/
	void CloseAll ();
/*!

*/
	std::list<std::string> &GetSupportedMimeTypes () {return m_SupportedMimeTypes;}
#ifdef HAVE_GO_CONF_SYNC
/*!

*/
	void OnConfigChanged (GOConfNode *node, gchar const *name);
#else
/*!

*/
	void OnConfigChanged (GConfClient *client,  guint cnxn_id, GConfEntry *entry);
#endif
/*!

*/
	std::list<std::string> &GetExtensions(std::string &mime_type);
/*!

*/
	void OnThemeNamesChanged ();
/*!

*/
	void AddMimeType (std::list<std::string> &l, std::string const& mime_type);

/*!
@param cb: the BuildMenuCb callback to call when building the menu.

adds a callback for adding entries to the windows menus.
*/
	void AddMenuCallback (BuildMenuCb cb);

/*!
@param manager: the GtkUIManager to populate.

Populates the user interface by calling all callbacks registered
with AddMenuCallback.
*/
	void BuildMenu (GtkUIManager *manager);

/*!
@param entries: the entries to register.
@param translation_domain: the entries to register.

Adds new command line options. Typically called from a plugin. The new
options are added to the main group.
*/
	void RegisterOptions (GOptionEntry const *entries, char const *translation_domain = GETTEXT_PACKAGE);

/*!
@param context: a GOptionContext

Adds all registered options to the context. This should be called once
just after creating the application and before parsing options.
*/
	void AddOptions (GOptionContext *context);

	// virtual menus actions:
/*!

*/
	virtual void OnFileNew (char const *Theme = NULL) = 0;
/*!

*/
	gcu::Document *CreateNewDocument ();

protected:
/*!

*/
	void InitTools();
/*!

*/
	void BuildTools ();
/*!

*/
	void ShowTools (bool visible);

private:
	void TestSupportedType (char const *mime_type);

protected:
/*!

*/
	gcp::Document *m_pActiveDoc;
/*!

*/
	Target *m_pActiveTarget;
/*!

*/
	unsigned m_NumWindow; //used for new files (Untitled%d)

private:
	int m_CurZ;
	std::map <std::string, GtkWidget*> ToolItems;
	std::map <std::string, GtkWidget*> Toolbars;
	std::map <std::string, Tool*> m_Tools;
	Tool* m_pActiveTool;
	static bool m_bInit;
	static bool m_Have_Ghemical;
	static bool m_Have_InChI;
	xmlDocPtr XmlDoc;
	GtkIconFactory *IconFactory;
	std::list<char const*> UiDescs;
	GtkRadioActionEntry* RadioActions;
	int m_entries;
	std::map<int, std::string> ToolbarNames;
	unsigned m_NumDoc; //used to build the name of the action associated with the menu
	std::set<Target*> m_Targets;
	int visible_windows;
	std::list<std::string> m_SupportedMimeTypes;
	std::list<std::string> m_WriteableMimeTypes;
#ifdef HAVE_GO_CONF_SYNC
	GOConfNode *m_ConfNode;
#else
	GConfClient *m_ConfClient;
#endif
	guint m_NotificationId;
	gcu::Object *m_Dummy;
	std::list<BuildMenuCb> m_MenuCbs;
	std::list<option_data> m_Options;
};

}	// namespace gcp

#endif //GCHEMPAINT_APPLICATION_H
