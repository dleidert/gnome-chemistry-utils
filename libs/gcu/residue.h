// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * residue.h 
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

#ifndef GCU_RESIDUE_H
#define GCU_RESIDUE_H

#include "macros.h"
#include <libxml/parser.h>
#include <map>
#include <set>
#include <string>

using namespace std;

namespace gcu {

class Residue;


typedef struct {
	Residue *res;
	bool ambiguous;
} SymbolResidue;

typedef std::map<std::string, SymbolResidue>::iterator ResidueIterator;

class Residue
{
public:
	Residue ();
	Residue (char const *name);
	virtual ~Residue ();

	std::map<int,int> const &GetRawFormula () const {return m_Raw;}
	std::map<std::string, bool> const &GetSymbols () const {return m_Symbols;}
	void SetName (char const *name);
	void AddSymbol (char const *symbol);
	void RemoveSymbol (char const *symbol);
	virtual void Load (xmlNodePtr node);
	static Residue const *GetResidue (char const *symbol, bool *ambiguous = NULL);
	static Residue const *GetResiduebyName (char const *name);
	static std::string const *GetFirstResidueSymbol (ResidueIterator &i);
	static std::string const *GetNextResidueSymbol (ResidueIterator &i);

public:
	static unsigned MaxSymbolLength;

private:
	std::map<int,int> m_Raw;
	std::map<std::string, bool> m_Symbols; // boolean is true if the symbol is ambiguous

GCU_RO_PROP (char const *, Name)
GCU_PROP (bool, Generic)
};

}	//	namespace gcu

#endif	//	GCU_RESIDUE_H
