/* 
 * Gnome Chemisty Utils
 * gtkchem3dviewer.h 
 *
 * Copyright (C) 2003
 *
 * Developed by Jean Bréfort <jean.brefort@ac-dijon.fr>
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

#define GTK_TYPE_CHEM3D_VIEWER		  (gtk_chem3d_viewer_get_type ())
#define GTK_CHEM3D_VIEWER(obj)		  (GTK_CHECK_CAST ((obj), GTK_TYPE_CHEM3D_VIEWER, GtkChem3DViewer))
#define GTK_CHEM3D_VIEWER_CLASS(klass)	  (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_CHEM3D_VIEWER, GtkChem3DViewerClass))
#define GTK_IS_CHEM3D_VIEWER(obj)	  (GTK_CHECK_TYPE ((obj), GTK_TYPE_CHEM3D_VIEWER))
#define GTK_IS_CHEM3D_VIEWER_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_CHEM3D_VIEWER))
#define GTK_CHEM3D_VIEWER_GET_CLASS(obj)  (GTK_CHECK_GET_CLASS ((obj), GTK_TYPE_CHEM3D_VIEWER, GtkChem3DViewerClass))

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
GtkWidget*            gtk_chem3d_viewer_new               (gchar* uri);
void	gtk_chem3d_viewer_set_uri	(GtkChem3DViewer * viewer, gchar* uri);
void	gtk_chem3d_viewer_set_data	(GtkChem3DViewer * viewer, const gchar* data, const gchar* mime_type);

G_END_DECLS

#endif //GTK_CHEM3D_VIEWER_H
