/* 
 * Gnome Chemisty Utils
 * gtkcrystalviewer.h 
 *
 * Copyright (C) 2002-2003
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


#ifndef GTK_CRYSTAL_VIEWER_H
#define GTK_CRYSTAL_VIEWER_H

//#include <gtkgl/gtkglarea.h>
#include <gtk/gtkbin.h>
#include <libxml/tree.h>

G_BEGIN_DECLS

#define GTK_TYPE_CRYSTAL_VIEWER		  (gtk_crystal_viewer_get_type ())
#define GTK_CRYSTAL_VIEWER(obj)		  (GTK_CHECK_CAST ((obj), GTK_TYPE_CRYSTAL_VIEWER, GtkCrystalViewer))
#define GTK_CRYSTAL_VIEWER_CLASS(klass)	  (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_CRYSTAL_VIEWER, GtkCrystalViewerClass))
#define GTK_IS_CRYSTAL_VIEWER(obj)	  (GTK_CHECK_TYPE ((obj), GTK_TYPE_CRYSTAL_VIEWER))
#define GTK_IS_CRYSTAL_VIEWER_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_CRYSTAL_VIEWER))
#define GTK_CRYSTAL_VIEWER_GET_CLASS(obj)  (GTK_CHECK_GET_CLASS ((obj), GTK_TYPE_CRYSTAL_VIEWER, GtkCrystalViewerClass))

typedef struct _GtkCrystalViewer       GtkCrystalViewer;
typedef struct _GtkCrystalViewerClass  GtkCrystalViewerClass;

struct _GtkCrystalViewer
{
//  GtkGLArea area;
	GtkBin bin;

  /*< private >*/
	struct _GtkCrystalViewerPrivate *priv;
};

struct _GtkCrystalViewerClass
{
//	GtkGLAreaClass parent_class;
	GtkBinClass parent_class;
};

GType               gtk_crystal_viewer_get_type          (void) G_GNUC_CONST;
GtkWidget*            gtk_crystal_viewer_new               (xmlNodePtr node);
void	gtk_crystal_viewer_set_data	(GtkCrystalViewer * viewer, xmlNodePtr node);

G_END_DECLS

#endif //GTK_CRYSTAL_VIEWER_H
