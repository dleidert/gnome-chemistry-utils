// -*- C++ -*-

/* 
 * Gnome Crystal
 * application.cc 
 *
 * Copyright (C) 2001-2006 Jean Bréfort <jean.brefort@normalesup.org>
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
#include <gtk/gtk.h>
#include <glade/glade.h>
#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-job.h>
#include <libgnomeprintui/gnome-print-dialog.h>
#include <libgnomeprintui/gnome-print-job-preview.h>
#include "application.h"
#include "globals.h"
#include <gcu/filechooser.h>
#include "prefs.h"
#include "view-settings.h"
std::list<gcApplication*> Apps;

static unsigned short nNewDocs = 1;
guint TabPos =  0;

//Callbacks
static void on_file_new (GtkWidget *widget, gcApplication *app)
{
	app->OnFileNew ();
}

static void on_file_open (GtkWidget *widget, gcApplication *app)
{
	app->OnFileOpen ();
}

static void on_file_open_new_win (GtkWidget *widget, gcApplication *app)
{
	gcApplication* pApp = new gcApplication ();
	Apps.push_back (pApp);
	pApp->OnFileOpen ();
}

static void on_file_save (GtkWidget *widget, gcApplication *app)
{
	app->OnFileSave ();
}

static void on_file_save_as (GtkWidget *widget, gcApplication *app)
{
	app->OnFileSaveAs ();
}

static void on_file_close (GtkWidget *widget, gcApplication *app)
{
	app->OnFileClose ();
}

static void on_file_print (GtkWidget *widget, gcApplication *app)
{
	app->OnFilePrint ();
}

static void on_export_jpeg (GtkWidget *widget, gcApplication *app)
{
	app->OnExportJPEG ();
}

static void on_export_png (GtkWidget *widget, gcApplication *app)
{
	app->OnExportPNG ();
}

static void on_export_vrml (GtkWidget *widget, gcApplication *app)
{
	app->OnExportVRML ();
}

static void on_view_new (GtkWidget *widget, gcApplication *app)
{
	app->OnViewNew ();
}

static void on_view_close (GtkWidget *widget, gcApplication *app)
{
	app->OnViewClose ();
}

static void on_help (GtkWidget *widget, gcApplication *App)
{
	App->OnHelp ();
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

static void on_lattice(GtkWidget *widget, gcApplication *app)
{
	if (app)
		app->OnCrystalDefine (0);
}

static void on_atoms(GtkWidget *widget, gcApplication *app)
{
	if (app)
		app->OnCrystalDefine (1);
}

static void on_lines (GtkWidget *widget, gcApplication *app)
{
	if (app)
		app->OnCrystalDefine (2);
}

static void on_size (GtkWidget *widget, gcApplication *app)
{
	if (app)
		app->OnCrystalDefine (3);
}

static void on_cleavages (GtkWidget *widget, gcApplication *app)
{
	if (app)
		app->OnCrystalDefine (4);
}

static void on_view_settings (GtkWidget *widget, gcApplication *app)
{
	if (app)
		app->OnViewSettings ();
}

extern std::list<gcDocument*> Docs;

static bool on_quit (GtkWidget *widget, gcApplication *app)
{
	while (Apps.size()) {
		if (Apps.front ()->OnFileClose ())
			while (gtk_events_pending ())
				gtk_main_iteration ();
		else
			break;
	}
	if (Docs.size ())
		return false;
	delete Apps.front ();
	Apps.pop_front ();
	if  (Apps.size() != 0)
		return false;
	gtk_main_quit ();
	return true;
}

static void on_select (GtkWidget* widget, gcApplication *app)
{
	GtkWidget* w = GTK_WIDGET(g_object_get_data (G_OBJECT (widget), "page"));
	app->OnSelectView (w);
}

static bool on_delete (GtkWidget* widget, GdkEventAny *event, gcApplication *app)
{
	while (app->OnViewClose ())
		while (gtk_events_pending ())
			gtk_main_iteration ();
	std::list<gcApplication*>::iterator i;
	for (i = Apps.begin (); i != Apps.end (); i++)
	{
		if (app == *i) {
			if (!app->IsEmpty ())
				return true;
			Apps.remove (app);
			delete app;
			return false;
		}
	}
	return false;
}

static void on_destroy (GtkWidget* widget, gcApplication *app)
{
	if (Apps.empty ())
		gtk_main_quit ();
}

static void on_prefs (GtkWidget* widget, gcApplication *app)
{
	new gcPrefsDlg (app);
}

//Helper functions
static bool do_load (const gchar* filename, gcView* pView)
{
	gcApplication* pApp = pView->GetApp ();
	pView = pApp->GetDocView (filename);
	gcDocument* pDoc = pView->GetDocument ();
	pApp = pView->GetApp ();
	if (pDoc->GetFileName () && !strcmp (pDoc->GetFileName(), filename)) {
		pApp->SelectView (pView);
		if (!pDoc->IsDirty ())
			return true;
		else {
			gchar* str = g_strdup_printf (_("\"%s\" has been modified since last saving. Do you wish to come back to saved version?"), pDoc->GetTitle ());
			GtkWidget* mbox = gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, str);
			int res = gtk_dialog_run (GTK_DIALOG (mbox));
			if (res != GTK_RESPONSE_YES)
				return true;
		}
	}
	if (pDoc->Load (filename)) {
		gtk_label_set_text (pView->GetLabel (), pDoc->GetTitle ());
		GtkLabel *pLabel = pView->GetMenuLabel ();
		if (pLabel)
			gtk_label_set_text (pLabel, pDoc->GetTitle ());
		return true;
	}
	nNewDocs++;
	return false;
}

void do_save_as (const gchar* filename, gcView* pView)
{
	gcDocument *pDoc = pView->GetDocument ();
	pDoc->SetFileName (filename);
	pDoc->Save ();
}

void do_export_jpeg (const gchar *filename, gcView* pView)
{
	if (!pView)
		return;
	pView->ExportJPG (filename);
}

void do_export_png(const gchar *filename, gcView* pView)
{
	if (!pView)
		return;
	pView->ExportPNG (filename);
}

void do_export_vrml (const gchar *filename, gcView* pView)
{
	gcDocument *pDoc = pView->GetDocument ();
	if (pDoc)
		pDoc->OnExportVRML (filename, pView);;
}

bool RequestApp (gcView* pView)
{
	gcDocument* pDoc = pView->GetDocument ();
	gcApplication* pApp = NULL;
	std::list<gcApplication*>::iterator i;
	for (i = Apps.begin(); i!= Apps.end(); i++) {
		if (!(*i)->HasDocument (pDoc)) {
			pApp = *i;
			break;
		}
	}
	if (!pApp) {
		pApp = new gcApplication ();
		Apps.push_back (pApp);
	}
	if (pApp) {
		pApp->AddView (pView);
		gtk_label_set_text (pView->GetLabel (), pDoc->GetTitle ());
		GtkLabel *pLabel = pView->GetMenuLabel ();
		if (pLabel)
			gtk_label_set_text (pLabel, pDoc->GetTitle ());
	}
	return (pApp != NULL);
}

static void on_show_menu_tip (GtkWidget *proxy, gcApplication* App)
{
	GtkAction *action = (GtkAction*) g_object_get_data (G_OBJECT (proxy), "action");
	char *tip;
	g_object_get (action, "tooltip", &tip, NULL);
	if (tip != NULL){
		App->SetStatusText (tip);
		g_free (tip);
	}
}

static void on_clear_menu_tip (gcApplication* App)
{
		App->ClearStatus ();
}

static void on_connect_proxy (GtkUIManager *ui, GtkAction *action, GtkWidget *proxy, gcApplication* App)
{
	/* connect whether there is a tip or not it may change later */
	if (GTK_IS_MENU_ITEM (proxy)) {
		g_object_set_data (G_OBJECT (proxy), "action", action);
		g_object_connect (proxy,
			"signal::select",  G_CALLBACK (on_show_menu_tip), App,
			"swapped_signal::deselect", G_CALLBACK (on_clear_menu_tip), App,
			NULL);
	}
}

