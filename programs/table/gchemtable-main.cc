// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * programs/gchemtable-main.cc
 *
 * Copyright (C) 2005 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "gchemtable-app.h"
#include <libintl.h>

using namespace gcu;

int main (int argc, char *argv[])
{
	textdomain (GETTEXT_PACKAGE);
	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	gtk_init (&argc, &argv);

	new GChemTableApp ();
	/* Initialize plugins manager */
	GSList *l = NULL;
	l = g_slist_append (l, (void *) "GOffice_plot_xy");
	l = g_slist_append (l, (void *) "GOffice_reg_linear");
	go_plugins_init (NULL, NULL, l, NULL, FALSE, GO_TYPE_PLUGIN_LOADER_MODULE);
	g_slist_free (l);

	gtk_main();
	return 0;
}
