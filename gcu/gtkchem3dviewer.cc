/* 
 * Gnome Chemisty Utils
 * gtkchem3dviewer.c 
 *
 * Copyright (C) 2003-2007 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "gtkchem3dviewer.h"
#include "chem3ddoc.h"
#include "glview.h"
#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>
#include <cstring>

using namespace std;
using namespace OpenBabel;
using namespace gcu;

struct _GtkChem3DViewer
{
	GtkBin bin;

	Chem3dDoc *Doc;
	GtkWidget *widget;
};

struct _GtkChem3DViewerClass
{
	GtkBinClass parent_class;
};

enum {
	PROP_0,
	PROP_DISPLAY3D,
	PROP_BGCOLOR
};
GType
gtk_display3d_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { BALL_AND_STICK, "BALL_AND_STICK", "ball&stick" },
      { SPACEFILL, "SPACEFILL", "spacefill" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("Dispay3D", values);
  }
  return etype;
}

static GtkBinClass *parent_class = NULL;

static void gtk_chem3d_viewer_class_init (GtkChem3DViewerClass  *klass);
static void gtk_chem3d_viewer_init(GtkChem3DViewer *viewer);
static void gtk_chem3d_viewer_update(GtkChem3DViewer *viewer);
static void gtk_chem3d_viewer_set_property(GObject *object, guint property_id,
						const GValue *value, GParamSpec *pspec);
static void gtk_chem3d_viewer_get_property(GObject *object, guint property_id,
						GValue *value, GParamSpec *pspec);

extern "C"
{

GType
gtk_chem3d_viewer_get_type (void)
{
	static GType chem3d_viewer_type = 0;
  
	if (!chem3d_viewer_type)
	{
		static const GTypeInfo chem3d_viewer_info =
		{
			sizeof (GtkChem3DViewerClass),
			NULL,           /* base_init */
			NULL,           /* base_finalize */
			(GClassInitFunc) gtk_chem3d_viewer_class_init,
			NULL,           /* class_finalize */
			NULL,           /* class_data */
			sizeof (GtkChem3DViewer),
			0,              /* n_preallocs */
			(GInstanceInitFunc) gtk_chem3d_viewer_init,
		};

		chem3d_viewer_type = g_type_register_static (GTK_TYPE_BIN, "GtkChem3DViewer", &chem3d_viewer_info, (GTypeFlags)0);
	}
  
	return chem3d_viewer_type;
}

GtkWidget* gtk_chem3d_viewer_new (const gchar *uri)
{
	GtkChem3DViewer* viewer = (GtkChem3DViewer*) g_object_new (GTK_TYPE_CHEM3D_VIEWER, NULL);
	if (uri)
		gtk_chem3d_viewer_set_uri (viewer, uri);
	return GTK_WIDGET (viewer);
}

} //extern "C"

static void on_size(GtkWidget *w, GtkAllocation *allocation, gpointer data)
{
	if (GTK_BIN (w)->child && GTK_WIDGET_VISIBLE (GTK_BIN (w)->child))
		gtk_widget_size_allocate (GTK_BIN (w)->child, allocation);
}

static void gtk_chem3d_viewer_finalize (GObject *obj)
{
	GtkChem3DViewer *viewer = GTK_CHEM3D_VIEWER (obj);
	if (viewer->Doc)  {
		delete viewer->Doc->GetView ();
		delete viewer->Doc;
	}
	G_OBJECT_CLASS (parent_class)->finalize (obj);
}

void gtk_chem3d_viewer_class_init (GtkChem3DViewerClass  *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	parent_class = (GtkBinClass*)gtk_type_class(gtk_bin_get_type());
	
	gobject_class->set_property = gtk_chem3d_viewer_set_property;
	gobject_class->get_property = gtk_chem3d_viewer_get_property;
	gobject_class->finalize = gtk_chem3d_viewer_finalize;
	
	g_object_class_install_property (
		gobject_class,
		PROP_DISPLAY3D,
		g_param_spec_enum("display3d",
						"3D display mode",
						"Mode used to display the model",
						GTK_DISPLAY_3D,
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
}

void gtk_chem3d_viewer_init (GtkChem3DViewer *viewer)
{
	g_return_if_fail (GTK_IS_CHEM3D_VIEWER (viewer));
	viewer->Doc = new Chem3dDoc ();
	viewer->widget = viewer->Doc->GetView ()->GetWidget ();
	gtk_widget_show (GTK_WIDGET (viewer->widget));
	gtk_container_add (GTK_CONTAINER (viewer), viewer->widget);
	gtk_widget_show_all (GTK_WIDGET (viewer));
	g_signal_connect (G_OBJECT (viewer), "size_allocate", GTK_SIGNAL_FUNC (on_size), NULL);
}

void gtk_chem3d_viewer_set_uri (GtkChem3DViewer * viewer, const gchar *uri)
{
	gtk_chem3d_viewer_set_uri_with_mime_type (viewer, uri, NULL);
}

void gtk_chem3d_viewer_set_uri_with_mime_type (GtkChem3DViewer * viewer, const gchar * uri, const gchar* mime_type)
{
	g_return_if_fail (GTK_IS_CHEM3D_VIEWER (viewer));
	g_return_if_fail (uri);
	viewer->Doc->Load (uri, mime_type);
}

void gtk_chem3d_viewer_set_data (GtkChem3DViewer * viewer, const gchar *data, const gchar* mime_type)
{
	viewer->Doc->LoadData (data, mime_type);
}

void gtk_chem3d_viewer_update (GtkChem3DViewer *viewer)
{
	viewer->Doc->GetView ()->Update ();
}

static void gtk_chem3d_viewer_get_property (GObject *object, guint property_id,
				     GValue *value, GParamSpec *pspec)
{
	GtkChem3DViewer *viewer = GTK_CHEM3D_VIEWER(object);

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

static void gtk_chem3d_viewer_set_property (GObject *object, guint property_id,
				     const GValue *value, GParamSpec *pspec)
{
	GtkChem3DViewer *viewer = GTK_CHEM3D_VIEWER(object);

	switch (property_id) {
		case PROP_DISPLAY3D:
			viewer->Doc->SetDisplay3D ((Display3DMode) g_value_get_enum (value));
			break;
		case PROP_BGCOLOR:
			{
				const gchar* str = g_value_get_string (value);
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
	gtk_chem3d_viewer_update(viewer);
}

void gtk_chem3d_viewer_print (GtkChem3DViewer * viewer, GnomePrintContext *pc, gdouble width, gdouble height)
{
	viewer->Doc->GetView ()->Print (pc, width, height);
}
