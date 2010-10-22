// -*- C++ -*-

/* 
 * GChemPaint library
 * brackets.h 
 *
 * Copyright (C) 2010 Jean Bréfort <jean.brefort@normalesup.org>
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
#include <set>

/*!\file*/
namespace gcp {

extern gcu::TypeId BracketsType;

/*!\enum BracketsTypes
Enumeration of the known brackets types.
*/
typedef enum
{
/*!
*/
	BracketsTypeNormal,
/*!
*/
	BracketsTypeSquare,
/*!
*/
	BracketsTypeCurly
} BracketsTypes;

/*!\class Braclets gcp/brackets.h
*/
class Brackets: public gcu::Object, public gccv::ItemClient
{
public:
/*!
Used to create a brackets pair or a single bracket.
*/
	Brackets (BracketsTypes type = BracketsTypeNormal);
/*!
The destructor.
*/
	virtual ~Brackets();

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

Used to set the selection state of the arrow.
The values of state might be gcp::SelStateUnselected, gcp::SelStateSelected,
gcp::SelStateUpdating, or gcp::SelStateErasing.
*/
	void SetSelected (int state);
	void SetEmbeddedObjects (std::set <gcu::Object *> objects);

private:
	std::set <gcu::Object *> m_EmbeddedObjects;

GCU_PROP (BracketsTypes, Type)
};

}	//	namespace gcp

#endif	//GCHEMPAINT_BRACKETS_H
