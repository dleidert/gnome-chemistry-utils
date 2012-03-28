// -*- C++ -*-

/*
 * GChemPaint library
 * brackets.h
 *
 * Copyright (C) 2010-2012 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_BRACKETS_H
#define GCHEMPAINT_BRACKETS_H

#include <gcu/object.h>
#include <gccv/item-client.h>
#include <gccv/brackets.h>
#include <set>

/*!\file*/
namespace gcp {

extern gcu::TypeId BracketsType;

/*!\enum BracketContent
\brief The contents on a pair of brackets

Defines what is enclosed by a pair of brackets. One of the brackets might
be invisible.
*/
typedef enum {
/*!
Invalid or unknown content.
*/
	BracketContentInvalid,
/*!
The brackets enclose part of a molecule.
*/
	BracketContentFragment,
/*!
The brackets enclose a whole molecule.
*/
	BracketContentMolecule,
/*!
The brackets enclose several objects.
*/
	BracketContentGroup
} BracketContent;

/*!\enum BracketDecorations
\brief Describes the objects that can be attached to the brackets.

Defines which of a superscript and subscript can be attached to a bracket.
The values can be combined. Actually, only subscript are supported for now.
*/
typedef enum {
/*!
Nothing allowed.
*/
	BracketDecorationNone = 0,
/*!
A subscript is allowed.
*/
	BracketSubscript = 1,
/*!
A superscript is allowed.
*/
	BracketSuperscript = 2
} BracketsDecorations;

/*!\class Braclets gcp/brackets.h
\brief Brackets class.

Represents brackets of various types that might be used in different situations
as to enclose part of a molecule or a whole object or several objects.
*/
class Brackets: public gcu::Object, public gccv::ItemClient
{
public:
/*!
\param type whether to create a brackets pair or a single bracket.

Used to create a brackets pair or a single, closing, or opening bracket.
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
/*!
@param UIManager: the gcu::UIManager to populate.
@param object the atom on which occured the mouse click.
@param x x coordinate of the mouse click.
@param y y coordinate of the mouse click.

This method is called to build a contextual menu for the brackets.
*/
	bool BuildContextualMenu (gcu::UIManager *UIManager, Object *object, double x, double y);

/*!
@param objects: the objects to enclose.

Sets the collection of objects that should be enclosed inside the brackets.
*/
	void SetEmbeddedObjects (std::set < gcu::Object * > objects);

/*!
@return the collection of objects that should be enclosed inside the brackets.
*/
	std::set < gcu::Object * > const &GetEmbeddedObjects () {return m_EmbeddedObjects;}

/*!
Tests if a collection of atoms can be enclosed inside brackets. Enclosing non
connected atoms is not allowed.
@return true if all atoms in the set are connected.
*/
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
