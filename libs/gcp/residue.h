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

/*!\file*/
namespace gcp
{

class Application;
class Document;
class Molecule;

/*!\class Residue gcp/residue.h
\brief a GChemPaint specific Residue class.

This class adds some GChemPaint specific features to gcu::Residue.
*/
class Residue: public gcu::Residue
{
public:
/*!
The default constructor.
*/
	Residue ();
/*!
@param name the name of the new residue.

Constructs a new residue with the given name.
*/
	Residue (char const *name);
/*!
@param name the name of the new residue.
@param symbol the symbol of the new residue.
@param mol the molecule represented by the residue.
@param doc the document owning the residue when it does not have global scope.

Constructs a new residue using the given arguments.
*/
	Residue (char const *name, char const *symbol, Molecule *mol, Document *doc);
/*!
The destructor.
*/
	virtual ~Residue ();

/*!
@param node an XML node.
@param ro whether the residue is read-only or not.

Loads data GChemPaint specific data from \a node, and calls
gcu::Residue::Load().
*/
	void Load (xmlNodePtr node, bool ro, gcu::Application *app);
/*!
@param mol the molecule to compare to the residue.

@return true if \a mol is identical to the group represented by this residue,
including the pseudo atom.
*/
	bool operator== (gcu::Molecule const &mol) const;
/*!
Registers the residue in the database.
*/
	void Register ();
/*!
Increments the references number by one unit.
*/
	void Ref ();
/*!
Decreases the references number by one unit.
*/
	void Unref ();

/*!
@param cb a callback to call when a new residue is registered.

When a new residue is registered in the database, it might be necessary to execute
some extra code, hence this static method. Only one callback can be registered
in this version.
*/
	static void SetPostAddCallback (void (*cb) (Residue *res)) {m_AddCb = cb;}

private:
	static void (*m_AddCb) (Residue *res);

/*!\fn GetReadOnly()
@return true if the residue is read-only or false if it can be modified.
*/
GCU_RO_PROP (bool, ReadOnly);
/*!\fn GetNode()
@return the XML node representing the residue.
*/
GCU_RO_PROP (xmlNodePtr, Node);
/*!\fn GetMolNode()
@return the XML node representing the molecule associated to the residue.
*/
GCU_RO_PROP (xmlNodePtr, MolNode);
/*!\fn GetRefs()
@return the number of uses of the residue in currently opened documents. This
is used to prevent deleting an used writeable residue.
*/
GCU_RO_PROP (unsigned , Refs);
};

}	//	namespace gcp

#endif	//	GCP_RESIDUE_H

