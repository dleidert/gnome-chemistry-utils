#include <gtkcrystalviewer.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <libxml/parser.h>

int main(int argc, char *argv[])
{
	GtkWidget *window;
	GtkWidget *viewer;
	gtk_init (&argc, &argv);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "GtkCrystalViewer test");
	g_signal_connect(GTK_OBJECT(window), "destroy",
		 GTK_SIGNAL_FUNC(gtk_main_quit),
		 NULL);

	const char* filename;
	if (argc >= 2) filename = argv[1];
		else filename = "nickel.gcrystal";
	xmlDocPtr xml = xmlParseFile(filename);
	
	viewer = gtk_crystal_viewer_new(xml->children);
	gtk_container_add(GTK_CONTAINER(window), viewer);
	gtk_widget_show_all(window);

	gtk_main();
	
	return(0);
}
