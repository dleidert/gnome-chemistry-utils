// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * programs/3d/main.cc 
 *
 * Copyright (C) 2006-2007 Jean Bréfort <jean.brefort@normalesup.org>
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
#include <gtk/gtkglinit.h>
#include <libgnomevfs/gnome-vfs-init.h>
#include <libgnomevfs/gnome-vfs-utils.h>
#include <gcu/chemistry.h>
#include <goffice/goffice.h>
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

static char const *Display3DModeNames[] = {
	"ball&stick",
	"spacefill",
};

static Display3DMode display3d_mode_from_string (char const *mode)
{
	if (mode == NULL)
		return BALL_AND_STICK;
	// first ensure the string is in lower case
	char lcmode[16];
	int i, max = strlen (mode), res = SPACEFILL;
	if (max > 15)
		return BALL_AND_STICK;
	for (i = 0; i < max; i++)
		lcmode[i] = tolower (mode[i]);
	lcmode[i] = 0;
	while (res >= BALL_AND_STICK && strcmp (lcmode, Display3DModeNames[res]))
		res--;
	return (Display3DMode) res;
}

int main(int argc, char *argv[])
{
	GnomeVFSURI *uri, *auri;
	char *path, *dir;
	GError *error = NULL;
	GOptionContext *context;

	textdomain (GETTEXT_PACKAGE);
	gtk_init (&argc, &argv);
	gtk_gl_init (&argc, &argv);
	if (!gnome_vfs_init ()) {
		printf ("Could not initialize GnomeVFS\n");
		return 1;
	}
	/* Initialize libgoffice */
	libgoffice_init ();

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

	gc3dApplication *App = new gc3dApplication (display3d_mode_from_string (display3d), bgcolor);
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

	gtk_main();
	
	return(0);
}
