// -*- C++ -*-

/* 
 * Gnome Crystal
 * main.cc 
 *
 * Copyright (C) 2000-2008 Jean Bréfort <jean.brefort@normalesup.org>
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
#include <glade/glade.h>
#include <sys/stat.h>
#include <unistd.h>
#include <gcu/element.h>
#include <gcu/macros.h>
#include "application.h"
#include "document.h"
#include "view.h"
#include "globals.h"
#ifndef GOFFICE_HAS_GLOBAL_HEADER
#   include <goffice/utils/go-file.h>
#endif
#ifdef GOFFICE_IS_0_6
#	include <libgnomevfs/gnome-vfs-init.h>
#endif
#include <gtk/gtkglinit.h>
#include <glib/gi18n.h>
#include <cstdio>
#include <cstring>

using namespace gcu;
using namespace std;

extern GtkWidget *vbox1;

gcDocument* pDoc;
gcView* pView;
GtkWidget *mainwindow, *vbox1 ;
#ifdef HAVE_GO_CONF_SYNC
	GOConfNode *node;
#else
	GConfClient *conf_client;
#endif
guint NotificationId;

// defines used for GCU_GCONF_GET
#define ROOTDIR	"/apps/gchemutils/crystal/"
#ifdef HAVE_GO_CONF_SYNC
#	define m_ConfNode node
#else
#	define m_ConfClient conf_client
#endif

#ifdef HAVE_GO_CONF_SYNC
static void on_config_changed (GOConfNode *node, gchar const *name, gpointer user_data)
{
#else
static void on_config_changed (GConfClient *client,  guint cnxn_id, GConfEntry *entry, gpointer user_data)
{
	g_return_if_fail (client == conf_client);
	g_return_if_fail (cnxn_id == NotificationId);
#endif
	GCU_UPDATE_KEY ("printing/resolution", int, PrintResolution, {})
	GCU_UPDATE_KEY ("view/fov", int, FoV, {})
	GCU_UPDATE_KEY ("view/psi", float, Psi, {})
	GCU_UPDATE_KEY ("view/theta", float, Theta, {})
	GCU_UPDATE_KEY ("view/phi", float, Phi, {})
	GCU_UPDATE_KEY ("view/red", float, Red, {})
	GCU_UPDATE_KEY ("view/green", float, Green, {})
	GCU_UPDATE_KEY ("view/blue", float, Blue, {})
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

int main(int argc, char *argv[])
{
	GOptionContext *context;
	GError *error = NULL;

	textdomain(GETTEXT_PACKAGE);

	gtk_init (&argc, &argv);
	gtk_gl_init (&argc, &argv);
#ifdef GOFFICE_IS_0_6
	gnome_vfs_init ();
#endif
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
	
//Configuration loading
#ifdef HAVE_GO_CONF_SYNC
	node = go_conf_get_node (Application::GetConfDir (), "crystal");
#else
	conf_client = gconf_client_get_default ();
	gconf_client_add_dir (conf_client, "/apps/gchemutils/crystal/general", GCONF_CLIENT_PRELOAD_ONELEVEL, NULL);
	gconf_client_add_dir (conf_client, "/apps/gchemutils/crystal/printing", GCONF_CLIENT_PRELOAD_ONELEVEL, NULL);
	gconf_client_add_dir (conf_client, "/apps/gchemutils/crystal/views", GCONF_CLIENT_PRELOAD_ONELEVEL, NULL);
#endif
	GCU_GCONF_GET ("printing/resolution", int, PrintResolution, 300)
	GCU_GCONF_GET ("views/fov", int, FoV, 10)
	GCU_GCONF_GET_NO_CHECK ("views/psi", float, Psi, 70.)
	GCU_GCONF_GET_NO_CHECK ("views/theta", float,Theta, 10.)
	GCU_GCONF_GET_NO_CHECK ("views/phi", float, Phi, -90.)
	GCU_GCONF_GET_NO_CHECK ("views/red", float, Red, 1.)
	GCU_GCONF_GET_NO_CHECK ("views/green", float, Green, 1.)
	GCU_GCONF_GET_NO_CHECK ("views/blue", float, Blue, 1.)
#ifdef HAVE_GO_CONF_SYNC
	NotificationId = go_conf_add_monitor (node, NULL, (GOConfMonitorFunc) on_config_changed, NULL);
#else
	NotificationId = gconf_client_notify_add (conf_client, "/apps/gchemutils/crystal", on_config_changed, NULL, NULL, NULL);
#endif
	gcApplication* gcApp = new gcApplication ();
	gcDocument *pDoc = gcApp->OnFileNew();
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
			path = g_path_is_absolute (*argv)? g_strdup (*argv): g_build_filename (g_get_current_dir (), *argv, NULL);
			uri = g_filename_to_uri (path, NULL, NULL);
			g_free (path);
		}
		if (bres)
			pDoc = gcApp->OnFileNew ();
		bres = gcApp->FileProcess (uri, go_get_mime_type (uri), false, NULL, pDoc);
		g_free (uri);
		argv++;
	}
	gtk_main ();

#ifdef HAVE_GO_CONF_SYNC
	go_conf_remove_monitor (NotificationId);
	go_conf_free_node (node);
#else
	gconf_client_notify_remove (conf_client, NotificationId);
	gconf_client_remove_dir (conf_client, "/apps/gchemutils/crystal/general", NULL);
	gconf_client_remove_dir (conf_client, "/apps/gchemutils/crystal/printing", NULL);
	gconf_client_remove_dir (conf_client, "/apps/gchemutils/crystal/views", NULL);
	g_object_unref (G_OBJECT(conf_client));
#endif

#ifdef GOFFICE_IS_0_6
	gnome_vfs_shutdown ();
#endif
	return 0 ;
}
