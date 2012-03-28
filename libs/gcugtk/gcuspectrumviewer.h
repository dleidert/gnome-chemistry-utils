/*
 * Gnome Chemisty Utils
 * gcuspectrumviewer.h
 *
 * Copyright (C) 2007-2012 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCU_SPECTRUM_VIEWER_H
#define GCU_SPECTRUM_VIEWER_H

#include <goffice/goffice.h>

G_BEGIN_DECLS

/*!\file
Declaration of the GcuSpectrumViewer widget.
*/

/*!\return the GType associated to GcuSpectrumViewer */
#define GCU_TYPE_SPECTRUM_VIEWER		  (gcu_spectrum_viewer_get_type ())
/*!
Casts \a obj to a GcuSpectrumViewer * pointer.
\return a pointer to the GcuSpectrumViewer * or NULL if \a obj does not point to
a GcuSpectrumViewer widget.
*/
#define GCU_SPECTRUM_VIEWER(obj)		  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GCU_TYPE_SPECTRUM_VIEWER, GcuSpectrumViewer))
/*!
Casts \a klass to a GcuSpectrumViewerClass * pointer.
\return a pointer to the GcuSpectrumViewerClass * or NULL if \a obj not point to a GcuSpectrumViewerClass
*/
#define GCU_SPECTRUM_VIEWER_CLASS(klass)	  (G_TYPE_CHECK_CLASS_CAST ((klass), GCU_TYPE_SPECTRUM_VIEWER, GcuSpectrumViewerClass
/*!
\return TRUE if \a obj points to a GcuSpectrumViewer widget, FALSE otherwise.
*/
#define GCU_IS_SPECTRUM_VIEWER(obj)	  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GCU_TYPE_SPECTRUM_VIEWER))
/*!
\return TRUE if \a klass points to a GcuSpectrumViewerClass, FALSE otherwise.
*/
#define GCU_IS_SPECTRUM_VIEWER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GCU_TYPE_SPECTRUM_VIEWER))
/*!
\return the GcuSpectrumViewerClass * associated to \a obj if obj points to a GcuSpectrumViewer widget,
NULL otherwise.
*/
#define GCU_SPECTRUM_VIEWER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GCU_TYPE_SPECTRUM_VIEWER, GcuSpectrumViewerClass))

/*! \struct GcuSpectrumViewer gcugtk/gcuspectrumviewer.h
 The GcuSpectrumViewer displays using the goffice library. Only spectra in the JCAMP-DX format are supported in this
version.

The available functions related to the GcuSpectrumViewer Widget are described in the gcuspectrumviewer.h page.
*/

/*! The GcuSpectrumViewer widget.*/
typedef struct _GcuSpectrumViewer       GcuSpectrumViewer;
/*! The GcuSpectrumViewer widget object class.*/
typedef struct _GcuSpectrumViewerClass  GcuSpectrumViewerClass;

GType               gcu_spectrum_viewer_get_type          (void) G_GNUC_CONST;
/*!
@param uri the URI of the file containing the spectrum to display

Creates a GcuSpectrumViewer widget and fills it with the data from uri.
If uri is NULL, the widget will display an empty chart.
*/
GtkWidget*            gcu_spectrum_viewer_new               (const gchar* uri);
/*!
@param viewer a pointer to GcuSectrumViewer widget.
@param uri the URI of the file containing the spectrum to display.

Changes the molecule displayed by the one described in the uri. Nothing happens if uri is NULL.
*/
void	gcu_spectrum_viewer_set_uri	(GcuSpectrumViewer * viewer, const gchar * uri);

/*!
@param viewer a pointer to GcuSectrumViewer widget.

@return the graph displayed by the widget.
*/
GogGraph *gcu_spectrum_viewer_get_graph (GcuSpectrumViewer * viewer);

G_END_DECLS

#endif //	GCU_SPECTRUM_VIEWER_H
