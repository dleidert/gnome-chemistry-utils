// -*- C++ -*-

/* 
 * GChemPaint library
 * chain.h 
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

#ifndef GCHEMPAINT_CHAIN_H
#define GCHEMPAINT_CHAIN_H

#include "atom.h"
#include "bond.h"
#include <map>

using namespace gcu;

namespace gcp {

class Molecule;

typedef struct
{
	Bond *fwd, *rev;
} ChainElt;

class Chain: public Object
{
public:
	Chain (Bond* pBond, Atom* pAtom = NULL, TypeId Type = ChainType);
	Chain (Molecule* Molecule, Bond* pBond, TypeId Type = ChainType);
	Chain (Molecule* Molecule, Atom* pAtom, TypeId Type = ChainType);
	virtual ~Chain();
	
	void FindCycles (Atom* pAtom);
	bool FindCycle (Atom* pAtom, Bond* pBond);
	virtual void Erase (Atom* pAtom1, Atom* pAtom2);
	virtual void Insert (Atom* pAtom1, Atom* pAtom2, Chain& Chain);
	void Extract (Atom* pAtom1, Atom* pAtom2, Chain& Chain);
	void Reverse ();
	void AddBond (Atom* start, Atom* end);
	unsigned GetUnsaturations ();
	unsigned GetHeteroatoms ();
	bool Contains (Atom* pAtom);
	bool Contains (Bond* pBond);
	unsigned GetLength ();
	double GetMeanBondLength ();
	Atom* GetNextAtom (Atom* pAtom);

protected:
	map<Atom*, ChainElt> m_Bonds;
	Molecule* m_Molecule;
	guint m_nMolIndex;
};

}	//	namespace gcp

#endif // GCHEMPAINT_CHAIN_H
