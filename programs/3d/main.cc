// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * programs/3d/main.cc 
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include "config.h"
#include "application.h"
#include "document.h"
#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <libgnomevfs/gnome-vfs-init.h>
#include <libgnomevfs/gnome-vfs-utils.h>
#include <gcu/chemistry.h>
#include <goffice/utils/go-file.h>

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
	{ "version", 'v', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK, (void*) cb_print_version, N_("Prints GChem3d Viewer version"), NULL },
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

	gc3dApplication *App = new gc3dApplication ();
	gc3dDocument *pDoc = App->OnFileNew();
	path = g_get_current_dir ();
	dir = g_strconcat (path, "/", NULL);
	g_free (path);
	uri = gnome_vfs_uri_new (dir);
	bool bres = false;
	argv++;
	while (*argv) {
		if (**argv == '-') {
			printf (_("Invalid or misplaced argument: %s\n"), *argv);
			delete App;
			g_free (dir);
			gnome_vfs_uri_unref (uri);
			exit (-1);
		}
		auri = gnome_vfs_uri_resolve_relative (uri, *argv);
		path = gnome_vfs_uri_to_string (auri, GNOME_VFS_URI_HIDE_NONE);
		if (bres)
			pDoc = App->OnFileNew ();
		bres = App->FileProcess (path, go_get_mime_type (path), false, NULL, pDoc);
		g_free (path);
		gnome_vfs_uri_unref (auri);
		argv++;
	}
/*	if (argc > 1) {
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
	}*/

	gtk_main();
	
	return(0);
}
