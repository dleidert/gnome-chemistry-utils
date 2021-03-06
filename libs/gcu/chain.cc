// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * libs/gcu/chain.cc
 *
 * Copyright (C) 2001-2011 Jean Bréfort <jean.brefort@normalesup.org>
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

#include "config.h"
#include "atom.h"
#include "bond.h"
#include "chain.h"
#include "cycle.h"
#include "molecule.h"
#include "document.h"
#include <glib/gi18n-lib.h>

using namespace std;

namespace gcu {

Chain::Chain (Bond* pBond, Atom* pAtom, TypeId Type): Object (Type)
{
	Atom *pAtom0;
	if (pAtom)
		pAtom0 = (Atom*) pBond->GetAtom (pAtom);
	else {
		pAtom0 = (Atom*) pBond->GetAtom (1);
		pAtom = (Atom*) pBond->GetAtom (0);
	}
	m_Bonds[pAtom].fwd = pBond;
	m_Bonds[pAtom0].rev = pBond;
}

Chain::Chain (Molecule* molecule, Atom* pAtom, TypeId Type): Object (Type)
{
	m_Molecule = molecule;
	if (pAtom) {
		FindCycles (pAtom);
	}
}

/*
* Add a bond in an existing molecule and update cycles
* Implementation might have to be changed
*/
Chain::Chain (Molecule* molecule, Bond* pBond, TypeId Type): Object (Type)
{
	m_Molecule = molecule;
	if (pBond) {
		Atom *pAtom0, *pAtom;
		pAtom0 = (Atom*) pBond->GetAtom (0);
		m_Bonds[pAtom0].fwd = pBond;
		pAtom = (Atom*) pBond->GetAtom (1);
		m_Bonds[pAtom].rev = pBond;
		map < gcu::Bondable *, gcu::Bond * >::iterator i;
		Bond* pBond0 = (Bond*) pAtom->GetFirstBond (i);
		while (pBond0) {
			if ((pBond0 != pBond) && FindCycle (pAtom, pBond0))
				break;
			pBond0 = (Bond*) pAtom->GetNextBond (i);
		}
	}
}

Chain::~Chain ()
{
	m_Bonds.clear ();
}

bool Chain::FindCycle (Atom* pAtom, Bond* pBond)
{
	Atom* pAtom1 = (Atom*) pBond->GetAtom (pAtom);
	if (m_Bonds[pAtom1].fwd != NULL) {
		if (m_Bonds[pAtom1].rev != NULL)
			return false;
		Cycle* pCycle = new Cycle (m_Molecule);
		pCycle->m_Bonds[pAtom1].rev = pBond;
		pCycle->m_Bonds[pAtom1].fwd = m_Bonds[pAtom1].fwd;
		pCycle->m_Bonds[pAtom].fwd = pBond;
		pCycle->m_Bonds[pAtom].rev= m_Bonds[pAtom].rev;
		m_Bonds[pAtom].rev->AddCycle (pCycle);
		pBond->AddCycle (pCycle);
		while (pBond = pCycle->m_Bonds[pAtom1].fwd, pAtom1 = (Atom*) pBond->GetAtom (pAtom1), pAtom != pAtom1) {
			pCycle->m_Bonds[pAtom1].rev = pBond;
			pCycle->m_Bonds[pAtom1].fwd = m_Bonds[pAtom1].fwd;
			pBond->AddCycle (pCycle);
		}
		pCycle->Simplify ();	//to reduce size of fused cycles
		m_Molecule->m_Cycles.push_back (pCycle);
		return true;
	}
	m_Bonds[pAtom].fwd = pBond;
	m_Bonds[pAtom1].rev = pBond;
	map < gcu::Bondable *, gcu::Bond * >::iterator i;
	Bond* pBond1 = (Bond*) pAtom1->GetFirstBond (i);
	while (pBond1) {
		if ((pBond1 != pBond) && FindCycle (pAtom1, pBond1))
			return true;
		pBond1 = (Bond*) pAtom1->GetNextBond (i);
	}
	m_Bonds[pAtom].fwd = NULL;
	m_Bonds.erase (pAtom1);
	return false;
}

void Chain::FindCycles (Atom* pAtom)
{
	map < gcu::Bondable *, gcu::Bond * >::iterator i;
	Bond* pBond = (Bond*) pAtom->GetFirstBond (i);
	Atom* pAtom0;
	Molecule *mol;
	while (pBond != NULL) {
		m_Bonds[pAtom].fwd = pBond;
		pAtom0 = (Atom*) pBond->GetAtom (pAtom);
		if ((mol = static_cast < Molecule * > (pBond->GetMolecule ())) != m_Molecule) {
			if (mol)
				mol->ClearCycles ();
			m_Molecule->AddChild (pBond);
		}
		if ((pAtom0)->GetMolecule () != m_Molecule) {
			if (pAtom0->GetMolecule () != m_Molecule)
				m_Molecule->AddChild (pAtom0);
			FindCycles (pAtom0);
		} else {
			if (m_Bonds[pAtom0].fwd != NULL) {
				Bond* pBond0 = m_Bonds[pAtom0].fwd;
				if (pAtom != pBond0->GetAtom (pAtom0)) {
					//Cycle found
					Cycle* pCycle = new Cycle (m_Molecule);
					pCycle->m_Bonds[pAtom0].rev = pBond;
					pCycle->m_Bonds[pAtom0].fwd = pBond0;
					pBond0->AddCycle (pCycle);
					while (pAtom != pAtom0) {
						pAtom0 = (Atom*) pBond0->GetAtom (pAtom0);
						pCycle->m_Bonds[pAtom0].rev = pBond0;
						pBond0 = m_Bonds[pAtom0].fwd;
						pCycle->m_Bonds[pAtom0].fwd = pBond0;
						pBond0->AddCycle (pCycle);
					}
					pCycle->Simplify ();	//to reduce size of fused cycles
					m_Molecule->m_Cycles.push_back (pCycle);
				}
			}
		}
		pBond = (Bond*) pAtom->GetNextBond (i);
	}
	m_Bonds.erase (pAtom);
}

void Chain::Reverse ()
{
	map<Atom*, ChainElt>::iterator i, end = m_Bonds.end ();
	Bond* pBond;
	for (i = m_Bonds.begin (); i != end; i++) {
		pBond = (*i).second.fwd;
		(*i).second.fwd = (*i).second.rev;
		(*i).second.rev = pBond;
	}
}

void Chain::Erase (Atom* pAtom1, Atom* pAtom2)
{
//This function is not safe
	Atom *pAtom = (Atom*) m_Bonds[pAtom1].fwd->GetAtom (pAtom1), *pAtom0;
	m_Bonds[pAtom1].fwd = NULL;
	while (pAtom != pAtom2) {
		pAtom = (Atom*) m_Bonds[pAtom].fwd->GetAtom (pAtom0 = pAtom);
		m_Bonds.erase (pAtom0);
	}
	m_Bonds[pAtom2].rev = NULL;
}

void Chain::Insert (Atom* pAtom1, Atom* pAtom2, Chain& chain)
{
//This function is not safe
	m_Bonds[pAtom1].fwd = chain.m_Bonds[pAtom1].fwd;
	Atom *pAtom = (Atom*) m_Bonds[pAtom1].fwd->GetAtom (pAtom1);
	while (pAtom != pAtom2) {
		m_Bonds[pAtom] = chain.m_Bonds[pAtom];
		pAtom = (Atom*) m_Bonds[pAtom].fwd->GetAtom (pAtom);
	}
	m_Bonds[pAtom2].rev = chain.m_Bonds[pAtom2].rev;
}

void Chain::Extract (Atom* pAtom1, Atom* pAtom2, Chain& chain)
{
	chain.m_Bonds.clear ();
	if (m_Bonds[pAtom1].fwd == NULL) {
		if (m_Bonds[pAtom1].rev == NULL)
			m_Bonds.erase (pAtom1);	//pAtom1 is not in the chain
		return;
	}
	chain.m_Bonds[pAtom1].fwd = m_Bonds[pAtom1].fwd;
	chain.m_Bonds[pAtom1].rev = NULL;
	Atom *pAtom = (Atom*) chain.m_Bonds[pAtom1].fwd->GetAtom (pAtom1);
	while (pAtom != pAtom2) {
		chain.m_Bonds[pAtom] = m_Bonds[pAtom];
		if (m_Bonds[pAtom].fwd == NULL)
			return; //Chain never reach pAtom2
		pAtom = (Atom*)m_Bonds[pAtom].fwd->GetAtom (pAtom);
	}
	chain.m_Bonds[pAtom2].rev = m_Bonds[pAtom2].rev;
	chain.m_Bonds[pAtom2].fwd = NULL;
}

unsigned Chain::GetUnsaturations ()
{
	unsigned n = 0;
	std::map<Atom*, ChainElt>::iterator i, end = m_Bonds.end ();
	for (i = m_Bonds.begin (); i != end; i++)
		if ((*i).second.fwd && ((*i).second.fwd->GetOrder () > 1))
		n += 1;
	return n;
}

unsigned Chain::GetHeteroatoms ()
{
	unsigned n = 0;
	std::map<Atom*, ChainElt>::iterator i, end = m_Bonds.end ();
	for (i = m_Bonds.begin (); i != end; i++)
		if ((*i).first->GetZ () != 6)
		n += 1;
	return n;
}

void Chain::AddBond (Atom* start, Atom* end)
{
	Bond* pBond = (Bond*) start->GetBond (end);
	m_Bonds[start].fwd = pBond;
	m_Bonds[end].rev = pBond;
}

bool Chain::Contains (Atom* pAtom)
{
	if ((m_Bonds[pAtom].fwd == NULL) && (m_Bonds[pAtom].rev == NULL)) {
		m_Bonds.erase (pAtom);
		return false;
	}
	return true;
}

bool Chain::Contains (Bond* pBond)
{
	Atom *pAtom = (Atom*) pBond->GetAtom (0);
	if ((m_Bonds[pAtom].fwd == NULL) && (m_Bonds[pAtom].rev == NULL)) {
		m_Bonds.erase (pAtom);
		return false;
	}
	if ((m_Bonds[pAtom].fwd == pBond) && (m_Bonds[pAtom].rev == pBond))
		return true;
	return false;
}

unsigned Chain::GetLength ()
{
	unsigned n = 0;
	std::map<Atom*, ChainElt>::iterator i, end = m_Bonds.end ();
	for (i = m_Bonds.begin(); i != end; i++)
		if ((*i).second.fwd)
			n++;
	return n;
}

double Chain::GetMeanBondLength ()
{
	unsigned n = 0;
	double l = 0;
	std::map<Atom*, ChainElt>::iterator i, end = m_Bonds.end ();
	for (i = m_Bonds.begin (); i != end; i++)
		if ((*i).second.fwd) {
			n++;
			l += (*i).second.fwd->Get2DLength ();
		}
	return l / n;
}

Atom* Chain::GetFirstAtom ()
{
	std::map < Atom*, ChainElt >::iterator i = m_Bonds.begin ();
	if (GetType () == CycleType)
		return (*i).first;
	Atom *res = (*i).first, *cur = res;
	while (cur) {
		cur = m_Bonds[res].rev->GetAtom (res);
		if (cur == NULL)
			break;
		res = cur;
	}
	return res;
}

Atom* Chain::GetNextAtom (Atom* pAtom)
{
	return (Atom*) m_Bonds[pAtom].fwd->GetAtom (pAtom);
}

std::string Chain::Name ()
{
	return _("Chain");
}

unsigned Chain::BuildLength (unsigned *cycle_size, unsigned *cycle_pos)
{
	// searching the longest chain starting from the first bond, and stopping at any cycle
	// cycle_size is the largest encountered cycle
	unsigned length = 0;
	unsigned nb;
	unsigned max_cycle_size = 0;
	unsigned min_cycle_pos = 0;
	// searching from there
	std::map < Atom *, ChainElt >::iterator i, end = m_Bonds.end ();
	std::map < Bondable *, Bond * >::iterator b;
	Bond *bond, *last_bond = NULL;
	Atom *atom = NULL;
	for (i = m_Bonds.begin(); i != end; i++) {
		if ((*i).second.fwd)
			length++;
		else {
			atom = (*i).first;
			last_bond = (*i).second.rev;
		}
	}
	while (atom && (nb = atom->GetBondsNumber ()) != 1)
		switch (atom->GetBondsNumber ()) {
		case 2:
			bond = atom->GetFirstBond (b);
			if (bond == last_bond)
				bond = atom->GetNextBond (b);
			m_Bonds[atom].fwd = bond;
			atom = bond->GetAtom (atom);
			m_Bonds[atom].rev = bond;
				last_bond = bond;
			length++;
			break;
		default: {
			// we reached a ramification
			// exploring each chain
			unsigned rcycle_size = 0, rcycle_pos = 0, rlength = 0;
			Chain *longchain = NULL;
			for (bond = atom->GetFirstBond (b); bond; bond = atom->GetNextBond (b)) {
				if (bond == last_bond)
					continue;
				if (bond->IsCyclic ()) {
					if (min_cycle_pos == 0)
						min_cycle_pos = length;
					if (min_cycle_pos == length) {
						std::list < Cycle * >::iterator c;
						Cycle * cycle = bond->GetFirstCycle (c, NULL);
						while (cycle) {
							if (cycle->GetLength () > max_cycle_size)
								max_cycle_size = cycle->GetLength ();
							cycle = bond->GetNextCycle (c, NULL);
						}
					}
					continue;
				}
				// if we are there, we have normal chain that we explore
				unsigned cycle_pos = 0, cycle_size = 0, length;
				Chain *chain = new Chain (bond, atom);
				length = chain->BuildLength (&cycle_size, &cycle_pos);
				if (length > rlength) {
					if (longchain)
						delete longchain;
					longchain = chain;
					if ((cycle_pos = !0) && (cycle_pos < rcycle_pos || rcycle_pos == 0)) {
						rcycle_pos = cycle_pos;
						rcycle_size = cycle_size;
					}
					if (cycle_pos == rcycle_pos && cycle_size >  rcycle_size)
						rcycle_size = cycle_size;
				}
			}
			if (longchain) {
				Append (*longchain);
				if (min_cycle_pos == 0) {
					min_cycle_pos = rcycle_pos;
					max_cycle_size = rcycle_size;
				}
				length += rlength;
				delete longchain;
			}
			atom = NULL;
			break;
		}
		}
	//and now ending
	if (cycle_size)
		*cycle_size = max_cycle_size;
	if (cycle_pos)
		*cycle_pos = min_cycle_pos;
	return length;
}

void Chain::Append (Chain& chain)
{
	std::map < Atom *, ChainElt >::iterator i;
	for (i = m_Bonds.begin (); (*i).second.fwd; i++);
	Atom *atom = (*i).first;
	if (chain.m_Bonds.find (atom) == chain.m_Bonds.end ())
		return;
	m_Bonds[atom].fwd = chain.m_Bonds[atom].fwd;
	atom = m_Bonds[atom].fwd->GetAtom (atom);
	while (chain.m_Bonds[atom].fwd) {
		m_Bonds[atom] = chain.m_Bonds[atom];
		atom = m_Bonds[atom].fwd->GetAtom (atom);
	}

}

}	//	namespace gcp
