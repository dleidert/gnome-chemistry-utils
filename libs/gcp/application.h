// -*- C++ -*-

/* 
 * GChemPaint library
 * application.h 
 *
 * Copyright (C) 2004-2007 Jean Bréfort <jean.brefort@normalesup.org>
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
#include <gconf/gconf-client.h>
#include <set>
#include <string>
#include <map>
#include <list>

using namespace gcu;

namespace gcp {

typedef struct
{
	char const *name;
	unsigned char const *data_24;
} IconDesc;

class Window;
class NewFileDlg;
class Tool;
class Document;
	
class Application: public gcu::Application
{
public:
	Application ();
	virtual ~Application ();

	void ActivateTool (const string& toolname, bool activate);
	void ActivateWindowsActionWidget (const char *path, bool activate);
	virtual void ClearStatus ();
	virtual void SetStatusText (const char* text);
	virtual GtkWindow* GetWindow () = 0;
	void SetMenu (const string& menuname, GtkWidget* menu) {Menus[menuname] = menu;}
	GtkWidget* GetMenu (const string& name) {return Menus[name];}
	Tool* GetActiveTool () {return m_pActiveTool;}
	gcp::Document* GetActiveDocument () {return m_pActiveDoc;}
	void SetActiveDocument (gcp::Document* pDoc) {m_pActiveDoc = pDoc;}
	Tool* GetTool (const string& name) {return m_Tools[name];}
	void SetTool (const string& toolname, Tool* tool) {m_Tools[toolname] = tool;}
	GtkWidget* GetToolItem(const string& name) {return ToolItems[name];}
	void SetToolItem (const string& name, GtkWidget* w) {ToolItems[name] = w;}
	void SetCurZ (int Z) {m_CurZ = Z;}
	int GetCurZ () {return m_CurZ;}
	void OnSaveAs ();
	bool FileProcess (const gchar* filename, const gchar* mime_type, bool bSave, GtkWindow *window, Document *pDoc = NULL);
	void SaveWithBabel (string const &filename, const gchar *mime_type, gcp::Document* pDoc);
	void OpenWithBabel (string const &filename, const gchar *mime_type, gcp::Document* pDoc);
	void SaveGcp (string const &filename, gcp::Document* pDoc);
	void OpenGcp (string const &filename, gcp::Document* pDoc);
	xmlDocPtr GetXmlDoc () {return XmlDoc;}
	void SetCallback (const string& name, GCallback cb) {Callbacks[name] = cb;}
	GCallback GetCallback (const string& name) {return Callbacks[name];}
	void OnSaveAsImage ();
	bool HaveGhemical () {return m_Have_Ghemical;}
	bool HaveInChI () {return m_Have_InChI;}
	int GetDocsNumber () {return m_Docs.size ();}
	void Zoom (double zoom);
	void AddActions (GtkRadioActionEntry const *entries, int nb, char const *ui_description, IconDesc const *icons);
	void RegisterToolbar (char const *name, int index);
	void OnToolChanged (GtkAction *current);
	void AddWindow (Window *window);
	void DeleteWindow (Window *window);
	void NotifyIconification (bool iconified);
	void NotifyFocus (bool has_focus, Window *window = NULL);
	void CheckFocus ();
	void CloseAll ();
	list<string> &GetSupportedMimeTypes () {return m_SupportedMimeTypes;}
	void OnConfigChanged (GConfClient *client,  guint cnxn_id, GConfEntry *entry);
	list<string> &GetExtensions(string &mime_type);
	void OnThemeNamesChanged ();

	// virtual menus actions:
	virtual void OnFileNew (char const *Theme = NULL) = 0;

protected:
	void InitTools();
	void BuildTools ();
	void ShowTools (bool visible);

private:
	void TestSupportedType (char const *mime_type);

protected:
	int m_CurZ;
	gcp::Document *m_pActiveDoc;
	Window *m_pActiveWin;
	map <string, GtkWidget*> Menus;
	map <string, GtkWidget*> ToolItems;
	map <string, GtkWidget*> Toolbars;
	map <string, Tool*> m_Tools;
	map <string, GCallback> Callbacks;
	Tool* m_pActiveTool;
	static bool m_bInit, m_Have_Ghemical, m_Have_InChI;
	xmlDocPtr XmlDoc;
	unsigned m_NumWindow; //used for new files (Untitled%d)

private:
	GtkIconFactory *IconFactory;
	list<char const*> UiDescs;
	GtkRadioActionEntry* RadioActions;
	int m_entries;
	map<int, string> ToolbarNames;
	unsigned m_NumDoc; //used to build the name of the action associated with the menu
	std::set<Window*> m_Windows;
	int visible_windows;
	list<string> m_SupportedMimeTypes;
	list<string> m_WriteableMimeTypes;
	GConfClient *m_ConfClient;
	guint m_NotificationId;
	Object *m_Dummy;
};

}	// namespace gcp

#endif //GCHEMPAINT_APPLICATION_H
