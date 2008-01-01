// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * libs/gcu/molecule.h 
 *
 * Copyright (C) 2001-2007 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCU_MOLECULE_H
#define GCU_MOLECULE_H

#include "object.h"

namespace gcu {

class Atom;
class Bond;
class Chain;
class Cycle;

class Molecule: public Object
{
public:
	Molecule (TypeId Type = MoleculeType);
	Molecule (Atom* pAtom);
	virtual ~Molecule ();

	void AddChild (Object* object);
	virtual void AddAtom (Atom* pAtom);
	virtual void AddBond (Bond* pBond);
	virtual void Remove (gcu::Object* pObject);
	void UpdateCycles (Bond* pBond);
	void UpdateCycles ();

protected:
	std::list<Cycle*> m_Cycles;
	std::list<Chain*> m_Chains;
	std::list<Atom*> m_Atoms;
	std::list<Bond*> m_Bonds;
};

}	//	namespace gcu

#endif	//	GCU_MOLECULE_H
