// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * programs/spectra/window.cc
 *
 * Copyright (C) 2007-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include <gcugtk/print-setup-dlg.h>
#include <gsf/gsf-output-memory.h>
#include <glib/gi18n.h>

using namespace std;

//Callbacks
static bool on_delete_event (G_GNUC_UNUSED GtkWidget* widget, G_GNUC_UNUSED GdkEvent *event, gsvWindow* Win)
{
	delete Win;
	return false;
}

static void on_file_open (G_GNUC_UNUSED GtkWidget *widget, gsvWindow* Win)
{
	Win->OnFileOpen ();
}

static void on_file_save_as_image(G_GNUC_UNUSED GtkWidget* widget, gsvWindow* Win)
{
	Win->GetApp ()->OnSaveAsImage (Win->GetDoc ());
}

static void on_file_close (G_GNUC_UNUSED GtkWidget *widget, gsvWindow* Win)
{
	Win->OnFileClose ();
}

static void on_page_setup (G_GNUC_UNUSED GtkWidget *widget, gsvWindow* Win)
{
	Win->OnPageSetup ();
}

static void on_print_preview (G_GNUC_UNUSED GtkWidget *widget, gsvWindow* Win)
{
	Win->GetDoc ()->Print (true);
}

static void on_file_print (G_GNUC_UNUSED GtkWidget *widget, gsvWindow* Win)
{
	Win->GetDoc ()->Print (false);
}

static void on_quit (G_GNUC_UNUSED GtkWidget *widget, gsvWindow* Win)
{
	gsvApplication *App = Win->GetApp ();
	App->OnQuit ();
}

static void on_copy (G_GNUC_UNUSED GtkWidget* widget, gsvWindow* Win)
{
	Win->OnCopy ();
}

static void on_help (G_GNUC_UNUSED GtkWidget *widget, gsvWindow* window)
{
	window->GetApp ()->OnHelp ();
}

static void on_web (G_GNUC_UNUSED GtkWidget *widget, gsvWindow* window)
{
	window->GetApp ()->OnWeb ();
}

static void on_live_assistance (G_GNUC_UNUSED GtkWidget *widget, gsvWindow *Win)
{
	Win->GetApp ()->OnLiveAssistance ();
}

static void on_mail (G_GNUC_UNUSED GtkWidget *widget, gsvWindow* window)
{
	window->GetApp ()->OnMail ();
}

static void on_bug (G_GNUC_UNUSED GtkWidget *widget, gsvWindow* window)
{
	window->GetApp ()->OnBug ();
}

static void on_about (G_GNUC_UNUSED GtkWidget *widget, G_GNUC_UNUSED gsvWindow *Win)
{
	const gchar * authors[] = {"Jean Bréfort", NULL};
	const gchar * comments = _("GSpectrum is a spectrum viewer for Gnome");
	/* const gchar * documentors[] = {NULL}; */
	const gchar * copyright = _("Copyright © 2007-2010 Jean Bréfort\n");
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
	                       "program-name", "GSpectrum",
	                       "authors", authors,
	                       "comments", comments,
	                       "copyright", copyright,
	                       "license", license,
	                       "translator_credits", translator_credits,
	                       "version", VERSION,
	                       "website", "http://gchemutils.nongnu.org",
	                       NULL);
}

static void on_recent (GtkRecentChooser *widget, gsvWindow *Win)
{
	gsvApplication *App = Win->GetApp ();
	GtkRecentInfo *info = gtk_recent_chooser_get_current_item (widget);
	App->FileProcess (gtk_recent_info_get_uri (info), gtk_recent_info_get_mime_type (info), false, NULL, Win->GetDoc ());
	gtk_recent_info_unref(info);
}


