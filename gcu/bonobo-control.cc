/* 
 * Gnome Chemisty Utils
 * bonobo-control.cc 
 *
 * Copyright (C) 2003
 *
 * Developed by Jean Bréfort <jean.brefort@ac-dijon.fr>
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
 * License along with this library; if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA  02111-1307, USA.
 */

#include "config.h"
#include "gtkchem3dviewer.h"
#include <libgnome/libgnome.h>
#include <libbonobo.h>
#include <libbonoboui.h>
#include <string.h>
#include <data.h>

extern OpenBabel::OBExtensionTable et;
/*******************************************************************************
 * Persist Stream implementation
 ******************************************************************************/
#define PERSIST_STREAM_TYPE         (persist_stream_get_type ())
#define PERSIST_STREAM(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), PERSIST_STREAM_TYPE, PersistStream))
#define PERSIST_STREAM_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), PERSIST_STREAM_TYPE, PersistStreamClass))
#define PERSIST_STREAM_IS_OBJECT(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), PERSIST_STREAM_TYPE))
#define PERSIST_STREAM_IS_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), PERSIST_STREAM_TYPE))
#define PERSIST_STREAM_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), PERSIST_STREAM_TYPE, PersistStreamClass))

typedef struct {
	BonoboPersist parent;
	GtkChem3DViewer *viewer;
} PersistStream;

typedef struct {
	BonoboPersistClass parent_class;

	POA_Bonobo_PersistStream__epv epv;
} PersistStreamClass;

GType persist_stream_get_type (void);

static void on_load(gpointer p, Bonobo_Stream stream, const Bonobo_Persist_ContentType type, CORBA_Environment  *ev)
{
	const gchar *mime_type;
	if ((type == CORBA_OBJECT_NIL) || (*type == 0))
	{
		char mime[32];
		Bonobo_StorageInfo   *info =  Bonobo_Stream_getInfo (stream, 0, ev);
		et.TypeToMIME(et.FilenameToType(info->name), mime);
		mime_type = mime;
	}
	else mime_type = type;
	PersistStream *ps = (PersistStream*)bonobo_object(p);
	if (!ps->viewer) return;
	CORBA_long l = Bonobo_Stream_seek(stream, 0, Bonobo_Stream_SeekEnd, ev);
	Bonobo_Stream_seek(stream, 0, Bonobo_Stream_SeekSet, ev);
	Bonobo_Stream_iobuf*  buffer;
	Bonobo_Stream_read(stream, l, &buffer, ev);
	char buf[l+1];
	memcpy(buf, buffer->_buffer, l);
	buf[l] = 0;
	gtk_chem3d_viewer_set_data(ps->viewer, buf, (const gchar*)mime_type);
	CORBA_free (buffer);
}

static void on_save(gpointer p, Bonobo_Stream stream, const Bonobo_Persist_ContentType type, CORBA_Environment  *ev)
{
}

static PersistStream* persist_stream_new()
{
	PersistStream* ps = (PersistStream*) g_object_new (PERSIST_STREAM_TYPE, NULL);
	return ps;
}

static void persist_stream_init(PersistStream *ps)
{
}

static void persist_stream_class_init(PersistStreamClass *klass)
{
	POA_Bonobo_PersistStream__epv* epv = &(klass->epv);
	epv->load = (void(*)(void*, CORBA_Object_type*, const CORBA_char*, CORBA_Environment*)) on_load;
	epv->save = (void(*)(void*, CORBA_Object_type*, const CORBA_char*, CORBA_Environment*)) on_save;
}

 BONOBO_TYPE_FUNC_FULL (
	PersistStream,                /* Glib class name */
	Bonobo_PersistStream,  /* CORBA interface name */
	BONOBO_TYPE_PERSIST,  /* parent type */
	persist_stream);               /* local prefix ie. 'echo'_class_init */

// BonoboPropertyBag callback functions

enum {
	PROP_DISPLAY3D,
	PROP_BGCOLOR
};

