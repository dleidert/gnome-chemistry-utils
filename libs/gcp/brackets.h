// -*- C++ -*-

/*
 * GChemPaint library
 * brackets.h
 *
 * Copyright (C) 2010-2011 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_BRACKETS_H
#define GCHEMPAINT_BRACKETS_H

#include <gcu/object.h>
#include <gccv/item-client.h>
#include <gccv/brackets.h>
#include <set>

/*!\file*/
namespace gcp {

extern gcu::TypeId BracketsType;

typedef enum {
	BracketContentInvalid,
	BracketContentFragment,
	BracketContentMolecule,
	BracketContentGroup
} BracketContent;

typedef enum {
	BracketSubscript = 1,
	BracketSuperscript = 2
} BracketsDecorations;

/*!\class Braclets gcp/brackets.h
*/
class Brackets: public gcu::Object, public gccv::ItemClient
{
public:
/*!
Used to create a brackets pair or a single bracket.
*/
	Brackets (gccv::BracketsTypes type = gccv::BracketsTypeNormal);
/*!
The destructor.
*/
	virtual ~Brackets();


/*!
Used to add a representation of the brackets in the view.
*/
	void AddItem ();

/*!
@param node: a pointer to the xmlNode containing the serialized brackets.

Used to load a Brackets instance in memory.

@return true on succes, false otherwise.
*/
	bool Load (xmlNodePtr node);
/*!
	@param xml the xmlDoc used to save the document.

	Used to save the Brackects to the xmlDoc.
*/
	xmlNodePtr Save (xmlDocPtr xml) const;
/*!
@param state: the selection state of the brackets.

Used to set the selection state of the brackets.
The values of state might be gcp::SelStateUnselected, gcp::SelStateSelected,
gcp::SelStateUpdating, or gcp::SelStateErasing.
*/
	void SetSelected (int state);
/*!
For brackets, OnLoaded() is called each time an embedded object is added.
*/
	void OnLoaded ();

/*!
@param object the object just unlinked by Object::Unlink.

Called when an object has been unlinked. Programs should not call it
directly, but should call Object::OnUnlink instead.
*/
	void OnUnlink (Object *object);

	void SetEmbeddedObjects (std::set < gcu::Object * > objects);
	std::set < gcu::Object * > const &GetEmbeddedObjects () {return m_EmbeddedObjects;}
	static bool ConnectedAtoms (std::set < gcu::Object * > const &objects);

private:
	std::set < gcu::Object * > m_EmbeddedObjects;
	bool m_Valid;
	BracketContent m_Content;

GCU_PROP (gccv::BracketsTypes, Type)
GCU_PROP (gccv::BracketsUses, Used)
GCU_RO_PROP (unsigned, Decorations)
GCU_PROP (std::string, FontDesc)
};

}	//	namespace gcp

#endif	//GCHEMPAINT_BRACKETS_H
