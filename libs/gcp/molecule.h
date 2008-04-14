// -*- C++ -*-

/* 
 * GChemPaint library
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

#ifndef GCHEMPAINT_MOLECULE_H
#define GCHEMPAINT_MOLECULE_H

#include "fragment.h"
#include <list>
#include <gcu/molecule.h>
#include <openbabel/mol.h>

namespace gcp {

class Bond;

class Molecule: public gcu::Molecule
{
public:
	Molecule (gcu::TypeId Type = gcu::MoleculeType);
	Molecule (Atom* pAtom);
	virtual ~Molecule ();
	void AddChild (gcu::Object* object);
	void AddAtom (gcu::Atom* pAtom);
	void AddFragment (Fragment* pFragment);
	void AddBond (gcu::Bond* pBond);
	void Add (GtkWidget* w) const;
	void Remove (gcu::Object* pObject);
	bool Merge (Molecule* pMolecule, bool RemoveDuplicates = false);
	bool Load (xmlNodePtr);
	xmlNodePtr Save (xmlDocPtr xml) const;
	void Clear ();
	void SetSelected (GtkWidget* w, int state);
	void Transform2D (gcu::Matrix2D& m, double x, double y);
	Object* GetAtomAt (double x, double y, double z = 0.);
	double GetYAlign ();
	bool BuildContextualMenu (GtkUIManager *UIManager, gcu::Object *object, double x, double y);
	bool OnSignal (gcu::SignalId Signal, gcu::Object *Child);
	void ExportToGhemical ();
	void SelectAlignmentItem (gcu::Object *child);
	std::string GetAlignmentId () {return (m_Alignment)? m_Alignment->GetId (): "";}
	void BuildOBMol (OpenBabel::OBMol &Mol);
	void BuildOBMol2D (OpenBabel::OBMol &Mol);
	void ShowInChI ();
	void BuildInChI ();
	void BuildSMILES ();
	void ShowWebBase (char const *uri_start, char const *uri_end);
	void OpenCalc ();
	void CheckCrossings (Bond *pBond);
	char const *GetInChI ();
	std::string GetRawFormula () const;
	void OnLoaded ();
	unsigned GetAtomsNumber () const;

private:
	std::list<Fragment*> m_Fragments;
	gcu::Object *m_Alignment;
	std::string m_InChI;
	bool m_Changed;
	bool m_IsResidue;
};

}	//	namespace gcp

#endif // GCHEMPAINT_MOLECULE_H
