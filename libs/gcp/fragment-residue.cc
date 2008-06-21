// -*- C++ -*-

/* 
 * GChemPaint library
 * fragment-residue.cc 
 *
 * Copyright (C) 2008 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "fragment-residue.h"
#include "document.h"
#include "fragment.h"
#include <cstring>

using namespace gcu;

namespace gcp {

FragmentResidue::FragmentResidue (): FragmentAtom (), m_Residue (NULL)
{
	SetZ (-1);
}

FragmentResidue::FragmentResidue (Fragment *fragment, char const *symbol): FragmentAtom (fragment, -1)
{
	if (symbol) {
		m_Abbrev = symbol;
		m_Residue = dynamic_cast <Residue const*> (Residue::GetResidue (symbol, NULL));
	}
}

FragmentResidue::~FragmentResidue () {}


void FragmentResidue::SetResidue (Residue const *res)
{
	m_Residue = res;
}

const gchar* FragmentResidue::GetSymbol () const
{
	return m_Abbrev.c_str ();
}

xmlNodePtr FragmentResidue::Save (xmlDocPtr xml) const
{
	xmlNodePtr node = FragmentAtom::Save (xml);
	if (node) {
		xmlNodeSetName (node, (xmlChar const *) "residue");
		static_cast <Document const*> (GetFragment ()->GetDocument ())->SaveResidue (m_Residue, node);
	}
	return node;
}

bool FragmentResidue::Load (xmlNodePtr node)
{
	bool result = FragmentAtom::Load (node);
	if (!result)
		return false;
	m_Z = -1; // FragmentAtom::Load will have set it to 0.
	xmlNodePtr child = node->children;
	if (!child)
		return false;
	char *buf = NULL;
	if (!strcmp ((const char*) child->name, "text"))
		buf = (char*) xmlNodeGetContent (child);
	if (!buf || !strlen (buf))
		return false;
	Residue *residue = (Residue*) Residue::GetResidue (buf, NULL);
	if (residue) {
		if (child->next) {
		}
		// TODO: compare molecules
		m_Abbrev = buf;
		m_Residue = residue;
	} else {
		// TODO: append residue to the local residues ( or to the document?)
	}
	xmlFree ((xmlChar*) buf);
	return true;
}

}	//	namespace gcp
