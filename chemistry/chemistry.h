// -*- C -*-

/* 
 * Gnome Chemisty Utils
 * chemistry.h 
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


#ifndef GCU_CHEMISTRY_H
#define GCU_CHEMISTRY_H

#include <glib.h>

G_BEGIN_DECLS

const gdouble* gcu_element_get_default_color(gint Z);
const gchar* gcu_element_get_symbol(gint Z);
const gchar* gcu_element_get_name(gint Z);
gint gcu_element_get_Z(gchar* symbol);

G_END_DECLS

#endif //GCU_CHEMISTRY_H
