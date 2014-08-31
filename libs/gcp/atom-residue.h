// -*- C++ -*-

/*
 * GChemPaint library
 * atom-residue.h
 *
 * Copyright (C) 2008-2014 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_ATOM_RESIDUE_H
#define GCHEMPAINT_ATOM_RESIDUE_H

#include "atom.h"
#include "residue.h"
#include <string>

/*!\file*/
namespace gcp {

extern gcu::TypeId ResidueType;

/*!\class FragmentResidue gcp/fragment-residue.h
Represents a residue inside a atoms group (see gcp::Fragment class).
*/
class AtomResidue: public Atom
{
public:
/*!
The default constructor.
*/
	AtomResidue ();
	virtual ~AtomResidue ();

/*!
@param xml 	the xmlDoc used to save the document.

Builds an XML node representing this instance.
@return the new XML node or NULL on error.
*/
	xmlNodePtr Save (xmlDocPtr xml) const;
/*!
@param node an XML node.

Loads the symbol and associated residue from \a node.
@return true on success, false otherwise.
*/
	bool Load (xmlNodePtr node);
/*!
@param res the residue corresponding to the symbol.
@param symbol the symbol to use, or NULL to use the default symbol.

Sets the associated residue.
*/
	void SetResidue (Residue const *res, char const *symbol = NULL);
/*!
@return the symbol of this Residue.
*/
	char const *GetSymbol () const;

/*!\fn GetResidue()
@return the associated residue.
*/
GCU_RO_PROP (Residue const *, Residue)
/*!\fn GetAbbrev()
@return the used symbol for the residue.
*/
GCU_RO_PROP (std::string, Abbrev)
};

}	//	namespace gcp

#endif	//	GCHEMPAINT_ATOM_RESIDUE_H