static void on_disconnect_proxy (GtkUIManager *ui, GtkAction *action, GtkWidget *proxy, gcApplication* App)
{
	if (GTK_IS_MENU_ITEM (proxy)) {
		g_object_set_data (G_OBJECT (proxy), "action", NULL);
		g_object_disconnect (proxy,
			"any_signal::select",  G_CALLBACK (on_show_menu_tip), App,
			"any_signal::deselect", G_CALLBACK (on_clear_menu_tip), App,
			NULL);
	}
}

//Implementation of gcApplication class

static GtkActionEntry entries[] = {
  { "FileMenu", NULL, N_("_File") },
	  { "New", GTK_STOCK_NEW, N_("_New File"), NULL,
		  N_("Create a new file"), G_CALLBACK (on_file_new) },
	  { "Open", GTK_STOCK_OPEN, N_("_Open..."), "<control>O",
		  N_("Open a file"), G_CALLBACK (on_file_open) },
	  { "OpenNewWin", NULL, N_("O_pen in a new window..."), "<shift><control>O",
		  N_("Open a file in a new window"), G_CALLBACK (on_file_open_new_win) },
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
	  { "About", NULL, N_("_About"), NULL,
		  N_("About Gnome Crystal"), G_CALLBACK (on_about) }
};

static const char *ui_description =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu action='FileMenu'>"
"      <menuitem action='New'/>"
"      <menuitem action='Open'/>"
"      <menuitem action='OpenNewWin'/>"
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

