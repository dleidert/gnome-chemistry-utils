/*
 * Gnome Chemisty Utils
 * tests/testgcucrystalviewer.c
 *
 * Copyright (C) 2008-2010 Jean Bréfort <jean.brefort@normalesup.org>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <gcr/gcrcrystalviewer.h>
#include <gcu/chemistry.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <libxml/parser.h>

/*!\file
A simple sample of the use of the GcuCrystalViewer widget.
*/

/*!
\a main function of the test program. It fulfills the following operations, in order:
- initialize Gtk;
- loads atomic radii.
- build the widow embedding the sample GcuCrystalViewer.
- build the GcuCrystalViewer with either a file from the command line or the default
nickel.gcrystal provided with the sources.
- shows everything and enter \a gtk_main().
*/
int main (int argc, char *argv[])
{
	GtkWidget *window;
	GtkWidget *viewer;
	const char* filename;
	xmlDocPtr xml;
	gtk_init (&argc, &argv);

	gcu_element_load_databases ("radii", NULL);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), "GcuCrystalViewer test");
	g_signal_connect (G_OBJECT (window), "destroy",
		 G_CALLBACK (gtk_main_quit),
		 NULL);

	if (argc >= 2)
		filename = argv[1];
	else
		filename = SRCDIR"/nickel.gcrystal";
	xml = xmlParseFile (filename);

	viewer = gcr_crystal_viewer_new (xml->children);
	gtk_container_add (GTK_CONTAINER (window), viewer);
	gtk_widget_show_all (window);

	gtk_main ();

	return 0;
}
