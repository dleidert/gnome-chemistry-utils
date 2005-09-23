/* 
 * Gnome Chemistry Utils
 * programs/gchem3d-viewer.c
 *
 * Copyright (C) 2004-2005
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
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
#include <libgnomevfs/gnome-vfs.h>

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

static void on_quit (GtkWidget *widget, void *data)
{
	gtk_main_quit();
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
		"Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307\n"
		"USA";
/* Note to translators: replace the following string with the appropriate credits for you lang */
	char *translator_credits = _("translator_credits");
	gtk_show_about_dialog (NULL,
					"name", "GChem3D",
					"authors", authors,
					"comments", _("GChem3D is a molecular structures viewer for Gnome"),
					"copyright", _("(C) 2004-2005 by Jean Bréfort"),
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
 	  { "Quit", GTK_STOCK_QUIT, N_("_Quit"), "<control>Q",
		  N_("Quit GChem3D"), G_CALLBACK (on_quit) },
  { "HelpMenu", NULL, N_("_Help") },
	  { "About", NULL, N_("_About"), NULL,
		  N_("About GChem3D"), G_CALLBACK (on_about) }
};

static const char *ui_description =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu action='FileMenu'>"
"      <menuitem action='Open'/>"
"      <menuitem action='Quit'/>"
"    </menu>"
"    <menu action='HelpMenu'>"
"      <menuitem action='About'/>"
"    </menu>"
"  </menubar>"
"</ui>";

int main(int argc, char *argv[])
{
	GtkWidget *window;
	GtkWidget *viewer, *vbox, *bar;
	GnomeVFSURI *uri, *auri;
	char *path, *dir;
	GtkUIManager *ui_manager;
	GtkActionGroup *action_group;
	GtkAccelGroup *accel_group;
	GError *error;

	gtk_init (&argc, &argv);
	if (!gnome_vfs_init ()) {
		printf ("Could not initialize GnomeVFS\n");
		return 1;
	}

	gcu_element_load_databases ("radii", NULL);

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

	if (argc >= 2) {
		path = g_get_current_dir ();
		dir = g_strconcat (path, "/", NULL);
		g_free (path);
		uri = gnome_vfs_uri_new (dir);
		auri = gnome_vfs_uri_resolve_relative (uri, argv[1]);
		path = gnome_vfs_uri_to_string (auri, GNOME_VFS_URI_HIDE_NONE);
		viewer = gtk_chem3d_viewer_new(path);
		g_free (path);
		gnome_vfs_uri_unref (auri);
		gnome_vfs_uri_unref (uri);
		g_free (dir);
	} else
		viewer = gtk_chem3d_viewer_new("");
	gtk_container_add(GTK_CONTAINER(vbox), viewer);
	g_object_set_data (G_OBJECT (window), "viewer", viewer);
	gtk_widget_show_all(window);
	gtk_main();
	
	return(0);
}
