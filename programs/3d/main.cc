// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * programs/3d/main.cc 
 *
 * Copyright (C) 2006-2008 Jean Bréfort <jean.brefort@normalesup.org>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
 * USA
 */

#include "config.h"
#include "application.h"
#include "document.h"
#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gtk/gtkglinit.h>
#include <gcu/chemistry.h>
#include <goffice/goffice.h>
#include <goffice/utils/go-file.h>
#include <gio/gio.h>
#include <cstring>
#include <cstdio>

void cb_print_version (G_GNUC_UNUSED const gchar *option_name, G_GNUC_UNUSED const gchar *value, G_GNUC_UNUSED gpointer data, G_GNUC_UNUSED GError **error)
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
	{ "display3d", 'd', 0, G_OPTION_ARG_STRING, &display3d, N_("How molecules are displayed; possible values are BallnStick (the default), SpaceFill, Cylinders, and Wireframe"), NULL },
	{ NULL, 0, 0, G_OPTION_ARG_NONE, NULL, NULL, NULL }
};

static char const *Display3DModeNames[] = {
	"ball&stick",
	"spacefill",
	"cylinders",
	"wireframe"
};

static Display3DMode display3d_mode_from_string (char const *mode)
{
	if (mode == NULL)
		return BALL_AND_STICK;
	// first ensure the string is in lower case
	char lcmode[16];
	int i, max = strlen (mode), res = WIREFRAME;
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
	char *path, *dir, *uri;
	GError *error = NULL;
	GOptionContext *context;

	textdomain (GETTEXT_PACKAGE);
	gtk_init (&argc, &argv);
	gtk_gl_init (&argc, &argv);

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
	bool bres = false;
	argv++;
	while (*argv) {
		if (**argv == '-') {
			printf (_("Invalid or misplaced argument: %s\n"), *argv);
			delete App;
			g_free (dir);
			exit (-1);
		}
		if (bres)
			pDoc = App->OnFileNew ();
		if (strstr (*argv, "://"))
			uri = g_strdup (*argv);
		else {
			path = g_path_is_absolute (*argv)? g_strdup (*argv): g_build_filename (g_get_current_dir (), *argv, NULL);
			uri = g_filename_to_uri (path, NULL, NULL);
			g_free (path);
		}
		bres = App->FileProcess (uri, go_get_mime_type (uri), false, NULL, pDoc);
		g_free (uri);
		argv++;
	}

	gtk_main();
	
	return(0);
}
