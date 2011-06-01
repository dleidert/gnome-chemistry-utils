// -*- C -*-

/* 
 * Gnome Chemisty Utils
 * chemistry.cc
 *
 * Copyright (C) 2003-2005 Jean Bréfort <jean.brefort@normalesup.org>
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

#include "config.h"
#include "chemistry.h"
#include "element.h"
#include <cstring>
#include <cmath>

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

void gcu_element_load_databases (char const *name, ...)
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
	gchar *format, *str;
	char const *sign = value->value < 0? "−": "";
	if (value->delta > 0) {
		int delta = value->delta, prec = value->prec;
		while (delta >= 100) {
			delta /= 10;
			prec--;
		}
		format = g_strdup_printf ("%%s%%.%df(%%d)", prec);
		str = g_strdup_printf (format, sign, fabs (value->value), delta);
	} else {
		format = g_strdup_printf ("%%s%%.%df", value->prec);
		str = g_strdup_printf (format, sign, fabs (value->value));
	}
	g_free (format);
	return str;
}

gchar* gcu_dimensional_value_get_string (GcuDimensionalValue const *value)
{
	gchar *format, *str;
	char const *sign = value->value < 0? "−": "";
	if (value->delta > 0) {
		int delta = value->delta, prec = value->prec;
		while (delta >= 100) {
			delta /= 10;
			prec--;
		}
		// FIXME: what to do if we have a negative precision?
		format = (prec > 0)? g_strdup_printf ("%%s%%.%df(%%d) %%s", prec): g_strdup ("%.0f %s");
		str = g_strdup_printf (format, sign, fabs (value->value), delta, value->unit);
	} else {
		format = g_strdup_printf ("%%s%%.%df %%s", value->prec);
		str = g_strdup_printf (format, sign, fabs (value->value), value->unit);
	}
	g_free (format);
	return str;
}

GcuDimensionalValue const *gcu_element_get_ionization_energy (int Z, int rank)
{
	return Element::GetElement(Z)->GetIonizationEnergy (rank);
}

GcuDimensionalValue const *gcu_element_get_electron_affinity (int Z, int rank)
{
	return Element::GetElement(Z)->GetElectronAffinity (rank);
}

} //extern "C"
