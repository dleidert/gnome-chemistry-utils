// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * programs/3d/window.cc
 *
 * Copyright (C) 2006-2011 Jean Bréfort <jean.brefort@normalesup.org>
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

#include "config.h"
#include "application.h"
#include "document.h"
#include "view.h"
#include "window.h"
#include <gcugtk/molecule.h>
#include <gcugtk/print-setup-dlg.h>
#include <gcugtk/stringdlg.h>
#include <gcugtk/stringinputdlg.h>
#include <gsf/gsf-input-memory.h>
#include <glib/gi18n.h>

class gc3dWindowPrivate {
public:
	static void OnOpen2D (GtkWidget *widget, gc3dWindow *Win);
	static void ShowInChIKey (GtkWidget *widget, gc3dWindow *Win);
	static void ShowInChI (GtkWidget *widget, gc3dWindow *Win);
	static void ShowSMILES (GtkWidget *widget, gc3dWindow *Win);
	static void ImportMolecule (G_GNUC_UNUSED GtkWidget* widget, gc3dWindow *Win);
	static void DoImportMol (gcu::Document *doc, char const *str);
};

void gc3dWindowPrivate::ImportMolecule (G_GNUC_UNUSED GtkWidget* widget, gc3dWindow* Win)
{
	gcu::Dialog *dlg = Win->GetDoc ()->GetDialog ("string-input");
	if (dlg)
		dlg->Present ();
	else
		new gcugtk::StringInputDlg (Win->GetDoc (), &DoImportMol, _("Import molecule from InChI or SMILES"));
}

void gc3dWindowPrivate::DoImportMol (gcu::Document *doc, char const *str)
{
	if (!str || !*str)
		return;
	gcu::Application *app = doc->GetApplication ();
	gc3dDocument *Doc = static_cast < gc3dDocument * > (doc);
	GsfInput *input = gsf_input_memory_new (reinterpret_cast < guint8 const * > (str), strlen (str), false);
	char *cml = app->ConvertToCML (input, ((!strncmp (str, "InChI=", 6))? "inchi": "smi"), "--Gen3D");
	g_object_unref (input);
	if (!cml) // TODO: add an error message handler
		return;
	if (Doc->GetMol ())
		Doc = static_cast < gc3dApplication * > (app)->OnFileNew ();
	Doc->LoadData (cml, "chemical/x-cml");
	gcugtk::Molecule *mol = static_cast < gcugtk::Molecule * > (Doc->GetMol ());
	if (mol && mol->GetChildrenNumber ())
		static_cast < gc3dWindow * > (Doc->GetWindow ())->AddMoleculeMenus (static_cast < gcugtk::Molecule * > (mol));
}

void gc3dWindowPrivate::ShowInChIKey (G_GNUC_UNUSED GtkWidget *widget, gc3dWindow *Win)
{
	gc3dDocument *Doc  = Win->GetDoc ();
	gcu::Molecule *Mol = Doc->GetMol ();

	new gcugtk::StringDlg (Doc, Mol->GetInChIKey (), gcugtk::StringDlg::INCHIKEY);
}

void gc3dWindowPrivate::ShowInChI (G_GNUC_UNUSED GtkWidget *widget, gc3dWindow *Win)
{
	gc3dDocument *Doc  = Win->GetDoc ();
	gcu::Molecule *Mol = Doc->GetMol ();

	new gcugtk::StringDlg (Doc, Mol->GetInChI (), gcugtk::StringDlg::INCHI);
}

void gc3dWindowPrivate::ShowSMILES (G_GNUC_UNUSED GtkWidget *widget, gc3dWindow *Win)
{
	gc3dDocument *Doc  = Win->GetDoc ();
	gcu::Molecule *Mol = Doc->GetMol ();

	new gcugtk::StringDlg (Doc, Mol->GetSMILES (), gcugtk::StringDlg::SMILES);
}

