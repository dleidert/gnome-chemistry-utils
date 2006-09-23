// -*- C++ -*-

/* 
 * Gnome Crystal
 * main.cc 
 *
 * Copyright (C) 2000-2006 Jean Bréfort <jean.brefort@normalesup.org>
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
#include <glade/glade.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <gcu/element.h>
#include <gcu/macros.h>
#include "application.h"
#include "document.h"
#include "view.h"
#include "globals.h"
#include <libgnomevfs/gnome-vfs-init.h>
#include <libgnomevfs/gnome-vfs-utils.h>
#include <glib/gi18n.h>

using namespace gcu;
using namespace std;

extern GtkWidget *vbox1;

gcDocument* pDoc;
gcView* pView;
GtkWidget *mainwindow, *vbox1 ;
GConfClient *conf_client;
guint NotificationId;

/* Following code is removed because libgnomeui is going to be deprecated
* Just I don't know what will replace GnomeClient *
static void session_die(GnomeClient *client, gpointer data)
{
   gtk_main_quit();
}

static gint session_save(GnomeClient *client, gint phase, GnomeSaveStyle save_style, gint is_shutdown, GnomeInteractStyle interact_style, gint is_fast, gpointer client_data)
{
	gchar **argv;
	gint argc;
	if (IsEmbedded())
	{
		argv = (gchar**) g_malloc0(sizeof(gchar*) * 3);
		argc = 2;
		argv[0] = (gchar*) client_data;
		argv[1] = "--bonobo-server";
		gnome_client_set_clone_command(client, argc, argv);
		gnome_client_set_restart_command(client, argc, argv);
	}
}*/

static void on_config_changed (GConfClient *client,  guint cnxn_id, GConfEntry *entry, gpointer user_data)
{
	g_return_if_fail (client == conf_client);
	g_return_if_fail (cnxn_id == NotificationId);
	if (!strcmp (entry->key,"/apps/gcrystal/printing/resolution"))
		PrintResolution = gconf_value_get_int (entry->value);
	else if (!strcmp (entry->key,"/apps/gcrystal/views/fov"))
		FoV = gconf_value_get_int (entry->value);
	else if (!strcmp (entry->key,"/apps/gcrystal/views/psi"))
		Psi = gconf_value_get_float (entry->value);
	else if (!strcmp (entry->key,"/apps/gcrystal/views/theta"))
		Theta = gconf_value_get_float (entry->value);
	else if (!strcmp (entry->key,"/apps/gcrystal/views/phi"))
		Phi = gconf_value_get_float (entry->value);
	else if (!strcmp (entry->key,"/apps/gcrystal/views/red"))
		Red = gconf_value_get_float (entry->value);
	else if (!strcmp (entry->key,"/apps/gcrystal/views/green"))
		Green = gconf_value_get_float (entry->value);
	else if (!strcmp (entry->key,"/apps/gcrystal/views/blue"))
		Blue = gconf_value_get_float (entry->value);
}

static void cb_print_version (const gchar *option_name, const gchar *value, gpointer data, GError **error)
{
	char *version = g_strconcat (_("Gnome Chemistry Utils version: "), VERSION, NULL);
	puts (version);
	g_free (version);
	exit (0);
}

static GOptionEntry entries[] = 
{
  { "version", 'v', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK, (void*) cb_print_version, "prints Gnome Crystal version", NULL },
   { NULL }
};

// defines used for GCU_GCONF_GET
#define ROOTDIR	"/apps/gchemutils/crystal/"
#define m_ConfClient conf_client

int main(int argc, char *argv[])
{
	GOptionContext *context;
	GError *error = NULL;

	textdomain(GETTEXT_PACKAGE);

	gtk_init (&argc, &argv);
	gnome_vfs_init ();
	Element::LoadRadii ();
//	gnome_authentication_manager_init ();
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
	
//Configuration loading
	conf_client = gconf_client_get_default ();
	gconf_client_add_dir (conf_client, "/apps/gchemutils/crystal/general", GCONF_CLIENT_PRELOAD_ONELEVEL, NULL);
	gconf_client_add_dir (conf_client, "/apps/gchemutils/crystal/printing", GCONF_CLIENT_PRELOAD_ONELEVEL, NULL);
	gconf_client_add_dir (conf_client, "/apps/gchemutils/crystal/views", GCONF_CLIENT_PRELOAD_ONELEVEL, NULL);
	GCU_GCONF_GET (ROOTDIR"printing/resolution", int, PrintResolution, 300)
	GCU_GCONF_GET (ROOTDIR"views/fov", int, FoV, 10)
	GCU_GCONF_GET_NO_CHECK (ROOTDIR"views/psi", float, Psi, 70.)
	GCU_GCONF_GET_NO_CHECK (ROOTDIR"views/theta", float,Theta, 10.)
	GCU_GCONF_GET_NO_CHECK (ROOTDIR"views/phi", float, Phi, -90.)
	GCU_GCONF_GET_NO_CHECK (ROOTDIR"views/red", float, Red, 1.)
	GCU_GCONF_GET_NO_CHECK (ROOTDIR"views/green", float, Green, 1.)
	GCU_GCONF_GET_NO_CHECK (ROOTDIR"views/blue", float, Blue, 1.)
	NotificationId = gconf_client_notify_add (conf_client, "/apps/gchemutils/crystal", on_config_changed, NULL, NULL, NULL);
	gcApplication* gcApp = new gcApplication ();
	gcDocument *pDoc = gcApp->OnFileNew();
	gcApp->SetOpening();

	GnomeVFSURI *uri, *auri;
	char *path = g_get_current_dir (), *dir;
	dir = g_strconcat (path, "/", NULL);
	g_free (path);
	uri = gnome_vfs_uri_new (dir);
	bool bres = false;
	while (*argv) {
		if (**argv == '-') {
			printf (_("Invalid or misplaced argument: %s\n"), *argv);
			delete gcApp;
			g_free (dir);
			gnome_vfs_uri_unref (uri);
			exit (-1);
		}
		auri = gnome_vfs_uri_resolve_relative (uri, *argv);
		path = gnome_vfs_uri_to_string (auri, GNOME_VFS_URI_HIDE_NONE);
		if (bres)
			pDoc = gcApp->OnFileNew ();
		bres = gcApp->FileProcess (path, "application/x-gcrystal", false, NULL, pDoc);
		g_free (path);
		gnome_vfs_uri_unref (auri);
		argv++;
	}
	gnome_vfs_uri_unref (uri);
	g_free (dir);
	gtk_main ();

	gconf_client_notify_remove (conf_client, NotificationId);
	gconf_client_remove_dir (conf_client, "/apps/gchemutils/crystal/general", NULL);
	gconf_client_remove_dir (conf_client, "/apps/gchemutils/crystal/printing", NULL);
	gconf_client_remove_dir (conf_client, "/apps/gchemutils/crystal/views", NULL);
	g_object_unref (G_OBJECT(conf_client));

	gnome_vfs_shutdown ();

	return 0 ;
}
