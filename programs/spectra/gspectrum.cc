// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * programs/spectra/gspectrum.cc
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
#include <libgnomevfs/gnome-vfs-init.h>
#include <libgnomevfs/gnome-vfs-utils.h>
#include <goffice/goffice.h>
#include <goffice/app/go-plugin.h>
#include <goffice/app/go-plugin-loader-module.h>
#include <goffice/utils/go-file.h>
#include <gtk/gtkmain.h>
#include <glib.h>
#include <glib/gi18n.h>

int main (int argc, char *argv[])
{
	GnomeVFSURI *uri, *auri;
	char *path, *dir;
	GError *error = NULL;
	GOptionContext *context;

	textdomain (GETTEXT_PACKAGE);
	gtk_init (&argc, &argv);
	if (!gnome_vfs_init ()) {
		printf ("Could not initialize GnomeVFS\n");
		return 1;
	}

	if (argc > 1 && argv[1][0] == '-') {
		context = g_option_context_new (_(" [file]"));
		g_option_context_add_group (context, gtk_get_option_group (TRUE));
		g_option_context_set_help_enabled (context, TRUE);
		g_option_context_parse (context, &argc, &argv, &error);
		if (error) {
			puts (error->message);
			g_error_free (error);
			return -1;
		}
	}

	// create a new Application. This initialize Goffice
	gsvApplication *App = new gsvApplication ();
	// Initialize plugins manager
	go_plugins_init (NULL, NULL, NULL, NULL, TRUE, GO_PLUGIN_LOADER_MODULE_TYPE);
	gsvDocument *pDoc = App->OnFileNew();
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

	gtk_main();

	return 0;
}
