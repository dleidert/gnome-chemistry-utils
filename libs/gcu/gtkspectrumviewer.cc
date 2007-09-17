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
#include <goffice/gtk/go-graph-widget.h>
#include <gsf/gsf-impl-utils.h>
#include <gtk/gtkbin.h>

struct _GtkSpectrumViewer
{
	GtkBin base;

	GogGraph *graph;
};

struct _GtkSpectrumViewerClass
{
	GtkBinClass base;
};

GtkWidget*
gtk_spectrum_viewer_new  (const gchar* uri)
{
	GtkSpectrumViewer *viewer = GTK_SPECTRUM_VIEWER (g_object_new (GTK_TYPE_SPECTRUM_VIEWER, NULL));
	gtk_spectrum_viewer_set_uri (viewer, uri);
	return reinterpret_cast<GtkWidget*> (viewer);
}

void
gtk_spectrum_viewer_set_uri	(GtkSpectrumViewer * viewer, const gchar * uri)
{
	if (!uri)
		return;
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
	   GTK_TYPE_WIDGET)
