// -*- C -*-

/* 
 * Gnome Chemisty Utils
 * gtkperiodic.h 
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

#ifndef GTK_PERIODIC_H
#define GTK_PERIODIC_H

#include <gdk/gdk.h>
#include <gtk/gtkbin.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtktogglebutton.h>

G_BEGIN_DECLS

typedef enum
{
  GTK_PERIODIC_COLOR_NONE,
  GTK_PERIODIC_COLOR_DEFAULT,
} GtkPeriodicColorStyle;

#define GTK_TYPE_PERIODIC		  (gtk_periodic_get_type ())
#define GTK_PERIODIC(obj)		  (GTK_CHECK_CAST ((obj), GTK_TYPE_PERIODIC, GtkPeriodic))
#define GTK_PERIODIC_CLASS(klass)	  (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_PERIODIC, GtkPeriodicClass))
#define GTK_IS_PERIODIC(obj)	  (GTK_CHECK_TYPE ((obj), GTK_TYPE_PERIODIC))
#define GTK_IS_PERIODIC_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_PERIODIC))
#define GTK_PERIODIC_GET_CLASS(obj)  (GTK_CHECK_GET_CLASS ((obj), GTK_TYPE_PERIODIC, GtkPeriodicClass))

typedef struct _GtkPeriodic       GtkPeriodic;
typedef struct _GtkPeriodicClass  GtkPeriodicClass;
typedef struct _GtkPeriodicPrivate       GtkPeriodicPrivate;

struct _GtkPeriodic
{
	GtkBin bin;
	
	GtkPeriodicPrivate *priv;
};

struct _GtkPeriodicClass
{
	GtkBinClass parent_class;

	void (* element_changed_event)(GtkPeriodic *periodic);
};

GType               gtk_periodic_get_type          (void) G_GNUC_CONST;
GtkWidget*            gtk_periodic_new               (void);

guint				gtk_periodic_get_element		(GtkPeriodic* periodic);
void				gtk_periodic_set_element		(GtkPeriodic* periodic, guint element);

G_END_DECLS

#endif //GTK_PERIODIC_H