static bool on_motion (GOGraphWidget *widget, GdkEventMotion *event, G_GNUC_UNUSED gpointer data)
{
	GogChart *chart = go_graph_widget_get_chart (widget);
	GogRenderer *rend = go_graph_widget_get_renderer (widget);
	GogView *graph_view;
	GogAxis *x_axis, *y_axis;
	g_object_get (G_OBJECT (rend), "view", &graph_view, NULL);
	GSList *l = gog_object_get_children (GOG_OBJECT (chart), gog_object_find_role_by_name (GOG_OBJECT (chart), "Plot"));
	GogPlot *plot = reinterpret_cast <GogPlot *> (l->data);
	g_slist_free (l);
	GogView *view = gog_view_find_child_view (graph_view, GOG_OBJECT (plot));
	l = gog_chart_get_axes (chart, GOG_AXIS_X);
	x_axis = GOG_AXIS (l->data);
	g_slist_free (l);
	l = gog_chart_get_axes (chart, GOG_AXIS_Y);
	// we need to use the first y axis, which comes last in the list, is this a kludge?
	GSList *ptr;
	for (ptr = l; ptr && ptr->next; ptr = ptr->next);
	y_axis = GOG_AXIS (ptr->data);
	g_slist_free (l);
    GogChartMap *map = gog_chart_map_new (chart,
            &(view->allocation), x_axis, y_axis, NULL, FALSE);
	if (gog_chart_map_is_valid (map) &&
			event->x >= view->allocation.x && event->x < view->allocation.x + view->allocation.w &&
			event->y >= view->allocation.y && event->y < view->allocation.y + view->allocation.h) {
		GogAxisMap *x_map = gog_chart_map_get_axis_map (map, 0);
		GogAxisMap *y_map = gog_chart_map_get_axis_map (map, 1);
		double x = gog_axis_map_from_view (x_map, event->x);
		double y = gog_axis_map_from_view (y_map, event->y);
		char *buf = g_strdup_printf ("x=%g y=%g", x, y);
		gtk_widget_set_tooltip_text (GTK_WIDGET (widget), buf);
		g_free (buf);
	} else
		gtk_widget_set_tooltip_text (GTK_WIDGET (widget), "");
	gog_chart_map_free (map);
	g_object_unref (G_OBJECT (graph_view));
	return true;
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
		  N_("Print the current file"), G_CALLBACK (on_file_print) },
	  { "Close", GTK_STOCK_CLOSE, N_("_Close"), "<control>W",
		  N_("Close the current file"), G_CALLBACK (on_file_close) },
 	  { "Quit", GTK_STOCK_QUIT, N_("_Quit"), "<control>Q",
		  N_("Quit GSpectrum"), G_CALLBACK (on_quit) },
  { "EditMenu", NULL, N_("_Edit"), NULL, NULL, NULL },
	  { "Copy", GTK_STOCK_COPY, N_("_Copy"), "<control>C",
		  N_("Copy the selection"), G_CALLBACK (on_copy) },
  { "HelpMenu", NULL, N_("_Help"), NULL, NULL, NULL },
	  { "Help", GTK_STOCK_HELP, N_("_Contents"), "F1",
		  N_("View help for the Spectra Viewer"), G_CALLBACK (on_help) },
	  { "Web", NULL, N_("Gnome Chemistry Utils on the _web"), NULL,
		  N_("Browse the Gnome Chemistry Utils's web site"), G_CALLBACK (on_web) },
	  { "LiveAssistance", NULL, N_("Live assistance"), NULL,
		  N_("Open the Gnome Chemistry Utils IRC channel"), G_CALLBACK (on_live_assistance) },
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
"      <menuitem action='Mail'/>"
"      <menuitem action='Web'/>"
"      <menuitem action='LiveAssistance'/>"
"      <menuitem action='Bug'/>"
"      <menuitem action='About'/>"
"    </menu>"
"  </menubar>"
"</ui>";

gsvWindow::gsvWindow (gsvApplication *App, gsvDocument *Doc)
{
	m_App = App;
	m_Doc = Doc;
	m_Window = GTK_WINDOW (gtk_window_new (GTK_WINDOW_TOPLEVEL));
	gtk_window_set_default_size (m_Window, 600, 400);
	gtk_window_set_icon_name (m_Window, App->GetIconName ().c_str ());
	g_signal_connect (G_OBJECT (m_Window), "delete-event", G_CALLBACK (on_delete_event), this);

	GtkWidget *grid = gtk_grid_new ();
	g_object_set (G_OBJECT (grid), "orientation", GTK_ORIENTATION_VERTICAL, NULL);
	gtk_container_add (GTK_CONTAINER (m_Window), grid);
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
	GtkWidget *menu = gtk_ui_manager_get_widget (ui_manager, "/MainMenu/FileMenu/Open");
	GtkWidget *w = gtk_recent_chooser_menu_new_for_manager (App->GetRecentManager ());
	gtk_recent_chooser_set_sort_type (GTK_RECENT_CHOOSER (w), GTK_RECENT_SORT_MRU);
	GtkRecentFilter *filter = gtk_recent_filter_new ();
	std::list<std::string> &mime_types = App->GetSupportedMimeTypes ();
	std::list<std::string>::iterator it, end = mime_types.end ();
	for (it = mime_types.begin (); it != end; it++)
		gtk_recent_filter_add_mime_type (filter, (*it).c_str ());
	gtk_recent_chooser_add_filter (GTK_RECENT_CHOOSER (w), filter);
	g_signal_connect (G_OBJECT (w), "item-activated", G_CALLBACK (on_recent), this);
	GtkWidget *item = gtk_menu_item_new_with_label (_("Open recent"));
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), w);
	gtk_widget_show_all (item);
	gtk_menu_shell_insert (GTK_MENU_SHELL (gtk_widget_get_parent (menu)), item, 2);
	GtkWidget *bar = gtk_ui_manager_get_widget (ui_manager, "/MainMenu");
	gtk_container_add (GTK_CONTAINER (grid), bar);
	m_View = dynamic_cast<gsvView *> (m_Doc->GetView ());
	m_View->SetWindow (this);
	gtk_container_add (GTK_CONTAINER (grid), m_View->GetOptionBox ());
	w = m_View->GetWidget ();
	g_object_set (G_OBJECT (w), "margin", 6, "expand", true, NULL);
	g_signal_connect (G_OBJECT (w), "motion-notify-event", G_CALLBACK (on_motion), NULL);
	gtk_container_add (GTK_CONTAINER (grid), w);
	gtk_widget_show_all (GTK_WIDGET (m_Window));
	g_object_unref (ui_manager);
	// Initialize print settings
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
	new gcugtk::PrintSetupDlg (m_App, m_Doc);
}

static GtkTargetEntry const targets[] = {
	{(char *) "application/x-goffice-graph",  0, 0},
	{(char *) "image/svg+xml", 0, 2},
	{(char *) "image/svg", 0, 1},
	{(char *) "image/png", 0, 3}
};

static void on_get_data (G_GNUC_UNUSED GtkClipboard *clipboard, GtkSelectionData *selection_data,  guint info, GogGraph *graph)
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
			gog_object_write_xml_sax (GOG_OBJECT (graph), xout, NULL);
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
					gtk_selection_data_get_target (selection_data), 8,
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
