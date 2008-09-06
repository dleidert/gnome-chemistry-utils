// -*- C++ -*-

/* 
 * GChemPaint library
 * residue.h 
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

#ifndef GCP_RESIDUE_H
#define GCP_RESIDUE_H

#include <gcu/macros.h>
#include <gcu/residue.h>

namespace gcp
{

class Application;
class Document;
class Molecule;

class Residue: public gcu::Residue
{
public:
	Residue ();
	Residue (char const *name);
	Residue (char const *name, char const *symbol, Molecule *mol, Document *doc);
	virtual ~Residue ();

	void Load (xmlNodePtr node, bool ro);
	bool operator== (gcu::Molecule const &mol) const;
	void Register ();
	void Ref ();
	void Unref ();

	static void SetPostAddCallback (void (*cb) (Residue *res)) {m_AddCb = cb;}

private:
	static void (*m_AddCb) (Residue *res);

GCU_RO_PROP (bool, ReadOnly);
GCU_RO_PROP (xmlNodePtr, Node);
GCU_RO_PROP (xmlNodePtr, MolNode);
GCU_RO_PROP (unsigned , Refs);
};

}	//	namespace gcp

#endif	//	GCP_RESIDUE_H

