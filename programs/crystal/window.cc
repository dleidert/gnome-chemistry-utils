// -*- C++ -*-

/* 
 * Gnome Crystal
 * window.cc 
 *
 * Copyright (C) 2006 Jean Bréfort <jean.brefort@normalesup.org>
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

#include "config.h"
#include "window.h"
#include "application.h"
#include "document.h"
#include "prefs.h"
#include "view-settings.h"
#include <gcu/crystalview.h>

//Callbacks
bool on_delete_event (GtkWidget* widget, GdkEvent *event, gcWindow* Win)
{
	if (Win->TryClose ()) {
		delete Win;
		return false;
	}
	return true;
}
static void on_file_new (GtkWidget *widget, gcWindow* Win)
{
	Win->GetApplication ()->OnFileNew ();
}

static void on_file_open (GtkWidget *widget, gcWindow* Win)
{
	Win->GetApplication ()->OnFileOpen ();
}

static void on_file_save (GtkWidget *widget, gcWindow* Win)
{
	Win->GetApplication ()->OnFileSave ();
}

static void on_file_save_as (GtkWidget *widget, gcWindow* Win)
{
	Win->GetApplication ()->OnFileSaveAs ();
}

static void on_file_close (GtkWidget *widget, gcWindow* Win)
{
	Win->GetApplication ()->OnFileClose ();
}

static void on_file_print (GtkWidget *widget, gcWindow* Win)
{
	Win->GetApplication ()->OnFilePrint ();
}

static void on_export_jpeg (GtkWidget *widget, gcWindow* Win)
{
	Win->GetApplication ()->OnExportJPEG ();
}

static void on_export_png (GtkWidget *widget, gcWindow* Win)
{
	Win->GetApplication ()->OnExportPNG ();
}

static void on_export_vrml (GtkWidget *widget, gcWindow* Win)
{
	Win->GetApplication ()->OnExportVRML ();
}

static void on_view_new (GtkWidget *widget, gcWindow* Win)
{
	new gcWindow (Win->GetApplication (), Win->GetDocument ());
}

static void on_view_close (GtkWidget *widget, gcWindow* Win)
{
	Win->Destroy ();
}

static bool on_quit (GtkWidget *widget, gcWindow* Win)
{
	if (!Win->GetApplication ()->OnQuit ())
		return false;
	gtk_main_quit ();
	return true;
}

static void on_prefs (GtkWidget* widget, gcApplication *app)
{
	new gcPrefsDlg (app);
}

static void on_about (GtkWidget *widget, void *data)
{
	char * authors[] = {"Jean Bréfort", NULL};
//	char * documentors[] = {NULL};
	char * artists[] = {"Nestor Diaz", NULL};
	char license[] = "This program is free software; you can redistribute it and/or\n" 
		"modify it under the terms of the GNU General Public License as\n"
 		"published by the Free Software Foundation; either version 2 of the\n"
		"License, or (at your option) any later version.\n\n"
		"This program is distributed in the hope that it will be useful,\n"
		"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
		"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
		"GNU General Public License for more details.\n\n"
		"You should have received a copy of the GNU General Public License\n"
		"along with this program; if not, write to the Free Software\n"
		"Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02111-1307\n"
		"USA";
/* Note to translators: replace the following string with the appropriate credits for you lang */
	char *translator_credits = _("translator_credits");
	gtk_show_about_dialog (NULL,
					"name", _("Gnome Crystal"),
					"authors", authors,
					"artists", artists,
					"comments", _("Gnome Crystal is a lightweight crystal structures viewer for Gnome"),
					"copyright", _("(C) 1999-2006 by Jean Bréfort"),
					"license", license,
					"translator_credits", strcmp (translator_credits, "translator_credits") != 0 ? 
											(const char *)translator_credits : NULL,
					"version", VERSION,
					"website", "http://www.nongnu.org/gchemutils",
					NULL);
}

static void on_lattice(GtkWidget *widget, gcWindow *Win)
{
	if (Win)
		Win->GetDocument ()->Define (0);
}

