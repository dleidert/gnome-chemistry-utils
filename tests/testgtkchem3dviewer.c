/* 
 * Gnome Chemisty Utils
 * testgtkchem3dviewer.c 
 *
 * Copyright (C) 2006 Jean Bréfort <jean.brefort@normalesup.org>
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
#include <gcu/chemistry.h>
#include <gcu/gtkchem3dviewer.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <libgnomevfs/gnome-vfs.h>

/*!\file
A simple sample of the use of the GtkChem3DViewer widget.
*/
int main(int argc, char *argv[])
{
	GtkWidget *window;
	GtkWidget *viewer;
	gchar* uri;
	gtk_init (&argc, &argv);
	if (!gnome_vfs_init ()) {
		printf ("Could not initialize GnomeVFS\n");
		return 1;
	}
	
	gcu_element_load_databases ("radii", NULL);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "GtkChem3dViewer test");
	g_signal_connect(GTK_OBJECT(window), "destroy",
		 GTK_SIGNAL_FUNC(gtk_main_quit),
		 NULL);

	if (argc >= 2) uri = argv[1];
		else uri = "file://"SRCDIR"/methane.xyz";
	
	viewer = gtk_chem3d_viewer_new(uri);
	gtk_container_add(GTK_CONTAINER(window), viewer);
	gtk_widget_show_all(window);

	gtk_main();
	
	return(0);
}
