// -*- C++ -*-

/* 
 * GChemPaint library
 * fragment-atom.h 
 *
 * Copyright (C) 2003-2009 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_FRAGMENT_ATOM_H
#define GCHEMPAINT_FRAGMENT_ATOM_H

#include "atom.h"
#include <gcu/macros.h>

/*!\file*/
namespace gcp {

class Fragment;
class View;

/*!\class FragmentAtom gcp/fragment-atom.h
Represents an atom inside a atoms group (see gcp::Fragment class).
*/
class FragmentAtom: public Atom
{
public:
/*!
The default constructor.
*/
	FragmentAtom ();
/*!
@param fragment the parent Fragment.
@param Z the atomic number.

Constructs a FragmentAtom inside \a fragment with atomic number Z. This does
not add the symbol to the string.
*/
	FragmentAtom (Fragment *fragment, int Z);
/*!
The destructor.
*/
	virtual ~FragmentAtom ();

/*!
@param Z the new atomic number.

Changes the atomic number of the atom.
*/
	void SetZ (int Z);
/*!
@param nb the number of bonds to add, taking orders into account.

This atom class does not currently support more than one single bond and
no multiple bond at all.
@return true if the operation is allowed, false otherwise.
*/
	bool AcceptNewBonds (int nb);
/*!
Overrided to avoid Atom::Update execution. Just call Fragment::Update() method.
*/
	void Update ();
/*!
Overrided to avoid Atom::AddItem execution. Don't do anything.
*/
	void AddItem ();
/*!
Overrided to avoid Atom::UpdateItem execution. Just call Fragment::UpdateItem().
*/
	void UpdateItem ();
/*!
@param state the selection state of the atom.

Overrided to avoid Atom::SetSelected execution. Just call Fragment::SetSelected
method.
*/
	void SetSelected (int state);
/*!
@param xml the xmlDoc used to save the document.

Used to save the atome specific data to the xmlDoc.
@return the xmlNode containing the serialized atom.
*/
	xmlNodePtr Save (xmlDocPtr xml) const;
/*!
param node a pointer to the xmlNode containing the serialized atom.

Used to load the atom specific properties in memory. The FragmentAtom must
already exist.
@return true on succes, false otherwise.
*/
	bool Load (xmlNodePtr node);
/*!
@param Pos the approximate position of the charge.
@param Angle the angle from horizontal left.
@param x the x position of the charge symbol.
@param y the y position of the charge symbol.

On input \a Pos can be one of POSITION_E, POSITION_N,... or 0xff, in which case,
it will be given a default value. \a x and \a y are set to the position where the charge
sign should be displayed usding the alignment code returned by this method.
@return the anchor for the charge symbol. On error, gccv::AnchorCenter is used as
the returned value.
*/
	gccv::Anchor GetChargePosition (unsigned char& Pos, double Angle, double& x, double& y);
/*!
@param x the x position.
@param y the y position.

This method finds an available position for drawing a charge sign and returns
it as a symbolic value (see POSITION_E, POSITION_N,...). The \a x and \a y are updated so
that they give the absolute position.
@return an available position.
*/
	int GetAvailablePosition (double& x, double& y);
/*!
@param angle the angle at which a charge sign should be displayed.
@param x the x position.
@param y the y position.

Updates \a x and \a y so that they become the absolute position corresponding to the angle
when the position is available.
@return true on success, false otherwise.
*/
	bool GetPosition (double angle, double& x, double& y);
/*!
@param charge the charge that might be set.

Currently, these atoms only accept -1, 0, and +1 as charges.
@return true if the charge is acceptable.
*/
	bool AcceptCharge (int charge);

/*!
@param Mol: a pointer to a molecule

Adds the fragment containing the atom to the molecule calling gcpMolecule::AddFragment()
*/
	void AddToMolecule (Molecule* Mol);
/*!
@param atom the atom to which the this instance is to be compared.
@param state the AtomMatchState representing the current comparison state.

Try to match atoms from two molecules which are compared. This function calls
itself recursively until all atoms from the two molecules have been matched or
until an difference is found. Overriden methods should call this base function
and return its result. FragmentAtom instances can't be matched currently.
@return always false.
*/
	bool Match (gcu::Atom *atom, gcu::AtomMatchState &state);
/*!
@param pView the document view.

Builds the symbol geometry if necessary.
*/
	void DoBuildSymbolGeometry (View *pView);

/*!\fn GetFragment()
@return the Fragment eclosing the FragmentAtom.
*/
GCU_RO_POINTER_PROP (Fragment, Fragment)
};

}	//	namespace gcp

#endif // GCHEMPAINT_FRAGMENT_ATOM_H
