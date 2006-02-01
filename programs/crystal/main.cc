// -*- C++ -*-

/* 
 * Gnome Crystal
 * main.cc 
 *
 * Copyright (C) 2000-2004
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
#include <glade/glade.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <list>
#include <gcu/element.h>
#include "application.h"
#include "document.h"
#include "view.h"
#include "globals.h"

using namespace gcu;
using namespace std;

extern GtkWidget *vbox1;

gcDocument* pDoc;
std::list<gcDocument*> Docs;
gcView* pView;
GtkWidget * mainwindow, *vbox1 ;
static GtkMenu *windowsmenu;
GConfClient *conf_client;
gint NotificationId;

gcDocument* GetNewDocument()
{
	gcDocument* pDoc = new gcDocument();
	Docs.push_back(pDoc);
	return pDoc;
}

void RemoveDocument(gcDocument* pDoc)
{
	Docs.remove(pDoc);
	delete pDoc;
}

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
}

static void on_config_changed (GConfClient *client,  guint cnxn_id, GConfEntry *entry, gpointer user_data)
{
	g_return_if_fail(client == conf_client);
	g_return_if_fail(cnxn_id == NotificationId);
	if (!strcmp(entry->key,"/apps/gcrystal/general/tab_pos"))
		TabPos = gconf_value_get_int(entry->value);
	else if (!strcmp(entry->key,"/apps/gcrystal/printing/resolution"))
		PrintResolution = gconf_value_get_int(entry->value);
	else if (!strcmp(entry->key,"/apps/gcrystal/views/fov"))
		FoV = gconf_value_get_int(entry->value);
	else if (!strcmp(entry->key,"/apps/gcrystal/views/psi"))
		Psi = gconf_value_get_float(entry->value);
	else if (!strcmp(entry->key,"/apps/gcrystal/views/theta"))
		Theta = gconf_value_get_float(entry->value);
	else if (!strcmp(entry->key,"/apps/gcrystal/views/phi"))
		Phi = gconf_value_get_float(entry->value);
	else if (!strcmp(entry->key,"/apps/gcrystal/views/red"))
		Red = gconf_value_get_float(entry->value);
	else if (!strcmp(entry->key,"/apps/gcrystal/views/green"))
		Green = gconf_value_get_float(entry->value);
	else if (!strcmp(entry->key,"/apps/gcrystal/views/blue"))
		Blue = gconf_value_get_float(entry->value);
}

//static char* opt_filename = NULL;
static bool bonobo_server_flag = false;
struct poptOption options[] =
{
    {"bonobo-server", '\0', POPT_ARG_NONE, &bonobo_server_flag, 0, N_("Allow Gnome Crystal to act as a Bonobo server."), NULL},
//    {"file", 'f', POPT_ARG_STRING, &opt_filename, 0, N_("Load data from FILENAME"), N_("FILENAME")},
	{NULL, '\0', 0, NULL, 0, NULL, NULL}
};

bool IsEmbedded() {return bonobo_server_flag;}

main(int argc, char *argv[])
{
	poptContext pctx;
	GError *error;

	bindtextdomain(GETTEXT_PACKAGE, DATADIR"/locale");
#ifdef ENABLE_NLS
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
#endif
	textdomain(GETTEXT_PACKAGE);

	GnomeProgram* prog = gnome_program_init(PACKAGE, VERSION, LIBGNOMEUI_MODULE, argc, argv, 
                   GNOME_PARAM_POPT_TABLE, options, 
                   GNOME_PROGRAM_STANDARD_PROPERTIES, NULL);
	error = NULL;
	if (gconf_init(argc, argv, &error) == FALSE)
	{
		g_assert(error != NULL);
		g_message("GConf init failed: %s", error->message);
		g_error_free(error);
		exit(EXIT_FAILURE);
	}

	g_object_get(G_OBJECT(prog), GNOME_PARAM_POPT_CONTEXT, &pctx, NULL);

	GnomeClient* client = gnome_master_client();
	g_signal_connect(G_OBJECT(client), "save_yourself", GTK_SIGNAL_FUNC(session_save), argv[0]);
	g_signal_connect(G_OBJECT(client), "die", GTK_SIGNAL_FUNC(session_die), NULL);
	
//Configuration loading
	conf_client = gconf_client_get_default();
	gconf_client_add_dir(conf_client, "/apps/gcrystal/general", GCONF_CLIENT_PRELOAD_ONELEVEL, NULL);
	gconf_client_add_dir(conf_client, "/apps/gcrystal/printing", GCONF_CLIENT_PRELOAD_ONELEVEL, NULL);
	gconf_client_add_dir(conf_client, "/apps/gcrystal/views", GCONF_CLIENT_PRELOAD_ONELEVEL, NULL);
	TabPos = gconf_client_get_int(conf_client, "/apps/gcrystal/general/tab_pos", &error);
	if (error)
	{
		TabPos = 0;
		g_message("GConf failed: %s", error->message);
		g_error_free(error);
		error = NULL;
	}
	PrintResolution = gconf_client_get_int(conf_client, "/apps/gcrystal/printing/resolution", &error);
	if (error)
	{
		PrintResolution = 300;
		g_message("GConf failed: %s", error->message);
		g_error_free(error);
		error = NULL;
	}
	FoV = gconf_client_get_int(conf_client, "/apps/gcrystal/views/fov", &error);
	if (error)
	{
		FoV = 10;
		g_message("GConf failed: %s", error->message);
		g_error_free(error);
		error = NULL;
	}
	Psi = gconf_client_get_float(conf_client, "/apps/gcrystal/views/psi", &error);
	if (error)
	{
		Psi = 70.;
		g_message("GConf failed: %s", error->message);
		g_error_free(error);
		error = NULL;
	}
	Theta = gconf_client_get_float(conf_client, "/apps/gcrystal/views/theta", &error);
	if (error)
	{
		Theta = 10.;
		g_message("GConf failed: %s", error->message);
		g_error_free(error);
		error = NULL;
	}
	Phi = gconf_client_get_float(conf_client, "/apps/gcrystal/views/phi", &error);
	if (error)
	{
		Phi = -90.;
		g_message("GConf failed: %s", error->message);
		g_error_free(error);
		error = NULL;
	}
	Red = gconf_client_get_float(conf_client, "/apps/gcrystal/views/red", &error);
	if (error)
	{
		Red = 1.;
		g_message("GConf failed: %s", error->message);
		g_error_free(error);
		error = NULL;
	}
	Green = gconf_client_get_float(conf_client, "/apps/gcrystal/views/green", &error);
	if (error)
	{
		Green = 1.;
		g_message("GConf failed: %s", error->message);
		g_error_free(error);
		error = NULL;
	}
	Blue	= gconf_client_get_float(conf_client, "/apps/gcrystal/views/blue", &error);
	if (error)
	{
		Blue = 1.;
		g_message("GConf failed: %s", error->message);
		g_error_free(error);
		error = NULL;
	}
	NotificationId = gconf_client_notify_add(conf_client, "/apps/gcrystal", on_config_changed, NULL, NULL, NULL);
	gcApplication* gcApp = new gcApplication();
	Apps.push_back(gcApp);
	gcApp->OnFileNew();
	gcApp->SetOpening();

	const char ** args = poptGetArgs(pctx);
	if (args)
	{
		char* directory = getcwd(NULL, 0);
		bool bres = false;
		gint err;
		for (int i = 0; args[i] != NULL; i++)
		{	
			struct stat buf;
			chdir(directory);
			err = stat(args[i], &buf);
			gchar* filename;
			if (args[i][0] == '/')
			{
				if (err) filename = g_strdup_printf("%s%s", args[i], ".gcrystal");
				else filename = g_strdup(args[i]);
			}
			else
			{
				if (err) filename = g_strdup_printf("%s%c%s%s", directory, '/', args[i], ".gcrystal");
				else filename = g_strdup_printf("%s%c%s", directory, '/', args[i]);
			}
			if (bres)
			{
				gcApp->OnFileNew();
				gcApp->SetOpening();
			}
			bres = gcApp->LoadFile(filename);
			g_free(filename);
		}
		free(directory);
	}
	gtk_main();

	gconf_client_notify_remove(conf_client, NotificationId);
	gconf_client_remove_dir(conf_client, "/apps/gcrystal/general", NULL);
	gconf_client_remove_dir(conf_client, "/apps/gcrystal/printing", NULL);
	gconf_client_remove_dir(conf_client, "/apps/gcrystal/views", NULL);
	g_object_unref(G_OBJECT(conf_client));

	return 0 ;
}
