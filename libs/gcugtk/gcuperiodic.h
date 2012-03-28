// -*- C -*-

/*
 * Gnome Chemisty Utils
 * gcuperiodic.h
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

#ifndef GCU_PERIODIC_H
#define GCU_PERIODIC_H

#include <gdk/gdk.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

/*!\file
Declaration of the GcuPeriodic widget.
*/
/*! \enum GcuPeriodicColorSchemes
 Predefined coloring scheme used for the buttons when displaying the periodic table of the GcuPeriodic widget.
	 Possible values are:
	 - GCU_PERIODIC_COLOR_NONE: the default Gtk theme is used.
	 - GCU_PERIODIC_COLOR_DEFAULT: the default color for each element is used as returned by gcu_element_get_default_color.
 Other values can be added using gcu_periodic_add_color_scheme.
*/
enum GcuPeriodicColorSchemes
{
  GCU_PERIODIC_COLOR_NONE,
  GCU_PERIODIC_COLOR_DEFAULT,
  GCU_PERIODIC_COLOR_MAX,
};

/*! \enum GcuPeriodicTipSchemes
Predefined tipsg scheme used when the mouse cursor is over an element button.
*/
enum GcuPeriodicTipSchemes
{
/*!
Show the element name in the tips popup.
*/
	GCU_PERIODIC_TIP_NAME,
/*!
Show the element atomic number, symbol, name, electronic configuration,
and atomic weight.
*/
	GCU_PERIODIC_TIP_STANDARD
};

/*!\return the GType associated to GcuPeriodic */
#define GCU_TYPE_PERIODIC		  (gcu_periodic_get_type ())
/*!
Casts \a obj to a GcuPeriodic * pointer.
\return a pointer to the GcuPeriodic * or NULL if \a obj does not point to
a GcuPeriodic widget.
*/
#define GCU_PERIODIC(obj)		  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GCU_TYPE_PERIODIC, GcuPeriodic))
/*!
Casts \a klass to a GcuPeriodicClass * pointer.
\return a pointer to the GcuPeriodicClass * or NULL if \a obj not point to a GcuPeriodicClass.
*/
#define GCU_PERIODIC_CLASS(klass)	  (G_TYPE_CHECK_CLASS_CAST ((klass), GCU_TYPE_PERIODIC, GcuPeriodicClass))
/*!
\return TRUE if \a obj points to a GcuPeriodic widget, FALSE otherwise.
*/
#define GCU_IS_PERIODIC(obj)	  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GCU_TYPE_PERIODIC))
/*!
\return TRUE if \a klass points to a GcuPeriodicClass, FALSE otherwise.
*/
#define GCU_IS_PERIODIC_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GCU_TYPE_PERIODIC))
/*!
\return the GcuPeriodicClass * associated to \a obj if obj points to a GcuPeriodic widget,
NULL otherwise.
*/
#define GCU_PERIODIC_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GCU_TYPE_PERIODIC, GcuPeriodicClass))

/*! The GcuPeriodic widget.*/
typedef struct _GcuPeriodic       GcuPeriodic;
/*! The GcuPeriodic widget object class.*/
typedef struct _GcuPeriodicClass  GcuPeriodicClass;

/*! \struct GcuPeriodic gcugtk/gcuperiodic.h
 The GcuPeriodic Widget displays a Periodic table of the elements, each element being represented in a toggle button.
 A test program is available in the tests directory of the Gnome Chemistry Utils source archive
(source in testgcuperiodic.c).
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

- "color-style": GcuPeriodicColorStyle (Read / Write).
	<br>This property is used to set the coloring scheme used for the buttons when displaying the periodic table.
	There are two predefined values:
	 - GCU_PERIODIC_COLOR_NONE: the default Gtk theme is used.
	 - GCU_PERIODIC_COLOR_DEFAULT: the default color for each element is used as returned by gcu_element_get_default_color.
	 Other values can be added using gcu_periodic_add_color_scheme().
	 Default value is GCU_PERIODIC_COLOR_NONE.
.
<hr>
<h2>Functions</h2>

Functions related to the GcuPeriodic Widget are described in the gcuperiodic.h page.
*/

/*!
The callback used for color schemes. It takes three arguments:
\li the atomic number Z.
\li a pointer to the GdkColor structure to be filled by the callback
\li a pointer to user's data.
*/
typedef void (*GcuPeriodicColorFunc) (int, GdkRGBA*, gpointer);

/*!\return the GType associated to GcuPeriodic */
GType               gcu_periodic_get_type          (void) G_GNUC_CONST;
/*!\return a new GcuPeriodic */
GtkWidget*            gcu_periodic_new               (void);

/**
 * gcu_periodic_get_element:
 * \param periodic  a GcuPeriodic widget
 *
 * Description: used to get the currently selected element in the periodic table.
 *
 * Returns: the atomic number of the currently selected element or %0 if none is selected.
 */
guint				gcu_periodic_get_element		(GcuPeriodic* periodic);
/**
 * gcu_periodic_set_element:
 * \param periodic  a GcuPeriodic widget
 * \param element the atomic number of the element to select or 0
 *
 * Description: sets the selected element in the periodic table.
 */

void				gcu_periodic_set_element		(GcuPeriodic* periodic, guint element);

/*!
\param periodic a GcuPeriodic widget.
\param func the callback used to get the color for an element in the new color scheme.
\param extra_widget a widget to be added as a child of \a periodic.
\param user_data data to be passed to the \a func callback.

Using this function and the appropriate callback, the color used for the elements buttons
can be changed to depend on any property of the elements.
\return the identifier of the new color scheme.
*/
int					gcu_periodic_add_color_scheme	(GcuPeriodic *periodic,
											GcuPeriodicColorFunc func,
											GtkWidget *extra_widget,
										gpointer user_data);

/*!
\param periodic a GcuPeriodic widget.

Forces the update of the current color scheme. This is useful when the color scheme depends
upon a parameter which has changed.
*/
void				gcu_periodic_set_colors (GcuPeriodic *periodic);

/*!
@param periodic a GcuPeriodic widget.
@param scheme a tips scheme identifier.

Configures the element buttons tips. \a scheme must be one of the values defined
in the GcuPeriodicTipSchemes enum.
*/
void				gcu_periodic_set_tips (GcuPeriodic *periodic, unsigned scheme);

G_END_DECLS

#endif //GCU_PERIODIC_H
