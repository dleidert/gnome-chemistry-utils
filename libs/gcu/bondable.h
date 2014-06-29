// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gcu/bondable.h
 *
 * Copyright (C) 2014 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCU_BONDABLE_H
#define GCU_BONDABLE_H

#include "object.h"
#include "bond.h"

namespace gcu {

class Bondable: public Object
{
public:
	Bondable (TypeId type);
	virtual ~Bondable ();

public:
	virtual unsigned GetMaxBonds ();
	virtual unsigned GetMaxBondNumber ();
	virtual unsigned GetMaxMultiplicity ();
/*!
@param pBond a pointer to the new Bond to this Bondable.

Adds a Bond.
*/
	virtual void AddBond (Bond* pBond) = 0;
/*!
@param pBond a pointer to the Bond to remove from this Bondable.

Removes a Bond.
*/
	virtual void RemoveBond (Bond* pBond) = 0;
/*!
@return the x coordinate of this Bondable.
*/
	virtual double x () const = 0;
	virtual double &x () = 0;
/*!
@return the y coordinate of this Bondable.
*/
	virtual double y () const = 0;
	virtual double &y () = 0;
/*!
@return the z coordinate of this Bondable.
*/
	virtual double z () const = 0;
	virtual double &z () = 0;
/*!
@param i a C++ std::map iterator.

Use this function to retrieve the first Bond of this Bondable and initialize the iterator.
@return the first Bond of this Bondable or NULL if the Bondable has is not bonded.
*/
	Bond *GetFirstBond (std::map < Bondable *, Bond * >::iterator& i);
/*!
@param i a C++ std::map constant iterator.

Use this function to retrieve the first Bond of this constant Bondable and initialize the iterator.
@return the first Bond of this Bondable or NULL if the Bondable has is not bonded.
*/
	Bond const *GetFirstBond (std::map < Bondable *, Bond * >::const_iterator& i) const;
/*!
@param i a C++ std::map iterator initialized by Bondable::GetFirstBond.

Use this method to iterate through the list of Bond instances of this Bondable.
@return the next Bond of this Bondable or NULL.
*/
	Bond *GetNextBond (std::map <Bondable*, Bond*>::iterator& i);
/*!
@param i a C++ std::map constant iterator initialized by Bondable::GetFirstBond(std::map < Bondable *, Bond * >::const_iterator&).

Use this method to iterate through the list of Bond instances of this Bondable.
@return the next Bond of this Bondable or NULL.
*/
	Bond const *GetNextBond (std::map< Bondable *, Bond * >::const_iterator& i) const;
/*!
@param pBondable a pointer to an Bondable instance.
@return a pointer to the Bond shared by pBondable and this Bondable if it exists or NULL.
*/
	Bond* GetBond (Bondable* pBondable) const;
/*!
@return the number of Bond instances shared by this Bondable. It does not take multiplicity of bonds into account.
*/
	virtual int GetBondsNumber () const {return m_Bonds.size();}

protected:
/*!
The Bond instances of the Atom. The index of the map is a pointer to the other end of the Bond.
*/
	std::map < Bondable *, Bond * > m_Bonds;
};

}   //  namespace gcu

#endif  //  GCU_BONDABLE_H
