/* 
 * Gnome Chemisty Utils
 * gtkchem3dviewer.c 
 *
 * Copyright (C) 2002-2003
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

#include "gtkchem3dviewer.h"

extern "C"
{

static void on_size(GtkChem3DViewer* w, GtkAllocation *allocation, gpointer user_data)
{
	if (GTK_BIN(w)->child && GTK_WIDGET_VISIBLE (GTK_BIN(w)->child))
		gtk_widget_size_allocate (GTK_BIN(w)->child, allocation);
}

typedef struct _GtkChem3DViewerPrivate
{
	guint glList;
} GtkChem3DViewerPrivate;


static GtkBinClass *parent_class = NULL;

static void gtk_chem3d_viewer_class_init (GtkChem3DViewerClass  *klass);
static void gtk_chem3d_viewer_init(GtkChem3DViewer *viewer);
static void gtk_chem3d_viewer_finalize(GObject* object);

GType
gtk_chem3d_viewer_get_type (void)
{
	static GType crystal_viewer_type = 0;
  
	if (!crystal_viewer_type)
	{
		static const GTypeInfo crystal_viewer_info =
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

//		crystal_viewer_type = g_type_register_static (GTK_TYPE_GL_AREA, "GtkChem3DViewer", &crystal_viewer_info, (GTypeFlags)0);
		crystal_viewer_type = g_type_register_static (GTK_TYPE_BIN, "GtkChem3DViewer", &crystal_viewer_info, (GTypeFlags)0);
	}
  
	return crystal_viewer_type;
}

void gtk_chem3d_viewer_class_init(GtkChem3DViewerClass  *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
//	parent_class = (GtkGLAreaClass*)gtk_type_class(gtk_gl_area_get_type());
	parent_class = (GtkBinClass*)gtk_type_class(gtk_bin_get_type());
	
	gobject_class->finalize = gtk_chem3d_viewer_finalize;
}

void gtk_chem3d_viewer_init(GtkChem3DViewer *viewer)
{
}

GtkWidget* gtk_chem3d_viewer_new(gchar *uri)
{
	GtkChem3DViewer* viewer = (GtkChem3DViewer*)g_object_new(GTK_TYPE_CHEM3D_VIEWER, NULL);
	viewer->priv = new GtkChem3DViewerPrivate;
	g_signal_connect(G_OBJECT(viewer), "size_allocate", GTK_SIGNAL_FUNC(on_size), NULL);
	gtk_chem3d_viewer_set_data (viewer, uri);
/*	gtk_widget_show(w);*/
	return GTK_WIDGET(viewer);
}

void gtk_chem3d_viewer_finalize(GObject* object)
{
	((GObjectClass*)parent_class)->finalize(object);
	GtkChem3DViewer* viewer = GTK_CHEM3D_VIEWER(object);
	delete viewer->priv;
}

void gtk_chem3d_viewer_set_data (GtkChem3DViewer * viewer, gchar *uri)
{
	g_return_if_fail (GTK_IS_CHEM3D_VIEWER(viewer));
	g_return_if_fail(uri);
}

} //extern "C"
