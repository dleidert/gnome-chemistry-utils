/* 
 * Gnome Chemisty Utils
 * gcuspectrumviewer.cc
 *
 * Copyright (C) 2007-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "gcuspectrumviewer.h"
#include "spectrumdoc.h"
#include "spectrumview.h"
#include <gsf/gsf-impl-utils.h>
#include <gtk/gtkbin.h>

struct _GcuSpectrumViewer
{
	GtkBin base;

	gcugtk::SpectrumDocument *doc;
	GogGraph *graph;
};

struct _GcuSpectrumViewerClass
{
	GtkBinClass base;
};

static void
on_size (GcuSpectrumViewer* w, GtkAllocation *allocation, G_GNUC_UNUSED gpointer user_data)
{
	GtkWidget *widget = gtk_bin_get_child (GTK_BIN (w));
	bool visible = false;
	if (widget)
		g_object_get (G_OBJECT (widget), "visible", &visible, NULL);
	if (visible)
		gtk_widget_size_allocate (widget, allocation);
}

GtkWidget*
gcu_spectrum_viewer_new  (const gchar* uri)
{
	GcuSpectrumViewer *viewer = GCU_SPECTRUM_VIEWER (g_object_new (GCU_TYPE_SPECTRUM_VIEWER, NULL));
	viewer->doc = new gcugtk::SpectrumDocument ();
	gcugtk::SpectrumView *View = viewer->doc->GetView();
	GtkWidget* w = View->GetWidget ();
	viewer->graph = go_graph_widget_get_graph (GO_GRAPH_WIDGET (w));
	gtk_container_add (GTK_CONTAINER (viewer), w);
	g_signal_connect (G_OBJECT (viewer), "size_allocate", G_CALLBACK (on_size), NULL);
	gtk_widget_show (w);
	gcu_spectrum_viewer_set_uri (viewer, uri);
	return reinterpret_cast<GtkWidget*> (viewer);
}

void
gcu_spectrum_viewer_set_uri	(GcuSpectrumViewer * viewer, const gchar * uri)
{
	if (!uri)
		return;
	viewer->doc->Load (uri, "chemical/x-jcamp-dx");
	g_return_if_fail (GCU_IS_SPECTRUM_VIEWER (viewer));
}

GogGraph *
gcu_spectrum_viewer_get_graph (GcuSpectrumViewer * viewer)
{
	g_return_val_if_fail (GCU_IS_SPECTRUM_VIEWER (viewer), NULL);
	return viewer->graph;
}

void
gcu_spectrum_viewer_init (G_GNUC_UNUSED GcuSpectrumViewer * viewer)
{
}

void
gcu_spectrum_viewer_class_init (G_GNUC_UNUSED GcuSpectrumViewerClass *klass)
{
}

GSF_CLASS (GcuSpectrumViewer, gcu_spectrum_viewer,
	   gcu_spectrum_viewer_class_init, gcu_spectrum_viewer_init,
	   GTK_TYPE_BIN)
