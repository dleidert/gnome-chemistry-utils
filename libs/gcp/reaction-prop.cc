// -*- C++ -*-

/*
 * GChemPaint library
 * reaction-prop.cc
 *
 * Copyright (C) 2007-2010 Jean Bréfort <jean.brefort@normalesup.org>
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

#include "config.h"
#include "document.h"
#include "reaction-prop.h"
#include "reaction-arrow.h"
#include <glib/gi18n-lib.h>
#include <cstring>

using namespace gcu;

namespace gcp {

TypeId ReactionPropType = NoType;

char const *ReactionPropRoles[] = {
	N_("Unknown"),
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
		   !strcmp (ReactionPropRoles[--res], role)) ;
	return res;
}

ReactionProp::ReactionProp ():
	Object (ReactionPropType),
	DialogOwner (),
	m_Object (NULL),
	m_Role (REACTION_PROP_UNKNOWN),
	m_Step (0),
	m_Line (0),
	m_Rank (0)
{
}

ReactionProp::ReactionProp (ReactionArrow *parent, Object *child):
	Object (ReactionPropType),
	DialogOwner (),
	m_Object (child),
	m_Role (REACTION_PROP_UNKNOWN),
	m_Step (0),
	m_Line (0),
	m_Rank (0)
{
	SetParent (parent);
	AddChild (child);
}

ReactionProp::~ReactionProp ()
{
}

xmlNodePtr ReactionProp::Save (xmlDocPtr xml) const
{
	xmlNodePtr node;
	node = Object::Save (xml);
	if (!node)
		return NULL;
	xmlNewProp (node, (xmlChar*) "role",  (xmlChar*) ReactionPropRoles[m_Role]);
	char *buf;
	if (m_Step > 1) {
			buf = g_strdup_printf ("%u", m_Step);
			xmlNewProp (node, (xmlChar*) "step", (xmlChar*) buf);
			g_free (buf);
	}
	if (m_Line > 1) {
			buf = g_strdup_printf ("%u", m_Line);
			xmlNewProp (node, (xmlChar*) "line", (xmlChar*) buf);
			g_free (buf);
	}
	if (m_Rank > 1) {
			buf = g_strdup_printf ("%u", m_Rank);
			xmlNewProp (node, (xmlChar*) "rank", (xmlChar*) buf);
			g_free (buf);
	}
	return node;
}

bool ReactionProp::Load (xmlNodePtr node)
{
	bool res = Object::Load (node);
	if (GetChildrenNumber () != 1)
		return false;
	std::map < std::string, gcu::Object * >::iterator i;
	m_Object = GetFirstChild (i);
	if (res) {
		char *buf = (char*) xmlGetProp (node, (xmlChar*) "role");
		if (buf) {
			m_Role = RoleFromString (buf);
			xmlFree (buf);
		}
		buf = (char*) xmlGetProp (node, (xmlChar*) "step");
		if (buf) {
			m_Step = strtoul (buf, NULL, 10);
			xmlFree (buf);
		} else
			m_Step = 1;
		buf = (char*) xmlGetProp (node, (xmlChar*) "line");
		if (buf) {
			m_Line = strtoul (buf, NULL, 10);
			xmlFree (buf);
		} else
			m_Line = 1;
		buf = (char*) xmlGetProp (node, (xmlChar*) "rank");
		if (buf) {
			m_Rank = strtoul (buf, NULL, 10);
			xmlFree (buf);
		} else
			m_Rank = 1;
	}
	return res;
}

bool ReactionProp::OnSignal (SignalId Signal, G_GNUC_UNUSED Object *Child)
{
	if (Signal == OnChangedSignal && !HasChildren ())
		delete this;
	return true;
}

std::string ReactionProp::Name ()
{
	return _("Reaction property");
}

double ReactionProp::GetYAlign ()
{
	return m_Object->GetYAlign ();
}

}	//	namespace gcp