gcApplication::gcApplication(): Application ("gcrystal-unstable", DATADIR)
{
	GtkWidget *vbox;
	GtkWidget *bar;
	GtkActionGroup *action_group;
	GtkAccelGroup *accel_group;
	GError *error;

	m_MessageId = 0;
	m_Window = GTK_WINDOW (gtk_window_new (GTK_WINDOW_TOPLEVEL));
	gtk_window_set_title (m_Window, _("Gnome Crystal"));
	gtk_window_set_icon_name (m_Window, "gcrystal");
	g_signal_connect (G_OBJECT (m_Window), "delete_event", G_CALLBACK (on_delete), this);
	g_signal_connect (G_OBJECT (m_Window), "destroy", G_CALLBACK (on_destroy), NULL);
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
	if (!gtk_ui_manager_add_ui_from_string (m_UIManager, ui_description, -1, &error))
	  {
		g_message ("building menus failed: %s", error->message);
		g_error_free (error);
		exit (EXIT_FAILURE);
	  }
	bar = gtk_ui_manager_get_widget (m_UIManager, "/MainMenu");
	gtk_box_pack_start (GTK_BOX (vbox), bar, false, false, 0);
	bar = gtk_ui_manager_get_widget (m_UIManager, "/MainToolbar");
	gtk_toolbar_set_style(GTK_TOOLBAR(bar), GTK_TOOLBAR_ICONS);
	gtk_toolbar_set_show_arrow(GTK_TOOLBAR(bar), false);
	gtk_toolbar_set_tooltips(GTK_TOOLBAR(bar), true);
	gtk_box_pack_start (GTK_BOX (vbox), bar, false, false, 0);

	m_Notebook = GTK_NOTEBOOK (gtk_notebook_new ());
	gtk_widget_set_size_request (GTK_WIDGET (m_Notebook), 300, 300);
	gtk_notebook_set_tab_pos (m_Notebook, GTK_POS_TOP);
	gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (m_Notebook), true, true, 0);

	m_Bar = gtk_statusbar_new ();
	m_statusId = gtk_statusbar_get_context_id (GTK_STATUSBAR (m_Bar), "status");
	gtk_statusbar_push (GTK_STATUSBAR (m_Bar), m_statusId, _("Ready"));
	gtk_box_pack_start (GTK_BOX (vbox), m_Bar, false, false, 0);
	UpdateConfig ();
	gtk_widget_show_all (GTK_WIDGET (m_Window));
}

gcApplication::~gcApplication ()
{
}

void gcApplication::OnFileNew ()
{
	gcDocument* pDoc = GetNewDocument ();
	gchar tmp[32];
	g_snprintf (tmp, sizeof (tmp), _("Untitled%d"), nNewDocs++);
	pDoc->SetTitle (tmp);
	gcView* pView = (gcView*) pDoc->GetView ();
	AddView (pView);
	m_bFileOpening = false;
}

void gcApplication::OnFileOpen ()
{
	gcView* pView;
	gcDocument* pDoc;
	if (m_Views.empty ()) {
		OnFileNew ();
		m_bFileOpening = true;
	}
	pView = m_Views.back ();
	pDoc = pView->GetDocument ();
	list<char const*> l;
	l.push_front ("application/x-gcrystal");
	FileChooser (this, false, l, pDoc);
}

