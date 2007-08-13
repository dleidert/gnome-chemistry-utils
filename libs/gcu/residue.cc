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
	map<string, Residue*>::iterator i, end = rtbn.end  ();
	for (i = rtbn.begin (); i != end; i++)
		delete (*i).second;
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

}	//	namespace gcu
