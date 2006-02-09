/* 
 * Gnome Chemisty Utils
 * gtkcrystalviewer.c 
 *
 * Copyright (C) 2002-2004 Jean Bréfort <jean.brefort@normalesup.org>
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

#include "gtkcrystalviewer.h"
#include "crystalview.h"
#include "crystaldoc.h"

extern "C"
{

static void on_size(GtkCrystalViewer* w, GtkAllocation *allocation, gpointer user_data)
{
	if (GTK_BIN(w)->child && GTK_WIDGET_VISIBLE (GTK_BIN(w)->child))
		gtk_widget_size_allocate (GTK_BIN(w)->child, allocation);
}

typedef struct _GtkCrystalViewerPrivate
{
	gcu::CrystalView *pView;
	gcu::CrystalDoc *pDoc;
	guint glList;
} GtkCrystalViewerPrivate;


static GtkBinClass *parent_class = NULL;

static void gtk_crystal_viewer_class_init (GtkCrystalViewerClass  *klass);
static void gtk_crystal_viewer_init(GtkCrystalViewer *viewer);
static void gtk_crystal_viewer_finalize(GObject* object);

GType
gtk_crystal_viewer_get_type (void)
{
	static GType crystal_viewer_type = 0;
  
	if (!crystal_viewer_type)
	{
		static const GTypeInfo crystal_viewer_info =
		{
			sizeof (GtkCrystalViewerClass),
			NULL,           /* base_init */
			NULL,           /* base_finalize */
			(GClassInitFunc) gtk_crystal_viewer_class_init,
			NULL,           /* class_finalize */
			NULL,           /* class_data */
			sizeof (GtkCrystalViewer),
			0,              /* n_preallocs */
			(GInstanceInitFunc) gtk_crystal_viewer_init,
		};

//		crystal_viewer_type = g_type_register_static (GTK_TYPE_GL_AREA, "GtkCrystalViewer", &crystal_viewer_info, (GTypeFlags)0);
		crystal_viewer_type = g_type_register_static (GTK_TYPE_BIN, "GtkCrystalViewer", &crystal_viewer_info, (GTypeFlags)0);
	}
  
	return crystal_viewer_type;
}

void gtk_crystal_viewer_class_init(GtkCrystalViewerClass  *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
//	parent_class = (GtkGLAreaClass*)gtk_type_class(gtk_gl_area_get_type());
	parent_class = (GtkBinClass*)gtk_type_class(gtk_bin_get_type());
	
	gobject_class->finalize = gtk_crystal_viewer_finalize;
}

void gtk_crystal_viewer_init(GtkCrystalViewer *viewer)
{
}

GtkWidget* gtk_crystal_viewer_new(xmlNodePtr node)
{
	GtkCrystalViewer* viewer = (GtkCrystalViewer*)g_object_new(GTK_TYPE_CRYSTAL_VIEWER, NULL);
	viewer->priv = new GtkCrystalViewerPrivate;
	viewer->priv->pDoc = new gcu::CrystalDoc();
	viewer->priv->pView = viewer->priv->pDoc->GetView();
	GtkWidget* w = viewer->priv->pView->CreateNewWidget();
	gtk_container_add(GTK_CONTAINER(viewer), w);
	if (node) viewer->priv->pDoc->ParseXMLTree(node);
	g_signal_connect(G_OBJECT(viewer), "size_allocate", GTK_SIGNAL_FUNC(on_size), NULL);
	gtk_widget_show(w);
	return GTK_WIDGET(viewer);
}

void gtk_crystal_viewer_finalize (GObject* object)
{
	((GObjectClass*) parent_class)->finalize (object);
	GtkCrystalViewer* viewer = GTK_CRYSTAL_VIEWER (object);
	delete viewer->priv->pView;
	delete viewer->priv->pDoc;
	delete viewer->priv;
}

void gtk_crystal_viewer_set_data (GtkCrystalViewer * viewer, xmlNodePtr node)
{
	g_return_if_fail (GTK_IS_CRYSTAL_VIEWER(viewer));
	g_return_if_fail(node);
	viewer->priv->pDoc->ParseXMLTree(node);
}

} //extern "C"
