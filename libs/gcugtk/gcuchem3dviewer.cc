/*
 * Gnome Chemisty Utils
 * gcuchem3dviewer.c
 *
 * Copyright (C) 2003-2011 Jean Bréfort <jean.brefort@normalesup.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the
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
#include "gcuchem3dviewer.h"
#include "glview.h"
#include "chem3ddoc.h"
#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>
#include <cstring>

using namespace std;
using namespace gcu;

struct _GcuChem3DViewer
{
	GtkBin bin;

	gcugtk::Chem3dDoc *Doc;
	GtkWidget *widget;
	double psi, theta, phi;
};

struct _GcuChem3DViewerClass
{
	GtkBinClass parent_class;
};

enum {
	PROP_0,
	PROP_DISPLAY3D,
	PROP_BGCOLOR
};

GType
gcu_display3d_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { BALL_AND_STICK, "BALL_AND_STICK", "ball&stick" },
      { SPACEFILL, "SPACEFILL", "spacefill" },
      { CYLINDERS, "CYLINDERS", "cylinders" },
      { WIREFRAME, "WIREFRAME", "wireframe" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GcuDispay3D", values);
  }
  return etype;
}

static GtkBinClass *parent_class = NULL;

static void gcu_chem3d_viewer_class_init (GcuChem3DViewerClass  *klass);
static void gcu_chem3d_viewer_init(GcuChem3DViewer *viewer);
static void gcu_chem3d_viewer_update(GcuChem3DViewer *viewer);
static void gcu_chem3d_viewer_set_property(GObject *object, guint property_id,
						const GValue *value, GParamSpec *pspec);
static void gcu_chem3d_viewer_get_property(GObject *object, guint property_id,
						GValue *value, GParamSpec *pspec);

extern "C"
{

GType
gcu_chem3d_viewer_get_type (void)
{
	static GType chem3d_viewer_type = 0;

	if (!chem3d_viewer_type)
	{
		static const GTypeInfo chem3d_viewer_info =
		{
			sizeof (GcuChem3DViewerClass),
			NULL,           /* base_init */
			NULL,           /* base_finalize */
			(GClassInitFunc) gcu_chem3d_viewer_class_init,
			NULL,           /* class_finalize */
			NULL,           /* class_data */
			sizeof (GcuChem3DViewer),
			0,              /* n_preallocs */
			(GInstanceInitFunc) gcu_chem3d_viewer_init,
			NULL
		};

		chem3d_viewer_type = g_type_register_static (GTK_TYPE_BIN, "GcuChem3DViewer", &chem3d_viewer_info, (GTypeFlags)0);
	}

	return chem3d_viewer_type;
}

GtkWidget* gcu_chem3d_viewer_new (char const *uri)
{
	GcuChem3DViewer* viewer = (GcuChem3DViewer*) g_object_new (GCU_TYPE_CHEM3D_VIEWER, NULL);
	if (uri)
		gcu_chem3d_viewer_set_uri (viewer, uri);
	return GTK_WIDGET (viewer);
}

} //extern "C"

static void on_size(GtkWidget *w, GtkAllocation *allocation, G_GNUC_UNUSED gpointer data)
{
	GtkWidget *widget = gtk_bin_get_child (GTK_BIN (w));
	if (widget && gtk_widget_get_visible (widget))
		gtk_widget_size_allocate (widget, allocation);
}

static void gcu_chem3d_viewer_get_preferred_height (GtkWidget *w, gint *minimum_height, gint *natural_height)
{
	GtkWidget *child = gtk_bin_get_child (GTK_BIN (w));
	gboolean visible = FALSE;
	if (child)
		g_object_get (G_OBJECT (child), "visible", &visible, NULL);
	if (visible)
		gtk_widget_get_preferred_height (child, minimum_height, natural_height);
	else
		*minimum_height = *natural_height = 0;
}

