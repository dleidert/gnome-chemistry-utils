// -*- C++ -*-

/* 
 * GChemPaint library
 * reaction-prop.cc 
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

#include "config.h"
#include "reaction-prop.h"
#include "reaction-arrow.h"
#include <glib/gi18n-lib.h>

namespace gcp {

TypeId ReactionPropType = NoType;

char const *ReactionPropRoles[] = {
	N_("Unkown"),
	N_("Catalyst"),
	N_("Reactant"),
	N_("Product"),
	N_("Solvent"),
	N_("Temperature"),
	N_("Pressure"),
	N_("Time"),
	N_("Enthalpy"),
};

static unsigned RoleFromString (char const *role)
{
	unsigned res = REACTION_PROP_MAX;
	while (res > REACTION_PROP_UNKNOWN &&
		   !strcmp (ReactionPropRoles[--res], role));
	return res;
}

ReactionProp::ReactionProp ():
	Object (ReactionPropType)
{
}

ReactionProp::ReactionProp (ReactionArrow *parent, Object *child):
	gcu::Object (ReactionPropType),
	m_Object (child),
	m_Role (REACTION_PROP_UNKNOWN)
{
	SetParent (parent);
	AddChild (child);
}

ReactionProp::~ReactionProp ()
{
}

xmlNodePtr ReactionProp::Save (xmlDocPtr xml)
{
	xmlNodePtr node;
	node = Object::Save (xml);
	if (!node)
		return NULL;
	xmlNewProp (node, (xmlChar*) "role",  (xmlChar*) ReactionPropRoles[m_Role]);
	return node;
}

bool ReactionProp::Load (xmlNodePtr node)
{
	bool res = Object::Load (node);
	if (res) {
		char *buf = (char*) xmlGetProp (node, (xmlChar*) "role");
		if (buf) {
			m_Role = RoleFromString (buf);
			xmlFree (buf);
		}
	}
	return res;
}

}	//	namespace gcp
