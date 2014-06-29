// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gcp/weak-bond.h
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

#ifndef GCP_WEAK_BOND_H
#define GCP_WEAK_BOND_H

#include <gcu/object.h>

namespace gcp {

extern gcu::TypeId WeakBondType;

/*!\class WeakBond gcp/weak-bond.h
\brief the representation of a weak bond.

The WeakBond class represents weak bonds such as hydrogen bonds, either inside
a molecule or between differnet molecules grouped inside a supermolecule.
*/
class WeakBond: public gcu::Object
{
public:
/*!
The default constructor.
*/
	WeakBond ();
/*!
The destructor.
*/
	virtual ~WeakBond ();
};

}   //  namespace gcc

#endif  //  GCP_WEAK_BOND_H
