/* 
 * Gnome Chemistry Utils
 * programs/gchem3d-viewer.c
 *
 * Copyright (C) 2004-2006
 *
 * Developed by Jean Bréfort <jean.brefort@normalesup.org>
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
#include <gcu/chemistry.h>
#include <gcu/gtkchem3dviewer.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <goffice/gtk/go-action-combo-color.h>
#include <libgnomevfs/gnome-vfs.h>
#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-job.h>
#include <libgnomeprintui/gnome-print-dialog.h>
#include <libgnomeprintui/gnome-print-job-preview.h>

/*!\file
A simple sample of the use of the GtkChem3DViewer widget.
*/

static void on_file_open (GtkWidget *widget, void *data)
{
	char* filename;
	GtkFileChooserDialog *dialog = (GtkFileChooserDialog*) gtk_file_chooser_dialog_new (
															_("Open"),
															GTK_WINDOW (data),
															GTK_FILE_CHOOSER_ACTION_OPEN,
															GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
															GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
															NULL);
	GtkFileChooser* chooser = GTK_FILE_CHOOSER (dialog);
	GtkFileFilter* filter = gtk_file_filter_new ();
	GtkChem3DViewer *viewer = GTK_CHEM3D_VIEWER (g_object_get_data (G_OBJECT (data), "viewer"));
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
	gtk_file_filter_add_mime_type (filter, "chemical/x-mdl-molfile");
	gtk_file_filter_add_mime_type (filter, "chemical/x-pdb");
	gtk_file_filter_add_pattern (filter, "*.pdb"); /* I don't know why mime type is not enough */
	gtk_file_filter_add_mime_type (filter, "chemical/x-xyz");
	gtk_file_chooser_set_filter (chooser, filter);
	gtk_file_chooser_set_local_only (chooser, FALSE);
	while (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		filename = gtk_file_chooser_get_uri (chooser);
		gtk_chem3d_viewer_set_uri (viewer, filename);
		break;
	}
	gtk_widget_destroy (GTK_WIDGET (dialog));
}