static void gcu_chem3d_viewer_get_preferred_width (GtkWidget *w, gint *minimum_width, gint *natural_width)
{
	GtkWidget *child = gtk_bin_get_child (GTK_BIN (w));
	gboolean visible = FALSE;
	if (child)
		g_object_get (G_OBJECT (child), "visible", &visible, NULL);
	if (visible)
		gtk_widget_get_preferred_width (child, minimum_width, natural_width);
	else
		*minimum_width = *natural_width = 0;
}

static void gcu_chem3d_viewer_size_allocate (GtkWidget *w, GtkAllocation *alloc)
{
	GtkWidget *child = gtk_bin_get_child (GTK_BIN (w));
	gboolean visible = FALSE;
	if (child)
		g_object_get (G_OBJECT (child), "visible", &visible, NULL);
	if (visible)
		gtk_widget_size_allocate (child, alloc);
	(GTK_WIDGET_CLASS (parent_class))->size_allocate (w, alloc);
}

static void gcu_chem3d_viewer_finalize (GObject *obj)
{
	GcuChem3DViewer *viewer = GCU_CHEM3D_VIEWER (obj);
	if (viewer->Doc)  {
		delete viewer->Doc->GetView ();
		delete viewer->Doc;
	}
	G_OBJECT_CLASS (parent_class)->finalize (obj);
}

void gcu_chem3d_viewer_class_init (GcuChem3DViewerClass  *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	GtkWidgetClass *widget_class = reinterpret_cast < GtkWidgetClass * > (klass);
	parent_class = (GtkBinClass*) g_type_class_peek_parent (klass);

	gobject_class->set_property = gcu_chem3d_viewer_set_property;
	gobject_class->get_property = gcu_chem3d_viewer_get_property;
	gobject_class->finalize = gcu_chem3d_viewer_finalize;

	g_object_class_install_property (
		gobject_class,
		PROP_DISPLAY3D,
		g_param_spec_enum("display3d",
						"3D display mode",
						"Mode used to display the model",
						GCU_DISPLAY_3D,
						BALL_AND_STICK,
						(GParamFlags)G_PARAM_READWRITE));
        g_object_class_install_property
                (gobject_class,
                 PROP_BGCOLOR,
                 g_param_spec_string ("bgcolor",
				      _("Background Color"),
				      _("Color used to paint the background"),
                                      "black",
                                      (GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE)));
	widget_class->get_preferred_height = gcu_chem3d_viewer_get_preferred_height;
	widget_class->get_preferred_width = gcu_chem3d_viewer_get_preferred_width;
	widget_class->size_allocate = gcu_chem3d_viewer_size_allocate;
}

void gcu_chem3d_viewer_init (GcuChem3DViewer *viewer)
{
	g_return_if_fail (GCU_IS_CHEM3D_VIEWER (viewer));
	viewer->Doc = new gcugtk::Chem3dDoc ();
	viewer->widget = static_cast <gcugtk::GLView *> (viewer->Doc->GetView ())->GetWidget ();
	gtk_widget_show (GTK_WIDGET (viewer->widget));
	gtk_container_add (GTK_CONTAINER (viewer), viewer->widget);
	gtk_widget_show_all (GTK_WIDGET (viewer));
	g_signal_connect (G_OBJECT (viewer), "size_allocate", G_CALLBACK (on_size), NULL);
}

void gcu_chem3d_viewer_set_uri (GcuChem3DViewer * viewer, char const *uri)
{
	gcu_chem3d_viewer_set_uri_with_mime_type (viewer, uri, NULL);
}

void gcu_chem3d_viewer_set_uri_with_mime_type (GcuChem3DViewer * viewer, char const *uri, char const *mime_type)
{
	g_return_if_fail (GCU_IS_CHEM3D_VIEWER (viewer));
	g_return_if_fail (uri);
	viewer->Doc->Load (uri, mime_type);
	viewer->psi = viewer->Doc->GetView ()->GetPsi ();
	viewer->theta = viewer->Doc->GetView ()->GetTheta ();
	viewer->phi = viewer->Doc->GetView ()->GetPhi ();
}