static void get_prop(BonoboPropertyBag *bag, BonoboArg *arg, guint arg_id, CORBA_Environment *ev, GObject *object)
{
	Display3DMode mode3d;
	const gchar* str;

	switch (arg_id)
	{
		case PROP_DISPLAY3D:
			g_object_get(object, "display3d", &mode3d, NULL);
			switch (mode3d)
			{
				case BALL_AND_STICK: BONOBO_ARG_SET_STRING(arg, "ball&stick"); break;
				case SPACEFILL: BONOBO_ARG_SET_STRING(arg, "spacefill"); break;
				default: bonobo_exception_set(ev, ex_Bonobo_PropertyBag_BackendFailed);
			}
			break;
		case PROP_BGCOLOR:
			g_object_get(object, "bgcolor", &str, NULL);
			BONOBO_ARG_SET_STRING(arg, str);
			break;
		default:
			bonobo_exception_set(ev, ex_Bonobo_PropertyBag_NotFound);
			break;
	}
}

static void set_prop (BonoboPropertyBag *bag,  const BonoboArg *arg, guint arg_id, CORBA_Environment *ev, GObject *object)
{
	switch (arg_id)
	{
		case PROP_DISPLAY3D:
			if (!strcmp(BONOBO_ARG_GET_STRING(arg), "ball&stick"))
				g_object_set(object, "display3d", BALL_AND_STICK, NULL);
			else if (!strcmp(BONOBO_ARG_GET_STRING(arg), "spacefill"))
				g_object_set(object, "display3d", SPACEFILL, NULL);
			else bonobo_exception_set(ev, ex_Bonobo_PropertyBag_BackendFailed);
			break;
		case PROP_BGCOLOR:
			g_object_set(object, "bgcolor", BONOBO_ARG_GET_STRING(arg));
			break;
		default:
			bonobo_exception_set(ev, ex_Bonobo_PropertyBag_NotFound);
			break;
	}
}


#define GC3D_BONOBO_CONTROL_TYPE           (gc3d_bonobo_control_get_type ())
#define GC3D_BONOBO_CONTROL(o)             (G_TYPE_CHECK_INSTANCE_CAST ((o), GC3D_BONOBO_CONTROL_TYPE, GC3DBonoboControl))
#define GC3D_BONOBO_CONTROL_CLASS(k)       (G_TYPE_CHECK_CLASS_CAST((k), GC3D_BONOBO_CONTROL_TYPE, GC3DBonoboControlClass))

#define GC3D_BONOBO_IS_CONTROL(o)          (G_TYPE_CHECK_INSTANCE_TYPE ((o), GC3D_BONOBO_CONTROL_TYPE))
#define GC3D_BONOBO_IS_CONTROL_CLASS(k)    (G_TYPE_CHECK_CLASS_TYPE ((k), GC3D_BONOBO_CONTROL_TYPE))
#define GC3D_BONOBO_CONTROL_GET_CLASS(o)   (G_TYPE_INSTANCE_GET_CLASS ((o), GC3D_BONOBO_CONTROL_TYPE, GC3DBonoboControlClass))

typedef struct _GC3DBonoboControl         GC3DBonoboControl;
typedef struct _GC3DBonoboControlClass    GC3DBonoboControlClass;

struct _GC3DBonoboControl {
	BonoboControl control;
	PersistStream *ps;

	GtkChem3DViewer *viewer;
};

struct _GC3DBonoboControlClass {
	BonoboControlClass parent_class;
};

static GType gc3d_bonobo_control_get_type(void);
static GC3DBonoboControl *gc3d_bonobo_control_new(GtkChem3DViewer* viewer);
static GC3DBonoboControl *gc3d_bonobo_control_construct(GC3DBonoboControl *control, GtkChem3DViewer* viewer);

static GObjectClass *gc3d_bonobo_control_parent_class;

static void
gc3d_bonobo_control_destroy (BonoboObject *object)
{
	g_return_if_fail (object != NULL);
	g_return_if_fail (GC3D_BONOBO_IS_CONTROL (object));

	BONOBO_OBJECT_CLASS (gc3d_bonobo_control_parent_class)->destroy (object);
}

static void
gc3d_bonobo_control_finalize (GObject *object)
{
	GC3DBonoboControl *control;
	g_return_if_fail (object != NULL);
	g_return_if_fail (GC3D_BONOBO_IS_CONTROL (object));

	control = GC3D_BONOBO_CONTROL (object);
		
	G_OBJECT_CLASS (gc3d_bonobo_control_parent_class)->finalize (object);
}

