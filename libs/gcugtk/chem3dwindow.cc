// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gcugtk/chem3dwindow.h
 *
 * Copyright (C) 2011 Jean Bréfort <jean.brefort@normalesup.org>
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

#include "config.h"
#include "application.h"
#include "chem3dapplication.h"
#include "chem3ddoc.h"
#include "chem3dview.h"
#include "chem3dwindow.h"
#include "ui-manager.h"
#include <gcugtk/molecule.h>
#include <gcugtk/print-setup-dlg.h>
#include <gcugtk/stringdlg.h>
#include <gcugtk/stringinputdlg.h>
#include <gsf/gsf-input-memory.h>
#include <glib/gi18n-lib.h>
#include <sstream>

namespace gcugtk {

class Chem3dWindowPrivate {
public:
	static void OnOpen2D (GtkWidget *widget, Chem3dWindow *Win);
	static void ShowInChIKey (GtkWidget *widget, Chem3dWindow *Win);
	static void ShowInChI (GtkWidget *widget, Chem3dWindow *Win);
	static void ShowSMILES (GtkWidget *widget, Chem3dWindow *Win);
	static void ImportMolecule (G_GNUC_UNUSED GtkWidget* widget, Chem3dWindow *Win);
	static void DoImportMol (gcu::Document *doc, char const *str);
	static void OnOpenCalc (G_GNUC_UNUSED GtkWidget *widget, Chem3dWindow *Win);
	static void Save (GtkWidget *widget, Chem3dWindow *Win);
};

void Chem3dWindowPrivate::ImportMolecule (G_GNUC_UNUSED GtkWidget* widget, Chem3dWindow* Win)
{
	gcu::Dialog *dlg = Win->GetDocument ()->GetDialog ("string-input");
	if (dlg)
		dlg->Present ();
	else
		new gcugtk::StringInputDlg (Win->GetDocument (), &DoImportMol, _("Import molecule from InChI or SMILES"));
}

void Chem3dWindowPrivate::DoImportMol (gcu::Document *doc, char const *str)
{
	if (!str || !*str)
		return;
	gcu::Application *app = doc->GetApplication ();
	Chem3dDoc *Doc = static_cast < Chem3dDoc * > (doc);
	GsfInput *input = gsf_input_memory_new (reinterpret_cast < guint8 const * > (str), strlen (str), false);
	char *cml = app->ConvertToCML (input, ((!strncmp (str, "InChI=", 6))? "inchi": "smi"), "--Gen3D");
	g_object_unref (input);
	if (!cml) // TODO: add an error message handler
		return;
	if (Doc->GetMol ())
		Doc = static_cast < Chem3dApplication * > (app)->OnFileNew ();
	Doc->LoadData (cml, "chemical/x-cml");
	gcugtk::Molecule *mol = static_cast < gcugtk::Molecule * > (Doc->GetMol ());
	if (mol && mol->GetChildrenNumber ())
		static_cast < Chem3dWindow * > (Doc->GetWindow ())->AddMoleculeMenus (static_cast < gcugtk::Molecule * > (mol));
}

void Chem3dWindowPrivate::ShowInChIKey (G_GNUC_UNUSED GtkWidget *widget, Chem3dWindow *Win)
{
	gcugtk::Chem3dDoc *Doc  = Win->GetDocument ();
	gcu::Molecule *Mol = Doc->GetMol ();

	new gcugtk::StringDlg (Doc, Mol->GetInChIKey (), gcugtk::StringDlg::INCHIKEY);
}

void Chem3dWindowPrivate::ShowInChI (G_GNUC_UNUSED GtkWidget *widget, Chem3dWindow *Win)
{
	Chem3dDoc *Doc  = Win->GetDocument ();
	gcu::Molecule *Mol = Doc->GetMol ();

	new gcugtk::StringDlg (Doc, Mol->GetInChI (), gcugtk::StringDlg::INCHI);
}

void Chem3dWindowPrivate::ShowSMILES (G_GNUC_UNUSED GtkWidget *widget, Chem3dWindow *Win)
{
	Chem3dDoc *Doc  = Win->GetDocument ();
	gcu::Molecule *Mol = Doc->GetMol ();

	new gcugtk::StringDlg (Doc, Mol->GetSMILES (), gcugtk::StringDlg::SMILES);
}

void Chem3dWindowPrivate::OnOpen2D (G_GNUC_UNUSED GtkWidget *widget, Chem3dWindow *Win)
{
	gcu::Molecule *mol = Win->GetDocument ()->GetMol ();
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

void Chem3dWindowPrivate::OnOpenCalc (G_GNUC_UNUSED GtkWidget *widget, Chem3dWindow *Win)
{
	gcu::Molecule *mol = Win->GetDocument ()->GetMol ();
	std::ostringstream ofs;
	ofs << "gchemcalc-" API_VERSION " " << mol->GetRawFormula ();
	g_spawn_command_line_async (ofs.str ().c_str (), NULL);
}

void Chem3dWindowPrivate::Save (G_GNUC_UNUSED GtkWidget* widget, Chem3dWindow* Win)
{
	Win->Save ();
}

//Callbacks
static bool on_delete_event (G_GNUC_UNUSED GtkWidget *widget, G_GNUC_UNUSED GdkEvent *event, Chem3dWindow *Win)
{
	delete Win;
	return false;
}

static void on_file_open (G_GNUC_UNUSED GtkWidget *widget, Chem3dWindow *Win)
{
	static_cast < Chem3dApplication * > (Win->GetApplication ())->OnFileOpen (Win->GetDocument ());
}

static void on_file_save_as_image(G_GNUC_UNUSED GtkWidget *widget, Chem3dWindow *Win)
{
	static_cast < Chem3dApplication * > (Win->GetApplication ())->OnSaveAsImage (Win->GetDocument ());
}

static void on_file_close (G_GNUC_UNUSED GtkWidget *widget, Chem3dWindow *Win)
{
	gtk_widget_destroy (GTK_WIDGET (Win->GetWindow ()));
	delete Win;
}

static void on_page_setup (G_GNUC_UNUSED GtkWidget *widget, Chem3dWindow *Win)
{
	new gcugtk::PrintSetupDlg (Win->GetApplication (), Win->GetView ());
}

static void on_print_preview (G_GNUC_UNUSED GtkWidget *widget, Chem3dWindow *Win)
{
	Win->GetView ()->Print (true);
}

static void on_file_print (G_GNUC_UNUSED GtkWidget *widget, Chem3dWindow *Win)
{
	Win->GetView ()->Print (false);
}

static void on_quit (G_GNUC_UNUSED GtkWidget *widget, Chem3dWindow *Win)
{
	static_cast < Chem3dApplication * > (Win->GetApplication ())->OnQuit ();
}

static void on_color_changed (GOActionComboColor *combo, Chem3dWindow *window)
{
	GOColor color = go_action_combo_color_get_color (combo, FALSE);
	Chem3dView *View = window->GetView ();
	View->SetRed (GO_COLOR_DOUBLE_R (color));
	View->SetGreen (GO_COLOR_DOUBLE_G (color));
	View->SetBlue (GO_COLOR_DOUBLE_B (color));
	View->SetAlpha (GO_COLOR_DOUBLE_A (color));
	View->Update ();
}

static void on_display (GtkRadioAction *action, G_GNUC_UNUSED GtkRadioAction *current, G_GNUC_UNUSED Chem3dWindow *window)
{
	window->GetDocument ()->SetDisplay3D (static_cast <gcu::Display3DMode> (gtk_radio_action_get_current_value (action)));
	window->GetView ()->Update ();
}

static void on_help (G_GNUC_UNUSED GtkWidget *widget, Chem3dWindow *window)
{
	window->GetApplication ()->OnHelp ();
}

static void on_web (G_GNUC_UNUSED GtkWidget *widget, Chem3dWindow *window)
{
	window->GetApplication ()->OnWeb (window->GetScreen ());
}

static void on_mail (G_GNUC_UNUSED GtkWidget *widget, Chem3dWindow *window)
{
	window->GetApplication ()->OnMail (window->GetScreen ());
}

static void on_bug (G_GNUC_UNUSED GtkWidget *widget, Chem3dWindow *window)
{
	window->GetApplication ()->OnBug (window->GetScreen ());
}

static void on_live_assistance (G_GNUC_UNUSED GtkWidget *widget, Chem3dWindow *window)
{
	window->GetApplication ()->OnLiveAssistance (window->GetScreen ());
}

static void on_about (G_GNUC_UNUSED GtkWidget *widget, G_GNUC_UNUSED Chem3dWindow *window)
{
	char const *authors[] = {"Jean Bréfort", NULL};
	char const *comments = _("GChem3D is a molecular structures viewer for Gnome");
	/* char const *documentors[] = {NULL}; */
	char const *copyright = _("Copyright © 2004-2010 Jean Bréfort\n");
	char const *license =
		"This program is free software; you can redistribute it and/or\n"
		"modify it under the terms of the GNU General Public License as\n"
		"published by the Free Software Foundation; either version 3 of the\n"
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
	char const *translator_credits = _("translator_credits");
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

static void on_recent (GtkRecentChooser *widget, Chem3dWindow *Win)
{
	Chem3dApplication *App = static_cast < Chem3dApplication * > (Win->GetApplication ());
	Chem3dDoc *doc = Win->GetDocument ();
	GtkRecentInfo *info = gtk_recent_chooser_get_current_item (widget);
	App->FileProcess (gtk_recent_info_get_uri (info), gtk_recent_info_get_mime_type (info), false, NULL, doc);
	gtk_recent_info_unref(info);
	Molecule *mol = static_cast < gcugtk::Molecule * > (doc->GetMol ());
	if (mol && mol->GetChildrenNumber ())
		Win->AddMoleculeMenus (mol);
}

static GtkActionEntry entries[] = {
  { "FileMenu", NULL, N_("_File"), NULL, NULL, NULL },
	  { "Open", GTK_STOCK_OPEN, N_("_Open..."), "<control>O",
		  N_("Open a file"), G_CALLBACK (on_file_open) },
	  { "Save", GTK_STOCK_SAVE, N_("_Save"), "<control>S",
		  N_("Save the current settings"), G_CALLBACK (Chem3dWindowPrivate::Save) },
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
	  { "ImportMol", NULL, N_("_Import molecule..."), NULL,
		  N_("Import a molecule either from InChI or SMILES"), G_CALLBACK (Chem3dWindowPrivate::ImportMolecule) },
	  { "GChemPaint", NULL, N_("Open in GChemPaint"), NULL,
		  N_("Open a 2D model for this molecule using GChemPaint"), G_CALLBACK (Chem3dWindowPrivate::OnOpen2D) },
	  { "GChemCalc", NULL, N_("Open in GChemCalc"), NULL,
		  N_("Open the raw formula in the chemical calculator"), G_CALLBACK (Chem3dWindowPrivate::OnOpenCalc) },
	  { "InChI", NULL, N_("Show InChI"), NULL,
		  N_("Show the InChI for this molecule"), G_CALLBACK (Chem3dWindowPrivate::ShowInChI) },
	  { "InChIKey", NULL, N_("Show InChiKey"), NULL,
		  N_("Show the InChIKey for this molecule"), G_CALLBACK (Chem3dWindowPrivate::ShowInChIKey) },
	  { "SMILES", NULL, N_("Show SMILES"), NULL,
		  N_("Show the SMILES for this molecule"), G_CALLBACK (Chem3dWindowPrivate::ShowSMILES) },
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
"      <placeholder name='file1'/>"
"      <menuitem action='SaveAsImage'/>"
"	   <separator name='file-sep1'/>"
"      <menuitem action='PageSetup'/>"
"      <menuitem action='PrintPreview'/>"
"      <menuitem action='Print'/>"
"	   <separator name='file-sep2'/>"
"      <menuitem action='Close'/>"
"	   <placeholder name='file2'/>"
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

Chem3dWindow::Chem3dWindow (Application *app, Chem3dDoc *doc, char const *extra_ui):
Window (),
m_Application (app),
m_Document (doc),
m_View (NULL)
{
	GtkWidget *grid, *bar;
	GtkActionGroup *action_group;
	GtkAccelGroup *accel_group;
	GError *error = NULL;

	m_Window = GTK_WINDOW (gtk_window_new (GTK_WINDOW_TOPLEVEL));
	gtk_window_set_default_size (m_Window, 250, 280);
	gtk_window_set_icon_name (m_Window, app->GetIconName ().c_str ());
	g_signal_connect (G_OBJECT (m_Window), "delete-event", G_CALLBACK (on_delete_event), this);

	grid = gtk_grid_new ();
	g_object_set (G_OBJECT (grid), "orientation", GTK_ORIENTATION_VERTICAL, NULL);
	gtk_container_add (GTK_CONTAINER (m_Window), grid);

	m_UIManager = new gcugtk::UIManager (gtk_ui_manager_new ());
	GtkUIManager *manager = static_cast < gcugtk::UIManager * > (m_UIManager)->GetUIManager ();
	action_group = gtk_action_group_new ("MenuActions");
	gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);
	gtk_action_group_add_actions (action_group, entries, G_N_ELEMENTS (entries), this);
	GOActionComboColor *combo = go_action_combo_color_new ("Background", "gcu_Background", "", GO_COLOR_BLACK, NULL);
	g_object_set (G_OBJECT (combo), "label", _("Background color"), "tooltip",
		_("Choose a new background color"), NULL);
	g_signal_connect (G_OBJECT (combo), "activate", G_CALLBACK (on_color_changed), this);
	gtk_action_group_add_action (action_group, GTK_ACTION (combo));
	gtk_action_group_add_radio_actions (action_group, radios, G_N_ELEMENTS (radios), 0, G_CALLBACK (on_display), this);
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
	// add database access menus
	GtkWidget *menu = gtk_ui_manager_get_widget (manager, "/MainMenu/FileMenu/Open");
	GtkWidget *w = gtk_recent_chooser_menu_new_for_manager (app->GetRecentManager ());
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
	bar = gtk_ui_manager_get_widget (manager, "/MainMenu");
	gtk_container_add (GTK_CONTAINER (grid), bar);
	m_View = static_cast < Chem3dView * > (doc->GetView ());
	m_View->SetWindow (this);
	g_object_set (G_OBJECT (m_View->GetWidget ()), "expand", true, NULL);
	gtk_container_add (GTK_CONTAINER (grid), m_View->GetWidget ());
	switch (doc->GetDisplay3D ()) {
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

Chem3dWindow::~Chem3dWindow ()
{
	delete m_Document;
}

static const char *ui_mol_description =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu action='ToolsMenu'>"
"	   <separator name='tools-sep1'/>"
"      <menuitem action='GChemPaint'/>"
"      <menuitem action='GChemCalc'/>"
"      <menuitem action='InChI'/>"
"      <menuitem action='InChIKey'/>"
"      <menuitem action='SMILES'/>"
"    </menu>"
"  </menubar>"
"</ui>";

void Chem3dWindow::AddMoleculeMenus (Molecule *mol)
{
	GtkUIManager *manager = static_cast < gcugtk::UIManager * > (m_UIManager)->GetUIManager ();
	gtk_ui_manager_add_ui_from_string (manager, ui_mol_description, -1, NULL);
	mol->BuildDatabasesMenu (manager, "<ui><menubar name='MainMenu'><menu action='ToolsMenu'>", "</menu></menubar></ui>");
}

void Chem3dWindow::Save ()
{
}

}	// namespace gcugtk
