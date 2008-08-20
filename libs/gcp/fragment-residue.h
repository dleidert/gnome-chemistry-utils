// -*- C++ -*-

/* 
 * GChemPaint library
 * fragment-residue.h 
 *
 * Copyright (C) 2008 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_FRAGMENT_RESIDUE_H
#define GCHEMPAINT_FRAGMENT_RESIDUE_H

#include "fragment-atom.h"
#include "residue.h"
#include <string>

/*!\file*/
namespace gcp {

class FragmentResidue: public FragmentAtom
{
public:
	FragmentResidue ();
	FragmentResidue (Fragment *fragment, char const *symbol);
	virtual ~FragmentResidue ();

	xmlNodePtr Save (xmlDocPtr xml) const;
	bool Load (xmlNodePtr node);
	void SetResidue (Residue const *res);
/*!
@return the symbol of this Residue.
*/
	const gchar* GetSymbol () const;

GCU_RO_PROP (Residue const *, Residue)
GCU_RO_PROP (std::string, Abbrev)
};

}	//	namespace gcp

#endif	//	GCHEMPAINT_FRAGMENT_RESIDUE_H