void gc3dWindowPrivate::OnOpen2D (G_GNUC_UNUSED GtkWidget *widget, gc3dWindow *Win)
{
	gcu::Molecule *mol = Win->GetDoc ()->GetMol ();
	std::string const &InChI = mol->GetInChI ();
	GsfInput *in = gsf_input_memory_new (reinterpret_cast <guint8 const *> (InChI.c_str ()), InChI.length (), false);
	char *cml = mol->GetDocument ()->GetApp ()->ConvertToCML (in, "inchi", "--Gen2D");
	g_object_unref (in);
	char *tmpname = g_strdup ("/tmp/cmlXXXXXX.cml");
	int f = g_mkstemp (tmpname);
	write (f, cml, strlen (cml));
	close (f);
	g_free (cml);
	char *command_line = g_strconcat ("gchempaint-", API_VERSION, " ", tmpname, NULL);
	g_free (tmpname);
	g_spawn_command_line_async (command_line, NULL);
	g_free (command_line);
}

//Callbacks
static bool on_delete_event (G_GNUC_UNUSED GtkWidget *widget, G_GNUC_UNUSED GdkEvent *event, gc3dWindow *Win)
{
	delete Win;
	return false;
}

static void on_file_open (G_GNUC_UNUSED GtkWidget *widget, gc3dWindow *Win)
{
	Win->OnFileOpen ();
}

static void on_file_save_as_image(G_GNUC_UNUSED GtkWidget *widget, gc3dWindow *Win)
{
	Win->GetApp ()->OnSaveAsImage (Win->GetDoc ());
}

static void on_file_close (G_GNUC_UNUSED GtkWidget *widget, gc3dWindow *Win)
{
	Win->OnFileClose ();
}

static void on_page_setup (G_GNUC_UNUSED GtkWidget *widget, gc3dWindow *Win)
{
	Win->OnPageSetup ();
}

static void on_print_preview (G_GNUC_UNUSED GtkWidget *widget, gc3dWindow *Win)
{
	Win->GetView ()->Print (true);
}

static void on_file_print (G_GNUC_UNUSED GtkWidget *widget, gc3dWindow *Win)
{
	Win->GetView ()->Print (false);
}

static void on_quit (G_GNUC_UNUSED GtkWidget *widget, gc3dWindow *Win)
{
	gc3dApplication *App = Win->GetApp ();
	App->OnQuit ();
}

static void on_color_changed (GOActionComboColor *combo, gc3dWindow *window)
{
	GOColor color = go_action_combo_color_get_color (combo, FALSE);
	gc3dView *View = window->GetView ();
	View->SetRed (GO_COLOR_DOUBLE_R (color));
	View->SetGreen (GO_COLOR_DOUBLE_G (color));
	View->SetBlue (GO_COLOR_DOUBLE_B (color));
	View->SetAlpha (GO_COLOR_DOUBLE_A (color));
	View->Update ();
}

static void on_display (GtkRadioAction *action, G_GNUC_UNUSED GtkRadioAction *current, G_GNUC_UNUSED gc3dWindow *window)
{
	window->GetDoc ()->SetDisplay3D (static_cast <gcu::Display3DMode> (gtk_radio_action_get_current_value (action)));
	window->GetView ()->Update ();
}

static void on_help (G_GNUC_UNUSED GtkWidget *widget, gc3dWindow *window)
{
	window->GetApp ()->OnHelp ();
}

static void on_web (G_GNUC_UNUSED GtkWidget *widget, gc3dWindow *window)
{
	window->GetApp ()->OnWeb ();
}

static void on_mail (G_GNUC_UNUSED GtkWidget *widget, gc3dWindow *window)
{
	window->GetApp ()->OnMail ();
}

static void on_bug (G_GNUC_UNUSED GtkWidget *widget, gc3dWindow *window)
{
	window->GetApp ()->OnBug ();
}

static void on_live_assistance (G_GNUC_UNUSED GtkWidget *widget, gc3dWindow *window)
{
	window->GetApp ()->OnLiveAssistance ();
}

static void on_about (G_GNUC_UNUSED GtkWidget *widget, G_GNUC_UNUSED gc3dWindow *window)
{
	const gchar * authors[] = {"Jean Bréfort", NULL};
	const gchar * comments = _("GChem3D is a molecular structures viewer for Gnome");
	/* const gchar * documentors[] = {NULL}; */
	const gchar * copyright = _("Copyright © 2004-2010 Jean Bréfort\n");
	const gchar * license =
		"This program is free software; you can redistribute it and/or\n"
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
	const gchar * translator_credits = _("translator_credits");
	gtk_show_about_dialog (NULL,
	                       "program-name", "GChem3D",
	                       "authors", authors,
	                       "comments", comments,
	                       "copyright", copyright,
	                       "license", license,
	                       "translator_credits", translator_credits,
	                       "version", VERSION,
	                       "website", "http://gchemutils.nongnu.org",
	                       NULL);
}

