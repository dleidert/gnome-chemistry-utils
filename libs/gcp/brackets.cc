// -*- C++ -*-

/* 
 * GChemPaint library
 * brackets.cc
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

#include "config.h"
#include "brackets.h"
#include <gcu/document.h>
#include <sstream>
#include <cstring>

namespace gcp {

gcu::TypeId BracketsType = gcu::NoType;
static gcu::Object *last_loaded;

Brackets::Brackets (BracketsTypes type): gcu::Object (BracketsType), ItemClient ()
{
	m_Type = type;
}

Brackets::~Brackets ()
{
}

void Brackets::OnLoaded ()
{
	if (last_loaded) {
		// this is NOT thread safe
		m_EmbeddedObjects.insert (last_loaded);
		last_loaded = NULL;
	}
}

bool Brackets::Load (xmlNodePtr node)
{
	char *buf;
	gcu::Document *doc = GetDocument ();
	buf = reinterpret_cast <char *> (xmlGetProp (node, (xmlChar*) "type"));
	if (!buf)
		m_Type = BracketsTypeNormal;
	else if (!strcmp (buf, "square"))
		m_Type = BracketsTypeSquare;
	else if (!strcmp (buf, "curly"))
		m_Type = BracketsTypeCurly;
	else
		m_Type = BracketsTypeNormal;
	if (buf)
		xmlFree (buf);
	buf = reinterpret_cast <char *> (xmlGetProp (node, (xmlChar*) "objects"));
	if (buf) {
		char **ids = g_strsplit (buf, ",", -1);
		unsigned i = 0;
		while (ids[i])
			doc->SetTarget (ids[i++], &last_loaded, doc, this);
		g_strfreev (ids);
		xmlFree (buf);
	}
	return true;
}

xmlNodePtr Brackets::Save (xmlDocPtr xml) const
{
	if (m_EmbeddedObjects.size () == 0)
		return NULL;
	xmlNodePtr node = xmlNewDocNode (xml, NULL, (xmlChar*) "brackets", NULL);
	SaveId (node);
	char const *type = NULL;
	switch (m_Type) {
	case BracketsTypeNormal:
	default:
		break;
	case BracketsTypeSquare:
		type = "square";
		break;
	case BracketsTypeCurly:
		type = "curly";
		break;
	}
	if (type)
		xmlNewProp (node, reinterpret_cast <xmlChar const *> ("type"), reinterpret_cast <xmlChar const *> (type));
	// now save embedded objects as a list of Ids
	std::set <gcu::Object *>::iterator i, end = m_EmbeddedObjects.end ();
	i = m_EmbeddedObjects.begin ();
	std::ostringstream str;
	str << (*i)->GetId ();
	for (i++ ; i != end; i++)
		str << "," << (*i)->GetId ();
	xmlNewProp (node, reinterpret_cast <xmlChar const *> ("objects"), reinterpret_cast <xmlChar const *> (str.str ().c_str ()));
	return node;
}

void Brackets::SetSelected (int state)
{
}

void Brackets::SetEmbeddedObjects (std::set <gcu::Object *> objects)
{
	m_EmbeddedObjects = objects;
}

}