static void
gc3d_bonobo_control_activate (BonoboControl *object, gboolean state)
{
	GC3DBonoboControl *control;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GC3D_BONOBO_IS_CONTROL (object));

	control = GC3D_BONOBO_CONTROL (object);

	if (BONOBO_CONTROL_CLASS (gc3d_bonobo_control_parent_class)->activate)
		BONOBO_CONTROL_CLASS (gc3d_bonobo_control_parent_class)->activate (object, state);
}

void gc3d_bonobo_control_class_init (GC3DBonoboControlClass *klass)
{
	GObjectClass *gobject_class = (GObjectClass *)klass;
	BonoboObjectClass *bonobo_object_class = (BonoboObjectClass *)klass;
	BonoboControlClass *control_class = (BonoboControlClass *)klass;

	gc3d_bonobo_control_parent_class = (GObjectClass *)g_type_class_peek_parent ((GObjectClass *)klass);

	bonobo_object_class->destroy = gc3d_bonobo_control_destroy;
	gobject_class->finalize = gc3d_bonobo_control_finalize;
	control_class->activate = gc3d_bonobo_control_activate;
}

static void
gc3d_bonobo_control_init (GC3DBonoboControl *control)
{
}

BONOBO_TYPE_FUNC (GC3DBonoboControl, BONOBO_TYPE_CONTROL, gc3d_bonobo_control);

GC3DBonoboControl* gc3d_bonobo_control_construct(GC3DBonoboControl *control, GtkChem3DViewer* viewer)
{
	BonoboPropertyBag     *pb;
	BonoboPropertyControl *pc;
	CORBA_Environment ev;
	
	g_return_val_if_fail (viewer != NULL, NULL);
	g_return_val_if_fail (control != NULL, NULL);
	g_return_val_if_fail (GC3D_BONOBO_IS_CONTROL (control), NULL);

	bonobo_control_construct (BONOBO_CONTROL (control), GTK_WIDGET(viewer));
	pb = bonobo_property_bag_new( 
			(void (*)(BonoboPropertyBag*, BonoboArg*, unsigned int,
   			CORBA_Environment*, void*))get_prop,
			(void (*)(BonoboPropertyBag*, const BonoboArg*, unsigned int,
   			CORBA_Environment*, void*))set_prop,
			viewer);
	bonobo_control_set_properties((BonoboControl*)control, bonobo_object_corba_objref(BONOBO_OBJECT(pb)), &ev);

	bonobo_property_bag_add(pb, "display3d", PROP_DISPLAY3D,
				 BONOBO_ARG_STRING, NULL,
				 _("Display3D mode"),
				 0);
	bonobo_property_bag_add(pb, "bgcolor", PROP_BGCOLOR,
				 BONOBO_ARG_STRING, NULL,
				 _("Background color"),
				 0);
	
	control->ps = persist_stream_new();
	control->ps->viewer = viewer;
	bonobo_object_add_interface((BonoboObject*)control, (BonoboObject*)control->ps);

	bonobo_object_unref (BONOBO_OBJECT (pb));

	return control;
}

GC3DBonoboControl *
gc3d_bonobo_control_new (GtkChem3DViewer* viewer)
{
	GC3DBonoboControl *control;
	control = (GC3DBonoboControl*) g_object_new (GC3D_BONOBO_CONTROL_TYPE, NULL);

	return gc3d_bonobo_control_construct (control, viewer);
}

BonoboObject * gc3d_factory(BonoboGenericFactory * fact, const char *component_id, void* data)
{
	g_return_val_if_fail (fact != NULL, NULL);
	g_return_val_if_fail (component_id != NULL, NULL);

	BonoboObject *retval;
	GtkChem3DViewer* viewer= (GtkChem3DViewer*)gtk_chem3d_viewer_new("");
	if (!viewer) return NULL;
	if (!strcmp (component_id, "OAFIID:gchem3d_control"))
	{
		retval = (BonoboObject*) gc3d_bonobo_control_new(viewer);
		if (!retval) gtk_widget_destroy((GtkWidget*)viewer);
	}
	else
	{
		g_warning ("Unknown IID `%s' requested", component_id);
		gtk_widget_destroy((GtkWidget*)viewer);
		return NULL;
	}

	return retval;
}

int main(int argc, char *argv[])
{
	gtk_init(&argc, &argv);
	BONOBO_FACTORY_INIT ("gchem3d", VERSION, &argc, argv);		
	return bonobo_generic_factory_main ("OAFIID:gchem3d_factory", gc3d_factory, NULL);
}
