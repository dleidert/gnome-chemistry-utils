// -*- C++ -*-

/* 
 * GChemPaint library
 * fragment-atom.h 
 *
 * Copyright (C) 2003-2007 Jean Bréfort <jean.brefort@normalesup.org>
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

namespace gcp {

class Fragment;

class FragmentAtom: public Atom
{
public:
	FragmentAtom ();
	FragmentAtom (Fragment *fragment, int Z);
	virtual ~FragmentAtom ();

	virtual void SetZ (int Z);
	virtual bool AcceptNewBonds (int nb);
	virtual void Add (GtkWidget* w);
	virtual void Update ();
	virtual void Update (GtkWidget* w);
	virtual void SetSelected (GtkWidget* w, int state);
	virtual xmlNodePtr Save (xmlDocPtr xml);
	virtual bool Load (xmlNodePtr node);
	virtual int GetChargePosition (unsigned char& Pos, double Angle, double& x, double& y);
	virtual int GetAvailablePosition (double& x, double& y);
	virtual bool GetPosition (double angle, double& x, double& y);
	virtual bool AcceptCharge (int charge);

/*!
@param Mol: a pointer to a molecule

Adds the fragment containing the atom to the molecule calling gcpMolecule::AddFragment()
*/
	virtual void AddToMolecule (Molecule* Mol);

private:
	Fragment *m_Fragment;
};

}	//	namespace gcp

#endif // GCHEMPAINT_FRAGMENT_ATOM_H
