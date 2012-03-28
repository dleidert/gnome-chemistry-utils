// -*- C++ -*-

/*
 * Gnome Crystal
 * main.cc
 *
 * Copyright (C) 2000-2011 Jean Bréfort <jean.brefort@normalesup.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the
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
#include <sys/stat.h>
#include <unistd.h>
#include <gcu/element.h>
#include <gcu/macros.h>
#include "application.h"
#include "document.h"
#include "view.h"
#include <glib/gi18n.h>
#include <cstdio>
#include <cstring>

using namespace gcu;
using namespace std;

extern GtkWidget *vbox1;

gcDocument* pDoc;
gcView* pView;
GtkWidget *mainwindow, *vbox1 ;
GOConfNode *node;

static void cb_print_version (G_GNUC_UNUSED const gchar *option_name, G_GNUC_UNUSED const gchar *value, G_GNUC_UNUSED gpointer data, G_GNUC_UNUSED GError **error)
{
	char *version = g_strconcat (_("Gnome Chemistry Utils version: "), VERSION, NULL);
	puts (version);
	g_free (version);
	exit (0);
}

static GOptionEntry entries[] =
{
  { "version", 'v', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK, (void*) cb_print_version, "prints Gnome Crystal version", NULL },
   { NULL, 0, 0, G_OPTION_ARG_NONE, NULL, NULL, NULL }
};

int main(int argc, char *argv[])
{
	GOptionContext *context;
	GError *error = NULL;

	textdomain(GETTEXT_PACKAGE);

	gtk_init (&argc, &argv);
	Element::LoadRadii ();
	if (argc > 1 && argv[1][0] == '-') {
		context = g_option_context_new (_(" [file...]"));
		g_option_context_add_main_entries (context, entries, GETTEXT_PACKAGE);
		g_option_context_add_group (context, gtk_get_option_group (TRUE));
		g_option_context_set_help_enabled (context, TRUE);
		g_option_context_parse (context, &argc, &argv, &error);
		if (error) {
			puts (error->message);
			g_error_free (error);
			return -1;
		}
	} else {
		argc --;
		argv ++;
	}

	gcr::Application* gcApp = new gcApplication ();
	gcr::Document *pDoc = gcApp->OnFileNew();
	gcApp->SetOpening();

	char *path, *uri;
	bool bres = false;
	while (*argv) {
		if (**argv == '-') {
			printf (_("Invalid or misplaced argument: %s\n"), *argv);
			delete gcApp;
			exit (-1);
		}
		if (strstr (*argv, "://"))
			uri = g_strdup (*argv);
		else {
			if (g_path_is_absolute (*argv))
				path = g_strdup (*argv);
			else {
				char *dir = g_get_current_dir ();
				path = g_build_filename (dir, *argv, NULL);
				g_free (dir);
			}
			uri = g_filename_to_uri (path, NULL, NULL);
			g_free (path);
		}
		if (bres)
			pDoc = gcApp->OnFileNew ();
		char *mime_type = go_get_mime_type (uri);
		bres = gcApp->FileProcess (uri, mime_type, false, NULL, pDoc);
		g_free (mime_type);
		g_free (uri);
		argv++;
	}
	gtk_main ();

	delete gcApp;

	return 0 ;
}
