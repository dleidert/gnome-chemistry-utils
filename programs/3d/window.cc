// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * programs/3d/window.cc 
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include "config.h"
#include "application.h"
#include "document.h"
#include "view.h"
#include "window.h"
#include <goffice/gtk/go-action-combo-color.h>
#include <libgnomeprint/gnome-print-job.h>
#include <libgnomeprintui/gnome-print-dialog.h>
#include <libgnomeprintui/gnome-print-job-preview.h>
#include <libgnomevfs/gnome-vfs.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>

//Callbacks
static bool on_delete_event (GtkWidget* widget, GdkEvent *event, gc3dWindow* Win)
{
	delete Win;
	return false;
}

static void on_file_open (GtkWidget *widget, gc3dWindow* Win)
{
	Win->OnFileOpen ();
}

static void on_file_save_as_image(GtkWidget* widget, gc3dWindow* Win)
{
	Win->GetApp ()->OnSaveAsImage (Win->GetDoc ());
}

static void on_file_close (GtkWidget *widget, gc3dWindow* Win)
{
	Win->OnFileClose ();
}

static void on_file_print (GtkWidget *widget, gc3dWindow* Win)
{
	Win->OnFilePrint ();
}

static void on_quit (GtkWidget *widget, gc3dWindow* Win)
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

static void on_help (GtkWidget *widget, gc3dWindow* window)
{
	window->GetApp ()->OnHelp ();
}

static void on_web (GtkWidget *widget, gc3dWindow* window)
{
	window->GetApp ()->OnWeb ();
}

static void on_mail (GtkWidget *widget, gc3dWindow* window)
{
	window->GetApp ()->OnMail ();
}

static void on_bug (GtkWidget *widget, gc3dWindow* window)
{
	window->GetApp ()->OnBug ();
}

static void on_about_activate_url (GtkAboutDialog *about, const gchar *url, gpointer data)
{
	GnomeVFSResult error = gnome_vfs_url_show(url);
	if (error != GNOME_VFS_OK) {
		g_print("GnomeVFSResult while trying to launch URL in about dialog: error %u\n", error);
	}
}

static void on_about (GtkWidget *widget, void *data)
{
	const gchar * authors[] = {"Jean Bréfort", NULL};
	const gchar * comments = _("GChem3D is a molecular structures viewer for Gnome");
	/* const gchar * documentors[] = {NULL}; */
	const gchar * copyright = _("Copyright © 2004-2007 Jean Bréfort\n");
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
	
	gtk_about_dialog_set_url_hook(on_about_activate_url, NULL, NULL);

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
		0 },
	{ "SpaceFill", "NULL", N_("Space filling"), NULL,
		N_("Display a space filling model"),
		1 },
};

static const char *ui_description =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu action='FileMenu'>"
"      <menuitem action='Open'/>"
"      <menuitem action='SaveAsImage'/>"
"	   <separator name='file-sep1'/>"
"      <menuitem action='Print'/>"
"      <menuitem action='Close'/>"
"      <menuitem action='Quit'/>"
"    </menu>"
"    <menu action='ViewMenu'>"
"      <menuitem action='BallnStick'/>"
"      <menuitem action='SpaceFill'/>"
"	   <separator name='view-sep1'/>"
"      <menuitem action='Background'/>"
"    </menu>"
"    <menu action='HelpMenu'>"
"      <menuitem action='Help'/>"
"      <placeholder name='mail'/>"
"      <placeholder name='web'/>"
"      <placeholder name='bug'/>"
"      <menuitem action='About'/>"
"    </menu>"
"  </menubar>"
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
	if (App->HasWebBrowser () && !gtk_ui_manager_add_ui_from_string (ui_manager, ui_web_description, -1, &error)) {
		g_message ("building menus failed: %s", error->message);
		g_error_free (error);
	}
	if (App->HasMailAgent () && !gtk_ui_manager_add_ui_from_string (ui_manager, ui_mail_description, -1, &error)) {
		g_message ("building menus failed: %s", error->message);
		g_error_free (error);
	}
	GtkWidget *menu = gtk_ui_manager_get_widget (ui_manager, "/MainMenu/FileMenu/Open");
	GtkWidget *w = gtk_recent_chooser_menu_new_for_manager (App->GetRecentManager ());
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

void gc3dWindow::OnFilePrint ()
{
	GnomePrintConfig* config = gnome_print_config_default ();
	GnomePrintContext *pc;
	GnomePrintJob *gpj = gnome_print_job_new (config);
	int do_preview = 0, copies = 1, collate = 0;
	GnomePrintDialog *gpd;
	gpd = GNOME_PRINT_DIALOG (gnome_print_dialog_new (gpj, (const guchar*) "Print test", GNOME_PRINT_DIALOG_COPIES));
	gnome_print_dialog_set_copies (gpd, copies, collate);
	switch (gtk_dialog_run (GTK_DIALOG (gpd)))
	{
	case GNOME_PRINT_DIALOG_RESPONSE_PREVIEW:
		do_preview = 1;
		break;
	case GNOME_PRINT_DIALOG_RESPONSE_CANCEL:
		gtk_widget_destroy (GTK_WIDGET (gpd));
		g_object_unref (gpj);
		gnome_print_config_unref (config);
		return;
	}
	gtk_widget_destroy (GTK_WIDGET (gpd));
	pc = gnome_print_job_get_context (gpj);
	gnome_print_beginpage (pc, (const guchar*)"");
	gdouble width, height;
	gnome_print_config_get_double (config, (const guchar*) GNOME_PRINT_KEY_PAPER_WIDTH, &width);
	gnome_print_config_get_double (config, (const guchar*) GNOME_PRINT_KEY_PAPER_HEIGHT, &height);
	m_View->Print (pc, width, height);
	gnome_print_showpage (pc);
	g_object_unref (pc);
	gnome_print_job_close (gpj);
	if (do_preview)
	{
		GtkWidget *preview = gnome_print_job_preview_new (gpj, (const guchar*) _("Preview"));
		gtk_widget_show (preview);
	} else {
		gnome_print_job_print (gpj);
	}
	g_object_unref (gpj);
	gnome_print_config_unref (config);
}

void gc3dWindow::SetTitle (char const *title)
{
	gtk_window_set_title (m_Window, title);
}
