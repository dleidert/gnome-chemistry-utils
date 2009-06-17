/* 
 * Gnome Chemisty Utils
 * gtkspectrumviewer.cc
 *
 * Copyright (C) 2007 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "gtkspectrumviewer.h"
#include "spectrumdoc.h"
#include "spectrumview.h"
#ifndef GOFFICE_HAS_GLOBAL_HEADER
#   include <goffice/gtk/go-graph-widget.h>
#   include <gtk/gtk.h>
#endif
#include <gsf/gsf-impl-utils.h>

struct _GtkSpectrumViewer
{
	GtkBin base;

	gcu::SpectrumDocument *doc;
	GogGraph *graph;
};

struct _GtkSpectrumViewerClass
{
	GtkBinClass base;
};

static void
on_size (GtkSpectrumViewer* w, GtkAllocation *allocation, gpointer user_data)
{
	if (GTK_BIN (w)->child && GTK_WIDGET_VISIBLE (GTK_BIN (w)->child))
		gtk_widget_size_allocate (GTK_BIN (w)->child, allocation);
}

GtkWidget*
gtk_spectrum_viewer_new  (const gchar* uri)
{
	GtkSpectrumViewer *viewer = GTK_SPECTRUM_VIEWER (g_object_new (GTK_TYPE_SPECTRUM_VIEWER, NULL));
	viewer->doc = new gcu::SpectrumDocument ();
	gcu::SpectrumView *View = viewer->doc->GetView();
	GtkWidget* w = View->GetWidget ();
	viewer->graph = go_graph_widget_get_graph (GO_GRAPH_WIDGET (w));
	gtk_container_add (GTK_CONTAINER (viewer), w);
	g_signal_connect (G_OBJECT (viewer), "size_allocate", G_CALLBACK (on_size), NULL);
	gtk_widget_show (w);
	gtk_spectrum_viewer_set_uri (viewer, uri);
	return reinterpret_cast<GtkWidget*> (viewer);
}

void
gtk_spectrum_viewer_set_uri	(GtkSpectrumViewer * viewer, const gchar * uri)
{
	if (!uri)
		return;
	char *old_locale = g_strdup (setlocale (LC_NUMERIC, NULL));
	setlocale (LC_NUMERIC, "C");
	viewer->doc->Load (uri, "chemical/x-jcamp-dx");
	setlocale (LC_NUMERIC, old_locale);
	g_free (old_locale);
	g_return_if_fail (GTK_IS_SPECTRUM_VIEWER (viewer));
}

GogGraph *
gtk_spectrum_viewer_get_graph (GtkSpectrumViewer * viewer)
{
	g_return_val_if_fail (GTK_IS_SPECTRUM_VIEWER (viewer), NULL);
	return viewer->graph;
}

void
gtk_spectrum_viewer_init (GtkSpectrumViewer * viewer)
{
}

void
gtk_spectrum_viewer_class_init (GtkSpectrumViewerClass *klass)
{
}

GSF_CLASS (GtkSpectrumViewer, gtk_spectrum_viewer,
	   gtk_spectrum_viewer_class_init, gtk_spectrum_viewer_init,
	   GTK_TYPE_BIN)
