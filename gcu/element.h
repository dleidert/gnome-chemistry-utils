// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * element.h 
 *
 * Copyright (C) 2002-2004
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
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef GCU_ELEMENT_H
#define GCU_ELEMENT_H

#include <glib.h>
#include <string>
#include <vector>
#include "chemistry.h"

using namespace std;


/*!\namespace gcu
The namespace used for all C++ classes provided by the Gnome Chemistry Utils.
*/
namespace gcu
{

class EltTable;

/*!\class Element gcu/element.h
	Represents a chemical element.
	This class has no public constructor or destructor. The instances are created by the framework
	from data in the elements.xml file and
	a user cannot create a new element.
*/
class Element
{
friend class EltTable;
private:
	/*!
	\param Z: the atomic number corresponding to the element
	\param Symbol: the atomic symbol of the element

	This constructor is private and cannot be used ouside of this class.
	*/
	Element (int Z, const char* Symbol);
	virtual ~Element ();

public:
	/*!
	@param Z: the atomic number of a chemical element.
	@return The chemical symbol of the element whose atomic number is Z or NULL if the element is unknown.
	*/
	static const gchar* Symbol (gint Z);
	/*!
	@param Z: the atomic number of a chemical element.

	This static method is used to know on what side of the symbol of the element whose atomic number is Z
	attached hydrogens should be written.
	@return true if hydrogens should be written on the right and false when it should be written on the left side.
	*/
	static bool BestSide (gint Z);
	/*!
	@param symbol: the symbol of a chemical element.
	@return The atomic number of the element whose chemical symbol is used as parameter or 0 if the element is unknown.
	*/
	static gint Z (const gchar* symbol);
	/*!
	@param Z: the atomic number of a chemical element.
	@return a pointer to the Element whose atomic number is Z or NULL if the element is unknown.
	*/
	static Element* GetElement (gint Z);
	/*!
	@param symbol: the symbol of a chemical element.
	@return a pointer to the Element whose symbol is used as parameter or NULL if the element is unknown.
	*/
	static Element* GetElement (const gchar* symbol);
	/*!
	@param radius: a pointer to a GcuAtomicRadius structure.
	
	Before calling this function, most fields in the GcuAtomicRadius structure must be filled:
	- Z: the atomic number, mandatory
	- type: the type of the radius searched
	- charge: the charge of the atom. 0 for all radii except ionic radii.
	- cn: the coordination number or -1 if not significant
	- spin: the spin state or GCU_N_A_SPIN if not significant
	- scale: the name of the scale (e.g. "Pauling") or NULL
	
	The programs searches a value corresponding to the fields having a non default value. If one is found
	the other fields are given the corresponding values f the first match before returning.
	
	@return true if a radius has been found and false if not.
	*/
	static bool GetRadius (GcuAtomicRadius* radius);
	/*!
	@param en: a pointer to a GcuElectronegativity structure.
	
	Before calling this function, the following fields in the GcuElectronegativity structure must be filled:
	- Z: the atomic number, mandatory
	- scale: the name of the scale (e.g. "Pauling") or NULL
	
	The programs searches an electronegativity value for the element in the scale if given. If one is found
	the value and the scale (if NULL on calling)  are given the corresponding values of the first match before returning.

	@return true if a match has been found and false if not.
	*/
	static bool GetElectronegativity (GcuElectronegativity* en);
	/*!
	@param Z: the atomic number of a chemical element.
	
	The value returned by this method might be too low in some cases and is only indicative. Instances of the Atom class
	accept any number of bonds. This behavior might change in future versions.
	@return the maximum number of bonds an atom of the element can be involved in.
	*/
	static unsigned GetMaxBonds (gint Z);

	/*!
	\return The atomic number of the chemical element.
	*/
	int GetZ () {return m_Z;}
	/*!
	\return The chemical symbol of the element.
	*/
	const char* GetSymbol () {return m_Symbol;}
	/*!
	\return The default valence of the element for some elements, mainly non metals. For others, the returned value is -1
	and should not be taken into account.
	*/
	char GetDefaultValence () {return m_DefaultValence;}
	/*!
	The value returned by this method might be too low in some cases and is only indicative. Instances of the Atom class
	accept any number of bonds. This behavior might change in future versions.
	@return the maximum number of bonds an atom of the element can be involved in.
	*/
	unsigned GetMaxBonds () {return m_MaxBonds;}
	/*!
	This static method is used to know on what side of the symbol of the element
	attached hydrogens should be written.
	@return true if hydrogens should be written on the right and false when it should be written on the left side.
	*/
	bool GetBestSide () {return m_BestSide;}
	/*!
	Retreives the default color used for the element.
	@return an array of three double values for the red, green and blue components of the color.
	*/
	double* GetDefaultColor () {return m_DefaultColor;}
	/*!
	@return the name of the element in the current locale or in english if the current locale is not supported in the database.
	*/
	const char* GetName () {return name.c_str();}
	/*!
	@return a pointer to the array of pointers to GcuAtomicRadius structures for all known radii for the element.
	Last value in the array is NULL.
	*/
	const GcuAtomicRadius** GetRadii ();
	/*!
	@return a pointer to the array of pointers to GcuElectronegativity structures for all known electronegativities for the element.
	Last value in the array is NULL.
	*/
	const GcuElectronegativity** GetElectronegativities ();
	/*!
	@return the number of valence electrons of the neutral atom.
	*/
	unsigned GetValenceElectrons () {return m_nve;}
	/*!
	@return the number of valence electrons of the neutral atom,
	including d and f electrons.
	*/
	unsigned GetTotalValenceElectrons () {return m_tve;}
	/*!
	@return the maximume number of valence electrons of the neutral atom,
	including d and f electrons.
	*/
	unsigned GetMaxValenceElectrons () {return m_maxve;}
	
private:
	unsigned char m_Z, m_nve, m_tve, m_maxve;
	char m_Symbol[4];
	char m_DefaultValence;
	unsigned char m_MaxBonds;
	bool m_BestSide;
	double m_DefaultColor[3];
	string name;
	vector<GcuAtomicRadius*> m_radii;
	vector<GcuElectronegativity*> m_en;
};

} // namespace gcu

#endif // GCU_ELEMENT_H
