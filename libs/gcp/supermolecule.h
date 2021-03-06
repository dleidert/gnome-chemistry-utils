// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gcp/supermolecule.h
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

#ifndef GCP_SUPERMOLECULE_H
#define GCP_SUPERMOLECULE_H

#include <gcu/object.h>

namespace gcp {

extern gcu::TypeId SuperMoleculeType;

/*!\class SuperMolecule gcp/supermolecule.h
\brief the representation of a supermolecule.

The Supermolecule class represents assemblies of molecules linked by weak bonds.
*/
class SuperMolecule: public gcu::Object
{
public:
/*!
The default constructor.
*/
	SuperMolecule ();
/*!
The destructor.
*/
	virtual ~SuperMolecule ();
};

}   //  namespace gcc

#endif  //  GCP_SUPERMOLECULE_H
