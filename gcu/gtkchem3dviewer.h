/* 
 * Gnome Chemisty Utils
 * gtkchem3dviewer.h 
 *
 * Copyright (C) 2003-2006
 *
 * Developed by Jean Bréfort <jean.brefort@normalesup.org>
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

//#include <gtkgl/gtkglarea.h>
#include <gtk/gtkbin.h>
#include <libxml/tree.h>
#include <libgnomeprint/gnome-print.h>

G_BEGIN_DECLS

/*!\file
Declaration of the GtkChem3DViewer widget.
*/

/*! \enum Display3DMode
 3D display mode.
 Possible values are:
	 - BALL_AND_STICK: use ball and stick representation; atoms are represented by spheres with a radius equal to 20% of
	 their van der Waals radius and bonds are displayed as cylinders.
	 - SPACEFILL: use space filling representation; atoms are represented by spheres with a radius equal
	 their van der Waals radius; bonds are not displayed.
	 .
*/
typedef enum
{
	BALL_AND_STICK,
	SPACEFILL
} Display3DMode;

GType gtk_display3d_get_type (void);
#define GTK_DISPLAY_3D (gtk_display3d_get_type ())

#define GTK_TYPE_CHEM3D_VIEWER		  (gtk_chem3d_viewer_get_type ())
#define GTK_CHEM3D_VIEWER(obj)		  (GTK_CHECK_CAST ((obj), GTK_TYPE_CHEM3D_VIEWER, GtkChem3DViewer))
#define GTK_CHEM3D_VIEWER_CLASS(klass)	  (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_CHEM3D_VIEWER, GtkChem3DViewerClass))
#define GTK_IS_CHEM3D_VIEWER(obj)	  (GTK_CHECK_TYPE ((obj), GTK_TYPE_CHEM3D_VIEWER))
#define GTK_IS_CHEM3D_VIEWER_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_CHEM3D_VIEWER))
#define GTK_CHEM3D_VIEWER_GET_CLASS(obj)  (GTK_CHECK_GET_CLASS ((obj), GTK_TYPE_CHEM3D_VIEWER, GtkChem3DViewerClass))

/*! \struct GtkChem3DViewer gcu/gtkchem3dviewer.h
 The GtkChem3DViewer displays 3D models of molecules using an OpenGL window. This widget is also available as a Bonobo Control.
 Test programs are available in the tests directory of the Gnome Chemistry Utils source archive (sources in testgtkchem3dviewer.c
for the use of the widget and in testbonobocontrol.c for the use of the Bonobo control).
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
	<br>The background color for the display, for example "black" or "#ffffe6".
<h2>Functions</h2>

Functions related to the GtkChem3DViewer Widget are described in the gtkchem3dviewer.h page.
*/
typedef struct _GtkChem3DViewer       GtkChem3DViewer;
typedef struct _GtkChem3DViewerClass  GtkChem3DViewerClass;

struct _GtkChem3DViewer
{
//  GtkGLArea area;
	GtkBin bin;

  /*< private >*/
	struct _GtkChem3DViewerPrivate *priv;
};

struct _GtkChem3DViewerClass
{
//	GtkGLAreaClass parent_class;
	GtkBinClass parent_class;
};

GType               gtk_chem3d_viewer_get_type          (void) G_GNUC_CONST;
/*!
@param uri: the URI of the file containing the molecular structure to display. Any file supported by
<a href="http://openbabel.sourceforge.net">OpenBabel</a> may be used.

Creates a GtkChem3DViewer widget and fills it with the data from uri. If uri is NULL, the widget will be empty.
*/
GtkWidget*            gtk_chem3d_viewer_new               (const gchar* uri);
/*!
@param viewer: a pointer to GtkChem3DViewer widget.
@param uri: the URI of the file containing the molecular structure to display. Any file supported by
@param mime_type: the mime_type of the data. Any type supported by
<a href="http://openbabel.sourceforge.net">OpenBabel</a> may be used.

Changes the molecule displayed by the one described in the uri. Nothing happens if uri is NULL.
*/
void	gtk_chem3d_viewer_set_uri_with_mime_type	(GtkChem3DViewer * viewer, const gchar * uri, const gchar* mime_type);
/*!
@param viewer: a pointer to GtkChem3DViewer widget.
@param uri: the URI of the file containing the molecular structure to display.
<a href="http://openbabel.sourceforge.net">OpenBabel</a> may be used.

Changes the molecule displayed by the one described in the uri. Nothing happens if uri is NULL.
*/
void	gtk_chem3d_viewer_set_uri	(GtkChem3DViewer * viewer, const gchar * uri);
/*!
@param viewer: a pointer to GtkChem3DViewer widget.
@param data: a pointer to the raw data representing a serialized version of molecule to display
@param mime_type: the mime_type of the data. Any type supported by
<a href="http://openbabel.sourceforge.net">OpenBabel</a> may be used.

Changes the molecule displayed by the one described in the data. Nothing happens if data or mime-type is NULL.
*/
void	gtk_chem3d_viewer_set_data	(GtkChem3DViewer * viewer, const gchar * data, const gchar* mime_type);

/*!
@param viewer: a pointer to GtkChem3DViewer widget.
@param pc: a pointer to the GnomePrintContext.
@param width: the width of the rectangle.
@param height: the height of the rectangle.

Prints the scene to a GnomePrintContext using a 300 dpi resolution.
*/
void gtk_chem3d_viewer_print(GtkChem3DViewer * viewer, GnomePrintContext *pc, gdouble width, gdouble height);

G_END_DECLS

#endif //GTK_CHEM3D_VIEWER_H