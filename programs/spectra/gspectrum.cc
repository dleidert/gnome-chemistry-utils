// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * programs/spectra/gspectrum.cc
 *
 * Copyright (C) 2007-2008 Jean Bréfort <jean.brefort@normalesup.org>
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
#include <goffice/goffice.h>
#include <goffice/app/go-plugin.h>
#include <goffice/app/go-plugin-loader-module.h>
#include <goffice/utils/go-file.h>
#include <gtk/gtkmain.h>
#include <glib.h>
#include <glib/gi18n.h>

int main (int argc, char *argv[])
{
	GError *error = NULL;
	GOptionContext *context;

	textdomain (GETTEXT_PACKAGE);
	gtk_init (&argc, &argv);

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
	go_plugins_init (NULL, NULL, NULL, NULL, TRUE, GO_TYPE_PLUGIN_LOADER_MODULE);
	gsvDocument *pDoc = App->OnFileNew();
	char *path, *uri;
	bool bres = false;
	while (*argv) {
		if (**argv == '-') {
			printf (_("Invalid or misplaced argument: %s\n"), *argv);
			delete App;
			exit (-1);
		}
		if (strstr (*argv, "://"))
			uri = g_strdup (*argv);
		else {
			path = g_path_is_absolute (*argv)? g_strdup (*argv): g_build_filename (g_get_current_dir (), *argv, NULL);
			uri = g_filename_to_uri (path, NULL, NULL);
			g_free (path);
		}
		if (bres)
			pDoc = App->OnFileNew ();
		bres = App->FileProcess (uri, go_get_mime_type (uri), false, NULL, pDoc);
		g_free (uri);
		argv++;
	}

	gtk_main();
	delete App;

	return 0;
}
