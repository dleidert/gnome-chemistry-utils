// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * libs/gcu/molecule.h
 *
 * Copyright (C) 2001-2012 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCU_MOLECULE_H
#define GCU_MOLECULE_H

#include "object.h"
#include "structs.h"

/*!\file*/
namespace gcu {

class Atom;
class Bond;
class Chain;
class Cycle;
class Formula;

/*!\class Molecule gcu/molecule.h
Represents molecules.
*/
class Molecule: public Object
{
friend class Chain;
public:
/*!
@param Type the type id of a derived class
@param ct the content type for the molecule (2d or 3d), defaults to
ContentType3D.

The constructor. The type id may be changed in derived classes, otherwise the
argument is not needed, since the default value is enough.
*/
	Molecule (TypeId Type = MoleculeType, ContentType ct = ContentType3D);
/*!
@param pAtom
@param ct the content type for the molecule (2d or 3d), defaults to
ContentType3D.

Builds a molecule from an atom. If the atom has bonds, the connectivity will
be explored and all atoms and bonds found will be added to the molecule.
*/
	Molecule (Atom* pAtom, ContentType ct = ContentType3D);
/*!
The destructor.
*/
	virtual ~Molecule ();

/*!
@param object an object, generally an atom or a bond.

Adds an object (which might be an atom or a bond) to the molecule.
*/
	void AddChild (Object* object);
/*!
@param pAtom an atom.

Adds an atom to the molecule.
*/
	virtual void AddAtom (Atom* pAtom);
/*!
@param pBond a bond.

Adds a bond to the molecule.
*/
	virtual void AddBond (Bond* pBond);
/*!
@param pObject an atom or a bond in the molecule.

Removes an atom or a bond from a molecule.
*/
	virtual void Remove (gcu::Object* pObject);
/*!
@param pBond a bond in the molecule.

Updates the cycles list after a change, starting the exploration from \a pBond.
*/
	void UpdateCycles (Bond* pBond);
/*!
Updates the cycles list after a change.
*/
	void UpdateCycles ();
/*!
@param molecule a molecule.
@return true if the molecules have identical atoms and the connection
framework between the atoms.
*/
	bool operator== (Molecule const& molecule) const;
/*!
@return the number of atoms in the molecule.
*/
	virtual unsigned GetAtomsNumber () const {return m_Atoms.size ();}
/*!
@param Doc a document.
@param formula a formula
@param add_pseudo tells if a pseudo atom (with Z = 0) has to be added (used when
building a gcu::Residue instance).

Tries to build a molecule from a formula, adding bonds between atoms. Atoms
coordinates are not calculated.
@return a molecule on success or NULL.
*/
	static Molecule *MoleculeFromFormula (Document *Doc, Formula const &formula, bool add_pseudo = true);

/*!
Clears cycles and chains and call gcu::Object::Clear().
*/
	void Clear ();
/*!
@param i an uninitialized iterator.

@return the first atom of the molecule.
*/
	Atom const *GetFirstAtom (std::list<Atom*>::const_iterator &i) const;
	Atom *GetFirstAtom (std::list<Atom*>::iterator &i);
/*!
@param i an iterator initialized by a call to GetFirstAtom().

@return the next atom of the molecule or NULL if all atoms have been previously returned.
*/
	Atom const *GetNextAtom (std::list<Atom*>::const_iterator &i) const;
	Atom *GetNextAtom (std::list<Atom*>::iterator &i);
/*!
@param i an uninitialized iterator.

@return the first bond of the molecule.
*/
	Bond const *GetFirstBond (std::list<Bond*>::const_iterator &i) const;
/*!
@param i an iterator initialized by a call to GetFirstBond().

@return the next bond of the molecule or NULL if all bonds have been previously returned.
*/
	Bond const *GetNextBond(std::list<Bond*>::const_iterator &i) const;
/*!
@return the localized object generic name.
*/
/*!
@param name a name.
@param convention a naming convention, might be NULL.

Adds a molecule name following \a convention. Only one name can be stored for
a given convention.
*/
	void SetName (char const *name, char const *convention);
/*!
@param convention a naming convention.

@return the molecule name using \a convention if known.
*/
	char const *GetName (char const *convention = NULL);
/*!
@return the localized object generic name.
*/
	std::string Name ();

/*!
Reinitialize all chemical identifiers for the molecule (InChI, InChIKey, and SMILES
*/
	void ResetIndentifiers ();

/*!
@return a CML representation of the molecule.
*/
	std::string const &GetCML ();

/*!
Clears all information related to cycles.
*/
	void ClearCycles ();

/*!
@return the InChI.
*/
	std::string const &GetInChI ();

/*!
@return the InChIKey.
*/
	std::string const &GetInChIKey ();

/*!
@return the canonical SMILES for the molecule.
*/
	std::string const &GetSMILES ();
/*!
@return the raw formula as a string.
*/
	virtual std::string GetRawFormula () const;

protected:
/*!
The cycles contained in the molecules.
*/
	std::list<Cycle*> m_Cycles;
/*!
The non cyclic chains contained in the molecules (not used at the moment).
*/
	std::list<Chain*> m_Chains;
/*!
The atoms in the molecule.
*/
	std::list<Atom*> m_Atoms;
/*!
The bonds in the molecule.
*/
	std::list<Bond*> m_Bonds;

private:
	std::map <std::string, std::string> m_Names;
	std::string m_CML;
	std::string m_InChI;
	std::string m_InChIKey;
	std::string m_SMILES;
	ContentType m_Content;
};

}	//	namespace gcu

#endif	//	GCU_MOLECULE_H
