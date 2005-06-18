/* 
 * Gnome Chemistry Utils
 * programs/gchem3d-viewer.c
 *
 * Copyright (C) 2004-2005
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
	GnomeVFSURI *uri, *auri;
	char *path, *dir;

	gtk_init (&argc, &argv);
	if (!gnome_vfs_init ()) {
		printf ("Could not initialize GnomeVFS\n");
		return 1;
	}

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "GtkChem3dViewer");
	g_signal_connect(GTK_OBJECT(window), "destroy",
		 GTK_SIGNAL_FUNC(gtk_main_quit),
		 NULL);

	if (argc >= 2) {
		path = g_get_current_dir ();
		dir = g_strconcat (path, "/", NULL);
		g_free (path);
		uri = gnome_vfs_uri_new (dir);
		auri = gnome_vfs_uri_resolve_relative (uri, argv[1]);
		path = gnome_vfs_uri_to_string (auri, GNOME_VFS_URI_HIDE_NONE);
		viewer = gtk_chem3d_viewer_new(path);
		g_free (path);
		gnome_vfs_uri_unref (auri);
		gnome_vfs_uri_unref (uri);
		g_free (dir);
		gtk_container_add(GTK_CONTAINER(window), viewer);
		gtk_widget_show_all(window);
	
		gtk_main();
	}
	
	return(0);
}
