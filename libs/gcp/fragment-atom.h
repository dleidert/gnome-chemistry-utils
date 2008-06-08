// -*- C++ -*-

/* 
 * GChemPaint library
 * fragment-atom.h 
 *
 * Copyright (C) 2003-2008 Jean Bréfort <jean.brefort@normalesup.org>
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

namespace gcp {

class Fragment;

class FragmentAtom: public Atom
{
public:
	FragmentAtom ();
	FragmentAtom (Fragment *fragment, int Z);
	virtual ~FragmentAtom ();

	void SetZ (int Z);
	bool AcceptNewBonds (int nb);
	void Add (GtkWidget* w) const;
	void Update ();
	void Update (GtkWidget* w) const;
	void SetSelected (GtkWidget* w, int state);
	xmlNodePtr Save (xmlDocPtr xml) const;
	bool Load (xmlNodePtr node);
	int GetChargePosition (unsigned char& Pos, double Angle, double& x, double& y);
	int GetAvailablePosition (double& x, double& y);
	bool GetPosition (double angle, double& x, double& y);
	bool AcceptCharge (int charge);

/*!
@param Mol: a pointer to a molecule

Adds the fragment containing the atom to the molecule calling gcpMolecule::AddFragment()
*/
	void AddToMolecule (Molecule* Mol);
	bool Match (gcu::Atom *atom, gcu::AtomMatchState &state);

	void DoBuildSymbolGeometry (View *pView);

GCU_RO_POINTER_PROP (Fragment, Fragment)
};

}	//	namespace gcp

#endif // GCHEMPAINT_FRAGMENT_ATOM_H
