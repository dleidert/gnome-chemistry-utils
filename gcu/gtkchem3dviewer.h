/* 
 * Gnome Chemisty Utils
 * gtkchem3dviewer.h 
 *
 * Copyright (C) 2003-2004
 *
 * Developed by Jean Bréfort <jean.brefort@normalesup.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA  02111-1307, USA.
 */


#ifndef GTK_CHEM3D_VIEWER_H
#define GTK_CHEM3D_VIEWER_H

//#include <gtkgl/gtkglarea.h>
#include <gtk/gtkbin.h>
#include <libxml/tree.h>

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
<<h2>Properties</h2>
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
.
To set one of these properties when using the Bonobo control, use the associated Bonobo_PropertyBag. Example
where control is an already existing BonoboControl widget (without error checking):
\code
BonoboControlFrame *control_frame;
Bonobo_PropertyBag prop_bag;
control_frame = bonobo_widget_get_control_frame(BONOBO_WIDGET(control));
prop_bag = bonobo_control_frame_get_control_property_bag (control_frame, NULL);
bonobo_pbclient_set_string (prop_bag, "display3d", "ball&stick", NULL);
\endcode
<hr>
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
GtkWidget*            gtk_chem3d_viewer_new               (gchar* uri);
/*!
@param viewer: a pointer to GtkChem3DViewer widget.
@param uri: the URI of the file containing the molecular structure to display. Any file supported by
<a href="http://openbabel.sourceforge.net">OpenBabel</a> may be used.

Changes the molecule displayed by the one described in the uri. Nothing happens if uri is NULL.
*/
void	gtk_chem3d_viewer_set_uri	(GtkChem3DViewer * viewer, gchar* uri);
/*!
@param viewer: a pointer to GtkChem3DViewer widget.
@param data: a pointer to the raw data representing a serialized version of molecule to display
@param mime_type: the URI of the representation of the molecular structure to display. Any type supported by
<a href="http://openbabel.sourceforge.net">OpenBabel</a> may be used.

Changes the molecule displayed by the one described in the data. Nothing happens if data or mime-type is NULL.
*/
void	gtk_chem3d_viewer_set_data	(GtkChem3DViewer * viewer, const gchar* data, const gchar* mime_type);

G_END_DECLS

#endif //GTK_CHEM3D_VIEWER_H
