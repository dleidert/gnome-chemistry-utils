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
#include <gcu/macros.h>

namespace gcp {

using namespace gcu;

extern TypeId ReactionPropType;

/*!\class ReactionProp gcp/reaction-prop.h
This is a container class for objects attached to a reaction arrow.
*/

class ReactionProp: public Object
{
public:
	ReactionProp ();
	ReactionProp (Object *child, char const *role);
	~ReactionProp ();

	xmlNodePtr Save (xmlDocPtr xml);
	bool Load (xmlNodePtr);

private:
GCU_RO_PROP (Object*, Object);
GCU_RO_PROP (string, Role);
};

}	//	namespace gcp


#endif	//	GCHEMPAINT_REACTION_PROP_H
