// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * molecule.h 
 *
 * Copyright (C) 2001-2008 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "molecule.h"
#include "atom.h"
#include "bond.h"
#include "chain.h"
#include "cycle.h"

using namespace std;

namespace gcu
{

Molecule::Molecule (TypeId Type): Object (Type)
{
}

Molecule::Molecule (Atom* pAtom): Object (MoleculeType)
{
	AddAtom (pAtom);
	Chain* pChain = new Chain (this, pAtom); //will find the cycles
	delete pChain;
}

Molecule::~Molecule ()
{
	std::list<Bond*>::iterator n, end = m_Bonds.end ();
	for (n = m_Bonds.begin (); n != end; n++)
		(*n)->RemoveAllCycles ();
	while (!m_Cycles.empty ()) {
		delete m_Cycles.front ();
		m_Cycles.pop_front ();
	}
	while (!m_Chains.empty ()) {
		delete m_Chains.front ();
		m_Chains.pop_front ();
	}
}

void Molecule::AddChild (Object* object)
{
	switch (object->GetType ()) {
	case AtomType: {
		Atom *atom = reinterpret_cast<Atom *> (object);
		AddAtom (atom);
		break;
	}
	case gcu::BondType: {
		Bond *bond = reinterpret_cast<Bond *> (object);
		m_Bonds.remove (bond);
		AddBond (bond);
		break;
	}
	}
}

void Molecule::AddAtom (Atom* pAtom)
{
	m_Atoms.remove (pAtom); // avoid duplicates
	m_Atoms.push_back (pAtom);
	Object::AddChild (pAtom);
}

void Molecule::AddBond (Bond* pBond)
{
	m_Bonds.remove (pBond); // avoid duplicates
	m_Bonds.push_back (pBond);
	Object::AddChild (pBond);
}

void Molecule::Remove (gcu::Object* pObject)
{
	switch (pObject->GetType ()) {
	case AtomType:
		m_Atoms.remove ((Atom*) pObject);
		break;
	case gcu::BondType:
		m_Bonds.remove ((Bond*) pObject);
		break;
	}
	pObject->SetParent (GetParent ());
}


void Molecule::UpdateCycles (Bond* pBond)	//FIXME: function not totally implemented
{
	Chain* pChain = new Chain (this, pBond); //will find the cycles
	delete pChain;
}

void Molecule::UpdateCycles ()
{
	Lock (true);
	std::list<Bond*>::iterator n, end = m_Bonds.end ();
	for (n = m_Bonds.begin (); n != end; n++)
		(*n)->RemoveAllCycles ();
	while (!m_Cycles.empty()) {
		delete m_Cycles.front ();
		m_Cycles.pop_front ();
	}
	if (!m_Atoms.empty()) {
		std::list<Atom*>::iterator i = m_Atoms.begin (), end = m_Atoms.end ();
		i++;
		for (; i != end; i++)
			(*i)->SetParent(NULL);
		Chain* pChain = new Chain (this, *(m_Atoms.begin ())); //will find the cycles
		delete pChain;
		end = m_Atoms.end ();
		list<Atom*> orphans;
		for (i = m_Atoms.begin (); i != end; i++)
			if ((*i)->GetParent () == NULL)
				orphans.push_back (*i);;
		end = orphans.end ();
		for (i = orphans.begin (); i != end; i++)
			(*i)->SetParent (this);
	}
	Lock (false);
}

bool Molecule::operator== (Molecule const& molecule) const
{
	// first examine each atom of each molecule and sort by Z.
	map<int, set<Atom*> > atoms1, atoms2;
	list<Atom*>::const_iterator ia, enda = m_Atoms.end ();
	for (ia = m_Atoms.begin ();  ia != enda; ia++)
		atoms1[(*ia)->GetZ ()].insert (*ia);
	enda = molecule.m_Atoms.end ();
	for (ia = molecule.m_Atoms.begin ();  ia != enda; ia++)
		atoms2[(*ia)->GetZ ()].insert (*ia);
	if (atoms1.size () != atoms2.size ())
		return false;
	map<int, set<Atom*> >::iterator ib, endb = atoms1.end (), ic, endc = atoms2.end ();
	unsigned n = m_Atoms.size (), m;
	int z = 200;
	for (ib = atoms1.begin (); ib != endb; ib++) {
		if ((ic = atoms2.find ((*ib).first)) == endc)
			return false;
		if ((m = (*ib).second.size ()) != (*ib).second.size ())
			return false;
		if (m < n)
			n = m;
		else if (m == n && (*ib).first < z)
			z = (*ib).first;

	}
	// TODO: write this code
	return true;
}

}	//namespace gcu