static void on_atoms(GtkWidget *widget, gcWindow *Win)
{
	if (Win)
		Win->GetDocument ()->Define (1);
}

static void on_lines (GtkWidget *widget, gcWindow *Win)
{
	if (Win)
		Win->GetDocument ()->Define (2);
}

static void on_size (GtkWidget *widget, gcWindow *Win)
{
	if (Win)
		Win->GetDocument ()->Define (3);
}

static void on_cleavages (GtkWidget *widget, gcWindow *Win)
{
	if (Win)
		Win->GetDocument ()->Define (4);
}

static void on_view_settings (GtkWidget *widget, gcWindow *Win)
{
	if (Win)
		new gcViewSettingsDlg (Win->GetView ());
}

static void on_help (GtkWidget *widget, gcWindow* Win)
{
	Win->GetApplication ()->OnHelp ();
}

static void on_web (GtkWidget *widget, gcWindow* Win)
{
	Win->GetApplication ()->OnWeb ();
}

static void on_mail (GtkWidget *widget, gcWindow* Win)
{
	Win->GetApplication ()->OnMail ();
}

static void on_bug (GtkWidget *widget, gcWindow* Win)
{
	Win->GetApplication ()->OnBug ();
}

static bool on_focus_in (GtkWidget *widget, GdkEventFocus *event, gcWindow* Win)
{
	gcApplication *App = Win->GetApplication ();
	Win->GetDocument ()->SetActiveView (Win->GetView ());
	App->SetActiveDocument (Win->GetDocument ());
	return false;
}

static void on_show_menu_tip (GtkWidget *proxy, gcWindow* Win)
{
	GtkAction *action = (GtkAction*) g_object_get_data (G_OBJECT (proxy), "action");
	char *tip;
	g_object_get (action, "tooltip", &tip, NULL);
	if (tip != NULL){
		Win->SetStatusText (tip);
		g_free (tip);
	}
}

static void on_clear_menu_tip (gcWindow* Win)
{
		Win->ClearStatus ();
}

static void on_connect_proxy (GtkUIManager *ui, GtkAction *action, GtkWidget *proxy, gcWindow* Win)
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

static void on_disconnect_proxy (GtkUIManager *ui, GtkAction *action, GtkWidget *proxy, gcWindow* Win)
{
	if (GTK_IS_MENU_ITEM (proxy)) {
		g_object_set_data (G_OBJECT (proxy), "action", NULL);
		g_object_disconnect (proxy,
			"any_signal::select",  G_CALLBACK (on_show_menu_tip), Win,
			"any_signal::deselect", G_CALLBACK (on_clear_menu_tip), Win,
			NULL);
	}
}

