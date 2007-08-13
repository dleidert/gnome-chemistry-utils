// -*- C++ -*-

/* 
 * GChemPaint library
 * molecule.h 
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

#ifndef GCHEMPAINT_MOLECULE_H
#define GCHEMPAINT_MOLECULE_H

#include "cycle.h"
#include "fragment.h"
#include <list>
#include <openbabel/mol.h>

namespace gcp {

class Molecule: public Object
{
public:
	Molecule (TypeId Type = MoleculeType);
	Molecule (Atom* pAtom);
	virtual ~Molecule ();
	void Add (GtkWidget* w);
	void AddAtom (Atom* pAtom);
	void AddFragment (Fragment* pFragment);
	void AddBond (Bond* pBond);
	void Remove (Object* pObject);
	void UpdateCycles (Bond* pBond);
	bool Merge (Molecule* pMolecule, bool RemoveDuplicates = false);
	void UpdateCycles ();
	virtual bool Load (xmlNodePtr);
	virtual xmlNodePtr Save (xmlDocPtr xml);
	void Clear ();
	virtual void SetSelected (GtkWidget* w, int state);
	virtual void Transform2D (Matrix2D& m, double x, double y);
	virtual Object* GetAtomAt (double x, double y, double z = 0.);
	virtual double GetYAlign ();
	virtual bool BuildContextualMenu (GtkUIManager *UIManager, Object *object, double x, double y);
	virtual bool OnSignal (SignalId Signal, Object *Child);
	void ExportToGhemical ();
	void SelectAlignmentItem (Object *child);
	string GetAlignmentId () {return (m_Alignment)? m_Alignment->GetId (): "";}
	void BuildOBMol (OBMol &Mol);
	void BuildOBMol2D (OBMol &Mol);
	void ShowInChI ();
	void BuildInChI ();
	void BuildSMILES ();
	void ShowWebBase (char const *uri_start, char const *uri_end);
	void OpenCalc ();
	void CheckCrossings (Bond *pBond);
	char const *GetInChI ();

private:
	list<Cycle*> m_Cycles;
	list<Chain*> m_Chains;
	list<Atom*> m_Atoms;
	list<Fragment*> m_Fragments;
	list<Bond*> m_Bonds;
	Object *m_Alignment;
	string m_InChI;
	bool m_Changed;
	bool m_IsResidue;
};

}	//	namespace gcp

#endif // GCHEMPAINT_MOLECULE_H
