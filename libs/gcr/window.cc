/*
 * GCrystal library
 * window.cc
 *
 * Copyright (C) 2010 Jean Bréfort <jean.brefort@normalesup.org>
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

#include "config.h"
#include "application.h"
#include "docprop.h"
#include "document.h"
#include "prefs.h"
#include "view.h"
#include "view-settings.h"
#include "window.h"
#include <gcugtk/print-setup-dlg.h>
#include <gcugtk/print-setup-dlg.h>
#include <gcugtk/ui-manager.h>
#include <gcu/spacegroup.h>
#include <glib/gi18n.h>

namespace gcr {

class WindowPrivate
{
public:
	static void OnFileSave (GtkWidget *widget, Window* Win);
};

void WindowPrivate::OnFileSave (G_GNUC_UNUSED GtkWidget *widget, Window* Win)
{
	Win->OnSave ();
}

//Callbacks
static bool on_delete_event (G_GNUC_UNUSED GtkWidget* widget, G_GNUC_UNUSED GdkEvent *event, Window* Win)
{
	if (Win->GetDocument ()->RemoveView (Win->GetView ())) {
		delete Win;
		return false;
	}
	return true;
}

void on_file_new (G_GNUC_UNUSED GtkWidget *widget, Window* Win)
{
	static_cast < gcr::Application * > (Win->GetApplication ())->OnFileNew ();
}

static void on_file_open (G_GNUC_UNUSED GtkWidget *widget, Window* Win)
{
	static_cast < gcr::Application * > (Win->GetApplication ())->OnFileOpen ();
}

static void on_file_save_as (G_GNUC_UNUSED GtkWidget *widget, Window* Win)
{
	static_cast < gcr::Application * > (Win->GetApplication ())->OnFileSaveAs ();
}

static void on_properties(G_GNUC_UNUSED GtkWidget* widget, Window* Win)
{
	new DocPropDlg (Win->GetDocument ());
}

static void on_file_close (G_GNUC_UNUSED GtkWidget *widget, Window* Win)
{
	static_cast < gcr::Application * > (Win->GetApplication ())->OnFileClose ();
}

static void on_page_setup (G_GNUC_UNUSED GtkWidget *widget, Window* Win)
{
	new gcugtk::PrintSetupDlg (static_cast < gcugtk::Application * > (Win->GetApplication ()), Win->GetView ());
}

static void on_print_preview (G_GNUC_UNUSED GtkWidget *widget, Window* Win)
{
	Win->GetView ()->Print (true);
}

static void on_file_print (G_GNUC_UNUSED GtkWidget *widget, Window* Win)
{
	Win->GetView ()->Print (false);
}

static void on_file_save_as_image(G_GNUC_UNUSED GtkWidget* widget, Window* Win)
{
	static_cast < gcr::Application * > (Win->GetApplication ())->OnSaveAsImage ();
}

static void on_view_new (G_GNUC_UNUSED GtkWidget *widget, Window* Win)
{
	static_cast < gcr::Application * > (Win->GetApplication ())->CreateNewWindow (Win->GetDocument ());
}

static void on_view_close (G_GNUC_UNUSED GtkWidget *widget, Window* Win)
{
	Win->Destroy ();
}

static bool on_quit (G_GNUC_UNUSED GtkWidget *widget, Window* Win)
{
	if (!static_cast < gcr::Application * > (Win->GetApplication ())->OnQuit ())
		return false;
	gtk_main_quit ();
	return true;
}

static void on_prefs (G_GNUC_UNUSED GtkWidget* widget, Window* Win)
{
	new PrefsDlg (static_cast < gcr::Application * > (Win->GetApplication ()));
}

static void on_about (G_GNUC_UNUSED GtkWidget *widget, G_GNUC_UNUSED Window* Win)
{
	char const *authors[] = {"Jean Bréfort", NULL};
//	char * documentors[] = {NULL};
	char const *artists[] = {"Nestor Diaz", NULL};
	char license[] = "This program is free software; you can redistribute it and/or\n"
		"modify it under the terms of the GNU General Public License as\n"
 		"published by the Free Software Foundation; either version 3 of the\n"
		"License, or (at your option) any later version.\n\n"
		"This program is distributed in the hope that it will be useful,\n"
		"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
		"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
		"GNU General Public License for more details.\n\n"
		"You should have received a copy of the GNU General Public License\n"
		"along with this program; if not, write to the Free Software\n"
		"Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301\n"
		"USA";

/* Note to translators: replace the following string with the appropriate credits for you lang */
	char const *translator_credits = _("translator_credits");
	GdkPixbuf *logo = gdk_pixbuf_new_from_file (PIXMAPSDIR"/gcrystal_logo.png", NULL);
	gtk_show_about_dialog (NULL,
					"program-name", _("Gnome Crystal"),
					"authors", authors,
					"artists", artists,
					"comments", _("Gnome Crystal is a lightweight crystal structures viewer for Gnome"),
					"copyright", _("Copyright © 1999-2010 by Jean Bréfort"),
					"license", license,
					"logo", logo,
					"translator_credits", strcmp (translator_credits, "translator_credits") != 0 ?
											translator_credits : NULL,
					"version", VERSION,
					"website", "http://gchemutils.nongnu.org",
					NULL);
	g_object_unref (logo);
}

