// -*- C -*-

/* 
 * Gnome Chemisty Utils
 * gtkperiodic.h 
 *
 * Copyright (C) 2002-2006 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GTK_PERIODIC_H
#define GTK_PERIODIC_H

#include <gdk/gdk.h>
#include <gtk/gtkbin.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtktogglebutton.h>

G_BEGIN_DECLS

/*!\file
Declaration of the GtkPeriodic widget.
*/
/*! \enum
 Coloring scheme used for the buttons when displaying the periodic table of the GtkPeriodic widget.
 Possible values are:
	 - GTK_PERIODIC_COLOR_NONE: the default Gtk theme is used.
	 - GTK_PERIODIC_COLOR_DEFAULT: the default color for each element is used as returned by gcu_element_get_default_color.
 Other values can be added using gtk_periodic_add_color_scheme.
*/
enum
{
  GTK_PERIODIC_COLOR_NONE,
  GTK_PERIODIC_COLOR_DEFAULT,
  GTK_PERIODIC_COLOR_MAX,
};

#define GTK_TYPE_PERIODIC		  (gtk_periodic_get_type ())
#define GTK_PERIODIC(obj)		  (GTK_CHECK_CAST ((obj), GTK_TYPE_PERIODIC, GtkPeriodic))
#define GTK_PERIODIC_CLASS(klass)	  (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_PERIODIC, GtkPeriodicClass))
#define GTK_IS_PERIODIC(obj)	  (GTK_CHECK_TYPE ((obj), GTK_TYPE_PERIODIC))
#define GTK_IS_PERIODIC_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_PERIODIC))
#define GTK_PERIODIC_GET_CLASS(obj)  (GTK_CHECK_GET_CLASS ((obj), GTK_TYPE_PERIODIC, GtkPeriodicClass))

typedef struct _GtkPeriodic       GtkPeriodic;
typedef struct _GtkPeriodicClass  GtkPeriodicClass;
typedef struct _GtkPeriodicPrivate       GtkPeriodicPrivate;

/*! \struct GtkPeriodic gcu/gtkperiodic.h
 The GtkPeriodic Widget displays a Periodic table of the elements, each element being represented in a toggle button.
 A test program is available in the tests directory of the Gnome Chemistry Utils source archive
(source in testgtkperiodic.c).
<hr>
<h2>Signals</h2>

 This widget has one signal:
- "element-changed": void	user_function (GtkWidget* periodic, guint Z, gpointer data).
\param periodic: the object which received the signal.
\param Z: the atomic number of the newly selected element or 0 if no element is selected.
\param data: user data set when the signal handler was connected.

 This signal is raised when the selected element changes in the widget.

<hr>
<h2>Properties</h2>
There are two properties:
- "can_unselect": gboolean (Read / Write).
	<br>When this property is set to TRUE, you can unselect the selected element by clicking on the active button, otherwise an element will always be selected.
	
- "color-style": GtkPeriodicColorStyle (Read / Write).
	<br>This property is used to set the coloring scheme used for the buttons when displaying the periodic table. Two values are allowed:
	 - GTK_PERIODIC_COLOR_NONE: the default Gtk theme is used.
	 - GTK_PERIODIC_COLOR_DEFAULT: the default color for each element is used as returned by gcu_element_get_default_color.
	 .
.
<hr>
<h2>Functions</h2>

Functions related to the GtkPeriodic Widget are described in the gtkperiodic.h page.
*/

typedef void (*GtkPeriodicColorFunc) (int, GdkColor*, gpointer);

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

/**
 * gtk_periodic_get_element:
 * \param periodic  a GtkPeriodic widget
 *
 * Description: used to get the currently selected element in the periodic table.
 *
 * Returns: the atomic number of the currently selected element or %0 if none is selected.
 */
guint				gtk_periodic_get_element		(GtkPeriodic* periodic);
/**
 * gtk_periodic_set_element:
 * \param periodic  a GtkPeriodic widget
 * \param element the atomic number of the element to select or 0
 *
 * Description: sets the selected element in the periodic table.
 */

void				gtk_periodic_set_element		(GtkPeriodic* periodic, guint element);

int					gtk_periodic_add_color_scheme	(GtkPeriodic *periodic,
											GtkPeriodicColorFunc func,
											GtkWidget *extra_widget,
											gpointer user_data);
G_END_DECLS

#endif //GTK_PERIODIC_H
