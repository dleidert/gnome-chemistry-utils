/*
 * Gnome Chemisty Utils
 * gcr/gcrcrystalviewer.cc
 *
 * Copyright (C) 2002-2010 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "gcrcrystalviewer.h"
#include "view.h"
#include "document.h"
#include "atom.h"
#include <gcu/application.h>
#include <gcu/loader.h>
#include <gcu/xml-utils.h>
#include <cstring>

extern "C"
{

struct _GcrCrystalViewer
{
	GtkBin bin;

	gcr::View *pView;
	gcr::Document *pDoc;
	guint glList;
	double psi, theta, phi;
};

struct _GcrCrystalViewerClass
{
	GtkBinClass parent_class;
};


static void on_size (GcrCrystalViewer* w, GtkAllocation *allocation, G_GNUC_UNUSED gpointer user_data)
{
	GtkWidget *widget = gtk_bin_get_child (GTK_BIN (w));
	if (widget && gtk_widget_get_visible (widget))
		gtk_widget_size_allocate (widget, allocation);
}

static GtkBinClass *parent_class = NULL;

static void gcr_crystal_viewer_class_init (GcrCrystalViewerClass  *klass);
static void gcr_crystal_viewer_init (GcrCrystalViewer *viewer);
static void gcr_crystal_viewer_finalize (GObject* object);

GType
gcr_crystal_viewer_get_type (void)
{
	static GType crystal_viewer_type = 0;

	if (!crystal_viewer_type)
	{
		static const GTypeInfo crystal_viewer_info =
		{
			sizeof (GcrCrystalViewerClass),
			NULL,           /* base_init */
			NULL,           /* base_finalize */
			(GClassInitFunc) gcr_crystal_viewer_class_init,
			NULL,           /* class_finalize */
			NULL,           /* class_data */
			sizeof (GcrCrystalViewer),
			0,              /* n_preallocs */
			(GInstanceInitFunc) gcr_crystal_viewer_init,
			NULL
		};

		crystal_viewer_type = g_type_register_static (GTK_TYPE_BIN, "GcrCrystalViewer", &crystal_viewer_info, (GTypeFlags)0);
	}

	return crystal_viewer_type;
}

GtkWidget* gcr_crystal_viewer_new (xmlNodePtr node)
{
	GcrCrystalViewer* viewer = (GcrCrystalViewer*) g_object_new (GCR_TYPE_CRYSTAL_VIEWER, NULL);
	viewer->pDoc = new gcr::Document (gcu::Application::GetDefaultApplication ());
	viewer->pView = viewer->pDoc->GetView();
	GtkWidget* w = viewer->pView->GetWidget ();
	gtk_container_add (GTK_CONTAINER (viewer), w);
	if (node)
		viewer->pDoc->ParseXMLTree (node);
	g_signal_connect (G_OBJECT (viewer), "size_allocate", G_CALLBACK (on_size), NULL);
	gtk_widget_show (w);
	return GTK_WIDGET (viewer);
}

static void gcr_crystal_viewer_get_preferred_height (GtkWidget *w, gint *minimum_height, gint *natural_height)
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

static void gcr_crystal_viewer_get_preferred_width (GtkWidget *w, gint *minimum_width, gint *natural_width)
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

static void gcr_crystal_viewer_size_allocate (GtkWidget *w, GtkAllocation *alloc)
{
	GtkWidget *child = gtk_bin_get_child (GTK_BIN (w));
	gboolean visible = FALSE;
	if (child)
		g_object_get (G_OBJECT (child), "visible", &visible, NULL);
	if (visible)
		gtk_widget_size_allocate (child, alloc);
	(GTK_WIDGET_CLASS (parent_class))->size_allocate (w, alloc);
}

void gcr_crystal_viewer_finalize (GObject* object)
{
	GcrCrystalViewer* viewer = GCR_CRYSTAL_VIEWER (object);
	delete viewer->pView;
	delete viewer->pDoc;
	((GObjectClass*) parent_class)->finalize (object);
}

void gcr_crystal_viewer_class_init (GcrCrystalViewerClass  *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	GtkWidgetClass *widget_class = reinterpret_cast < GtkWidgetClass * > (klass);
	parent_class = (GtkBinClass*) g_type_class_peek_parent (klass);

	gobject_class->finalize = gcr_crystal_viewer_finalize;
	widget_class->get_preferred_height = gcr_crystal_viewer_get_preferred_height;
	widget_class->get_preferred_width = gcr_crystal_viewer_get_preferred_width;
	widget_class->size_allocate = gcr_crystal_viewer_size_allocate;
}

void gcr_crystal_viewer_init (G_GNUC_UNUSED GcrCrystalViewer *viewer)
{
}

void gcr_crystal_viewer_set_data (GcrCrystalViewer * viewer, xmlNodePtr node)
{
	g_return_if_fail (GCR_IS_CRYSTAL_VIEWER (viewer));
	g_return_if_fail (node);
	viewer->pDoc->ParseXMLTree (node);
	viewer->pView->Update ();
	viewer->psi = viewer->pView->GetPsi ();
	viewer->theta = viewer->pView->GetTheta ();
	viewer->phi = viewer->pView->GetPhi ();
}

GdkPixbuf *gcr_crystal_viewer_new_pixbuf (GcrCrystalViewer * viewer, guint width, guint height, gboolean use_bg)
{
	return viewer->pDoc->GetView ()->BuildPixbuf (width, height, use_bg);
}

static gcu::Application *App = NULL;

static gcu::Object *CreateCrystalAtom ()
{
	return new gcr::Atom ();
}

void gcr_crystal_viewer_set_uri_with_mime_type (GcrCrystalViewer * viewer, const gchar * uri, const gchar* mime_type)
{
	if (mime_type == NULL) {
		g_message ("Cannot open an uri with unknown mime type.");
		return;
	}
	viewer->pDoc->Reinit ();
	if  (!strcmp (mime_type, "application/x-gcrystal")) {
		xmlDocPtr xml = gcu::ReadXMLDocFromURI (uri, NULL, NULL);
		if (!xml || !xml->children || strcmp ((char*) xml->children->name, "crystal")) {
			g_message ("Invalid data");
			return;
		}
		gcr_crystal_viewer_set_data (GCR_CRYSTAL_VIEWER (viewer), xml->children);
		xmlFree (xml);
	} else {
		if (!App) {
			App = viewer->pDoc->GetApp ();
			App->AddType ("atom", CreateCrystalAtom, gcu::AtomType);
		}
		if (App->Load (uri, mime_type, viewer->pDoc) != gcu::ContentTypeCrystal)
			g_message ("Invalid data");
		viewer->pDoc->Loaded ();
		viewer->pDoc->Update ();
		viewer->pDoc->GetView ()->Update ();
		viewer->psi = viewer->pView->GetPsi ();
		viewer->theta = viewer->pView->GetTheta ();
		viewer->phi = viewer->pView->GetPhi ();
	}
}

void gcr_crystal_viewer_set_uri	(GcrCrystalViewer * viewer, const gchar * uri)
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
	gcr_crystal_viewer_set_uri_with_mime_type (viewer, uri, g_file_info_get_content_type (info));
}

void gcr_crystal_viewer_back_to_initial_orientation (GcrCrystalViewer * viewer)
{
	g_return_if_fail (GCR_IS_CRYSTAL_VIEWER (viewer));
	viewer->pDoc->GetView ()->SetRotation (viewer->psi, viewer->theta, viewer->phi);
	viewer->pDoc->GetView ()->Update ();
}

} //extern "C"
