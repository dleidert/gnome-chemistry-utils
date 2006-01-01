/* 
 * Gnome Chemisty Utils
 * gtkcomboperiodic.h 
 *
 * Copyright (C) 2006
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

#ifndef GTK_COMBO_PERIODIC_H
#define GTK_COMBO_PERIODIC_H

#include <glib-object.h>

G_BEGIN_DECLS

#define GTK_COMBO_PERIODIC_TYPE	(gtk_combo_periodic_get_type ())
#define GTK_COMBO_PERIODIC(o)	(G_TYPE_CHECK_INSTANCE_CAST((o), GTK_COMBO_PERIODIC_TYPE, GtkComboPeriodic))
#define GTK_IS_COMBO_PERIODIC(o)	(G_TYPE_CHECK_INSTANCE_TYPE((o), GTK_COMBO_PERIODIC_TYPE))

typedef struct _GtkComboPeriodic	GtkComboPeriodic;

GType      gtk_combo_periodic_get_type	 (void);
GtkWidget *gtk_combo_periodic_new (void);

/**
 * gtk_combo_periodic_get_element:
 * \param combo  a GtkComboPeriodic widget
 *
 * Description: used to get the currently selected element in the periodic table.
 *
 * Returns: the atomic number of the currently selected element or %0 if none is selected.
 */
guint	gtk_combo_periodic_get_element	(GtkComboPeriodic *combo);

/**
 * gtk_periodic_set_element:
 * \param combo  a GtkComboPeriodic widget
 * \param element the atomic number of the element to select or 0
 *
 * Description: sets the selected element in the periodic table.
 */
void	gtk_combo_periodic_set_element	(GtkComboPeriodic *combo, guint element);

G_END_DECLS

#endif /* GTK_COMBO_PERIODIC_H */
