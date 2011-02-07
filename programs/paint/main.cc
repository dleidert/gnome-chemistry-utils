// -*- C++ -*-

/* 
 * GChemPaint
 * main.cc 
 *
 * Copyright (C) 2001-2010 Jean Bréfort <jean.brefort@normalesup.org>
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
#include <glib/gi18n-lib.h>
#include <cstring>
#include <signal.h>

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

static gcpStandaloneApp* App = NULL;

/* code copied from AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * Copyright (C) 2009 Hubert Figuiere
 */ 
void signalWrapper(int sig_num)
{
	/* make sure we have application, in case we have been called after
	 * the application object is gone
	 */
	if (App)
		App->CatchSignals (sig_num);
}
// end of copied code

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

	char *path, *uri;
	while (*argv) {
		if (**argv == '-') {
			printf (_("Invalid or misplaced argument: %s\n"), *argv);
			delete App;
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
		char *mime_type = go_get_mime_type (uri);
		App->FileProcess (uri, mime_type, false, NULL, NULL);
		g_free (mime_type);
		g_free (uri);
		argv++;
	}
	
	if (App->GetDocsNumber () == 0)
		App->OnFileNew ();

/* code copied from AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * Copyright (C) 2009 Hubert Figuiere
 */ 
	// Setup signal handlers, primarily for segfault
	// If we segfaulted before here, we *really* blew it
	struct sigaction sa;
	sa.sa_handler = signalWrapper;

	sigfillset (&sa.sa_mask);  // We don't want to hear about other signals
	sigdelset (&sa.sa_mask, SIGABRT); // But we will call abort(), so we can't ignore that
#if defined (SA_NODEFER) && defined (SA_RESETHAND)
	sa.sa_flags = SA_NODEFER | SA_RESETHAND; // Don't handle nested signals
#else
	sa.sa_flags = 0;
#endif

	sigaction (SIGSEGV, &sa, NULL);
	sigaction (SIGBUS, &sa, NULL);
	sigaction (SIGILL, &sa, NULL);
	sigaction (SIGQUIT, &sa, NULL);
	sigaction (SIGFPE, &sa, NULL);
	// end of copied code

	gtk_main ();
	delete App;		

	return 0;
}
