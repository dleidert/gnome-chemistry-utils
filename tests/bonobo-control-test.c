#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#include <libbonobo.h>
#include <libbonoboui.h>
#include <libgnomevfs/gnome-vfs-utils.h>
#include "config.h"

CORBA_Environment	 ev;

static guint
create_app (void)
{
	GtkWidget *box, *control, *label, *button;
	BonoboWindow *bonobo_win;

	BonoboUIContainer *uic;
	
	gchar* uri;
	Bonobo_Unknown bu, bs;
	Bonobo_Storage storage;
	Bonobo_Stream stream;
	
	/*
	 * create a bonobo application (window)
	 */
	bonobo_win = (BonoboWindow *) bonobo_window_new ("bonobo-control-test",
					"a container for GtkChem3D BonoboControl test");
	gtk_widget_set_size_request (GTK_WIDGET(bonobo_win), 320, 200);
	g_signal_connect(G_OBJECT(bonobo_win), "delete_event", (GtkSignalFunc)bonobo_main_quit, NULL);
	g_signal_connect(G_OBJECT(bonobo_win), "destroy", (GtkSignalFunc)bonobo_main_quit, NULL);

        /*
	 * connect a ui container to the application
	 */
	uic = bonobo_ui_container_new ();
	bonobo_window_construct (bonobo_win, uic, "bonobo-control-test",
					"a container for GtkChem3D BonoboControl test");

	/* get a widget, containing the control */
	control = bonobo_widget_new_control ("OAFIID:gchem3d_control", BONOBO_OBJREF (uic));
	if (!control) 
		g_error ("Can't create control\n");
	bonobo_window_set_contents(bonobo_win,control);

	gtk_widget_show_all (GTK_WIDGET(bonobo_win));
	
	uri = "file://"SRCDIR"/tests";
	bu = bonobo_widget_get_objref (BONOBO_WIDGET(control));
	bs = Bonobo_Unknown_queryInterface(bu,"IDL:Bonobo/PersistStream:1.0",NULL);
	storage = bonobo_get_object (uri, "IDL:Bonobo/Storage:1.0", &ev);
	if (BONOBO_EX (&ev) || !storage) return TRUE;
	stream = Bonobo_Storage_openStream(storage, "methane.xyz", Bonobo_Storage_READ, &ev);
	if (BONOBO_EX (&ev) || !stream) return TRUE;
	Bonobo_PersistStream_load(bs, stream, "chemical/x-xyz" ,&ev);

	return FALSE;
}

	  int 
main (int argc, char** argv)
{
	CORBA_ORB orb;

	GnomeProgram* prog;
	
	CORBA_exception_init (&ev);
	prog = gnome_program_init ("bonobo-control-test", "0.0", LIBGNOMEUI_MODULE, argc, argv, 
                   GNOME_PARAM_POPT_TABLE, NULL, 
                   GNOME_PROGRAM_STANDARD_PROPERTIES, NULL);

	/*
	 * initialize CORBA, OAF  and bonobo
	 */
	if (!bonobo_init (&argc, argv))
		g_error ("could not initialize Bonobo");


	/*
	 * We can't make any CORBA calls unless we're in the main
	 * loop.  So we delay creating the container here.
	 */
	gtk_idle_add ((GtkFunction) create_app, NULL);
	bonobo_main ();

	return 0;
}
