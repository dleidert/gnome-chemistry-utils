// -*- C++ -*-

/*
 * GChemPaint library
 * atom-residue.cc
 *
 * Copyright (C) 2014 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "atom-residue.h"
#include "document.h"
#include <cstring>

using namespace gcu;

namespace gcp {

gcu::TypeId ResidueType = gcu::NoType;

AtomResidue::AtomResidue (): Atom (), m_Residue (NULL)
{
	SetZ (-1);
}

AtomResidue::~AtomResidue ()
{
	if (m_Residue)
		const_cast <Residue *> (m_Residue)->Unref ();
}

void AtomResidue::SetResidue (Residue const *res, char const *symbol)
{
	if (m_Residue)
		const_cast <Residue *> (m_Residue)->Unref ();
	if (symbol)
		m_Abbrev = symbol;
	else
		m_Abbrev = (*res->GetSymbols ().begin ()).first;
	m_Residue = res;
	const_cast <Residue *> (m_Residue)->Ref ();
}

char const *AtomResidue::GetSymbol () const
{
	return m_Abbrev.c_str ();
}

xmlNodePtr AtomResidue::Save (xmlDocPtr xml) const
{
	xmlNodePtr node = Atom::Save (xml);
	if (node) {
		xmlNodeSetName (node, (xmlChar const *) "residue");
		static_cast <Document *> (GetDocument ())->SaveResidue (m_Residue, node);
	}
	return node;
}

bool AtomResidue::Load (xmlNodePtr node)
{
	bool result = Atom::Load (node);
	if (!result)
		return false;
	m_Z = -1; // Atom::Load will have set it to 0.
	xmlNodePtr child = node->children;
	if (!child)
		return false;
	char *buf = NULL;
	if (!strcmp ((const char*) child->name, "text"))
		buf = (char*) xmlNodeGetContent (child);
	if (!buf || !strlen (buf))
		return false;
	Residue *residue = (Residue*) Residue::GetResidue (buf, NULL);
	Document *doc = static_cast <Document *> (GetDocument ());
	gcu::Application *app = doc->GetApp ();
	if (residue) {
		if (child->next) {
			Residue *res0 = new Residue (NULL, NULL, NULL, doc);
			res0->Load (node, false, app);
			if (*residue == *(res0->GetMolecule ()))
				delete res0; // OK, same molecule
			else {
				// TODO: append residue to the document
			}
		}
	} else if (child->next) {
		residue = new Residue ();
		residue->Load (node, false, app);
		residue->Register ();
		// TODO: append residue to the local residues ( or to the document?)
	} else
		return false;
	m_Abbrev = buf;
	m_Residue = residue;
	const_cast <Residue *> (m_Residue)->Ref ();
	xmlFree ((xmlChar*) buf);
	return true;
}

}	//	namespace gcp