static GtkActionEntry entries[] = {
  { "FileMenu", NULL, N_("_File") },
	  { "New", GTK_STOCK_NEW, N_("_New File"), NULL,
		  N_("Create a new file"), G_CALLBACK (on_file_new) },
	  { "Open", GTK_STOCK_OPEN, N_("_Open..."), "<control>O",
		  N_("Open a file"), G_CALLBACK (on_file_open) },
	  { "Save", GTK_STOCK_SAVE, N_("_Save"), "<control>S",
		  N_("Save the current file"), G_CALLBACK (on_file_save) },
	  { "SaveAs", GTK_STOCK_SAVE_AS, N_("Save _As..."), "<shift><control>S",
		  N_("Save the current file with a different name"), G_CALLBACK (on_file_save_as) },
	  { "ExportFileMenu", NULL, N_("_Export") },
		{ "VRML", NULL, N_("VRML"), NULL,
		  N_("Export to VRML"), G_CALLBACK (on_export_vrml) },
		{ "PNG", NULL, N_("PNG"), NULL,
		  N_("Export view to png file"), G_CALLBACK (on_export_png) },
		{ "JPEG", NULL, N_("Jpeg"), NULL,
		  N_("Export view to jpeg file"), G_CALLBACK (on_export_jpeg) },
	  { "Print", GTK_STOCK_PRINT, N_("_Print..."), "<control>P",
		  N_("Print the current file"), G_CALLBACK (on_file_print) },
	  { "Close", GTK_STOCK_CLOSE, N_("_Close"), "<control>W",
		  N_("Close the current file"), G_CALLBACK (on_file_close) },
	  { "Quit", GTK_STOCK_QUIT, N_("_Quit"), "<control>Q",
		  N_("Quit Gnome Crystal"), G_CALLBACK (on_quit) },
  { "EditMenu", NULL, N_("_Edit") },
	  { "Prefs", GTK_STOCK_PREFERENCES, N_("Prefere_nces..."), NULL,
		  N_("Print the current file"), G_CALLBACK (on_prefs) },
  { "CrystalMenu", NULL, N_("_Crystal") },
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
  { "ViewMenu", NULL, N_("_View") },
	  { "ViewSettings", NULL, N_("View _settings..."), NULL,
		  N_("Choose background color and model position"), G_CALLBACK (on_view_settings) },
  { "WindowsMenu", NULL, N_("_Windows") },
/*	  { "SaveAll", GTK_STOCK_SAVE, N_("_Save All"), "<shift><control>L",
		  N_("Save all open files"), G_CALLBACK (on_save_all) },
	  { "CloseAll", GTK_STOCK_CLOSE, N_("_Close All"), "<control>W",
		  N_("Close all open files"), G_CALLBACK (on_close_all) },*/
	  { "NewView", NULL, N_("Create new _windowl"), NULL,
		  N_("Create a new window"), G_CALLBACK (on_view_new) },
	  { "CloseView", NULL, N_("_Close this window"), NULL,
		  N_("Close the current window"), G_CALLBACK (on_view_close) },
  { "HelpMenu", NULL, N_("_Help") },
	  { "Help", GTK_STOCK_HELP, N_("_Contents"), "F1",
		  N_("View help for Gnome Crystal"), G_CALLBACK (on_help) },
	  { "Web", NULL, N_("Gnome Chemistry Utils on the _web"), NULL,
		  N_("Browse the Gnome Chemistry Utils's web site"), G_CALLBACK (on_web) },
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
"      <separator name='file-sep1'/>"
"      <menu action='ExportFileMenu'>"
"        <menuitem action='VRML'/>"
"        <menuitem action='PNG'/>"
"        <menuitem action='JPEG'/>"
"      </menu>"
"      <separator name='file-sep2'/>"
"      <menuitem action='Print'/>"
"      <separator name='file-sep3'/>"
"      <menuitem action='Close'/>"
"      <menuitem action='Quit'/>"
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
"    <menu action='WindowsMenu'>"
"      <menuitem action='NewView'/>"
"      <menuitem action='CloseView'/>"
"    </menu>"
"    <menu action='HelpMenu'>"
"      <menuitem action='Help'/>"
"      <placeholder name='mail'/>"
"      <placeholder name='web'/>"
"      <placeholder name='bug'/>"
"      <menuitem action='About'/>"
"    </menu>"
"  </menubar>"
"  <toolbar name='MainToolbar'>"
"    <toolitem action='New'/>"
"    <toolitem action='Open'/>"
"    <toolitem action='Save'/>"
"    <toolitem action='Print'/>"
"    <toolitem action='Close'/>"
"    <toolitem action='Quit'/>"
"  </toolbar>"
"</ui>";

static const char *ui_mail_description =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu action='HelpMenu'>"
"      <placeholder name='mail'>"
"        <menuitem action='Mail'/>"
"      </placeholder>"
"    </menu>"
"  </menubar>"
"</ui>";

static const char *ui_web_description =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu action='HelpMenu'>"
"      <placeholder name='web'>"
"        <menuitem action='Web'/>"
"      </placeholder>"
"      <placeholder name='bug'>"
"        <menuitem action='Bug'/>"
"      </placeholder>"
"    </menu>"
"  </menubar>"
"</ui>";

gcWindow::gcWindow (gcApplication *App, gcDocument *Doc)
{
	GtkWidget *vbox;
	GtkWidget *bar;
	GtkActionGroup *action_group;
	GtkAccelGroup *accel_group;
	GError *error;

	m_App = App;
	m_Doc = Doc;
	m_Window = GTK_WINDOW (gtk_window_new (GTK_WINDOW_TOPLEVEL));
	gtk_window_set_title (m_Window, _("Gnome Crystal"));
	gtk_window_set_icon_name (m_Window, "gcrystal");
	gtk_window_set_default_size (m_Window, 300, 380);
	g_signal_connect (G_OBJECT (m_Window), "delete-event", G_CALLBACK (on_delete_event), this);
	g_signal_connect (G_OBJECT (m_Window), "focus_in_event", G_CALLBACK (on_focus_in), this);
	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (m_Window), vbox);
	action_group = gtk_action_group_new ("MenuActions");
	gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);
	gtk_action_group_add_actions (action_group, entries, G_N_ELEMENTS (entries), this);

	m_UIManager = gtk_ui_manager_new ();
	g_object_connect (m_UIManager,
		"signal::connect_proxy",    G_CALLBACK (on_connect_proxy), this,
		"signal::disconnect_proxy", G_CALLBACK (on_disconnect_proxy), this,
		NULL);
	gtk_ui_manager_insert_action_group (m_UIManager, action_group, 0);

	accel_group = gtk_ui_manager_get_accel_group (m_UIManager);
	gtk_window_add_accel_group (GTK_WINDOW (m_Window), accel_group);
	
	error = NULL;
	if (!gtk_ui_manager_add_ui_from_string (m_UIManager, ui_description, -1, &error)) {
		g_message ("building menus failed: %s", error->message);
		g_error_free (error);
		exit (EXIT_FAILURE);
	}
	if (App->HasWebBrowser () && !gtk_ui_manager_add_ui_from_string (m_UIManager, ui_web_description, -1, &error)) {
		g_message ("building menus failed: %s", error->message);
		g_error_free (error);
	}
	if (App->HasMailAgent () && !gtk_ui_manager_add_ui_from_string (m_UIManager, ui_mail_description, -1, &error)) {
		g_message ("building menus failed: %s", error->message);
		g_error_free (error);
	}

	bar = gtk_ui_manager_get_widget (m_UIManager, "/MainMenu");
	gtk_box_pack_start (GTK_BOX (vbox), bar, false, false, 0);
	bar = gtk_ui_manager_get_widget (m_UIManager, "/MainToolbar");
	gtk_box_pack_start (GTK_BOX (vbox), bar, false, false, 0);
	gtk_toolbar_set_tooltips(GTK_TOOLBAR(bar), true);
	m_View = dynamic_cast<gcView *> (m_Doc->GetView ());
	if (m_View->GetWindow () != NULL) {
		m_View = dynamic_cast<gcView *> (m_Doc->CreateNewView ());
		m_Doc->AddView (m_View);
	}
	m_View->SetWindow (this);
	gtk_box_pack_start (GTK_BOX (vbox), m_View->CreateNewWidget (), true, true, 0);
	m_Bar = gtk_statusbar_new ();
	m_statusId = gtk_statusbar_get_context_id (GTK_STATUSBAR (m_Bar), "status");
	gtk_statusbar_push (GTK_STATUSBAR (m_Bar), m_statusId, _("Ready"));
	m_MessageId = 0;
	gtk_box_pack_start (GTK_BOX (vbox), m_Bar, false, false, 0);
	
	gtk_widget_show_all (GTK_WIDGET (m_Window));
}

gcWindow::~gcWindow ()
{
}

void gcWindow::ClearStatus()
{
	if (m_MessageId) {
		gtk_statusbar_pop (GTK_STATUSBAR (m_Bar), m_statusId);
		m_MessageId = 0;
	}
}

void gcWindow::SetStatusText(const char* text)
{
	if (m_MessageId)
		gtk_statusbar_pop (GTK_STATUSBAR (m_Bar), m_statusId);
	m_MessageId = gtk_statusbar_push (GTK_STATUSBAR (m_Bar), m_statusId, text);
}

bool gcWindow::TryClose ()
{
	return m_Doc->RemoveView (m_View);
}

void gcWindow::Destroy ()
{
	if (TryClose ())
		gtk_widget_destroy (GTK_WIDGET (m_Window));
}