static void on_lattice(G_GNUC_UNUSED GtkWidget *widget, Window *Win)
{
	if (Win)
		Win->GetDocument ()->Define (0);
}

static void on_atoms(G_GNUC_UNUSED GtkWidget *widget, Window *Win)
{
	if (Win)
		Win->GetDocument ()->Define (1);
}

static void on_lines (G_GNUC_UNUSED GtkWidget *widget, Window *Win)
{
	if (Win)
		Win->GetDocument ()->Define (2);
}

static void on_size (G_GNUC_UNUSED GtkWidget *widget, Window *Win)
{
	if (Win)
		Win->GetDocument ()->Define (3);
}

static void on_cleavages (G_GNUC_UNUSED GtkWidget *widget, Window *Win)
{
	if (Win)
		Win->GetDocument ()->Define (4);
}

static void on_view_settings (G_GNUC_UNUSED GtkWidget *widget, Window *Win)
{
	if (Win)
		new ViewSettingsDlg (Win->GetView ());
}

static void on_help (G_GNUC_UNUSED GtkWidget *widget, Window* Win)
{
	Win->GetApplication ()->OnHelp ();
}

static void on_web (G_GNUC_UNUSED GtkWidget *widget, Window* Win)
{
	static_cast < gcr::Application * > (Win->GetApplication ())->OnWeb (Win->GetScreen ());
}

static void on_mail (G_GNUC_UNUSED GtkWidget *widget, Window* Win)
{
	static_cast < gcr::Application * > (Win->GetApplication ())->OnMail (Win->GetScreen ());
}

static void on_live_assistance (G_GNUC_UNUSED GtkWidget *widget, Window *Win)
{
	Win->GetApplication ()->OnLiveAssistance (Win->GetScreen ());
}

static void on_bug (G_GNUC_UNUSED GtkWidget *widget, Window* Win)
{
	Win->GetApplication ()->OnBug (Win->GetScreen ());
}

static bool on_focus_in (G_GNUC_UNUSED GtkWidget *widget, G_GNUC_UNUSED GdkEventFocus *event, Window* Win)
{
	Application *App = static_cast < gcr::Application * > (Win->GetApplication ());
	Win->GetDocument ()->SetActiveView (Win->GetView ());
	App->SetActiveDocument (Win->GetDocument ());
	return false;
}

static void on_show_menu_tip (GtkWidget *proxy, Window* Win)
{
	GtkAction *action = (GtkAction*) g_object_get_data (G_OBJECT (proxy), "action");
	char *tip;
	g_object_get (action, "tooltip", &tip, NULL);
	if (tip != NULL){
		Win->SetStatusText (tip);
		g_free (tip);
	}
}

static void on_clear_menu_tip (Window* Win)
{
		Win->ClearStatus ();
}

static void on_connect_proxy (G_GNUC_UNUSED GtkUIManager *ui, GtkAction *action, GtkWidget *proxy, Window* Win)
{
	/* connect whether there is a tip or not it may change later */
	if (GTK_IS_MENU_ITEM (proxy)) {
		g_object_set_data (G_OBJECT (proxy), "action", action);
		g_object_connect (proxy,
			"signal::select",  G_CALLBACK (on_show_menu_tip), Win,
			"swapped_signal::deselect", G_CALLBACK (on_clear_menu_tip), Win,
			NULL);
	}
}

static void on_disconnect_proxy (G_GNUC_UNUSED GtkUIManager *ui, G_GNUC_UNUSED GtkAction *action, GtkWidget *proxy, Window* Win)
{
	if (GTK_IS_MENU_ITEM (proxy)) {
		g_object_set_data (G_OBJECT (proxy), "action", NULL);
		g_object_disconnect (proxy,
			"any_signal::select",  G_CALLBACK (on_show_menu_tip), Win,
			"any_signal::deselect", G_CALLBACK (on_clear_menu_tip), Win,
			NULL);
	}
}

