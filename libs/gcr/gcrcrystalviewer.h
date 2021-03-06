/*
 * Gnome Chemisty Utils
 * gcr/gcrcrystalviewer.h
 *
 * Copyright (C) 2002-2012 Jean Bréfort <jean.brefort@normalesup.org>
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

/*!\file
Declaration of the GcrCrystalViewer widget.
*/

#ifndef GCR_CRYSTAL_VIEWER_H
#define GCR_CRYSTAL_VIEWER_H

#include <gtk/gtk.h>
#include <libxml/tree.h>

G_BEGIN_DECLS

/*!\return the GType associated to GcrCrystalViewer */
#define GCR_TYPE_CRYSTAL_VIEWER		  (gcr_crystal_viewer_get_type ())
/*!
Casts \a obj to a GcrCrystalViewer * pointer.
\return a pointer to the GcrCrystalViewer * or NULL if \a obj does not point to
a GcrCrystalViewer widget.
*/
#define GCR_CRYSTAL_VIEWER(obj)		  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GCR_TYPE_CRYSTAL_VIEWER, GcrCrystalViewer))
/*!
Casts \a klass to a GcrCrystalViewerClass * pointer.
\return a pointer to the GcrCrystalViewerClass * or NULL if \a obj not point to a GcrCrystalViewerClass.
*/
#define GCR_CRYSTAL_VIEWER_CLASS(klass)	  (G_TYPE_CHECK_CLASS_CAST ((klass), GCR_TYPE_CRYSTAL_VIEWER, GcrCrystalViewerClass))
/*!
\return TRUE if \a obj points to a GcrCrystalViewer widget, FALSE otherwise.
*/
#define GCR_IS_CRYSTAL_VIEWER(obj)	  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GCR_TYPE_CRYSTAL_VIEWER))
/*!
\return TRUE if \a klass points to a GcrCrystalViewerClass, FALSE otherwise.
*/
#define GCR_IS_CRYSTAL_VIEWER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GCR_TYPE_CRYSTAL_VIEWER))
/*!
\return the GcrCrystalViewerClass * associated to \a obj if obj points to a GcrCrystalViewer widget,
NULL otherwise.
*/
#define GCR_CRYSTAL_VIEWER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GCR_TYPE_CRYSTAL_VIEWER, GcrCrystalViewerClass))

/*! The GcrCrystalViewer widget.*/
typedef struct _GcrCrystalViewer       GcrCrystalViewer;
/*! The GcrCrystalViewer widget object class.*/
typedef struct _GcrCrystalViewerClass  GcrCrystalViewerClass;

/*!\struct GcrCrystalViewer
The GcrCrystalViewer widget displays a crystal structure using an OpenGL window. A test program is available in the tests
directory of the Gnome Chemistry Utils source archive (source in testgcrcrystalviewer.c).
<hr>
<h2>Functions</h2>

Functions related to the GcrCrystalViewer Widget are described in the gcrcrystalviewer.h page.
*/

GType               gcr_crystal_viewer_get_type          (void) G_GNUC_CONST;
/*!
@param node: a pointer to an xlNode (from libxml) containing the serialized version of the crystal to display as saved by
Gnome Crystal or NULL.

Builds a new GcrCrystalViewer widget and, if node is not NULL, fills it with the Crystal structure described in node.
@return a pointer to the new viewer.
*/
GtkWidget*            gcr_crystal_viewer_new               (xmlNodePtr node);
/*!
@param viewer: a pointer to a GcrCrystalViewer widget.
@param node: a pointer to an xlNode (from libxml) containing the serialized version of the crystal to display as saved by
Gnome Crystal.

Replaces the content of viewer by the Crystal structure described in node.
*/
void	gcr_crystal_viewer_set_data	(GcrCrystalViewer * viewer, xmlNodePtr node);

/*!
@param viewer a pointer to a GcrCrystalViewer widget.
@param width the width of the new pixbuf.
@param height the height of the new pixbuf.
@param use_bg whether to use the window background or a transparent background.

Renders the scene to a newly allocated pixbuf.
\return the new GdkPixbuf*.
*/
GdkPixbuf *gcr_crystal_viewer_new_pixbuf (GcrCrystalViewer * viewer, unsigned width, unsigned height, gboolean use_bg);

/*!
@param viewer a pointer to a GcrCrystalViewer widget.
@param uri the URI of the file containing the crystal structure to display.
@param mime_type: the mime_type of the data.

Changes the crystal structure displayed by the one described in the uri.
Nothing happens if uri is NULL.
*/
void	gcr_crystal_viewer_set_uri_with_mime_type	(GcrCrystalViewer * viewer, char const *uri, char const *mime_type);
/*!
@param viewer a pointer to a GcrCrystalViewer widget.
@param uri the URI of the file containing the crystal structure to display.

Changes the crystal structure displayed by the one described in the uri.
Nothing happens if uri is NULL.
*/
void	gcr_crystal_viewer_set_uri	(GcrCrystalViewer * viewer, char const *uri);

/*!
@param viewer a pointer to a GcrCrystalViewer widget.

Restore the initial orientation.
*/
void	gcr_crystal_viewer_back_to_initial_orientation (GcrCrystalViewer * viewer);
G_END_DECLS

#endif //	GCR_CRYSTAL_VIEWER_H
