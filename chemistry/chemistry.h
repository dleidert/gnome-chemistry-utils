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

enum gcu_spin_state
{
	GCU_N_A_SPIN,
	GCU_LOW_SPIN,
	GCU_HIGH_SPIN
};

enum gcu_radius_type
{
	GCU_ATOMIC,
	GCU_IONIC,
	GCU_METALLIC,
	GCU_COVALENT,
	GCU_VAN_DER_WAALS
};

typedef struct
{
	unsigned char Z;
	enum gcu_radius_type type;
	double value;
	char charge;
	char* scale;
	char cn;	//coordination number: -1: unspecified
	enum gcu_spin_state spin;
} GcuAtomicRadius;

typedef struct
{
	unsigned char Z;
	double value;
	char* scale;
} GcuElectronegativity;

const gdouble* gcu_element_get_default_color(gint Z);
const gchar* gcu_element_get_symbol(gint Z);
const gchar* gcu_element_get_name(gint Z);
gint gcu_element_get_Z(gchar* symbol);
gboolean gcu_element_get_radius(GcuAtomicRadius* radius);
gboolean gcu_element_get_electronegativity(GcuElectronegativity* en);
const GcuAtomicRadius** gcu_element_get_radii(gint Z);
const GcuElectronegativity** gcu_element_get_electronegativities(gint Z);

G_END_DECLS

#endif //GCU_CHEMISTRY_H
