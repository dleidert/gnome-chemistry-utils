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
#include <goffice/gtk/go-graph-widget.h>
#include <goffice/graph/gog-object-xml.h>
#include <goffice/utils/go-locale.h>
#include <goffice/utils/go-image.h>
#include <gsf/gsf-output-memory.h>
#include <libgnomevfs/gnome-vfs-ops.h>
#include <libgnomevfs/gnome-vfs-utils.h>
#include <gtk/gtkbox.h>
#include <gtk/gtkcontainer.h>
#include <gtk/gtkwindow.h>
#include <glib/gi18n.h>

using namespace std;

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

static void on_page_setup (GtkWidget *widget, gsvWindow* Win)
{
	Win->OnPageSetup ();
}

static void on_print_preview (GtkWidget *widget, gsvWindow* Win)
{
	Win->OnFilePrint (true);
}

static void on_file_print (GtkWidget *widget, gsvWindow* Win)
{
	Win->OnFilePrint (false);
}

static void on_quit (GtkWidget *widget, gsvWindow* Win)
{
	gsvApplication *App = Win->GetApp ();
	App->OnQuit ();
}

static void on_copy (GtkWidget* widget, gsvWindow* Win)
{
	Win->OnCopy ();
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
	  { "PageSetup", NULL, N_("Page Set_up..."), NULL,
		  N_("Setup the page settings for your current printer"), G_CALLBACK (on_page_setup) },
	  { "PrintPreview", GTK_STOCK_PRINT_PREVIEW, N_("Print Pre_view"), NULL,
		  N_("Print preview"), G_CALLBACK (on_print_preview) },
	  { "Print", GTK_STOCK_PRINT, N_("_Print..."), "<control>P",
		  N_("Print the current file"), G_CALLBACK (on_file_print) },
	  { "Close", GTK_STOCK_CLOSE, N_("_Close"), "<control>W",
		  N_("Close the current file"), G_CALLBACK (on_file_close) },
 	  { "Quit", GTK_STOCK_QUIT, N_("_Quit"), "<control>Q",
		  N_("Quit GSpectrum"), G_CALLBACK (on_quit) },
  { "EditMenu", NULL, N_("_Edit") },
	  { "Copy", GTK_STOCK_COPY, N_("_Copy"), "<control>C",
		  N_("Copy the selection"), G_CALLBACK (on_copy) },
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
"      <menuitem action='PageSetup'/>"
"      <menuitem action='PrintPreview'/>"
"      <menuitem action='Print'/>"
"	   <separator name='file-sep2'/>"
"      <menuitem action='Close'/>"
"      <menuitem action='Quit'/>"
"    </menu>"
"    <menu action='EditMenu'>"
"      <menuitem action='Copy'/>"
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
	gtk_container_add (GTK_CONTAINER (vbox), m_View->GetWidget ());
	gtk_widget_show_all (GTK_WIDGET (m_Window));
	// Initialize print settings
	m_PageSetup = gtk_page_setup_new ();
	m_PrintSettings = gtk_print_settings_new ();
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

void gsvWindow::OnPageSetup ()
{
	GtkPageSetup *setup = gtk_print_run_page_setup_dialog (
											m_Window,
											m_PageSetup,
											m_PrintSettings
										);
	g_object_unref (m_PageSetup);
	m_PageSetup = setup;
}

static void begin_print (GtkPrintOperation *print, GtkPrintContext *context, gpointer data)
{
	gtk_print_operation_set_n_pages (print, 1);
}

static void draw_page (GtkPrintOperation *print, GtkPrintContext *context, gint page_nr,gpointer data)
{
	((gsvWindow *) data)->DoPrint (print, context);
}

void gsvWindow::OnFilePrint (bool preview)
{
	GtkPrintOperation *print;
	GtkPrintOperationResult res;

	print = gtk_print_operation_new ();
	gtk_print_operation_set_use_full_page (print, false);

    gtk_print_operation_set_print_settings (print, m_PrintSettings);
    gtk_print_operation_set_default_page_setup (print, m_PageSetup);

	g_signal_connect (print, "begin_print", G_CALLBACK (begin_print), NULL);
	g_signal_connect (print, "draw_page", G_CALLBACK (draw_page), this);
	
	res = gtk_print_operation_run (print,
								   (preview)? GTK_PRINT_OPERATION_ACTION_PREVIEW:
								   GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,
								   m_Window, NULL);

	if (res == GTK_PRINT_OPERATION_RESULT_APPLY) {
		if (m_PrintSettings != NULL)
			g_object_unref (m_PrintSettings);
		m_PrintSettings = GTK_PRINT_SETTINGS (g_object_ref (gtk_print_operation_get_print_settings (print)));
	}

	g_object_unref (print);
}

void gsvWindow::DoPrint (GtkPrintOperation *print, GtkPrintContext *context)
{
	cairo_t *cr;
	gdouble width, height;

	cr = gtk_print_context_get_cairo_context (context);
	width = gtk_print_context_get_width (context);
	height = gtk_print_context_get_height (context);
	gog_graph_render_to_cairo (go_graph_widget_get_graph (GO_GRAPH_WIDGET (m_View->GetWidget ())), cr, width, height);
}

static GtkTargetEntry const targets[] = {
	{(char *) "application/x-goffice-graph",  0, 0},
	{(char *) "image/svg+xml", 0, 2},
	{(char *) "image/svg", 0, 1},
	{(char *) "image/png", 0, 3}
};

static void on_get_data (GtkClipboard *clipboard, GtkSelectionData *selection_data,  guint info, GogGraph *graph)
{
	guchar *buffer = NULL;
	char *format = NULL;
	GsfOutput *output;
	GsfOutputMemory *omem;
	gsf_off_t osize;
	GOImageFormat fmt = GO_IMAGE_FORMAT_UNKNOWN;
	double w, h;
	gog_graph_get_size (graph, &w, &h);
	output = gsf_output_memory_new ();
	omem   = GSF_OUTPUT_MEMORY (output);
	switch (info) {
	case 0: {
			GsfXMLOut *xout;
			char *old_num_locale, *old_monetary_locale;
		
			old_num_locale = g_strdup (go_setlocale (LC_NUMERIC, NULL));
			go_setlocale (LC_NUMERIC, "C");
			old_monetary_locale = g_strdup (go_setlocale (LC_MONETARY, NULL));
			go_setlocale (LC_MONETARY, "C");
			go_locale_untranslated_booleans ();
		
			xout = gsf_xml_out_new (output);
			gog_object_write_xml_sax (GOG_OBJECT (graph), xout);
			g_object_unref (xout);
		
			/* go_setlocale restores bools to locale translation */
			go_setlocale (LC_MONETARY, old_monetary_locale);
			g_free (old_monetary_locale);
			go_setlocale (LC_NUMERIC, old_num_locale);
			g_free (old_num_locale);
		}
		break;
	case 1:
	case 2:
		fmt = GO_IMAGE_FORMAT_SVG;
		break;
	case 3:
		fmt = GO_IMAGE_FORMAT_PNG;
		break;
	}
	/* FIXME Add a dpi editor. Default dpi to 150 for now */
	bool res = (fmt != GO_IMAGE_FORMAT_UNKNOWN)?
		gog_graph_export_image (graph, fmt, output, 150.0, 150.0):
		true;
	if (res) {
		osize = gsf_output_size (output);
				
		buffer = (guchar*) g_malloc (osize);
		memcpy (buffer, gsf_output_memory_get_bytes (omem), osize);
		gsf_output_close (output);
		g_object_unref (output);
		g_free (format);
		gtk_selection_data_set (selection_data,
					selection_data->target, 8,
					(guchar *) buffer, osize);
		g_free (buffer);
	}
}

void gsvWindow::OnCopy ()
{
	GtkClipboard* clipboard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
	gtk_clipboard_set_with_data (clipboard, targets, 4,
		(GtkClipboardGetFunc) on_get_data, NULL,
		go_graph_widget_get_graph (GO_GRAPH_WIDGET (m_View->GetWidget ())));
}

void gsvWindow::SetTitle (string const &title)
{
	gtk_window_set_title (m_Window, title.c_str ());
}
