// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * atom.h 
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
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA  02111-1307, USA.
 */

#ifndef GCU_ATOM_H
#define GCU_ATOM_H

#include <map>
#include <glib.h>
#include "object.h"

using namespace std;

namespace gcu
{
	
class Bond;
/*!\class Atom gcu/atom.h
This class is used to represent atoms.

*/
class Atom: public Object
{
public:
/*!
The default constructor. Creates an atom with atomic number set to 0.
*/
	Atom ();
/*!
@param Z: the atomic number of the new atom.
@param x: the x coordinate of the new atom.
@param y: the y coordinate of the new atom.
@param z: the z coordinate of the new atom.

Creates an atom.
*/
	Atom (int Z, double x, double y, double z = 0.);
/*!
@param a: the Atom to duplicate.

Creates a new atom identical to a without any bond.
*/
	Atom (Atom& a);
/*!
@param a: the Atom to copy.

@return an Atom identical to a without any bond.
*/
	Atom& operator= (Atom& a);
/*!
The destructor of Atom. It removes bonds but do not delete the associated Bond instances.
*/
	virtual ~Atom ();

public :
/*!
@param pAtom: a pointer to an Atom.
@return the distance between the Atom at pAtom and this Atom.
*/
	double Distance (Atom* pAtom);
/*!
@param ZoomFactor: the zoom factor.

Multiplies all three coordinates of this Atom by ZoomFactor.
*/
	void zoom (double ZoomFactor);
/*!
@param x: a pointer to the double value which will receive the x coordiante of the Atom.
@param y: a pointer to the double value which will receive the y coordiante of the Atom.
@param z: a pointer to the double value which will receive the z coordiante of the Atom or NULL for 2D representations.

Retrieves the coordinates of this Atom.
@return true if successful and false if an error occurs (if x or y is NULL).
*/
	virtual bool GetCoords (double *x, double *y, double *z = NULL);
/*!
@param x: the new x coordinate of the Atom.
@param y: the new y coordinate of the Atom.
@param z: the new z coordinate of the Atom.

Changes the position of this Atom.
*/
	void SetCoords (double x, double y, double z = 0) {m_x = x; m_y = y; m_z = z;}
/*!
@return the atomic number of the atom.
*/
	int GetZ () {return m_Z;}
/*!
@param Z: the new atomic number of the Atom.

This method might be overrided by derived class since changing the atomic number generally changes most properties.
The default behavior is just to change the atomic number and nothing else.
*/
	virtual void SetZ (int Z);
/*!
@param Charge: the new formal charge of the Atom.

Changes the formal charge of this Atom.
*/
	void SetCharge (char Charge) {m_Charge = Charge;}
/*!
@return the formal charge of this Atom.
*/
	char GetCharge () {return m_Charge;}
/*!
@return the atomic symbol of this Atom or NULL if the element is unknown.
*/
	const gchar* GetSymbol ();
/*!
@param pBond: a pointer to the new Bond to this Atom.

Adds a Bond.
*/
	virtual void AddBond (Bond* pBond);
/*!
@param pBond: a pointer to the Bond to remove from this Atom.

Removes a Bond.
*/
	virtual void RemoveBond (Bond* pBond);
/*!
@return the x coordinate of this Atom.
*/
	double x () {return m_x;}
/*!
@return the y coordinate of this Atom.
*/
	double y () {return m_y;}
/*!
@return the z coordinate of this Atom.
*/
	double z () {return m_z;}
/*!
@param i: a C++ std::map iterator.

Use this function to retrieve the first Bond of this Atom and initialize the iterator.
@return the first Bond of this Atom or NULL if the Atom has is not bonded.
*/
	Bond* GetFirstBond (map<Atom*, Bond*>::iterator& i);
/*!
@param i: a C++ std::map iterator initialized by Atom::GetFirstBond.

Use this method to iterate through the list of Bond instances of this Atom.
@return the next Bond of this Atom or NULL.
*/
	Bond* GetNextBond (map<Atom*, Bond*>::iterator& i);
/*!
@param pAtom: a pointer to an Atom instance.
@return a pointer to the Bond shared by pAtom and this Atom if it exists or NULL.
*/
	Bond* GetBond (Atom* pAtom);
/*!
@return the number of Bond instances shared by this Atom. It does not take multiplicity of bonds into account.
*/
	int GetBondsNumber () {return m_Bonds.size();}
/*!
@param xml: the xmlDoc used to save the document.
@return a pointer to the xmlNode representing this Atom or NULL if an error occured.
*/
	virtual xmlNodePtr Save (xmlDocPtr xml);
/*!
@param node: a pointer to the xmlNode containing the serialized Atom.

Loads an atom from an xmlNode.
*/
	virtual bool Load (xmlNodePtr node);
/*!
@param node: a pointer to the xmlNode containing the serialized Atom.

This virtual method is called at the end of the Atom::Load method. The default behavior is to do nothing.
It might be overrided for derived class when it is not convenient to override the Atom::Load method.
*/
	virtual bool LoadNode (xmlNodePtr node);
/*!
@param xml: the xmlDoc used to save the document.
@param node: a pointer to the xmlNode to which this Atom is serialized.

This virtual method is called at the end of the Atom::Save method. The default behavior is to do nothing.
It might be overrided for derived class when it is not convenient to override the Atom::Save method.
*/
	virtual bool SaveNode (xmlDocPtr xml, xmlNodePtr node);
/*!
@param x: the x component of the transation vector.
@param y: the y component of the transation vector.
@param z: the z component of the transation vector.

Used to move an Atom.
*/
	virtual void Move (double x, double y, double z = 0.);
/*!
@param m: the 2D Matrix of the transformation.
@param x: the x component of the center of the transformation.
@param y: the y component of the center of the transformation.

Used to move and/or transform an atom.
*/
	virtual void Transform2D (Matrix2D& m, double x, double y);

protected:
/*!
The atomic number of the Atom.
*/
	int m_Z;
/*!
The x coordinate of the Atom.
*/
	double m_x;
/*!
The x coordinate of the Atom.
*/
	double m_y;
/*!
The x coordinate of the Atom.
*/
	double m_z;
/*!
The charge of the Atom.
*/
	char m_Charge;
/*!
The Bond instances of the Atom. The index of the map is a pointer to the other end of the Bond.
*/
	map<Atom*, Bond*> m_Bonds;
};

} //namespace gcu
#endif // GCU_ATOM_H
