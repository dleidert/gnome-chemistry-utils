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


/*! \mainpage
  \section intro Introduction
The Gnome Chemistry Utils provide some widgets and C++ classes related to chemistry.

Available widgets are:
- GtkPeriodic: a periodic table of the elements.
- GtkCrystalViewer: a crystal structure viewer.
- GtkChem3DViewer: a 3D molecular structure viewer. This widget is also available as a Bonobo control.

The C++ classes are grouped in te gcu namespace. A C interface exists to use some of the
functionalities offered in these classes; the corresponding documentation is available in the
chemistry.h file.
*/

#ifndef GCU_CHEMISTRY_H
#define GCU_CHEMISTRY_H

#include <glib.h>

/** \file
* C interface to the chemistry library.

To use this file, add the following line to your source file:
\code
#include <chemistry/chemistry.h>
\endcode
*/

G_BEGIN_DECLS

/** gcu_spin_state
* gcu_spin_state describes the spin state of a central atom in a complex.
* Possible values are:
* - GCU_N_A_SPIN: unknown,
* - GCU_LOW_SPIN: low spin,
* - GCU_HIGH_SPIN: high spin.
*
*  This enumeration is used in the GcuAtomicRadius structure.
*/
enum gcu_spin_state
{
	GCU_N_A_SPIN,
	GCU_LOW_SPIN,
	GCU_HIGH_SPIN
};

/** gcu_radius_type
* gcu_radius_type describes the type of the atomic radius
* Possible values are:
* - GCU_RADIUS_UNKNOWN: unknown,.
* - GCU_ATOMIC: atomic radius,
* - GCU_IONIC: ionic radius,
* - GCU_METALLIC: metallic radius,
* - GCU_COVALENT: covalent radius,
* - GCU_VAN_DER_WAALS: van der Waals radius,
*
*  This enumeration is used in the GcuAtomicRadius structure.
*/
enum gcu_radius_type
{
	GCU_RADIUS_UNKNOWN,
	GCU_ATOMIC,
	GCU_IONIC,
	GCU_METALLIC,
	GCU_COVALENT,
	GCU_VAN_DER_WAALS
};

/** GcuAtomicRadius
* Structure used to describe an atomic radius.
*/
typedef struct
{
	/** The atomic number. */
	unsigned char Z;
	/** The radius type (unknown, ionic, metallic, covalent, or van der Waals).*/
	enum gcu_radius_type type;
	/** The radius value in pm. */
	double value;
	/** The charge of the atom (or ion). */
	char charge;
	/** The scale name, e.g. "Pauling" or "Shannon". */
	char* scale;
	/** the coordination number of the atom or -1 if unknown. */
	char cn;	//coordination number: -1: unspecified
	/** The spin state (unknown, low or high).*/
	enum gcu_spin_state spin;
} GcuAtomicRadius;

/*! GcuElectronegativity
Structure used to describe an electronegativity value.
*/
typedef struct
{
	/** The atomic number. */
	unsigned char Z;
	/*! The electronegativity value.*/
	double value;
	/** The scale name, e.g. "Pauling" or "Mulliken". */
	char* scale;
} GcuElectronegativity;


/*!
gcu_element_get_default_color:
\param Z: the atomic number of the element.

Retreives the default color used for the element.
\return an array of three double values for the red, green and blue components of the color.
*/
const gdouble* gcu_element_get_default_color(gint Z);
/*!
gcu_element_get_symbol:
\param Z: the atomic number of the element.
\return the symbol of the element.
*/
const gchar* gcu_element_get_symbol(gint Z);
/*!
gcu_element_get_name:
\param Z: the atomic number of the element.
\return the name of the element in the current locale or in english if the current locale is not supported in the database.
*/
const gchar* gcu_element_get_name(gint Z);
/*!
gcu_element_get_Z:
\param symbol: the symbol of the element (e.g. "C" ot "Pt").
\return the atomic number of the element.
*/
gint gcu_element_get_Z(gchar* symbol);
/*!
gcu_element_get_symbol:
\param radius: a pointer to a GcuAtomicRadius structure.

Before calling this function, most fields in the GcuAtomicRadius structure must be filled:
- Z: the atomic number, mandatory
- type: the type of the radius searched
- charge: the charge of the atom. 0 for all radii except ionic radii.
- cn: the coordination number or -1 if not significant
- spin: the spin state or GCU_N_A_SPIN if not significant
- scale: the name of the scale (e.g. "Pauling") or NULL

The programs searches a value corresponding to the fields having a non default value. If one is found
the other fields are given the corresponding values f the first match before returning.

\return TRUE if a radius has been found and FALSE if not.

*/
gboolean gcu_element_get_radius(GcuAtomicRadius* radius);
/*!
gcu_element_get_electronegativity:
\param en: a pointer to a GcuElectronegativity structure.

Before calling this function, the following fields in the GcuElectronegativity structure must be filled:
- Z: the atomic number, mandatory
- scale: the name of the scale (e.g. "Pauling") or NULL

The programs searches an electronegativity value for the element in the scale if given. If one is found
the value and the scale (if NULL on calling)  are given the corresponding values of the first match before returning.

\return TRUE if a match has been found and FALSE if not.
*/
gboolean gcu_element_get_electronegativity(GcuElectronegativity* en);
/*!
gcu_element_get_radii:
\param Z: the atomic number of the element.

\return a pointer to the array of pointers to GcuAtomicRadius structures for all known radii for the element.
Last value in the array is NULL.
*/
const GcuAtomicRadius** gcu_element_get_radii(gint Z);
/*!
gcu_element_get_electronegativities:
\param Z: the atomic number of the element.

\return a pointer to the array of pointers to GcuElectronegativity structures for all known electronegativities for the element.
Last value in the array is NULL.
*/
const GcuElectronegativity** gcu_element_get_electronegativities(gint Z);

G_END_DECLS

#endif //GCU_CHEMISTRY_H
