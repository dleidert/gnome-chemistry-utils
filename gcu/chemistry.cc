// -*- C -*-

/* 
 * Gnome Chemisty Utils
 * chemistry.cc
 *
 * Copyright (C) 2003-2005
 *
 * Developed by Jean Bréfort <jean.brefort@normalesup.org>
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

#include "config.h"
#include "chemistry.h"
#include "element.h"

extern "C"
{
	
using namespace gcu;
const gdouble* gcu_element_get_default_color(gint Z)
{
	return Element::GetElement(Z)->GetDefaultColor();
}

const gchar* gcu_element_get_symbol(gint Z)
{
	return Element::Symbol(Z);
}

const gchar* gcu_element_get_name(gint Z)
{
	return Element::GetElement(Z)->GetName();
}

gint gcu_element_get_Z(gchar* symbol)
{
	return Element::Z(symbol);
}

gboolean gcu_element_get_radius(GcuAtomicRadius* radius)
{
	return Element::GetRadius(radius);
}

gboolean gcu_element_get_electronegativity(GcuElectronegativity* en)
{
	return Element::GetElectronegativity(en);
}

const GcuAtomicRadius** gcu_element_get_radii(gint Z)
{
	return Element::GetElement(Z)->GetRadii();
}

const GcuElectronegativity** gcu_element_get_electronegativities(gint Z)
{
	return Element::GetElement(Z)->GetElectronegativities();
}

void gcu_element_load_databases (char* name, ...)
{
	va_list l;
	char const *base = name;
	va_start (l, name);
	while (base != NULL) {
		if (!strcmp (base, "radii"))
			Element::LoadRadii ();
		else if (!strcmp (base, "elecprops"))
			Element::LoadElectronicProps ();
		else if (!strcmp (base, "isotopes"))
			Element::LoadIsotopes ();
		base = va_arg (l, char const*);
	}
	va_end (l);
}

gchar* gcu_value_get_string (GcuValue const *value)
{
	return NULL;
}

gchar* gcu_dimensional_value_get_string (GcuValue const *value)
{
	return NULL;
}

} //extern "C"