void gcApplication::OnFileSave ()
{
	if (!m_pActiveView)
		return;
	gcDocument* pDoc = m_pActiveView->GetDocument ();
	if (!pDoc)
		return;
	if (pDoc->GetFileName ())
		pDoc->Save ();
	else
		OnFileSaveAs ();
}

void gcApplication::OnFileSaveAs ()
{
	if (!m_pActiveView)
		return;
	gcDocument* pDoc = m_pActiveView->GetDocument ();
	if (!pDoc)
		return;
	list<char const*> l;
	l.push_front ("application/x-gcrystal");
	FileChooser (this, true, l, pDoc);
}

bool gcApplication::OnFileClose ()
{
	if (m_pActiveView == NULL) {
		if (m_Views.empty () && (Apps.size () > 1)) {
			Apps.remove (this);
			gtk_widget_destroy (GTK_WIDGET (m_Window));
			return true;
		}
		else
			return false;
	}
	if (m_pActiveView->IsLocked ())
		return false;
	gcDocument* pDoc = m_pActiveView->GetDocument ();
	if (!pDoc->VerifySaved ())
		return false;
	std::list<gcApplication*>::iterator i, iend = Apps.end ();
	std::list<gcView*>::iterator j, jend;
	bool bEnded;
	do {
		bEnded = true;
		for (i = Apps.begin(); i != iend; i++) {
			if (*i == this)
				continue;
			jend = (*i)->m_Views.end ();
			for (j = (*i)->m_Views.begin (); j != jend; j++)
				if ((*j)->GetDocument () == pDoc) {
					(*i)->OnViewClose (*j);
					bEnded = false;
					break;
				}
			if (!bEnded)
				break;
		}
	} while (!bEnded);
	OnViewClose (m_pActiveView);
	return true;
}

void gcApplication::OnSelectView (GtkWidget* widget)
{
	gint i;
	i = gtk_notebook_page_num (m_Notebook, widget);
	if (i != gtk_notebook_get_current_page (m_Notebook))
		gtk_notebook_set_current_page (m_Notebook, i);
}

void gcApplication::OnCrystalDefine (int page)
{
	gcDocument* pDoc = m_pActiveView->GetDocument ();
	if (pDoc)
		pDoc->Define (page);
}

void gcApplication::OnViewSettings ()
{
	if (!m_pActiveView)
		return;
	new gcViewSettingsDlg (m_pActiveView);
}

void gcApplication::OnFilePrint ()
{
	GnomePrintConfig* config = gnome_print_config_default ();
	GnomePrintContext *pc;
	GnomePrintJob *gpj = gnome_print_job_new (config);
	int do_preview = 0, copies = 1, collate = 0;
	GnomePrintDialog *gpd;
	gpd = GNOME_PRINT_DIALOG (gnome_print_dialog_new (gpj, (const guchar*) "Print test", GNOME_PRINT_DIALOG_COPIES));
	gtk_window_set_icon_name (GTK_WINDOW (gpd), "gcrystal");
	gnome_print_dialog_set_copies (gpd, copies, collate);
	switch (gtk_dialog_run (GTK_DIALOG (gpd))) {
	case GNOME_PRINT_DIALOG_RESPONSE_PRINT:
		do_preview = 0;
		break;
	case GNOME_PRINT_DIALOG_RESPONSE_PREVIEW:
		do_preview = 1;
		break;
	case GNOME_PRINT_DIALOG_RESPONSE_CANCEL:
		gtk_widget_destroy (GTK_WIDGET (gpd));
		return;
	}
	gtk_widget_destroy (GTK_WIDGET (gpd));
	pc = gnome_print_job_get_context (gpj);
	gnome_print_beginpage (pc, (const guchar*) "");
	gdouble width, height;
	gnome_print_config_get_double (config, (const guchar*) GNOME_PRINT_KEY_PAPER_WIDTH, &width);
	gnome_print_config_get_double (config, (const guchar*) GNOME_PRINT_KEY_PAPER_HEIGHT, &height);
	m_pActiveView->Print (pc, width, height);
	gnome_print_showpage (pc);
	g_object_unref (pc);
	gnome_print_job_close (gpj);
	if (do_preview) {
		GtkWidget *preview = gnome_print_job_preview_new (gpj, (const guchar*)_("Preview"));
		gtk_window_set_icon_name (GTK_WINDOW (preview), "gcrystal");
		gtk_widget_show (preview);
	} else {
		gnome_print_job_print (gpj);
	}
	g_object_unref (gpj);
	gnome_print_config_unref (config);
}

