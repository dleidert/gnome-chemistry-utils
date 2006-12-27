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
/*! \enum ColorSchemes
 Predefined coloring scheme used for the buttons when displaying the periodic table of the GtkPeriodic widget.
 Possible values are:
	 - GTK_PERIODIC_COLOR_NONE: the default Gtk theme is used.
	 - GTK_PERIODIC_COLOR_DEFAULT: the default color for each element is used as returned by gcu_element_get_default_color.
 Other values can be added using gtk_periodic_add_color_scheme.
*/
enum GtkPeriodicColorSchemes
{
  GTK_PERIODIC_COLOR_NONE,
  GTK_PERIODIC_COLOR_DEFAULT,
  GTK_PERIODIC_COLOR_MAX,
};

/*! The GType associated to GtkPeriodic */
#define GTK_TYPE_PERIODIC		  (gtk_periodic_get_type ())
/*!
Casts \a obj to a GtkPeriodic * pointer.
\return a pointer to the GtkPeriodic * or NULL if \a obj does not point to 
a GtkPeriodic widget.
*/
#define GTK_PERIODIC(obj)		  (GTK_CHECK_CAST ((obj), GTK_TYPE_PERIODIC, GtkPeriodic))
/*!
Casts \a klass to a GtkPeriodicClass * pointer.
\return a pointer to the GtkPeriodicClass * or NULL if \a obj not point to a GtkPeriodicClass.
*/
#define GTK_PERIODIC_CLASS(klass)	  (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_PERIODIC, GtkPeriodicClass))
/*!
\return TRUE if \a obj points to a GtkPeriodic widget, FALSE otherwise.
*/
#define GTK_IS_PERIODIC(obj)	  (GTK_CHECK_TYPE ((obj), GTK_TYPE_PERIODIC))
/*!
\return TRUE if \a klass points to a GtkPeriodicClass, FALSE otherwise.
*/
#define GTK_IS_PERIODIC_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_PERIODIC))
/*!
return the GtkPeriodicClass * associated to \a obj if obj points to a GtkPeriodic widget,
NULL otherwise.
*/
#define GTK_PERIODIC_GET_CLASS(obj)  (GTK_CHECK_GET_CLASS ((obj), GTK_TYPE_PERIODIC, GtkPeriodicClass))

/*! The GtkPeriodic widget.*/
typedef struct _GtkPeriodic       GtkPeriodic;
/*! The GtkPeriodic widget object class.*/
typedef struct _GtkPeriodicClass  GtkPeriodicClass;

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
	Default value is FALSE.
	
- "color-style": GtkPeriodicColorStyle (Read / Write).
	<br>This property is used to set the coloring scheme used for the buttons when displaying the periodic table.
	There are two predefined values:
	 - GTK_PERIODIC_COLOR_NONE: the default Gtk theme is used.
	 - GTK_PERIODIC_COLOR_DEFAULT: the default color for each element is used as returned by gcu_element_get_default_color.
	 Other values can be added using gtk_periodic_add_color_scheme().
	 Default value is GTK_PERIODIC_COLOR_NONE.
.
<hr>
<h2>Functions</h2>

Functions related to the GtkPeriodic Widget are described in the gtkperiodic.h page.
*/

/*!
The callback used for color schemes. It takes three arguments:
\li the atomic number Z.
\li a pointer to the GdkColor structure to be filled by the callback
\li a pointer to user's data.
*/
typedef void (*GtkPeriodicColorFunc) (int, GdkColor*, gpointer);

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

/*!
\param periodic a GtkPeriodic widget.
\param func the callback used to get the color for an element in the new color scheme.
\param extra_widget a widget to be added as a child of \a periodic.
\param user_data data to be passed to the \a func callback.

Using this function and the appropriate callback, the color used for the elements buttons
can be changed to depend on any property of the elements.
\return the identifier of the new color scheme.
*/
int					gtk_periodic_add_color_scheme	(GtkPeriodic *periodic,
											GtkPeriodicColorFunc func,
											GtkWidget *extra_widget,
										gpointer user_data);

/*!
\param periodic a GtkPeriodic widget.

Forces the update of the current color scheme. This is useful when the color scheme depends
upon a parameter which has changed.
*/
void				gtk_periodic_set_colors(GtkPeriodic *periodic);
G_END_DECLS

#endif //GTK_PERIODIC_H
