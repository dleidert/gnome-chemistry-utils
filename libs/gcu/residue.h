// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * residue.h 
 *
 * Copyright (C) 2007 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCU_RESIDUE_H
#define GCU_RESIDUE_H

#include "macros.h"
#include <openbabel/mol.h>
#include <map>

namespace gcu {

class Residue
{
public:
	Residue ();
	Residue (char *symbol);
	~Residue ();

	std::map<int,int> &GetRawFormula ();
	OpenBabel::OBMol &GetMolecule ();

private:
	std::map<int,int> Raw;
	OpenBabel::OBMol mol;

GCU_RO_PROP (char const *, Symbol);
};

}	//	namespace gcu

#endif	//	GCU_RESIDUE_H