static void on_recent (GtkRecentChooser *widget, gc3dWindow *Win)
{
	gc3dApplication *App = Win->GetApp ();
	GtkRecentInfo *info = gtk_recent_chooser_get_current_item (widget);
	App->FileProcess (gtk_recent_info_get_uri (info), gtk_recent_info_get_mime_type (info), false, NULL, Win->GetDoc ());
	gtk_recent_info_unref(info);
}

static GtkActionEntry entries[] = {
  { "FileMenu", NULL, N_("_File"), NULL, NULL, NULL },
	  { "Open", GTK_STOCK_OPEN, N_("_Open..."), "<control>O",
		  N_("Open a file"), G_CALLBACK (on_file_open) },
	  { "SaveAsImage", GTK_STOCK_SAVE_AS, N_("Save As _Image..."), "<control>I",
		  N_("Save the current file as an image"), G_CALLBACK (on_file_save_as_image) },
	  { "PageSetup", NULL, N_("Page Set_up..."), NULL,
		  N_("Setup the page settings for your current printer"), G_CALLBACK (on_page_setup) },
	  { "PrintPreview", GTK_STOCK_PRINT_PREVIEW, N_("Print Pre_view"), NULL,
		  N_("Print preview"), G_CALLBACK (on_print_preview) },
	  { "Print", GTK_STOCK_PRINT, N_("_Print..."), "<control>P",
		  N_("Print the current scene"), G_CALLBACK (on_file_print) },
	  { "Close", GTK_STOCK_CLOSE, N_("_Close"), "<control>W",
		  N_("Close the current file"), G_CALLBACK (on_file_close) },
 	  { "Quit", GTK_STOCK_QUIT, N_("_Quit"), "<control>Q",
		  N_("Quit GChem3D"), G_CALLBACK (on_quit) },
  { "ViewMenu", NULL, N_("_View"), NULL, NULL, NULL },
  { "ToolsMenu", NULL, N_("_Tools"), NULL, NULL, NULL },
	  { "ImportMol", NULL, N_("_Import molecule"), NULL,
		  N_("Import a molecule either from InChI or SMILES"), G_CALLBACK (gc3dWindowPrivate::ImportMolecule) },
	  { "GChemPaint", NULL, N_("Open in GChemPaint"), NULL,
		  N_("Open a 2D model for this molecule using GChemPaint"), G_CALLBACK (gc3dWindowPrivate::OnOpen2D) },
	  { "InChI", NULL, N_("Show InChI"), NULL,
		  N_("Show the InChI for this molecule"), G_CALLBACK (gc3dWindowPrivate::ShowInChI) },
	  { "InChIKey", NULL, N_("Show InChiKey"), NULL,
		  N_("Show the InChIKey for this molecule"), G_CALLBACK (gc3dWindowPrivate::ShowInChIKey) },
	  { "SMILES", NULL, N_("Show SMILES"), NULL,
		  N_("Show the SMILES for this molecule"), G_CALLBACK (gc3dWindowPrivate::ShowSMILES) },
  { "HelpMenu", NULL, N_("_Help"), NULL, NULL, NULL },
	  { "Help", GTK_STOCK_HELP, N_("_Contents"), "F1",
		  N_("View help for the Molecules Viewer"), G_CALLBACK (on_help) },
	  { "Web", NULL, N_("Gnome Chemistry Utils on the _web"), NULL,
		  N_("Browse the Gnome Chemistry Utils's web site"), G_CALLBACK (on_web) },
	  { "LiveAssistance", NULL, N_("Live assistance"), NULL,
		  N_("Open the Gnome Chemistry Utils IRC channel"), G_CALLBACK (on_live_assistance) },
	  { "Mail", NULL, N_("_Ask a question"), NULL,
		  N_("Ask a question about the Gnome Chemistry Utils"), G_CALLBACK (on_mail) },
	  { "Bug", NULL, N_("Report _Bugs"), NULL,
		  N_("Submit a bug report for the Gnome Chemistry Utils"), G_CALLBACK (on_bug) },
	  { "About", GTK_STOCK_ABOUT, N_("_About"), NULL,
		  N_("About GChem3D"), G_CALLBACK (on_about) }
};

