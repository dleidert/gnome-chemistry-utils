#include <gcu/gtkcrystalviewer.h>
#include <gcu/chemistry.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <libxml/parser.h>

/*!\file
A simple sample of the use of the GtkCrystalViewer widget.
*/

/*!
\a main function of the test program. It fulfills the following operations, in order:
- initialize Gt and GnomeVFS;
- loads atomic radii.
- build the widow embedding the sample GtkCrystalViewer.
- build the GtkCrystalViewer with either a file from the command line or the default
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
	gtk_window_set_title (GTK_WINDOW (window), "GtkCrystalViewer test");
	g_signal_connect (G_OBJECT (window), "destroy",
		 G_CALLBACK (gtk_main_quit),
		 NULL);

	if (argc >= 2)
		filename = argv[1];
	else
		filename = SRCDIR"/nickel.gcrystal";
	xml = xmlParseFile (filename);
	
	viewer = gtk_crystal_viewer_new (xml->children);
	gtk_container_add (GTK_CONTAINER (window), viewer);
	gtk_widget_show_all (window);

	gtk_main ();
	
	return 0;
}
