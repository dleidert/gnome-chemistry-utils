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

static void on_about (GtkWidget *widget, void *data)
{
	char * authors[] = {"Jean Bréfort", NULL};
//	char * documentors[] = {NULL};
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
					"name", "GChem3D",
					"authors", authors,
					"comments", _("GChem3D is a molecular structures viewer for Gnome"),
					"copyright", _("(C) 2004-2006 by Jean Bréfort"),
					"license", license,
					"translator_credits", strcmp (translator_credits, "translator_credits") != 0 ? 
											(const char *)translator_credits : NULL,
					"version", VERSION,
					"website", "http://www.nongnu.org/gchemutils",
					NULL);
}

static GtkActionEntry entries[] = {
  { "FileMenu", NULL, N_("_File") },
	  { "Open", GTK_STOCK_OPEN, N_("_Open..."), "<control>O",
		  N_("Open a file"), G_CALLBACK (on_file_open) },
	  { "Print", GTK_STOCK_OPEN, N_("_Print..."), "<control>P",
		  N_("Print the current scene"), G_CALLBACK (on_file_print) },
	  { "Close", GTK_STOCK_CLOSE, N_("_Close"), "<control>W",
		  N_("Close the current file"), G_CALLBACK (on_file_close) },
 	  { "Quit", GTK_STOCK_QUIT, N_("_Quit"), "<control>Q",
		  N_("Quit GChem3D"), G_CALLBACK (on_quit) },
  { "ViewMenu", NULL, N_("_View") },
  { "HelpMenu", NULL, N_("_Help") },
	  { "About", NULL, N_("_About"), NULL,
		  N_("About GChem3D"), G_CALLBACK (on_about) }
};

static GtkRadioActionEntry radios[] = {
	{	"BallnStick", NULL, N_("Balls and sticks"), NULL,
		N_("Display a balls and sticks model"),
		0	},
	{	"SpaceFill", "NULL", N_("Space filling"), NULL,
		N_("Display a space filling model"),
		1	},
};

static const char *ui_description =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu action='FileMenu'>"
"      <menuitem action='Open'/>"
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
	gtk_window_set_default_size (m_Window, 200, 230);
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