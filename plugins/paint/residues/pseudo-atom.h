// -*- C++ -*-

/*
 * GChemPaint residues plugin
 * pseudo-atom.h
 *
 * Copyright (C) 2007-2008 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCP_PSEUDO_ATOM_H
#define GCP_PSEUDO_ATOM_H

#include <gcp/atom.h>

extern gcu::TypeId PseudoAtomType;

class gcpPseudoAtom: public gcp::Atom
{
public:
	gcpPseudoAtom ();
	gcpPseudoAtom (double x, double y);
	virtual ~gcpPseudoAtom ();

	void Update ();
	void AddItem ();
	void UpdateItem ();
	virtual bool Load (xmlNodePtr);
	xmlNodePtr Save (xmlDocPtr xml) const;
	bool LoadNode (xmlNodePtr);
	void SetSelected (int state);
	bool AcceptNewBonds (G_GNUC_UNUSED int nb = 1) {return false;}
	bool AcceptCharge (G_GNUC_UNUSED int charge) {return false;}
};

#endif	//	GCP_PSEUDO_ATOM_H
