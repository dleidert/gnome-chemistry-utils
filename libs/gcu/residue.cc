// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * residue.cc 
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
#include "residue.h"
#include <glib.h>
#include <map>
#include <iostream>
#include <sstream>
#include <string>

namespace gcu {

class ResiduesTable {
public:
	ResiduesTable ();
	~ResiduesTable ();

	map<string, Residue*> rtbs; // indexed by symbols
	map<string, Residue*> rtbn; // indexed by name
};

ResiduesTable::ResiduesTable ()
{
}

ResiduesTable::~ResiduesTable ()
{
	while (!rtbn.empty ())
		delete (*rtbn.begin ()).second;
}

static ResiduesTable tbl;

Residue::Residue ():
	m_Name (NULL)
{
}

Residue::Residue (char const *name)
{
	m_Name = g_strdup (name);
	tbl.rtbn[name] = this;
}

Residue::~Residue ()
{
	if (m_Name)
		tbl.rtbn.erase (m_Name);
	std::set<std::string>::iterator i, end = m_Symbols.end ();
	for (i = m_Symbols.begin (); i != end ; i++)
		tbl.rtbs.erase (*i);
	g_free (const_cast<char*> (m_Name));
}

void Residue::SetName (char const *name)
{
	if (m_Name) {
		tbl.rtbn.erase (m_Name);
		g_free (const_cast<char*> (m_Name));
	}
	m_Name = g_strdup (name);
	tbl.rtbn[name] = this;
}

void Residue::AddSymbol (char const *symbol)
{
	m_Symbols.insert (symbol);
	tbl.rtbs[symbol] = this;
}

void Residue::RemoveSymbol (char const *symbol)
{
	m_Symbols.erase (symbol);
	tbl.rtbs.erase (symbol);
}

void Residue::Load (xmlNodePtr node)
{
	static char *lang = getenv ("LANG");
	if (m_Name)
		return;
	char *generic = reinterpret_cast <char *> (xmlGetProp (node, reinterpret_cast <xmlChar const *> ("generic"))); 
	if (generic) {
		if (!strcmp (generic, "true"))
			m_Generic = true;
		xmlFree (generic);
	}
	xmlNodePtr child = node->children;
	char *name = NULL, *node_lang, *symbols;
	bool lang_matched = false;
	while (child) {
		if (!strcmp (reinterpret_cast <char const *> (child->name), "name")) {
			node_lang = (char*) xmlNodeGetLang (child);
			if (node_lang) {
				if (lang) {
					if (!strcmp (lang, node_lang) || (!lang_matched && !strncmp (lang, node_lang, 2))) {
						if (name)
							xmlFree (name);
						name = reinterpret_cast <char *> (xmlNodeGetContent (child));
						lang_matched = true;
					}
				}
				xmlFree (node_lang);
			} else if (!lang_matched) {
				if (name)
					xmlFree (name);
				name = reinterpret_cast <char *> (xmlNodeGetContent (child));
			}
		} else if (!strcmp (reinterpret_cast <char const *> (child->name), "symbols")) {
			symbols = reinterpret_cast <char *> (xmlNodeGetContent (child));
			std::istringstream s(symbols);
			char buf[10];
			while (!s.eof ()) {
				s.getline(buf, 10, ',');
				if (strlen (buf) > 8) {
					// Symbols longer than 8 chars are not currently allowed
					delete this;
					return;
				} else {
					if (GetResidue (buf) != NULL) {
						cerr << "A residue with symbol \"" << buf << "\" already exists" << endl;
						delete this;
						return;
					}
					AddSymbol (buf);
				}
			}
			xmlFree (symbols);
		}
		child = child->next;
	}
	if (name) {
		if (GetResiduebyName (name) != NULL) {
			cerr << "A residue named \"" << name << "\" already exists" << endl;
			delete this;
			return;
		}
		SetName (name);
		xmlFree (name);
	} else
		delete this;
}

Residue const *Residue::GetResidue (char const *symbol)
{
	map<string, Residue*>::iterator i = tbl.rtbs.find (symbol);
	return (i != tbl.rtbs.end ())? (*i).second: NULL;
}

Residue const *Residue::GetResiduebyName (char const *name)
{
	map<string, Residue*>::iterator i = tbl.rtbn.find (name);
	return (i != tbl.rtbn.end ())?  (*i).second: NULL;
}

string const *Residue::GetFirstResidueSymbol (ResidueIterator &i)
{
	i = tbl.rtbs.begin ();
	return (i == tbl.rtbs.end ())? NULL: &(*i).first;
}

string const *Residue::GetNextResidueSymbol (ResidueIterator &i)
{
	i++;
	return (i == tbl.rtbs.end ())? NULL: &(*i).first;
}

}	//	namespace gcu