void gcu_chem3d_viewer_set_data (GcuChem3DViewer * viewer, char const *data, char const *mime_type, size_t size)
{
	viewer->Doc->LoadData (data, mime_type, size);
	viewer->psi = viewer->Doc->GetView ()->GetPsi ();
	viewer->theta = viewer->Doc->GetView ()->GetTheta ();
	viewer->phi = viewer->Doc->GetView ()->GetPhi ();
}

void gcu_chem3d_viewer_update (GcuChem3DViewer *viewer)
{
	viewer->Doc->GetView ()->Update ();
}

static void gcu_chem3d_viewer_get_property (GObject *object, guint property_id,
				     GValue *value, GParamSpec *pspec)
{
	GcuChem3DViewer *viewer = GCU_CHEM3D_VIEWER(object);

	switch (property_id) {
	case PROP_DISPLAY3D:
		g_value_set_enum (value, viewer->Doc->GetDisplay3D ());
		break;
	case PROP_BGCOLOR:
		{
			int r = (int) (viewer->Doc->GetView ()->GetRed () * 255.),
				g = (int) (viewer->Doc->GetView ()->GetGreen () * 255.),
				b = (int) (viewer->Doc->GetView ()->GetBlue () * 255.);
			if ((r ==0) && (g == 0) && (b == 0))
				g_value_set_string (value, "black");
			else if ((r ==255) && (g == 255) && (b == 255))
				g_value_set_string (value, "white");
			else
			{
				char buf[10];
				g_snprintf(buf, sizeof(buf), "#%2x%2x%2x", r, g, b);
				g_value_set_string(value, buf);
			}
		}
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
		break;
	}
}

static void gcu_chem3d_viewer_set_property (GObject *object, guint property_id,
				     const GValue *value, GParamSpec *pspec)
{
	GcuChem3DViewer *viewer = GCU_CHEM3D_VIEWER(object);

	switch (property_id) {
		case PROP_DISPLAY3D:
			viewer->Doc->SetDisplay3D ((Display3DMode) g_value_get_enum (value));
			break;
		case PROP_BGCOLOR:
			{
				char const *str = g_value_get_string (value);
				if (!strcmp (str, "black")) {
					viewer->Doc->GetView ()->SetRed (0.);
					viewer->Doc->GetView ()->SetGreen (0.);
					viewer->Doc->GetView ()->SetBlue (0.);
				} else if (!strcmp (str, "white")) {
					viewer->Doc->GetView ()->SetRed (1.);
					viewer->Doc->GetView ()->SetGreen (1.);
					viewer->Doc->GetView ()->SetBlue (1.);
				} else {
					if ((strlen (str) != 7) || (*str != '#')) {
						g_warning ("Unrecognized color: %s\n", str);
						break;
					}
					int r, g, b;
					r = strtoul (str + 1, NULL, 16);
					b = r & 0xff;
					viewer->Doc->GetView ()->SetBlue ((float) b / 255.);
					r >>= 8;
					g = r & 0xff;
					viewer->Doc->GetView ()->SetGreen ((float) g / 255.);
					r >>=8;
					viewer->Doc->GetView ()->SetRed ((float) r / 255.);
				}
			}
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
			break;
	}
	gcu_chem3d_viewer_update(viewer);
}

GdkPixbuf *gcu_chem3d_viewer_new_pixbuf (GcuChem3DViewer * viewer, unsigned width, unsigned height, gboolean use_bg)
{
	return viewer->Doc->GetView ()->BuildPixbuf (width, height, use_bg);
}

void gcu_chem3d_viewer_back_to_initial_orientation (GcuChem3DViewer * viewer)
{
	g_return_if_fail (GCU_IS_CHEM3D_VIEWER (viewer));
	viewer->Doc->GetView ()->SetRotation (viewer->psi, viewer->theta, viewer->phi);
	viewer->Doc->GetView ()->Update ();
}
