/* 
 * Gnome Chemisty Utils
 * gtkchem3dviewer.h 
 *
 * Copyright (C) 2003-2006 Jean Bréfort <jean.brefort@normalesup.org>
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


#ifndef GTK_CHEM3D_VIEWER_H
#define GTK_CHEM3D_VIEWER_H

#include <gtk/gtkbin.h>

G_BEGIN_DECLS

/*!\file
Declaration of the GtkChem3DViewer widget.
*/

GType gtk_display3d_get_type (void);
/*!
\return the GType associated to the Display3D enum.
*/
#define GTK_DISPLAY_3D (gtk_display3d_get_type ())

/*!\return the GType associated to GtkChem3DViewer */
#define GTK_TYPE_CHEM3D_VIEWER		  (gtk_chem3d_viewer_get_type ())
/*!
Casts \a obj to a GtkChem3DViewer * pointer.
\return a pointer to the GtkChem3DViewer * or NULL if \a obj does not point to 
a GtkChem3DViewer widget.
*/
#define GTK_CHEM3D_VIEWER(obj)		  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_CHEM3D_VIEWER, GtkChem3DViewer))
/*!
Casts \a klass to a GtkChem3DViewerClass * pointer.
\return a pointer to the GtkChem3DViewerClass * or NULL if \a obj not point to a GtkChem3DViewerClass.
*/
#define GTK_CHEM3D_VIEWER_CLASS(klass)	  (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_CHEM3D_VIEWER, GtkChem3DViewerClass))
/*!
\return TRUE if \a obj points to a GtkChem3DViewer widget, FALSE otherwise.
*/
#define GTK_IS_CHEM3D_VIEWER(obj)	  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_CHEM3D_VIEWER))
/*!
\return TRUE if \a klass points to a GtkChem3DViewerClass, FALSE otherwise.
*/
#define GTK_IS_CHEM3D_VIEWER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_CHEM3D_VIEWER))
/*!
\return the GtkChem3DViewerClass * associated to \a obj if obj points to a GtkChem3DViewer widget,
NULL otherwise.
*/
#define GTK_CHEM3D_VIEWER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_CHEM3D_VIEWER, GtkChem3DViewerClass))

/*! \struct GtkChem3DViewer gcu/gtkchem3dviewer.h
 The GtkChem3DViewer displays 3D models of molecules using an OpenGL window.
 A test program is available in the tests directory of the Gnome Chemistry Utils source archive (sources in testgtkchem3dviewer.c).
<hr>
<h2>Properties</h2>
There are two properties:
- "display3d": Display3DMode (Read / Write).
	<br>This property is used to set the display mode. When using the Bonobo control, a string is used instead the enumeration.
	Possible values are:
	 - BALL_AND_STICK: use ball and stick representation; atoms are represented by spheres with a radius equal to 20% of
	 their van der Waals radius and bonds are displayed as cylinders. String version is "ball&stick".
	 - SPACEFILL: use space filling representation; atoms are represented by spheres with a radius equal
	 their van der Waals radius; bonds are not displayed. String version is "spacefill".
	.
	
- "bgcolor": gchar* (Read / Write).
	<br>The background color for the display, for example "black" or "#ffffe6". Only "black",
	"white" and "#rrggbb" are accepted in this version of the Gnome Chemistry Utils.
<h2>Functions</h2>

Functions related to the GtkChem3DViewer Widget are described in the gtkchem3dviewer.h page.
*/
/*! The GtkChem3DViewer widget.*/
typedef struct _GtkChem3DViewer       GtkChem3DViewer;
/*! The GtkChem3DViewer widget object class.*/
typedef struct _GtkChem3DViewerClass  GtkChem3DViewerClass;

GType               gtk_chem3d_viewer_get_type          (void) G_GNUC_CONST;
/*!
@param uri: the URI of the file containing the molecular structure to display. Any file supported by
<a href="http://openbabel.sourceforge.net">OpenBabel</a> may be used.

Creates a GtkChem3DViewer widget and fills it with the data from uri. If uri is NULL, the widget will be empty.
*/
GtkWidget*            gtk_chem3d_viewer_new               (const gchar* uri);
/*!
@param viewer a pointer to GtkChem3DViewer widget.
@param uri the URI of the file containing the molecular structure to display. Any file supported by
@param mime_type: the mime_type of the data. Any type supported by
<a href="http://openbabel.sourceforge.net">OpenBabel</a> may be used.

Changes the molecule displayed by the one described in the uri. Nothing happens if uri is NULL.
*/
void	gtk_chem3d_viewer_set_uri_with_mime_type	(GtkChem3DViewer * viewer, const gchar * uri, const gchar* mime_type);
/*!
@param viewer a pointer to GtkChem3DViewer widget.
@param uri the URI of the file containing the molecular structure to display.
<a href="http://openbabel.sourceforge.net">OpenBabel</a> may be used.

Changes the molecule displayed by the one described in the uri. Nothing happens if uri is NULL.
*/
void	gtk_chem3d_viewer_set_uri	(GtkChem3DViewer * viewer, const gchar * uri);
/*!
@param viewer a pointer to GtkChem3DViewer widget.
@param data a pointer to the raw data representing a serialized version of molecule to display
@param mime_type the mime_type of the data. Any type supported by
<a href="http://openbabel.sourceforge.net">OpenBabel</a> may be used.

Changes the molecule displayed by the one described in the data. Nothing happens if data or mime-type is NULL.
*/
void	gtk_chem3d_viewer_set_data	(GtkChem3DViewer * viewer, const gchar * data, const gchar* mime_type);

/*!
@param viewer a pointer to a GtkChem3DViewer widget.
@param width the width of the new pixbuf.
@param height the height of the new pixbuf.

Renders the scene to a newly allocated pixbuf.
\return the new GdkPixbuf*.
*/
GdkPixbuf *gtk_chem3d_viewer_new_pixbuf (GtkChem3DViewer * viewer, guint width, guint height);

G_END_DECLS

#endif //GTK_CHEM3D_VIEWER_H
