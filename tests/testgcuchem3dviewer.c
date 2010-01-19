/* 
 * Gnome Chemisty Utils
 * tests/testgcuchem3dviewer.c 
 *
 * Copyright (C) 2008-2010 Jean Bréfort <jean.brefort@normalesup.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the 
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

#include "config.h"
#include <gcu/chemistry.h>
#include <gcu/gcuchem3dviewer.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <stdio.h>

/*!\file
A simple sample of the use of the GcuChem3DViewer widget.
*/

/*!
\a main function of the test program. It fulfills the following operations, in order:
- initialize Gtk;
- loads atomic radii (the 3D viewer needs the van der Waals radii).
- build the window embedding the sample GcuChem3DViewer.
- build the GcuChem3DViewer with either a file from the command line or the default
methane.xyz provided with the sources.
- shows everything and enter \a gtk_main().
*/
int main (int argc, char *argv[])
{
	GtkWidget *window;
	GtkWidget *viewer;
	gchar* uri;
	gtk_init (&argc, &argv);
	
	gcu_element_load_databases ("radii", NULL);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), "GcuChem3dViewer test");
	g_signal_connect (G_OBJECT (window), "destroy",
		 G_CALLBACK (gtk_main_quit),
		 NULL);

	if (argc >= 2)
		uri = argv[1];
	else
		uri = "file://"SRCDIR"/methane.xyz";
	
	viewer = gcu_chem3d_viewer_new (uri);
	gtk_container_add (GTK_CONTAINER (window), viewer);
	gtk_widget_show_all (window);

	gtk_main ();
	
	return 0;
}