static void on_recent (GtkRecentChooser *widget, Window *Win)
{
	Application *App = static_cast < gcr::Application * > (Win->GetApplication ());
	GtkRecentInfo *info = gtk_recent_chooser_get_current_item (widget);
	App->FileProcess (gtk_recent_info_get_uri (info), gtk_recent_info_get_mime_type (info), false, NULL, Win->GetDocument ());
	gtk_recent_info_unref(info);
}

static GtkActionEntry entries[] = {
  { "FileMenu", NULL, N_("_File"), NULL, NULL, NULL },
	  { "New", GTK_STOCK_NEW, N_("_New File"), NULL,
		  N_("Create a new file"), G_CALLBACK (on_file_new) },
	  { "Open", GTK_STOCK_OPEN, N_("_Open..."), "<control>O",
		  N_("Open a file"), G_CALLBACK (on_file_open) },
	  { "Save", GTK_STOCK_SAVE, N_("_Save"), "<control>S",
		  N_("Save the current file"), G_CALLBACK (WindowPrivate::OnFileSave) },
	  { "SaveAs", GTK_STOCK_SAVE_AS, N_("Save _As..."), "<shift><control>S",
		  N_("Save the current file with a different name"), G_CALLBACK (on_file_save_as) },
	  { "SaveAsImage", GTK_STOCK_SAVE_AS, N_("Save As _Image..."), "<control>I",
		  N_("Save the current file as an image"), G_CALLBACK (on_file_save_as_image) },
	  { "PageSetup", NULL, N_("Page Set_up..."), NULL,
		  N_("Setup the page settings for your current printer"), G_CALLBACK (on_page_setup) },
	  { "PrintPreview", GTK_STOCK_PRINT_PREVIEW, N_("Print Pre_view"), NULL,
		  N_("Print preview"), G_CALLBACK (on_print_preview) },
	  { "Print", GTK_STOCK_PRINT, N_("_Print..."), "<control>P",
		  N_("Print the current file"), G_CALLBACK (on_file_print) },
	  { "Properties", GTK_STOCK_PROPERTIES, N_("Prope_rties..."), NULL,
		  N_("Modify the file's properties"), G_CALLBACK (on_properties) },
	  { "Close", GTK_STOCK_CLOSE, N_("_Close"), "<control>W",
		  N_("Close the current file"), G_CALLBACK (on_file_close) },
	  { "Quit", GTK_STOCK_QUIT, N_("_Quit"), "<control>Q",
		  N_("Quit Gnome Crystal"), G_CALLBACK (on_quit) },
  { "EditMenu", NULL, N_("_Edit"), NULL, NULL, NULL },
	  { "Prefs", GTK_STOCK_PREFERENCES, N_("Prefere_nces..."), NULL,
		  N_("Configure the application"), G_CALLBACK (on_prefs) },
  { "CrystalMenu", NULL, N_("_Crystal"), NULL, NULL, NULL },
	{ "Lattice", NULL, N_("_Lattice..."), NULL,
	  N_("Define the lattice"), G_CALLBACK (on_lattice) },
	{ "Atoms", NULL, N_("_Atoms..."), NULL,
	  N_("Add or edit atoms"), G_CALLBACK (on_atoms) },
	{ "Lines", NULL, N_("_Bonds and lines..."), NULL,
	  N_("Add or edit bonds and lines"), G_CALLBACK (on_lines) },
	{ "Size", NULL, N_("_Size..."), NULL,
	  N_("Define size"), G_CALLBACK (on_size) },
	{ "Cleavages", NULL, N_("_Cleavages..."), NULL,
	  N_("Add or edit cleavages to remove some planes"), G_CALLBACK (on_cleavages) },
  { "ViewMenu", NULL, N_("_View"), NULL, NULL, NULL },
	  { "ViewSettings", NULL, N_("View _settings..."), NULL,
		  N_("Choose background color and model position"), G_CALLBACK (on_view_settings) },
  { "WindowsMenu", NULL, N_("_Windows"), NULL, NULL, NULL },
	  { "NewView", NULL, N_("Create new _window"), NULL,
		  N_("Create a new window"), G_CALLBACK (on_view_new) },
	  { "CloseView", NULL, N_("_Close this window"), NULL,
		  N_("Close the current window"), G_CALLBACK (on_view_close) },
  { "HelpMenu", NULL, N_("_Help"), NULL, NULL, NULL },
	  { "Help", GTK_STOCK_HELP, N_("_Contents"), "F1",
		  N_("View help for Gnome Crystal"), G_CALLBACK (on_help) },
	  { "Web", NULL, N_("Gnome Chemistry Utils on the _web"), NULL,
		  N_("Browse the Gnome Chemistry Utils's web site"), G_CALLBACK (on_web) },
	  { "LiveAssistance", NULL, N_("Live assistance"), NULL,
		  N_("Open the Gnome Chemistry Utils IRC channel"), G_CALLBACK (on_live_assistance) },
	  { "Mail", NULL, N_("_Ask a question"), NULL,
		  N_("Ask a question about the Gnome Chemistry Utils"), G_CALLBACK (on_mail) },
	  { "Bug", NULL, N_("Report _Bugs"), NULL,
		  N_("Submit a bug report for the Gnome Chemistry Utils"), G_CALLBACK (on_bug) },
	  { "About", NULL, N_("_About"), NULL,
		  N_("About Gnome Crystal"), G_CALLBACK (on_about) }
};

