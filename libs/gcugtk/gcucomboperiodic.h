/* 
 * Gnome Chemisty Utils
 * gcucomboperiodic.h 
 *
 * Copyright (C) 2006-2011 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCU_COMBO_PERIODIC_H
#define GCU_COMBO_PERIODIC_H

#include <glib-object.h>

G_BEGIN_DECLS

/*!\return the GType associated to GcuComboPeriodic */
#define GCU_TYPE_COMBO_PERIODIC	(gcu_combo_periodic_get_type ())
/*!
Casts \a obj to a GcuComboPeriodic * pointer.
\return a pointer to the GcuComboPeriodic * or NULL if \a obj does not point to 
a GcuComboPeriodic widget.
*/
#define GCU_COMBO_PERIODIC(o)	(G_TYPE_CHECK_INSTANCE_CAST((o), GCU_TYPE_COMBO_PERIODIC, GcuComboPeriodic))
/*!
\return TRUE if \a obj points to a GcuComboPeriodic widget, FALSE otherwise.
*/
#define GCU_IS_COMBO_PERIODIC(o)	(G_TYPE_CHECK_INSTANCE_TYPE((o), GCU_TYPE_COMBO_PERIODIC))

/*!\file
Declaration of the GcuComboPeriodic widget.
*/

/*! \struct GcuComboPeriodic gcu/gcucomboperiodic.h
 The GcuComboPeriodic is a combo box with a dropdown periodic table widget.
<hr>
<h2>Signals</h2>

 This widget has one signal:
- "changed": void	user_function (GtkWidget* periodic, guint Z, gpointer data).
\param periodic: the object which received the signal.
\param Z: the atomic number of the newly selected element or %0 if none is selected.
\param data: user data set when the signal handler was connected.

 This signal is raised when the selected element changes in the widget.

<hr>
<h2>Functions</h2>

Functions related to the GcuComboPeriodic Widget are described in the gcucomboperiodic.h page.
*/
/*! The GcuComboPeriodic widget.*/
typedef struct _GcuComboPeriodic	GcuComboPeriodic;

GType      gcu_combo_periodic_get_type	 (void);
/*!
@return a pointer to a newly created GcuComboPeriodic widget.
*/
GtkWidget *gcu_combo_periodic_new (void);

/*!
@param combo  a GcuComboPeriodic widget
Used to get the currently selected element in the periodic table.
@return the atomic number of the currently selected element or %0 if none is selected.
 */
guint	gcu_combo_periodic_get_element	(GcuComboPeriodic *combo);

/*!
@param combo  a GcuComboPeriodic widget
@param element the atomic number of the element to select or 0
Sets the selected element in the periodic table.
*/
void	gcu_combo_periodic_set_element	(GcuComboPeriodic *combo, guint element);

G_END_DECLS

#endif /* GCU_COMBO_PERIODIC_H */
