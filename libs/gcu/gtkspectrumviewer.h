/* 
 * Gnome Chemisty Utils
 * gtkspectrumviewer.h
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

#ifndef GTK_SPECTRUM_VIEWER_H
#define GTK_SPECTRUM_VIEWER_H

#include <goffice/graph/gog-graph.h>
#include <gtk/gtkwidget.h>

G_BEGIN_DECLS

/*!\file
Declaration of the GtkSpectrumViewer widget.
*/

/*!\return the GType associated to GtkSpectrumViewer */
#define GTK_TYPE_SPECTRUM_VIEWER		  (gtk_spectrum_viewer_get_type ())
/*!
Casts \a obj to a GtkSpectrumViewer * pointer.
\return a pointer to the GtkSpectrumViewer * or NULL if \a obj does not point to 
a GtkSpectrumViewer widget.
*/
#define GTK_SPECTRUM_VIEWER(obj)		  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_SPECTRUM_VIEWER, GtkSpectrumViewer))
/*!
Casts \a klass to a GtkSpectrumViewerClass * pointer.
\return a pointer to the GtkSpectrumViewerClass * or NULL if \a obj not point to a GtkSpectrumViewerClass
*/
#define GTK_SPECTRUM_VIEWER_CLASS(klass)	  (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_SPECTRUM_VIEWER, GtkSpectrumViewerClass
/*!
\return TRUE if \a obj points to a GtkSpectrumViewer widget, FALSE otherwise.
*/
#define GTK_IS_SPECTRUM_VIEWER(obj)	  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_SPECTRUM_VIEWER))
/*!
\return TRUE if \a klass points to a GtkSpectrumViewerClass, FALSE otherwise.
*/
#define GTK_IS_SPECTRUM_VIEWER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_SPECTRUM_VIEWER))
/*!
\return the GtkSpectrumViewerClass * associated to \a obj if obj points to a GtkSpectrumViewer widget,
NULL otherwise.
*/
#define GTK_SPECTRUM_VIEWER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_SPECTRUM_VIEWER, GtkSpectrumViewerClass))

/*! \struct GtkSpectrumViewer gcu/gtkspectrumviewer.h
 The GtkSpectrumViewer displays using the goffice library. Only spectra in the JCAMP-DX format are supported in this
version.

The available functions related to the GtkSpectrumViewer Widget are described in the gtkspectrumviewer.h page.
*/

/*! The GtkSpectrumViewer widget.*/
typedef struct _GtkSpectrumViewer       GtkSpectrumViewer;
/*! The GtkSpectrumViewer widget object class.*/
typedef struct _GtkSpectrumViewerClass  GtkSpectrumViewerClass;

GType               gtk_spectrum_viewer_get_type          (void) G_GNUC_CONST;
/*!
@param uri the URI of the file containing the spectrum to display

Creates a GtkSpectrumViewer widget and fills it with the data from uri.
If uri is NULL, the widget will display an empty chart.
*/
GtkWidget*            gtk_spectrum_viewer_new               (const gchar* uri);
/*!
@param viewer a pointer to GtkSectrumViewer widget.
@param uri the URI of the file containing the spectrum to display.

Changes the molecule displayed by the one described in the uri. Nothing happens if uri is NULL.
*/
void	gtk_spectrum_viewer_set_uri	(GtkSpectrumViewer * viewer, const gchar * uri);

/*!
@param viewer a pointer to GtkSectrumViewer widget.

@return the graph displayed by the widget.
*/
GogGraph *gtk_spectrum_viewer_get_graph (GtkSpectrumViewer * viewer);

G_END_DECLS

#endif //	GTK_SPECTRUM_VIEWER_H