static GtkRadioActionEntry radios[] = {
	{ "BallnStick", NULL, N_("Balls and sticks"), NULL,
		N_("Display a balls and sticks model"),
		gcu::BALL_AND_STICK },
	{ "SpaceFill", "NULL", N_("Space filling"), NULL,
		N_("Display a space filling model"),
		gcu::SPACEFILL },
	{ "Cylinders", "NULL", N_("Cylinders"), NULL,
		N_("Display a cylinders model"),
		gcu::CYLINDERS },
	{ "Wireframe", "NULL", N_("Wireframe"), NULL,
		N_("Display a wireframe model"),
		gcu::WIREFRAME },
};

static const char *ui_description =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu action='FileMenu'>"
"      <menuitem action='Open'/>"
"      <menuitem action='SaveAsImage'/>"
"	   <separator name='file-sep1'/>"
"      <menuitem action='PageSetup'/>"
"      <menuitem action='PrintPreview'/>"
"      <menuitem action='Print'/>"
"	   <separator name='file-sep2'/>"
"      <menuitem action='Close'/>"
"      <menuitem action='Quit'/>"
"    </menu>"
"    <menu action='ViewMenu'>"
"      <menuitem action='BallnStick'/>"
"      <menuitem action='SpaceFill'/>"
"      <menuitem action='Cylinders'/>"
"      <menuitem action='Wireframe'/>"
"	   <separator name='view-sep1'/>"
"      <menuitem action='Background'/>"
"    </menu>"
"    <menu action='ToolsMenu'>"
"      <menuitem action='ImportMol'/>"
"    </menu>"
"    <menu action='HelpMenu'>"
"      <menuitem action='Help'/>"
"      <menuitem action='Mail'/>"
"      <menuitem action='Web'/>"
"      <menuitem action='LiveAssistance'/>"
"      <menuitem action='Bug'/>"
"      <menuitem action='About'/>"
"    </menu>"
"  </menubar>"
"</ui>";

