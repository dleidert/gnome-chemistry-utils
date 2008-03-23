// -*- C++ -*-

/* 
 * GChemPaint
 * main.cc 
 *
 * Copyright (C) 2001-2007 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "standaloneapp.h"
#include <gcu/loader.h>
#include <goffice/utils/go-file.h>
#include <gtk/gtk.h>
#include <libgnomevfs/gnome-vfs-init.h>
#include <libgnomevfs/gnome-vfs-utils.h>
#include <libgnomevfs/gnome-vfs-mime.h>
#include <glib/gi18n-lib.h>

extern "C" {
	void gnome_authentication_manager_init ();
}

// FIXME "the following lines should be removed for stable releases"
#undef PACKAGE
#define PACKAGE "gchempaint-unstable" 

void cb_print_version (const gchar *option_name, const gchar *value, gpointer data, GError **error)
{
	char *version = g_strconcat (_("GChemPaint version: "), VERSION, NULL);
	puts (version);
	g_free (version);
	exit (0);
}

gcpStandaloneApp* App;

static GOptionEntry entries[] = 
{
  { "version", 'v', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK, (void*) cb_print_version, "Prints GChemPaint version", NULL },
  { NULL }
};

int main(int argc, char *argv[])
{
	GOptionContext *context;
	GError *error = NULL;
	textdomain (GETTEXT_PACKAGE);

	gtk_init (&argc, &argv);
	gnome_vfs_init ();
	App = new gcpStandaloneApp();
	if (argc > 1 && argv[1][0] == '-') {
		context = g_option_context_new (_(" [file...]"));
		g_option_context_add_main_entries (context, entries, GETTEXT_PACKAGE);
		App->AddOptions (context);
		g_option_context_add_group (context, gtk_get_option_group (TRUE));
		g_option_context_set_help_enabled (context, TRUE);
		g_option_context_parse (context, &argc, &argv, &error);
		if (error) {
			puts (error->message);
			g_error_free (error);
			return -1;
		}
	}
	argv ++;
	argc --;

	GnomeVFSURI *uri, *auri;
	char *path = g_get_current_dir (), *dir;
	char const *mime_type;
	dir = g_strconcat (path, "/", NULL);
	g_free (path);
	uri = gnome_vfs_uri_new (dir);
	while (*argv)
	{
		if (**argv == '-') {
			printf (_("Invalid or misplaced argument: %s\n"), *argv);
			delete App;
			g_free (dir);
			gnome_vfs_uri_unref (uri);
			exit (-1);
		}
		auri = gnome_vfs_uri_resolve_relative (uri, *argv);
		path = gnome_vfs_uri_to_string (auri, GNOME_VFS_URI_HIDE_NONE);
		mime_type = go_get_mime_type (path);
		App->FileProcess(path, mime_type, false, NULL);
		g_free (path);
		gnome_vfs_uri_unref (auri);
		argv++;
	}
	gnome_vfs_uri_unref (uri);
	g_free (dir);
	
	if (App->GetDocsNumber () == 0)
		App->OnFileNew();

	gtk_main();
	delete App;

	return 0;
}
