// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * programs/3d/window.cc 
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include "config.h"
#include "application.h"
#include "document.h"
#include "view.h"
#include "window.h"
#include <gcu/print-setup-dlg.h>
#ifndef GOFFICE_HAS_GLOBAL_HEADER
#   include <goffice/gtk/go-action-combo-color.h>
#   include <goffice/utils/go-image.h>
#   include <gtk/gtk.h>
#else
#	include <gcu/goffice-compat.h>
#endif
#include <glib/gi18n.h>

//Callbacks
static bool on_delete_event (GtkWidget *widget, GdkEvent *event, gc3dWindow *Win)
{
	delete Win;
	return false;
}

static void on_file_open (GtkWidget *widget, gc3dWindow *Win)
{
	Win->OnFileOpen ();
}

static void on_file_save_as_image(GtkWidget *widget, gc3dWindow *Win)
{
	Win->GetApp ()->OnSaveAsImage (Win->GetDoc ());
}

static void on_file_close (GtkWidget *widget, gc3dWindow *Win)
{
	Win->OnFileClose ();
}

static void on_page_setup (GtkWidget *widget, gc3dWindow *Win)
{
	Win->OnPageSetup ();
}

static void on_print_preview (GtkWidget *widget, gc3dWindow *Win)
{
	Win->GetView ()->Print (true);
}

static void on_file_print (GtkWidget *widget, gc3dWindow *Win)
{
	Win->GetView ()->Print (false);
}

static void on_quit (GtkWidget *widget, gc3dWindow *Win)
{
	gc3dApplication *App = Win->GetApp ();
	App->OnQuit ();
}

static void on_color_changed (GOActionComboColor *combo, gc3dWindow *window)
{
	GOColor color = go_action_combo_color_get_color (combo, FALSE);
	gc3dView *View = window->GetView ();
	View->SetRed (DOUBLE_RGBA_R (color));
	View->SetGreen (DOUBLE_RGBA_G (color));
	View->SetBlue (DOUBLE_RGBA_B (color));
	View->SetAlpha (DOUBLE_RGBA_A (color));
	View->Update ();
}

static void on_display (GtkRadioAction *action, GtkRadioAction *current, gc3dWindow *window)
{
	window->GetDoc ()->SetDisplay3D (static_cast <Display3DMode> (gtk_radio_action_get_current_value (action)));
	window->GetView ()->Update ();
}

static void on_help (GtkWidget *widget, gc3dWindow *window)
{
	window->GetApp ()->OnHelp ();
}

static void on_web (GtkWidget *widget, gc3dWindow *window)
{
	window->GetApp ()->OnWeb ();
}

static void on_mail (GtkWidget *widget, gc3dWindow *window)
{
	window->GetApp ()->OnMail ();
}

static void on_bug (GtkWidget *widget, gc3dWindow *window)
{
	window->GetApp ()->OnBug ();
}

static void on_live_assistance (GtkWidget *widget, gc3dWindow *window)
{
	window->GetApp ()->OnLiveAssistance ();
}

static void on_about_activate_url (GtkAboutDialog *about, const gchar *url, gpointer data)
{
	reinterpret_cast <gc3dWindow *> (data)->GetApp ()->OnWeb (url);
}