gc3dWindow::gc3dWindow (gc3dApplication *App, gc3dDocument *Doc)
{
	m_App = App;
	m_Doc = Doc;
	GtkWidget *grid, *bar;
	GtkActionGroup *action_group;
	GtkAccelGroup *accel_group;
	GError *error = NULL;

	m_Window = GTK_WINDOW (gtk_window_new (GTK_WINDOW_TOPLEVEL));
	gtk_window_set_default_size (m_Window, 250, 280);
	gtk_window_set_icon_name (m_Window, App->GetIconName ().c_str ());
	g_signal_connect (G_OBJECT (m_Window), "delete-event", G_CALLBACK (on_delete_event), this);

	grid = gtk_grid_new ();
	g_object_set (G_OBJECT (grid), "orientation", GTK_ORIENTATION_VERTICAL, NULL);
	gtk_container_add (GTK_CONTAINER (m_Window), grid);
	m_UIManager = gtk_ui_manager_new ();
	action_group = gtk_action_group_new ("MenuActions");
	gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);
	gtk_action_group_add_actions (action_group, entries, G_N_ELEMENTS (entries), this);
	GOActionComboColor *combo = go_action_combo_color_new ("Background", "gcu_Background", "", GO_COLOR_BLACK, NULL);
	g_object_set (G_OBJECT (combo), "label", _("Background color"), "tooltip",
		_("Choose a new background color"), NULL);
	g_signal_connect (G_OBJECT (combo), "activate", G_CALLBACK (on_color_changed), this);
	gtk_action_group_add_action (action_group, GTK_ACTION (combo));
	gtk_action_group_add_radio_actions (action_group, radios, G_N_ELEMENTS (radios), 0, G_CALLBACK (on_display), this);
	gtk_ui_manager_insert_action_group (m_UIManager, action_group, 0);
	accel_group = gtk_ui_manager_get_accel_group (m_UIManager);
	gtk_window_add_accel_group (GTK_WINDOW (m_Window), accel_group);
	error = NULL;
	if (!gtk_ui_manager_add_ui_from_string (m_UIManager, ui_description, -1, &error)) {
		g_message ("building menus failed: %s", error->message);
		g_error_free (error);
		exit (EXIT_FAILURE);
	}
	// add database access menus
	GtkWidget *menu = gtk_ui_manager_get_widget (m_UIManager, "/MainMenu/FileMenu/Open");
	GtkWidget *w = gtk_recent_chooser_menu_new_for_manager (App->GetRecentManager ());
	gtk_recent_chooser_set_sort_type (GTK_RECENT_CHOOSER (w), GTK_RECENT_SORT_MRU);
	GtkRecentFilter *filter = gtk_recent_filter_new ();
	gtk_recent_filter_add_mime_type (filter, "chemical/x-cml");
	gtk_recent_filter_add_mime_type (filter, "chemical/x-mdl-molfile");
	gtk_recent_filter_add_mime_type (filter, "chemical/x-pdb");
	gtk_recent_filter_add_mime_type (filter, "chemical/x-xyz");
	gtk_recent_chooser_add_filter (GTK_RECENT_CHOOSER (w), filter);
	g_signal_connect (G_OBJECT (w), "item-activated", G_CALLBACK (on_recent), this);
	GtkWidget *item = gtk_menu_item_new_with_label (_("Open recent"));
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), w);
	gtk_widget_show_all (item);
	gtk_menu_shell_insert (GTK_MENU_SHELL (gtk_widget_get_parent (menu)), item, 2);
	bar = gtk_ui_manager_get_widget (m_UIManager, "/MainMenu");
	gtk_container_add (GTK_CONTAINER (grid), bar);
	m_View = dynamic_cast<gc3dView *> (m_Doc->GetView ());
	m_View->SetWindow (this);
	g_object_set (G_OBJECT (m_View->GetWidget ()), "expand", true, NULL);
	gtk_container_add (GTK_CONTAINER (grid), m_View->GetWidget ());
	switch (Doc->GetDisplay3D ()) {
	case gcu::BALL_AND_STICK:
		gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (gtk_action_group_get_action (action_group, "BallnStick")), true);
		break;
	case gcu::SPACEFILL:
		gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (gtk_action_group_get_action (action_group, "SpaceFill")), true);
		break;
	case gcu::CYLINDERS:
		gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (gtk_action_group_get_action (action_group, "Cylinders")), true);
		break;
	case gcu::WIREFRAME:
		gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (gtk_action_group_get_action (action_group, "Wireframe")), true);
		break;
	}
	gtk_widget_show_all (GTK_WIDGET (m_Window));
}

gc3dWindow::~gc3dWindow ()
{
	g_object_unref (m_UIManager);
	delete m_Doc;
}

void gc3dWindow::OnFileOpen ()
{
	m_App->OnFileOpen (m_Doc);
}

void gc3dWindow::OnFileClose ()
{
	gtk_widget_destroy (GTK_WIDGET (m_Window));
	delete this;
}

void gc3dWindow::OnPageSetup ()
{
	new gcugtk::PrintSetupDlg (m_App, m_View);
}

void gc3dWindow::SetTitle (char const *title)
{
	gtk_window_set_title (m_Window, title);
}

static const char *ui_mol_description =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu action='ToolsMenu'>"
"	   <separator name='tools-sep1'/>"
"      <menuitem action='GChemPaint'/>"
"      <menuitem action='InChI'/>"
"      <menuitem action='InChIKey'/>"
"      <menuitem action='SMILES'/>"
"    </menu>"
"  </menubar>"
"</ui>";

void gc3dWindow::AddMoleculeMenus (gcugtk::Molecule *mol)
{
	gtk_ui_manager_add_ui_from_string (m_UIManager, ui_mol_description, -1, NULL);
	mol->BuildDatabasesMenu (m_UIManager, "<ui><menubar name='MainMenu'><menu action='ToolsMenu'>", "</menu></menubar></ui>");
}