void gcApplication::OnExportJPEG ()
{
	if (!m_pActiveView)
		return;
	list<char const*> l;
	l.push_front ("image/jpeg");
	FileChooser (this, true, l, m_pActiveView->GetDocument (), _("Export model as a Jpeg file..."));
}

void gcApplication::OnExportPNG ()
{
	if (!m_pActiveView)
		return;
	list<char const*> l;
	l.push_front ("image/png");
	FileChooser (this, true, l, m_pActiveView->GetDocument (), _("Export model as a PNG file..."));
}

void gcApplication::OnExportVRML ()
{
	if (!m_pActiveView)
		return;
	list<char const*> l;
	l.push_front ("image/vrml");
	FileChooser (this, true, l, m_pActiveView->GetDocument (), _("Export model as a VRML file..."));
}

void gcApplication::OnViewNew ()
{
	if (!m_pActiveView)
		return;
	gcView* pView = new gcView (m_pActiveView);
	gcDocument* pDoc = m_pActiveView->GetDocument ();
	//Try to add the new view to an existing gcApplication
	gcApplication* pApp = NULL;
	std::list<gcApplication*>::iterator i, iend = Apps.end ();
	for (i = Apps.begin (); i!= iend; i++) {
		if (*i == this)
			continue;
		if (!(*i)->HasDocument (pDoc)) {
			pApp = *i;
			break;
		}
	}
	if (!pApp) {
		pApp = new gcApplication ();
		Apps.push_back (pApp);
	}
	if (pApp) {
		pApp->AddView (pView);
		pDoc->SetDirty ();
		gtk_label_set_text (pView->GetLabel (), pDoc->GetTitle ());
		GtkLabel *pLabel = pView->GetMenuLabel ();
		if (pLabel)
			gtk_label_set_text (pLabel, pDoc->GetTitle ());
	}
	else
		delete pView;
}

bool gcApplication::OnViewClose (gcView* pView)
{
	if (pView == NULL)
		pView = m_pActiveView;
	if (pView == NULL)
		return false;
	gcDocument* pDoc = pView->GetDocument ();
	gint i = 0;
	GtkWidget* w;
	if (pView == m_pActiveView)
		i = gtk_notebook_get_current_page(m_Notebook);
	else
		do {
		w = gtk_notebook_get_nth_page (m_Notebook, i);
		if (w == NULL)
			return false; //should not occur
		if (pView == ((gcView*) g_object_get_data (G_OBJECT (w), "view")))
			break;
		else
			i++;
	} while (w);
	if (!pDoc->RemoveView (pView))
		return false;
	m_Views.remove (pView);
	gtk_notebook_remove_page (m_Notebook, i);
	delete pView;
	if (pView == m_pActiveView) {
		i = gtk_notebook_get_current_page (m_Notebook);
		w = gtk_notebook_get_nth_page (m_Notebook, i);
		m_pActiveView = (w)? (gcView*) g_object_get_data(G_OBJECT(w), "view"): NULL;
	}
	if (m_Views.empty () && (Apps.size () > 1)) {
		Apps.remove(this);
		gtk_widget_destroy (GTK_WIDGET (m_Window));
		delete this;
	}
	return true;
}

bool gcApplication::HasDocument (gcDocument* pDoc)
{
	std::list<gcView*>::iterator i, iend = m_Views.end ();
	for (i = m_Views.begin(); i != iend; i++)
		if (pDoc == (*i)->GetDocument ())
			return true;
	return false;
}

void gcApplication::AddView (gcView* pView)
{
	pView->SetApp (this);
	GtkWidget* widget = pView->CreateNewWidget ();
	m_Views.push_back (pView);
	m_pActiveView = pView;
	gchar* tmp = pView->GetDocument ()->GetTitle ();
	GtkWidget *label = gtk_label_new (tmp);
	pView->SetLabel (GTK_LABEL (label));
	GtkWidget* item = gtk_menu_item_new_with_label (tmp);
	pView->SetMenu (GTK_MENU_ITEM (item));
	g_object_set_data (G_OBJECT (item), "page", widget);
	g_object_set_data (G_OBJECT (widget), "menu", item);
	g_object_set_data (G_OBJECT (widget), "view", pView);
	gtk_widget_show (item);
	gtk_notebook_append_page (m_Notebook, widget, label);
	gtk_notebook_set_current_page (m_Notebook, g_list_length(m_Notebook->children) - 1);
	g_signal_connect (G_OBJECT (item), "activate", (GtkSignalFunc) on_select, this); 
}