static const char *ui_description =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu action='FileMenu'>"
"      <menuitem action='New'/>"
"      <menuitem action='Open'/>"
"      <menuitem action='Save'/>"
"      <menuitem action='SaveAs'/>"
"      <menuitem action='SaveAsImage'/>"
"	   <separator name='file-sep1'/>"
"      <menuitem action='PageSetup'/>"
"      <menuitem action='PrintPreview'/>"
"      <menuitem action='Print'/>"
"      <separator name='file-sep2'/>"
"      <menuitem action='Properties'/>"
"      <separator name='file-sep3'/>"
"      <menuitem action='Close'/>"
"	   <placeholder name='file1'/>"
"    </menu>"
"    <menu action='EditMenu'>"
"      <menuitem action='Prefs'/>"
"    </menu>"
"    <menu action='CrystalMenu'>"
"      <menuitem action='Lattice'/>"
"      <menuitem action='Atoms'/>"
"      <menuitem action='Lines'/>"
"      <menuitem action='Size'/>"
"      <menuitem action='Cleavages'/>"
"    </menu>"
"    <menu action='ViewMenu'>"
"      <menuitem action='ViewSettings'/>"
"    </menu>"
"	 <placeholder name='menu1'/>"
"    <menu action='HelpMenu'>"
"      <menuitem action='Help'/>"
"      <menuitem action='Mail'/>"
"      <menuitem action='Web'/>"
"      <menuitem action='LiveAssistance'/>"
"      <menuitem action='Bug'/>"
"      <menuitem action='About'/>"
"    </menu>"
"  </menubar>"
"  <toolbar name='MainToolbar'>"
"    <toolitem action='New'/>"
"    <toolitem action='Open'/>"
"    <toolitem action='Save'/>"
"    <toolitem action='Print'/>"
"  </toolbar>"
"</ui>";