static void on_file_print (GtkWidget *widget, void *data)
{
	GnomePrintConfig* config = gnome_print_config_default ();
	GnomePrintContext *pc;
	GnomePrintJob *gpj = gnome_print_job_new (config);
	int do_preview = 0, copies = 1, collate = 0;
	GnomePrintDialog *gpd;
	GtkChem3DViewer *viewer = GTK_CHEM3D_VIEWER (g_object_get_data (G_OBJECT (data), "viewer"));
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
	gnome_print_config_get_double (config, GNOME_PRINT_KEY_PAPER_WIDTH, &width);
	gnome_print_config_get_double (config, GNOME_PRINT_KEY_PAPER_HEIGHT, &height);
	gtk_chem3d_viewer_print (viewer, pc, width, height);
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

static void on_quit (GtkWidget *widget, void *data)
{
	gtk_main_quit();
}

static void on_color_changed (GOActionComboColor *combo, GObject *obj)
{
	GOColor color = go_action_combo_color_get_color (combo, FALSE);
	char *scolor = g_strdup_printf ("#%02x%02x%02x", UINT_RGBA_R (color),
		UINT_RGBA_G (color), UINT_RGBA_B (color));
	g_object_set (obj, "bgcolor", scolor, NULL);
	g_free (scolor);
}

static void on_display (GtkRadioAction *action, GtkRadioAction *current, GObject *obj)
{
	g_object_set (obj, "display3d", gtk_radio_action_get_current_value (action), NULL);
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

void cb_print_version (const gchar *option_name, const gchar *value, gpointer data, GError **error)
{
	char *version = g_strconcat (_("GChem3d Viewer version: "), VERSION, NULL);
	puts (version);
	g_free (version);
	exit (0);
}

static char *bgcolor = NULL;
static char *display3d = NULL;

static GOptionEntry options[] = 
{
	{ "version", 'v', 0, G_OPTION_ARG_CALLBACK, (void*) cb_print_version, N_("Prints GChem3d Viewer version"), NULL },
	{ "bgcolor", 'b', 0, G_OPTION_ARG_STRING, &bgcolor, N_("Background color: white, black or #rrggbb (default is black)"), NULL },
	{ "display3d", 'd', 0, G_OPTION_ARG_STRING, &display3d, N_("How molecules are displayed; possible values are BallnStick (the default) and SpaceFill"), NULL },
	{ NULL }
};

int main(int argc, char *argv[])
{
	GtkWidget *window;
	GtkWidget *viewer, *vbox, *bar;
	GnomeVFSURI *uri, *auri;
	char *path, *dir;
	GtkUIManager *ui_manager;
	GtkActionGroup *action_group;
	GtkAccelGroup *accel_group;
	GError *error = NULL;
	GOptionContext *context;

	gtk_init (&argc, &argv);
	if (!gnome_vfs_init ()) {
		printf ("Could not initialize GnomeVFS\n");
		return 1;
	}

	gcu_element_load_databases ("radii", NULL);
	if (argc > 1 && argv[1][0] == '-') {
		context = g_option_context_new (_(" [file]"));
		g_option_context_add_main_entries (context, options, GETTEXT_PACKAGE);
		g_option_context_add_group (context, gtk_get_option_group (TRUE));
		g_option_context_set_help_enabled (context, TRUE);
		g_option_context_parse (context, &argc, &argv, &error);
		if (error) {
			puts (error->message);
			g_error_free (error);
			return -1;
		}
	}

	if (argc > 1) {
		path = g_get_current_dir ();
		dir = g_strconcat (path, "/", NULL);
		g_free (path);
		uri = gnome_vfs_uri_new (dir);
		auri = gnome_vfs_uri_resolve_relative (uri, argv[1]);
		path = gnome_vfs_uri_to_string (auri, GNOME_VFS_URI_HIDE_NONE);
		viewer = gtk_chem3d_viewer_new (path);
		g_free (path);
		gnome_vfs_uri_unref (auri);
		gnome_vfs_uri_unref (uri);
		g_free (dir);
	} else
		viewer = gtk_chem3d_viewer_new("");

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "GtkChem3dViewer");
	gtk_window_set_default_size (GTK_WINDOW(window), 200, 230);
	g_signal_connect(GTK_OBJECT(window), "destroy",
		GTK_SIGNAL_FUNC(gtk_main_quit),
		NULL);
	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (window), vbox);
	ui_manager = gtk_ui_manager_new ();
	action_group = gtk_action_group_new ("MenuActions");
	gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);
	gtk_action_group_add_actions (action_group, entries, G_N_ELEMENTS (entries), window);
	GOActionComboColor *combo = go_action_combo_color_new ("Background", "gcu_Background", "", RGBA_BLACK, NULL);
	g_object_set (G_OBJECT (combo), "label", _("Background color"), "tooltip",
		_("Choose a new background color"), NULL);
	g_signal_connect (G_OBJECT (combo), "activate", G_CALLBACK (on_color_changed), viewer);
	gtk_action_group_add_action (action_group, GTK_ACTION (combo));
	gtk_action_group_add_radio_actions (action_group, radios, G_N_ELEMENTS (radios), 0, G_CALLBACK (on_display), viewer);
	gtk_ui_manager_insert_action_group (ui_manager, action_group, 0);
	accel_group = gtk_ui_manager_get_accel_group (ui_manager);
	gtk_window_add_accel_group (GTK_WINDOW (window), accel_group);
	error = NULL;
	if (!gtk_ui_manager_add_ui_from_string (ui_manager, ui_description, -1, &error)) {
		g_message ("building menus failed: %s", error->message);
		g_error_free (error);
		exit (EXIT_FAILURE);
	}
	bar = gtk_ui_manager_get_widget (ui_manager, "/MainMenu");
	gtk_box_pack_start (GTK_BOX (vbox), bar, FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(vbox), viewer);
	g_object_set_data (G_OBJECT (window), "viewer", viewer);
	gtk_widget_show_all(window);
	if (bgcolor) {
		g_object_set (G_OBJECT (viewer), "bgcolor", bgcolor, NULL);
		g_free (bgcolor);
	}

	if (display3d) {
		Display3DMode mode = BALL_AND_STICK;
		if (!strcmp (display3d, "SpaceFill"))
			mode = SPACEFILL;
		else if (strcmp (display3d, "BallnStick"))
			g_warning (_("Unknown display mode"));
		g_object_set (G_OBJECT (viewer), "display3d", mode, NULL);
		g_free (display3d);
	}

	gtk_main();
	
	return(0);
}