bool gcApplication::LoadFile (char* filename)
{
	return do_load (filename, m_pActiveView);
}

void gcApplication::UpdateConfig ()
{
	if (TabPos == 0)
		gtk_notebook_set_show_tabs (m_Notebook, false);
	else {
		gtk_notebook_set_show_tabs (m_Notebook, true);
		gtk_notebook_set_tab_pos (m_Notebook, (GtkPositionType) (TabPos - 1));
	}
}

gcView* gcApplication::GetDocView (const char* filename)
{
	gcDocument* pDoc;
	gcView *pView = NULL;
	std::list<gcView*>::iterator i, iend = m_Views.end ();
	for (i = m_Views.begin (); i != iend; i++) {
		pDoc = (*i)->GetDocument ();
		if (!pDoc->GetFileName ())
			continue;
		if (!strcmp (pDoc->GetFileName (), filename)) {
			pView = *i;
			break;
		}
	}
	if (!pView) {
		//search doc in other apps
		std::list<gcApplication*>::iterator j, jend = Apps.end ();
		for (j = Apps.begin (); j != jend && !pView; j++) {
			if (*j == this)
				continue;
			iend = (*j)->m_Views.end ();
			for (i = (*j)->m_Views.begin (); i != iend; i++) {
				pDoc = (*i)->GetDocument ();
				if (!pDoc->GetFileName ())
					continue;
				if (!strcmp(pDoc->GetFileName (), filename)) {
					pView = *i;
					GdkWindow *win = GTK_WIDGET ((*j)->m_Window)->window;
					gdk_window_hide (win);
					gdk_window_show (win);
					gdk_window_raise (win);
					break;
				}
			}
		}
	}
	if (pView)
		return pView;
	if (m_bFileOpening) {
		pView = m_Views.back ();
		pDoc = pView->GetDocument ();
		if (!pDoc->IsEmpty () || pDoc->IsDirty ())
			pView = NULL;
	}
	if (!pView) {
		OnFileNew ();
		pView = m_Views.back ();
	}
	nNewDocs--;
	return pView;
}

gcView* gcApplication::GetView (gcDocument* pDoc)
{
	std::list<gcView*>::iterator i, iend = m_Views.end ();
	for (i = m_Views.begin (); i != iend; i++)
		if (pDoc == (*i)->GetDocument ())
			return *i;
	return NULL;
}

void gcApplication::SelectView (gcView* pView)
{
	GtkWidget *w;
	guint i = 0;
	do {
		w = gtk_notebook_get_nth_page (m_Notebook, i);
		if (w == NULL)
			return; //should not occur
		if (pView == ((gcView*) g_object_get_data (G_OBJECT (w), "view")))
			break;
		else i++;
	} while (w);
	gtk_notebook_set_current_page (m_Notebook, i);
}

void gcApplication::OnChangePage (int i)
{
	GtkWidget *w = gtk_notebook_get_nth_page (m_Notebook, i);
	m_pActiveView = (gcView*) g_object_get_data (G_OBJECT (w), "view");
}

void gcApplication::ClearStatus ()
{
	if (m_MessageId) {
		gtk_statusbar_pop (GTK_STATUSBAR (m_Bar), m_statusId);
		m_MessageId = 0;
	}
}

void gcApplication::SetStatusText (const char* text)
{
	if (m_MessageId) gtk_statusbar_pop (GTK_STATUSBAR (m_Bar), m_statusId);
	m_MessageId = gtk_statusbar_push (GTK_STATUSBAR (m_Bar), m_statusId, text);
}

GtkWindow * gcApplication::GetWindow ()
{
	return m_Window;
}

bool gcApplication::FileProcess (const gchar* filename, bool bSave, GtkWindow *window, Document *pDoc)
{
	if (bSave) {
		do_save_as (filename, m_pActiveView);
	} else
		return do_load (filename, m_pActiveView);
	return true;
}
