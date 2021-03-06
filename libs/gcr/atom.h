// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gcr/atom.h
 *
 * Copyright (C) 2002-2010 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCR_ATOM_H
#define GCR_ATOM_H

#include <gcu/atom.h>
#include <gcu/chemistry.h>
#include <gcu/macros.h>
#include <libxml/parser.h>
#include <libxml/parserInternals.h>
#include <libxml/xmlmemory.h>
#include <list>

/*!\file*/
namespace gcr
{

#define PREC 1e-3

/*! \class Atom gcr/atom.h
Used to represent atoms in a crystal.
*/
class Atom : public gcu::Atom
{
public:
/*
The default constructor.
*/
	Atom ();
/*
The destructor of Atom.
*/
	virtual ~Atom ();

public :
/*
@param Z: the atomic number of the new atom.
@param x: the x coordinate of the new atom.
@param y: the y coordinate of the new atom.
@param z: the z coordinate of the new atom.

Creates an atom.
*/
	Atom (int Z, double x, double y, double z);
/*!
@param caAtom: the Atom to duplicate.

Creates a new Atom identical to caAtom.
*/
	Atom (Atom& caAtom);
/*
@param caAtom: the Atom to copy.

@return an Atom identical to caAtom.
*/
	Atom& operator= (Atom& caAtom);

/*!
@param red: the red component of the color.
@param green: the green component of the color.
@param blue: the blue component of the color.
@param alpha: the alpha component of the color.

Sets a custom color to the atom.
*/
	void SetColor (float red, float green, float blue, float alpha);
/*!
Gives the atom the default color.
*/
	void SetDefaultColor ();
/*!
@return: true if the color is user defined and false if it is the default color.
*/
	bool HasCustomColor () {return m_bCustomColor;}
/*!
@param red: a pointer to the red component.
@param green: a pointer to the green component.
@param blue: a pointer to the blue component.
@param alpha: a pointer to the alpha component.

Used to retrieve the color used in the representation of the atom. Mainly useful for user defined
colors.
*/
	void GetColor (double *red, double *green, double *blue, double *alpha);
	void GetColor (GdkRGBA &rgba);
/*!
@param r: the new value of the atomic radius.

Sets the value of the radius (in pm).
*/
	void SetSize (double r);
/*!
@return the value of the radius (in pm).
*/
	double GetSize ();
/*!
@param caAtom: a Atom instance.
@return true if the atoms are at the same position and false if their positions are different.
*/
	bool operator== (Atom& caAtom);
/*!
Method used to cleave an atom. The inverse operation does not exist since the whole crystal must be recalculated
after a change in the definition.
*/
	void Cleave () {m_nCleave++;}
/*!
@param h: the h Miller index of a plane.
@param k: the k Miller index of a plane.
@param l: the l Miller index of a plane.

@return the product hx+ky+lz where x, y and z are the coordinates of the atom. This makes sense only if coordinates
are related to the net and are not the cartesian coordinates. This method should not be called after NetToCartesian().
*/
	double ScalProd (int h, int k, int l);
/*!
@param x: the x coordinate of the center.
@param y: the y coordinate of the center.
@param z: the z coordinate of the center.
@param bFixed: tells if cleaved atoms are taken into account.

This helper method is called when searching for the size of the crystal. When some cleavages are defined,
the procedure cn take into account atoms cleaved to get the same position in the view for the cleaved crystal
than for the whole crystal. If bFixed is true, all atoms are taken into account.

@return the distance of the atom to the center of the view or 0 if bFixed is false and the atom cleaved.
*/
	double Distance (double x, double y, double z, bool bFixed);
/*!
@return the value of the radius (in pm).
*/
	double r () {return m_Radius.value.value;}
/*!
@return the GcuAtomicRadius containing the caracteristics of the atom radius.
*/
	const GcuAtomicRadius& GetRadius () {return m_Radius;}
/*!
@param r: a GcuAtomicRadius with the caracteristics of the atom radius.
*/
	void SetRadius (const GcuAtomicRadius& r);
/*!
@return true if the atom is cleaved by at least one cleavage or false if the atom is not cleaved at all.
*/
	bool IsCleaved () {return m_nCleave != 0;}
/*!
@param xml: the xmlDoc used to save the document.
@param node: a pointer to the xmlNode to which this Atom is serialized.

Saves the color and the radius of the atom.
*/
	virtual bool SaveNode (xmlDocPtr xml, xmlNodePtr node) const;
/*!
@param node: a pointer to the xmlNode containing the serialized Atom.

Loads the color and the radius of the atom.
*/
	virtual bool LoadNode (xmlNodePtr node);

protected:
/*!
The blue component of the color of the sphere representing the atom.
*/
	float m_fBlue;
/*!
The red component of the color of the sphere representing the atom.
*/
	float m_fRed;
/*!
The green component of the color of the sphere representing the atom.
*/
	float m_fGreen;
/*!
The alpha component of the color of the sphere representing the atom.
*/
	float m_fAlpha;
/*!
False if the color used is the default color and true if it is a user defined color.
*/
	bool m_bCustomColor;
/*!
The GcuAtomicRadius containing the radius caracteristics of the atom.
*/
	GcuAtomicRadius m_Radius;
/*!
When cleavages (see gcr::Cleavage class documentation) are defined, the atom might be cleaved. m_nCleave is
the number of gcr::Cleavage instances which remove the atom. If this member is not 0, the atom will
not be displayed.
*/
	int m_nCleave; //0 if not cleaved

/*!
@param property the identity of the property as defined in objprops.h.
@param value the value of the property as a string.

Used by the gcu::Loader mechanism to load properties of atoms.
@return true on success.
*/
	bool SetProperty (unsigned property, char const *value);

/*!
@param property the identity of the property as defined in objprops.h.

Used by the gcu::Loader mechanism to retrieve properties of atoms.
@return the value of the property as a string.
*/
	std::string GetProperty (unsigned property) const;

GCU_PROP (double, EffectiveRadiusRatio);
};

/*!
a list of pointers to gcr::Atom instances derived from std::list.
*/
typedef std::list<Atom*> AtomList;
}// namespace gcr

#endif // GCR_ATOM_H
