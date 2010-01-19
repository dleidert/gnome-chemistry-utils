/* 
 * Gnome Chemisty Utils
 * gcucrystalviewer.h 
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

/*!\file
Declaration of the GtkCrystalViewer widget.
*/

#ifndef GCU_CRYSTAL_VIEWER_H
#define GCU_CRYSTAL_VIEWER_H

#include <gtk/gtkbin.h>
#include <libxml/tree.h>

G_BEGIN_DECLS

/*!\return the GType associated to GtkCrystalViewer */
#define GCU_TYPE_CRYSTAL_VIEWER		  (gcu_crystal_viewer_get_type ())
/*!
Casts \a obj to a GtkCrystalViewer * pointer.
\return a pointer to the GtkCrystalViewer * or NULL if \a obj does not point to 
a GcuCrystalViewer widget.
*/
#define GCU_CRYSTAL_VIEWER(obj)		  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GCU_TYPE_CRYSTAL_VIEWER, GcuCrystalViewer))
/*!
Casts \a klass to a GcuCrystalViewerClass * pointer.
\return a pointer to the GcuCrystalViewerClass * or NULL if \a obj not point to a GcuCrystalViewerClass.
*/
#define GCU_CRYSTAL_VIEWER_CLASS(klass)	  (G_TYPE_CHECK_CLASS_CAST ((klass), GCU_TYPE_CRYSTAL_VIEWER, GcuCrystalViewerClass))
/*!
\return TRUE if \a obj points to a GcuCrystalViewer widget, FALSE otherwise.
*/
#define GCU_IS_CRYSTAL_VIEWER(obj)	  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GCU_TYPE_CRYSTAL_VIEWER))
/*!
\return TRUE if \a klass points to a GcuCrystalViewerClass, FALSE otherwise.
*/
#define GCU_IS_CRYSTAL_VIEWER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GCU_TYPE_CRYSTAL_VIEWER))
/*!
\return the GcuCrystalViewerClass * associated to \a obj if obj points to a GcuCrystalViewer widget,
NULL otherwise.
*/
#define GCU_CRYSTAL_VIEWER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GCU_TYPE_CRYSTAL_VIEWER, GcuCrystalViewerClass))

/*! The GcuCrystalViewer widget.*/
typedef struct _GcuCrystalViewer       GcuCrystalViewer;
/*! The GcuCrystalViewer widget object class.*/
typedef struct _GcuCrystalViewerClass  GcuCrystalViewerClass;

/*!\struct GcuCrystalViewer
The GcuCrystalViewer widget displays a crystal structure using an OpenGL window. A test program is available in the tests
directory of the Gnome Chemistry Utils source archive (source in testgcucrystalviewer.c).
<hr>
<h2>Functions</h2>

Functions related to the GcuCrystalViewer Widget are described in the gcucrystalviewer.h page.
*/

GType               gcu_crystal_viewer_get_type          (void) G_GNUC_CONST;
/*!
@param node: a pointer to an xlNode (from libxml) containing the serialized version of the crystal to display as saved by
Gnome Crystal or NULL.

Builds a new GcuCrystalViewer widget and, if node is not NULL, fills it with the Crystal structure described in node.
@return a pointer to the new viewer.
*/
GtkWidget*            gcu_crystal_viewer_new               (xmlNodePtr node);
/*!
@param viewer: a pointer to a GcuCrystalViewer widget.
@param node: a pointer to an xlNode (from libxml) containing the serialized version of the crystal to display as saved by
Gnome Crystal.

Replaces the content of viewer by the Crystal structure described in node.
*/
void	gcu_crystal_viewer_set_data	(GcuCrystalViewer * viewer, xmlNodePtr node);

/*!
@param viewer a pointer to a GcuCrystalViewer widget.
@param width the width of the new pixbuf.
@param height the height of the new pixbuf.

Renders the scene to a newly allocated pixbuf.
\return the new GdkPixbuf*.
*/
GdkPixbuf *gcu_crystal_viewer_new_pixbuf (GcuCrystalViewer * viewer, guint width, guint height);

G_END_DECLS

#endif //GCU_CRYSTAL_VIEWER_H