Window::Window (gcu::Application *app, Document *doc, char const *extra_ui):
	gcugtk::Window ()
{
	GtkWidget *grid;
	GtkWidget *bar;
	GtkActionGroup *action_group;
	GtkAccelGroup *accel_group;
	GError *error;

	m_Application = static_cast < gcr::Application * > (app);
	m_Document = doc? doc: new Document (m_Application);
	m_Window = GTK_WINDOW (gtk_window_new (GTK_WINDOW_TOPLEVEL));
	gtk_window_set_title (m_Window, _("Gnome Crystal"));
	gtk_window_set_icon_name (m_Window, "gcrystal");
	gtk_window_set_default_size (m_Window, 300, 380);
	g_signal_connect (G_OBJECT (m_Window), "delete-event", G_CALLBACK (on_delete_event), this);
	g_signal_connect (G_OBJECT (m_Window), "focus_in_event", G_CALLBACK (on_focus_in), this);
	grid = gtk_grid_new ();
	g_object_set (G_OBJECT (grid), "orientation", GTK_ORIENTATION_VERTICAL, NULL);
	gtk_container_add (GTK_CONTAINER (m_Window), grid);
	action_group = gtk_action_group_new ("MenuActions");
	gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);
	gtk_action_group_add_actions (action_group, entries, G_N_ELEMENTS (entries), this);

	m_UIManager = new gcugtk::UIManager (gtk_ui_manager_new ());
	GtkUIManager *manager = static_cast < gcugtk::UIManager * > (m_UIManager)->GetUIManager ();
	g_object_connect (manager,
		"signal::connect_proxy",    G_CALLBACK (on_connect_proxy), this,
		"signal::disconnect_proxy", G_CALLBACK (on_disconnect_proxy), this,
		NULL);
	gtk_ui_manager_insert_action_group (manager, action_group, 0);

	accel_group = gtk_ui_manager_get_accel_group (manager);
	gtk_window_add_accel_group (GTK_WINDOW (m_Window), accel_group);

	error = NULL;
	if (!gtk_ui_manager_add_ui_from_string (manager, ui_description, -1, &error)) {
		g_message ("building menus failed: %s", error->message);
		g_error_free (error);
	}
	if (extra_ui && !gtk_ui_manager_add_ui_from_string (manager, extra_ui, -1, &error)) {
		g_message ("building menus failed: %s", error->message);
		g_error_free (error);
	}

	GtkWidget *menu = gtk_ui_manager_get_widget (manager, "/MainMenu/FileMenu/Open");
	GtkWidget *w = gtk_recent_chooser_menu_new_for_manager (m_Application->GetRecentManager ());
	gtk_recent_chooser_set_sort_type (GTK_RECENT_CHOOSER (w), GTK_RECENT_SORT_MRU);
	GtkRecentFilter *filter = gtk_recent_filter_new ();
	std::list<std::string>::iterator it;
	char const * mime = m_Application->GetFirstSupportedMimeType (it);
	while (mime) {
		gtk_recent_filter_add_mime_type (filter, mime);
		mime = m_Application->GetNextSupportedMimeType (it);
	}
	gtk_recent_chooser_add_filter (GTK_RECENT_CHOOSER (w), filter);
	g_signal_connect (G_OBJECT (w), "item-activated", G_CALLBACK (on_recent), this);
	GtkWidget *item = gtk_menu_item_new_with_label (_("Open recent"));
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), w);
	gtk_widget_show_all (item);
	gtk_menu_shell_insert (GTK_MENU_SHELL (gtk_widget_get_parent (menu)), item, 3);

	bar = gtk_ui_manager_get_widget (manager, "/MainMenu");
	gtk_container_add (GTK_CONTAINER (grid), bar);
	bar = gtk_ui_manager_get_widget (manager, "/MainToolbar");
	gtk_container_add (GTK_CONTAINER (grid), bar);
	m_View = static_cast < View * > (m_Document->GetView ());
	if (m_View->GetWindow () != NULL) {
		m_View = static_cast < View  *> (m_Document->CreateNewView ());
		m_View->SetWindow (this);
		m_Document->AddView (m_View);
	} else
		m_View->SetWindow (this);
	g_object_set (G_OBJECT (m_View->GetWidget ()), "margin-left", 6, "margin-right", 6, "expand", true, NULL);
	gtk_container_add (GTK_CONTAINER (grid), m_View->GetWidget ());
	m_Bar = gtk_statusbar_new ();
	m_statusId = gtk_statusbar_get_context_id (GTK_STATUSBAR (m_Bar), "status");
	gtk_statusbar_push (GTK_STATUSBAR (m_Bar), m_statusId, _("Ready"));
	m_MessageId = 0;
	ClearStatus ();
	gtk_container_add (GTK_CONTAINER (grid), m_Bar);

	gtk_widget_show_all (GTK_WIDGET (m_Window));
}

Window::~Window ()
{
}

void Window::Destroy ()
{
}

void Window::ClearStatus()
{
	if (m_MessageId)
		gtk_statusbar_pop (GTK_STATUSBAR (m_Bar), m_statusId);
	if (m_Document->GetSpaceGroup ()) {
		char *text = g_strdup_printf(_("Space group: %u"),m_Document->GetSpaceGroup ()->GetId ());
		m_MessageId = gtk_statusbar_push (GTK_STATUSBAR (m_Bar), m_statusId, text);
		g_free (text);
	} else
		m_MessageId = 0;
}

void Window::SetStatusText(const char* text)
{
	if (m_MessageId)
		gtk_statusbar_pop (GTK_STATUSBAR (m_Bar), m_statusId);
	m_MessageId = gtk_statusbar_push (GTK_STATUSBAR (m_Bar), m_statusId, text);
}

void Window::OnSave ()
{
	static_cast < gcr::Application * > (GetApplication ())->OnFileSave ();
}

}
