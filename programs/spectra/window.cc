// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * programs/spectra/window.cc
 *
 * Copyright (C) 2007 Jean Bréfort <jean.brefort@normalesup.org>
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
#include <libgnomevfs/gnome-vfs-ops.h>
#include <libgnomevfs/gnome-vfs-utils.h>
#include <gtk/gtkbox.h>
#include <gtk/gtkcontainer.h>
#include <gtk/gtkwindow.h>
#include <glib/gi18n.h>

//Callbacks
static bool on_delete_event (GtkWidget* widget, GdkEvent *event, gsvWindow* Win)
{
	delete Win;
	return false;
}

static void on_file_open (GtkWidget *widget, gsvWindow* Win)
{
	Win->OnFileOpen ();
}

static void on_file_save_as_image(GtkWidget* widget, gsvWindow* Win)
{
	Win->GetApp ()->OnSaveAsImage (Win->GetDoc ());
}

static void on_file_close (GtkWidget *widget, gsvWindow* Win)
{
	Win->OnFileClose ();
}

static void on_file_print (GtkWidget *widget, gsvWindow* Win)
{
	Win->OnFilePrint ();
}

static void on_quit (GtkWidget *widget, gsvWindow* Win)
{
	gsvApplication *App = Win->GetApp ();
	App->OnQuit ();
}

static void on_help (GtkWidget *widget, gsvWindow* window)
{
	window->GetApp ()->OnHelp ();
}

static void on_web (GtkWidget *widget, gsvWindow* window)
{
	window->GetApp ()->OnWeb ();
}

static void on_mail (GtkWidget *widget, gsvWindow* window)
{
	window->GetApp ()->OnMail ();
}

static void on_bug (GtkWidget *widget, gsvWindow* window)
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
	const gchar * comments = _("GSpectrum is a spectrum viewer for Gnome");
	/* const gchar * documentors[] = {NULL}; */
	const gchar * copyright = _("Copyright © 2007 Jean Bréfort\n");
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
	                       "name", "GSpectrum",
	                       "authors", authors,
	                       "comments", comments,
	                       "copyright", copyright,
	                       "license", license,
	                       "translator_credits", translator_credits,
	                       "version", VERSION,
	                       "website", "http://www.nongnu.org/gchemutils",
	                       NULL);
}

static void on_recent (GtkRecentChooser *widget, gsvWindow *Win)
{
	gsvApplication *App = Win->GetApp ();
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
		  N_("Quit GSpectrum"), G_CALLBACK (on_quit) },
  { "HelpMenu", NULL, N_("_Help") },
	  { "Help", GTK_STOCK_HELP, N_("_Contents"), "F1",
		  N_("View help for the Spetra Viewer"), G_CALLBACK (on_help) },
	  { "Web", NULL, N_("Gnome Chemistry Utils on the _web"), NULL,
		  N_("Browse the Gnome Chemistry Utils's web site"), G_CALLBACK (on_web) },
	  { "Mail", NULL, N_("_Ask a question"), NULL,
		  N_("Ask a question about the Gnome Chemistry Utils"), G_CALLBACK (on_mail) },
	  { "Bug", NULL, N_("Report _Bugs"), NULL,
		  N_("Submit a bug report for the Gnome Chemistry Utils"), G_CALLBACK (on_bug) },
	  { "About", GTK_STOCK_ABOUT, N_("_About"), NULL,
		  N_("About GSpectrum"), G_CALLBACK (on_about) }
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

gsvWindow::gsvWindow (gsvApplication *App, gsvDocument *Doc)
{
	m_App = App;
	m_Doc = Doc;
	m_Window = GTK_WINDOW (gtk_window_new (GTK_WINDOW_TOPLEVEL));
	gtk_window_set_default_size (m_Window, 600, 400);
	g_signal_connect (G_OBJECT (m_Window), "delete-event", G_CALLBACK (on_delete_event), this);

	GtkWidget *vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (m_Window), vbox);
	GtkUIManager *ui_manager = gtk_ui_manager_new ();
	GtkActionGroup *action_group = gtk_action_group_new ("MenuActions");
	gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);
	gtk_action_group_add_actions (action_group, entries, G_N_ELEMENTS (entries), this);
	gtk_ui_manager_insert_action_group (ui_manager, action_group, 0);
	GtkAccelGroup *accel_group = gtk_ui_manager_get_accel_group (ui_manager);
	gtk_window_add_accel_group (GTK_WINDOW (m_Window), accel_group);
	GError *error = NULL;
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
	gtk_recent_filter_add_mime_type (filter, "chemical/x-jcamp-dx");
	gtk_recent_chooser_add_filter (GTK_RECENT_CHOOSER (w), filter);
	g_signal_connect (G_OBJECT (w), "item-activated", G_CALLBACK (on_recent), this);
	GtkWidget *item = gtk_menu_item_new_with_label (_("Open recent"));
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), w);
	gtk_widget_show_all (item);
	gtk_menu_shell_insert (GTK_MENU_SHELL (gtk_widget_get_parent (menu)), item, 2);
	GtkWidget *bar = gtk_ui_manager_get_widget (ui_manager, "/MainMenu");
	gtk_box_pack_start (GTK_BOX (vbox), bar, FALSE, FALSE, 0);
	m_View = dynamic_cast<gsvView *> (m_Doc->GetView ());
	m_View->SetWindow (this);
//	gtk_container_add (GTK_CONTAINER (vbox), m_View->GetWidget ());
	gtk_widget_show_all (GTK_WIDGET (m_Window));
}

gsvWindow::~gsvWindow ()
{
	delete m_Doc;
}

void gsvWindow::OnFileOpen ()
{
	m_App->OnFileOpen (m_Doc);
}

void gsvWindow::OnFileClose ()
{
	gtk_widget_destroy (GTK_WIDGET (m_Window));
	delete this;
}

void gsvWindow::OnFilePrint ()
{
}

void gsvWindow::SetTitle (string const &title)
{
	gtk_window_set_title (m_Window, title.c_str ());
}
