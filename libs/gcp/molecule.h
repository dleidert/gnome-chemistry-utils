// -*- C++ -*-

/* 
 * GChemPaint library
 * molecule.h 
 *
 * Copyright (C) 2001-2011 Jean Bréfort <jean.brefort@normalesup.org>
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

/*!\file*/
namespace gcp {

class Bond;

/*!\class Molecule gcp/molecule.h
\brief GChemPaint molecule class.
*/
class Molecule: public gcu::Molecule
{
friend class MoleculePrivate;
public:
/*!
The default constructor. Builds a new empty molecule.
*/
	Molecule (gcu::TypeId Type = gcu::MoleculeType);
/*!
@param pAtom an atom.

Constructs a new molecule containing \a pAtom and all atoms which might be
bonded to it, and the corresponding bonds.
*/
	Molecule (Atom* pAtom);
/*!
The destructor.
*/
	virtual ~Molecule ();
/*!
@param object the object to add to the molecule.

Adds an object (atom, bond, or fragment) to the molecule.
*/
	void AddChild (gcu::Object* object);
/*!
@param pAtom an atom.

Adds an atom to the molecule.
*/
	void AddAtom (gcu::Atom* pAtom);
/*!
@param pFragment an atoms group.

Adds a fragment to the molecule.
*/
	void AddFragment (Fragment* pFragment);
/*!
@param pBond a bond.

Adds a bond to the molecule.
*/
	void AddBond (gcu::Bond* pBond);
/*!
@param pObject an atom, a bond, or a fragment in the molecule.

Removes an atom, a bond, or a fragment from a molecule.
*/
	void Remove (gcu::Object* pObject);
/*!
@param pMolecule a molecule.
@param RemoveDuplicates whether duplicate atoms should be unififed or not.

Adds all children from \a pMolecule in this instance, and removes one of the
atoms for each pair of duplicates (atoms with same atomic number and position)
if \a RemoveDuplicates is true. This might fail when it would end with
hypervalent atoms. On success \a pMolecule is deleted.
@return true on success, false otherwise.
*/
	bool Merge (Molecule* pMolecule, bool RemoveDuplicates = false);
/*!
@param node a pointer to the xmlNode containing the serialized molecule.

Used to load a molecule in memory. The Mlecule instance must already exist.
@return true on succes, false otherwise.
*/
	bool Load (xmlNodePtr node);
/*!
@param xml the xmlDoc used to save the document.

Used to save the molecule to the xmlDoc.
@return the xmlNode containing the serialized molecule.
*/
	xmlNodePtr Save (xmlDocPtr xml) const;
/*!
Removes all children from the molecule, resulting in a empty molecule.
*/
	void Clear ();
/*!
@param m the Matrix2D of the transformation.
@param x the x component of the center of the transformation.
@param y the y component of the center of the transformation.

Used to move and/or transform the molecule taking care of hydrogen atoms
positions around heteroatoms.
*/
	void Transform2D (gcu::Matrix2D& m, double x, double y);
/*!
@param x the x coordinate
@param y the y coordinate
@param z the z coordinate (not used)

@return a pointer to n atom at or near position defined by the coordinates
passed as parameters or NULL if none is found in the molecule.
*/
	Object* GetAtomAt (double x, double y, double z = 0.);
/*!
Used to retrieve the y coordinate for alignment.
@return y coordinate used for the molecule alignment.
*/
	double GetYAlign ();
/*!
@param UIManager the GtkUIManager to populate.
@param object the Object on which occured the mouse click.
@param x x coordinate of the mouse click.
@param y y coordinate of the mouse click.

This method is called to build the contextual menu for the molecule.
@return true if something is added to the UIManager, false otherwise.
*/
	bool BuildContextualMenu (GtkUIManager *UIManager, gcu::Object *object, double x, double y);
/*!
@param Signal the appropriate SignalId
@param Child the child which emitted the signal or NULL

Signals the molecule that at least one of its children changed.

@return true to propagate the signal to the parent.
*/
	bool OnSignal (gcu::SignalId Signal, gcu::Object *Child);
/*!
@param child the child used for alignment.

Selects the child used for alignment, which might be an atom or group of atoms,
or a bond. For bonds, the center of the bonds is used, a while for the others
a call to their gcu::Object::GetYAlign() method will be used.
*/
	void SelectAlignmentItem (gcu::Object *child);
/*!
@return the id string of the child used for alignment if any, or an empty string.
*/
	std::string GetAlignmentId () {return (m_Alignment)? m_Alignment->GetId (): "";}
/*!
Opens GChemCalc with the formula for the molecule. Molecules with fragments
are not currently supported.
*/
	void OpenCalc ();
/*!
@param pBond a bond in the molecule.

Checks if any other bond in the molecule crosses \a pBond, and notify both bonds
that they are crossing.
*/
	void CheckCrossings (Bond *pBond);
/*!
@return the raw formula as a string. Molecules with fragments
are not currently supported.
*/
	std::string GetRawFormula () const;
/*!
Updates all cycles after loading.
*/
	void OnLoaded ();
/*!
@return the number of atoms in the molecule. Atoms groups are counted for one
only, whatever their real atomic composition.
*/
	unsigned GetAtomsNumber () const;

	double GetMeanBondLength () const;
	void AddChiralAtom (Atom *atom) {m_ChiralAtoms.insert (atom);}

private:
	std::list< Fragment * > m_Fragments;
	std::set < Atom * > m_ChiralAtoms;
	gcu::Object *m_Alignment;
	bool m_IsResidue;
};

}	//	namespace gcp

#endif // GCHEMPAINT_MOLECULE_H
