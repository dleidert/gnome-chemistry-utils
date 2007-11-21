// -*- C++ -*-

/* 
 * GChemPaint library
 * reaction-prop.h 
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

#ifndef GCHEMPAINT_REACTION_PROP_H
#define GCHEMPAINT_REACTION_PROP_H

#include <gcu/object.h>
#include <gcu/dialog-owner.h>
#include <gcu/macros.h>

namespace gcp {

extern gcu::TypeId ReactionPropType;
class ReactionArrow;

enum {
	REACTION_PROP_UNKNOWN,
	REACTION_PROP_CATALYST,
	REACTION_PROP_REACTANT,
	REACTION_PROP_PRODUCT,
	REACTION_PROP_SOLVENT,
	REACTION_PROP_MAX_MOL,
	REACTION_PROP_TEMPERATURE = REACTION_PROP_MAX_MOL,
	REACTION_PROP_PRESSURE,
	REACTION_PROP_TIME,
	REACTION_PROP_ENTHALPY,
	REACTION_PROP_MAX,
};

extern char const *ReactionPropRoles[];

/*!\class ReactionProp gcp/reaction-prop.h
This is a container class for objects attached to a reaction arrow.
*/

class ReactionProp: public gcu::Object, public gcu::DialogOwner
{
public:
	ReactionProp ();
	ReactionProp (ReactionArrow *parent, gcu::Object *child);
	~ReactionProp ();

	xmlNodePtr Save (xmlDocPtr xml);
	bool Load (xmlNodePtr);
	bool OnSignal (gcu::SignalId Signal, gcu::Object *Child);

private:
GCU_RO_PROP (gcu::Object*, Object);
GCU_PROP (unsigned, Role);
};

}	//	namespace gcp


#endif	//	GCHEMPAINT_REACTION_PROP_H
