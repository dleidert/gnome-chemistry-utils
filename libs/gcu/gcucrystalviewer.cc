/* 
 * Gnome Chemisty Utils
 * gcucrystalviewer.cc 
 *
 * Copyright (C) 2002-2010 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "gcucrystalviewer.h"
#include "crystalview.h"
#include "crystaldoc.h"
#include "crystalatom.h"
#include "application.h"
#include "loader.h"
#include <cstring>

extern "C"
{

struct _GcuCrystalViewer
{
	GtkBin bin;

	gcu::CrystalView *pView;
	gcu::CrystalDoc *pDoc;
	guint glList;
};

struct _GcuCrystalViewerClass
{
	GtkBinClass parent_class;
};


static void on_size (GcuCrystalViewer* w, GtkAllocation *allocation, G_GNUC_UNUSED gpointer user_data)
{
	GtkWidget *widget = gtk_bin_get_child (GTK_BIN (w));
	if (widget && gtk_widget_get_visible (widget))
		gtk_widget_size_allocate (widget, allocation);
}

static GtkBinClass *parent_class = NULL;

static void gcu_crystal_viewer_class_init (GcuCrystalViewerClass  *klass);
static void gcu_crystal_viewer_init (GcuCrystalViewer *viewer);
static void gcu_crystal_viewer_finalize (GObject* object);

GType
gcu_crystal_viewer_get_type (void)
{
	static GType crystal_viewer_type = 0;
  
	if (!crystal_viewer_type)
	{
		static const GTypeInfo crystal_viewer_info =
		{
			sizeof (GcuCrystalViewerClass),
			NULL,           /* base_init */
			NULL,           /* base_finalize */
			(GClassInitFunc) gcu_crystal_viewer_class_init,
			NULL,           /* class_finalize */
			NULL,           /* class_data */
			sizeof (GcuCrystalViewer),
			0,              /* n_preallocs */
			(GInstanceInitFunc) gcu_crystal_viewer_init,
			NULL
		};

		crystal_viewer_type = g_type_register_static (GTK_TYPE_BIN, "GcuCrystalViewer", &crystal_viewer_info, (GTypeFlags)0);
	}
  
	return crystal_viewer_type;
}

void gcu_crystal_viewer_class_init (GcuCrystalViewerClass  *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	parent_class = (GtkBinClass*) g_type_class_peek_parent (klass);
	
	gobject_class->finalize = gcu_crystal_viewer_finalize;
}

void gcu_crystal_viewer_init (G_GNUC_UNUSED GcuCrystalViewer *viewer)
{
}

GtkWidget* gcu_crystal_viewer_new (xmlNodePtr node)
{
	GcuCrystalViewer* viewer = (GcuCrystalViewer*) g_object_new (GCU_TYPE_CRYSTAL_VIEWER, NULL);
	viewer->pDoc = new gcu::CrystalDoc (NULL);
	viewer->pView = viewer->pDoc->GetView();
	GtkWidget* w = viewer->pView->GetWidget ();
	gtk_container_add (GTK_CONTAINER (viewer), w);
	if (node)
		viewer->pDoc->ParseXMLTree (node);
	g_signal_connect (G_OBJECT (viewer), "size_allocate", G_CALLBACK (on_size), NULL);
	gtk_widget_show (w);
	return GTK_WIDGET (viewer);
}

void gcu_crystal_viewer_finalize (GObject* object)
{
	((GObjectClass*) parent_class)->finalize (object);
	GcuCrystalViewer* viewer = GCU_CRYSTAL_VIEWER (object);
	delete viewer->pView;
	delete viewer->pDoc;
}

void gcu_crystal_viewer_set_data (GcuCrystalViewer * viewer, xmlNodePtr node)
{
	g_return_if_fail (GCU_IS_CRYSTAL_VIEWER (viewer));
	g_return_if_fail (node);
	viewer->pDoc->ParseXMLTree (node);
	viewer->pView->Update ();
}

GdkPixbuf *gcu_crystal_viewer_new_pixbuf (GcuCrystalViewer * viewer, guint width, guint height)
{
	return viewer->pDoc->GetView ()->BuildPixbuf (width, height);
}

static gcu::Application *App = NULL;

static gcu::Object *CreateCrystalAtom ()
{
	return new gcu::CrystalAtom ();
}

void gcu_crystal_viewer_set_uri_with_mime_type (GcuCrystalViewer * viewer, const gchar * uri, const gchar* mime_type)
{
	if (mime_type == NULL) {
		g_message ("Cannot open an uri with unknown mime type.");
		return;
	}
	viewer->pDoc->Reinit ();
	if  (!strcmp (mime_type, "application/x-gcrystal")) {
/*		xmlDocPtr xml = xmlParseFile (filename.c_str ());
		if (!xml || !xml->children || strcmp ((char*) xml->children->name, "crystal")) {
			g_message ("Invalid data");
			return;
		}
		gcu_crystal_viewer_set_data (GCU_CRYSTAL_VIEWER (Viewer), xml->children);
		xmlFree (xml);*/
	} else {
		if (!App) {
			App = new gcu::Application ("GChemMoz");
			gcu::Object::AddType ("atom", CreateCrystalAtom, gcu::AtomType);
		}
		if (App->Load (uri, mime_type, viewer->pDoc) != gcu::ContentTypeCrystal)
			g_message ("Invalid data");
		viewer->pDoc->Loaded ();
		viewer->pDoc->Update ();
		viewer->pDoc->GetView ()->Update ();
	}
}

void gcu_crystal_viewer_set_uri	(GcuCrystalViewer * viewer, const gchar * uri)
{
	GVfs *vfs = g_vfs_get_default ();
	GFile *file = g_vfs_get_file_for_uri (vfs, uri);
	GError *error = NULL;
	GFileInfo *info = g_file_query_info (file,
										 G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE","G_FILE_ATTRIBUTE_STANDARD_SIZE,
										 G_FILE_QUERY_INFO_NONE,
										 NULL, &error);
	if (error) {
		g_message ("GIO querry failed: %s", error->message);
		g_error_free (error);
		g_object_unref (file);
		error = NULL;
		return;
	}
	gcu_crystal_viewer_set_uri_with_mime_type (viewer, uri, g_file_info_get_content_type (info));
}

} //extern "C"
