#include <gtkperiodic.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <stdio.h>

void on_changed(GtkPeriodic* periodic, guint Z, GData* data)
{
	printf("Selected element:%d\n", Z);
}

int main(int argc, char *argv[])
{
	GtkWidget *window;
	GtkWidget *periodic;

	gtk_init (&argc, &argv);
	
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "GtkPeriodic test");
	g_signal_connect(GTK_OBJECT(window), "destroy",
		 GTK_SIGNAL_FUNC(gtk_main_quit),
		 NULL);

	g_object_set(G_OBJECT(window), "allow-shrink", FALSE, NULL);

	periodic = gtk_periodic_new();
	g_signal_connect(G_OBJECT(periodic), "element_changed", (GCallback)on_changed, NULL);
	gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(GTK_PERIODIC(periodic)));
	gtk_widget_show_all(window);

	gtk_main();
	
	return(0);
}
