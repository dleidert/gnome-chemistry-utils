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
