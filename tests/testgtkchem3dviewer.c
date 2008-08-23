#include "config.h"
#include <gcu/chemistry.h>
#include <gcu/gtkchem3dviewer.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <stdio.h>
#ifdef GOFFICE_IS_0_6
#	include <libgnomevfs/gnome-vfs.h>
#endif

/*!\file
A simple sample of the use of the GtkChem3DViewer widget.
*/

/*!
\a main function of the test program. It fulfills the following operations, in order:
- initialize Gt and GnomeVFS;
- loads atomic radii (the 3D viewer needs the van der Waals radii).
- build the widow embedding the sample GtkChem3DViewer.
- build the GtkChem3DViewer with either a file from the command line or the default
methane.xyz provided with the sources.
- shows everything and enter \a gtk_main().
*/
int main (int argc, char *argv[])
{
	GtkWidget *window;
	GtkWidget *viewer;
	gchar* uri;
	gtk_init (&argc, &argv);
#ifdef GOFFICE_IS_0_6
	if (!gnome_vfs_init ()) {
		printf ("Could not initialize GnomeVFS\n");
		return 1;
	}
#endif
	
	gcu_element_load_databases ("radii", NULL);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), "GtkChem3dViewer test");
	g_signal_connect (G_OBJECT (window), "destroy",
		 G_CALLBACK (gtk_main_quit),
		 NULL);

	if (argc >= 2)
		uri = argv[1];
	else
		uri = "file://"SRCDIR"/methane.xyz";
	
	viewer = gtk_chem3d_viewer_new (uri);
	gtk_container_add (GTK_CONTAINER (window), viewer);
	gtk_widget_show_all (window);

	gtk_main ();
	
	return 0;
}