static void on_about (GtkWidget *widget, gc3dWindow *window)
{
	const gchar * authors[] = {"Jean Bréfort", NULL};
	const gchar * comments = _("GChem3D is a molecular structures viewer for Gnome");
	/* const gchar * documentors[] = {NULL}; */
	const gchar * copyright = _("Copyright © 2004-2008 Jean Bréfort\n");
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
	
	gtk_about_dialog_set_url_hook (on_about_activate_url, window, NULL);

	/* Note to translators: replace the following string with the appropriate credits for you lang */
	const gchar * translator_credits = _("translator_credits");
	gtk_show_about_dialog (NULL,
	                       "name", "GChem3D",
	                       "authors", authors,
	                       "comments", comments,
	                       "copyright", copyright,
	                       "license", license,
	                       "translator_credits", translator_credits,
	                       "version", VERSION,
	                       "website", "http://www.nongnu.org/gchemutils",
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
  { "FileMenu", NULL, N_("_File") },
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
  { "ViewMenu", NULL, N_("_View") },
  { "HelpMenu", NULL, N_("_Help") },
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
		BALL_AND_STICK },
	{ "SpaceFill", "NULL", N_("Space filling"), NULL,
		N_("Display a space filling model"),
		SPACEFILL },
	{ "Cylinders", "NULL", N_("Cylinders"), NULL,
		N_("Display a cylinders model"),
		CYLINDERS },
	{ "Wireframe", "NULL", N_("Wireframe"), NULL,
		N_("Display a wireframe model"),
		WIREFRAME },
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
	GtkWidget *vbox, *bar;
	GtkUIManager *ui_manager;
	GtkActionGroup *action_group;
	GtkAccelGroup *accel_group;
	GError *error = NULL;

	m_Window = GTK_WINDOW (gtk_window_new (GTK_WINDOW_TOPLEVEL));
	gtk_window_set_default_size (m_Window, 250, 280);
	g_signal_connect (G_OBJECT (m_Window), "delete-event", G_CALLBACK (on_delete_event), this);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (m_Window), vbox);
	ui_manager = gtk_ui_manager_new ();
	action_group = gtk_action_group_new ("MenuActions");
	gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);
	gtk_action_group_add_actions (action_group, entries, G_N_ELEMENTS (entries), this);
	GOActionComboColor *combo = go_action_combo_color_new ("Background", "gcu_Background", "", RGBA_BLACK, NULL);
	g_object_set (G_OBJECT (combo), "label", _("Background color"), "tooltip",
		_("Choose a new background color"), NULL);
	g_signal_connect (G_OBJECT (combo), "activate", G_CALLBACK (on_color_changed), this);
	gtk_action_group_add_action (action_group, GTK_ACTION (combo));
	gtk_action_group_add_radio_actions (action_group, radios, G_N_ELEMENTS (radios), 0, G_CALLBACK (on_display), this);
	gtk_ui_manager_insert_action_group (ui_manager, action_group, 0);
	accel_group = gtk_ui_manager_get_accel_group (ui_manager);
	gtk_window_add_accel_group (GTK_WINDOW (m_Window), accel_group);
	error = NULL;
	if (!gtk_ui_manager_add_ui_from_string (ui_manager, ui_description, -1, &error)) {
		g_message ("building menus failed: %s", error->message);
		g_error_free (error);
		exit (EXIT_FAILURE);
	}
	GtkWidget *menu = gtk_ui_manager_get_widget (ui_manager, "/MainMenu/FileMenu/Open");
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
	bar = gtk_ui_manager_get_widget (ui_manager, "/MainMenu");
	gtk_box_pack_start (GTK_BOX (vbox), bar, FALSE, FALSE, 0);
	m_View = dynamic_cast<gc3dView *> (m_Doc->GetView ());
	m_View->SetWindow (this);
	gtk_container_add (GTK_CONTAINER (vbox), m_View->GetWidget ());
	switch (Doc->GetDisplay3D ()) {
	case BALL_AND_STICK:
		gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (gtk_action_group_get_action (action_group, "BallnStick")), true);
		break;
	case SPACEFILL:
		gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (gtk_action_group_get_action (action_group, "SpaceFill")), true);
		break;
	case CYLINDERS:
		gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (gtk_action_group_get_action (action_group, "Cylinders")), true);
		break;
	case WIREFRAME:
		gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (gtk_action_group_get_action (action_group, "Wireframe")), true);
		break;
	}
	gtk_widget_show_all (GTK_WIDGET (m_Window));
}

gc3dWindow::~gc3dWindow ()
{
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
	new PrintSetupDlg (m_App, m_View);
}

void gc3dWindow::SetTitle (char const *title)
{
	gtk_window_set_title (m_Window, title);
}
