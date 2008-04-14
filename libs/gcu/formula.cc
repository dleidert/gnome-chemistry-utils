// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * formula.cc 
 *
 * Copyright (C) 2005-2008 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "formula.h"
#include "element.h"
#include "residue.h"
#include <glib/gi18n.h>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <sstream>

using namespace std;

namespace gcu
{

parse_error::parse_error(const string& __arg, int start, int length)
  : exception(), m_msg(__arg)
{
	m_start = start;
	m_length = length;
}

parse_error::~parse_error() throw() { }

const char*
parse_error::what() const throw()
{ return m_msg.c_str(); }

const char*
parse_error::what(int &start, int &length) const throw()
{
	start = m_start;
	length = m_length;
	return m_msg.c_str();
}
	
FormulaElt::FormulaElt ()
{
	stoich = 1;
}

FormulaElt::~FormulaElt ()
{
}

string FormulaElt::Markup ()
{
	ostringstream oss;
	if (stoich > 1)
		oss << "<sub>" << stoich << "</sub>";
	return oss.str ();
}

FormulaAtom::FormulaAtom (int Z): FormulaElt()
{
	elt = Z;
}

FormulaAtom::~FormulaAtom ()
{
}

string FormulaAtom::Markup ()
{
	string s = Element::Symbol (elt);
	s += FormulaElt::Markup ();
	return s;
}

int FormulaAtom::GetValence ()
{
	return Element::GetElement (elt)->GetDefaultValence ();
}

void FormulaAtom::BuildRawFormula (map<int, int> &raw)
{
	raw[elt] += stoich;
}

FormulaBlock::FormulaBlock (): FormulaElt()
{
}

FormulaBlock::~FormulaBlock ()
{
	list<FormulaElt *>::iterator i, end = children.end();
	for (i = children.begin (); i != end; i++)
		delete *i;
}

string FormulaBlock::Markup ()
{
	ostringstream oss;
	switch (parenthesis) {
		case 0:
			oss << "(";
			break;
		case 1:
			oss << "[";
			break;
		case 2:
			oss << "{";
			break;
	}
	list<FormulaElt *>::iterator i, end = children.end();
	for (i = children.begin (); i != end; i++) {
		oss << (*i)->Markup ();
	}
	switch (parenthesis) {
		case 0:
			oss << ")";
			break;
		case 1:
			oss << "]";
			break;
		case 2:
			oss << "}";
			break;
	}
	oss << FormulaElt::Markup ();
	return oss.str ();
}

void FormulaBlock::BuildRawFormula (map<int, int> &raw)
{
	map<int, int> local;
	list<FormulaElt *>::iterator i, iend = children.end();
	for (i = children.begin (); i != iend; i++)
		(*i)->BuildRawFormula (local);
	map<int, int>::iterator j, jend = local.end();
	for (j = local.begin (); j != jend; j++){
		raw[(*j).first] += stoich * (*j).second;}
}

int FormulaBlock::GetValence ()
{
	return -1; // FIXME !!!
}

FormulaResidue::FormulaResidue (Residue const *res, char const *symbol, int Z): FormulaElt()
{
	residue = res;
	Symbol = symbol;
	m_Z = Z;
}
	
FormulaResidue::~FormulaResidue ()
{
}

string FormulaResidue::Markup ()
{
	size_t n = Symbol.find ('-');
	if (n != string::npos) {
		string s = string ("<i>") + string (Symbol, 0, n) + "</i>" + string (Symbol, n);
		return s;
	} else
		return Symbol;
}

void FormulaResidue::BuildRawFormula (map<int, int> &raw)
{
	std::map<int,int> const &m = residue->GetRawFormula ();
	map<int, int>::const_iterator j, jend = m.end();
	for (j = m.begin (); j != jend; j++){
		raw[(*j).first] += stoich * (*j).second;}
}

int FormulaResidue::GetValence ()
{
	return 1; // residues with other valences are not currently supported
}

static bool AnalString (char *sz, list<FormulaElt *> &result, bool &ambiguous)
{
	if (*sz == 0)
		return true;
	unsigned i = 0;
	char sy[Residue::MaxSymbolLength + 1];
	Residue const *r = NULL;
	bool amb = ambiguous, local_amb;
	if (*sz) {
		// search for any abbreviation starting sz
		strncpy (sy, sz, Residue::MaxSymbolLength);
		i = strlen (sz);
		if (i > Residue::MaxSymbolLength)
			i = Residue::MaxSymbolLength;
		while (i > 0) {
			sy[i] = 0;
			r = Residue::GetResidue (sy, &local_amb);
			if (r)
				break;
			i--;
		}
		if (r) {
			result.push_back (new FormulaResidue (r, sy, (local_amb? Element::Z (sy): 0)));
			ambiguous |= local_amb;
			if (AnalString (sz + i, result, ambiguous))
				return true;
			ambiguous = amb; // restore ambiguity state
			delete result.back ();
			result.pop_back ();
		}
		if (islower (*sz)) {
			/* we might have some abbreviation around there */
		}
		*sz = toupper (*sz);
		if (strlen (sz) == 1) {
			i = Element::Z (sz);
			if (i > 0) {
				result.push_back (new FormulaAtom (i));
				return true;
			} else
				return false;
		}
		if (isupper (sz[1])) {
			sy [0] = *sz;
			sy [1] = 0;
			i = Element::Z (sy);
			if (i > 0) {
				result.push_back (new FormulaAtom (i));
				if (AnalString (sz + 1, result, ambiguous))
					return true;
				delete result.back ();
				result.pop_back ();
			}
			sy[1] = tolower (sz[1]);
			sy[2] = 0;
			i = Element::Z (sy);
			if (i > 0) {
				result.push_back (new FormulaAtom (i));
				if (AnalString (sz + 2, result, ambiguous))
					return true;
				delete result.back ();
				result.pop_back ();
			}
			if (*sz != 'U')
				return false;
			sy[2] = tolower (sz[2]);
			sy[3] = 0;
			i = Element::Z (sy);
			if (i > 0) {
				result.push_back (new FormulaAtom (i));
				if (AnalString (sz + 3, result, ambiguous))
					return true;
			}
			return false;
		} else {
			sy[0] = sz[0];	
			sy[1] = sz[1];	
			if (*sz == 'U') {
				// No 2 chars symbols begining with U exist, so try 3 chars symbols
				sy[2] = tolower (sz[2]);
				sy[3] = 0;
				i = Element::Z (sy);
				if (i > 0) {
					result.push_back (new FormulaAtom (i));
					if (AnalString (sz + 3, result, ambiguous))
						return true;
					delete result.back ();
					result.pop_back ();
				}
			}
			sy[2] = 0;
			i = Element::Z (sy);
			if (i > 0) {
				result.push_back (new FormulaAtom (i));
				if (AnalString (sz + 2, result, ambiguous))
					return true;
				delete result.back ();
				result.pop_back ();
			}
			sy[1] = 0;	
			i = Element::Z (sy);
			if (i > 0) {
				result.push_back (new FormulaAtom (i));
				if (AnalString (sz + 1, result, ambiguous))
					return true;
			}
		}
	}
	return false;
}

Formula::Formula (string entry, FormulaParseMode mode) throw (parse_error)
{
	Entry = entry;
	m_ParseMode = mode;
	Parse (Entry, Details);
	m_ConnectivityCached = m_WeightCached = false;
}

Formula::~Formula ()
{
	Clear ();
}

char const *Formula::GetMarkup ()
{
	return Markup.c_str ();
}

map<int,int> &Formula::GetRawFormula ()
{
	return Raw;
}

char const *Formula::GetRawMarkup ()
{
	return RawMarkup.c_str ();
}

void Formula::SetFormula (string entry) throw (parse_error)
{
	Entry = entry;
	Clear ();
	Parse (Entry, Details);
	list<FormulaElt *>::iterator i, iend = Details.end();
	// now update markups and raw formula
	for (i = Details.begin (); i != iend; i++) {
		Markup += (*i)->Markup ();
		(*i)->BuildRawFormula (Raw);
	}
	ostringstream oss;
	map<string, int> elts;
	int nC = 0, nH = 0;
	map<int, int>::iterator j, jend = Raw.end();
	for (j = Raw.begin (); j != jend; j++) {
		switch ((*j).first) {
		case 1:
			nH = (*j).second;
			break;
		case 6:
			nC = (*j).second;
			break;
		default:
			elts[Element::Symbol((*j).first)] = (*j).second;
			break;
		}
	}
	if (nC > 0) {
		oss << "C";
		if (nC > 1)
			oss << "<sub>" << nC << "</sub>";
	}
	if (nH > 0) {
		oss << "H";
		if (nH > 1)
			oss << "<sub>" << nH << "</sub>";
	}
	map<string, int>::iterator k, kend = elts.end ();
	for (k = elts.begin (); k != kend; k++) {
		nC = (*k).second;
		oss << (*k).first;
		if (nC > 1)
			oss << "<sub>" << nC << "</sub>";
	}
	RawMarkup = oss.str ();
}

void Formula::Clear ()
{
	list<FormulaElt *>::iterator i, end = Details.end();
	for (i = Details.begin (); i != end; i++)
		delete *i;
	Details.clear ();
	Markup = "";
	Raw.clear ();
	RawMarkup = "";
	m_ConnectivityCached = m_WeightCached = false;
}

void Formula::Parse (string &formula, list<FormulaElt *> &result) throw (parse_error)
{
	int i = 0, npo, size = formula.size (), j, k = 0; // parsing index, number of open parenthesis, string size
	char c = 0, *sz, *end;
	bool ambiguous = false;
	while (i < size) {
		if (formula[i] == '(' || formula[i] == '[' || formula[i] == '{') {
			switch (formula[i]) {
				case '(':
					c = ')';
					k = 0;
					break;
				case '[':
					c = ']';
					k = 1;
					break;
				case '{':
					c = '}';
					k = 2;
					break;
			}
			npo = 1;
			j = i + 1;
			while (j < size && npo > 0) {
				if (formula[j] == '(' || formula[j] == '[' || formula[j] == '{')
					npo++;
				else if (formula[j] == ')' || formula[j] == ']' || formula[j] == '}')
					npo--;
				j++;
			}
			if (npo || formula[j - 1] != c)
				throw parse_error (_("Unmatched parenthesis"), i, 1);
			string str (formula, i + 1, j - i - 2);
			FormulaBlock *block = new FormulaBlock ();
			block->parenthesis = k;
			result.push_back (block);
			try {
				Parse (str, block->children);
			}
			catch (parse_error &error) {
				error.add_offset (i + 1);
				throw error;
			}
			block->stoich = strtol (formula.c_str () + j, &end, 10);
			i = end - formula.c_str ();
			if (i == j)
				block->stoich = 1;
		} else if (isalpha (formula[i]) || formula[i] == '-') {
			j = i + 1;
			while (isalpha (formula[j]) || formula[j] == '-')
				j++;
			k = j - i;
			sz = new char[k + 1];
			strncpy (sz, formula.c_str () + i, k);
			sz[k] = 0;
			if (!AnalString (sz, result, ambiguous)) {
				delete [] sz;
				throw parse_error (_("Could not interpret the symbol list"), i, k);
			}
			delete [] sz;
			i = j;
			FormulaElt *elt = result.back ();
			if (!elt)
				throw runtime_error (_("Parser failed, please fill a bug report."));
			elt->stoich = strtol (formula.c_str () + j, &end, 10);
			i = end - formula.c_str ();
			if (i == j)
				elt->stoich = 1;
		} else if (formula[i] == ')' || formula[i] == ']' || formula[i] == '}') {
			throw parse_error (_("Unmatched parenthesis"), i, 1);
		} else
			throw parse_error (_("Invalid character"), i, 1);
	}
	if (ambiguous) {
		int replaced = 1, max = 0;
		// first count ambiguous symbols
		list<FormulaElt *>::iterator it, end = result.end ();
		for (it = result.begin (); it != end; it++) {
			if (dynamic_cast <FormulaResidue *> (*it) != NULL)
				max++;
		}
		if (!BuildConnectivity ()) {
			// for now just replace all ambiguous residues
			it = result.begin ();
			FormulaResidue *res;
			while (it != result.end ()) {
				res = dynamic_cast <FormulaResidue *> (*it);
				if (res && res->GetZ ()) {
					printf("found ambiguous with Z=%d\n",res->GetZ());
					FormulaAtom *elt = new FormulaAtom (res->GetZ());
					elt->stoich =  res->stoich;
					it = result.erase (it);
					delete res;
					it = result.insert (it, elt);
				} else
					it++;
			}
		}
	}
}

DimensionalValue Formula::GetMolecularWeight (bool &artificial)
{
	if (Raw.size () == 0) {
		return m_Weight;
	}
	if (!m_WeightCached) {
		DimensionalValue atom_weight;
		m_Artificial = false; // most formula don't have artificial elements
		map<int,int>::iterator i, end = Raw.end (), begin = Raw.begin ();
		for (i = begin; i != end; i++) {
			atom_weight = *Element::GetElement ((*i).first)->GetWeight ();
			if (atom_weight.GetValue ().prec == 0)
				m_Artificial = true;
			m_Weight = (i == begin)? atom_weight * (*i).second: m_Weight + atom_weight * (*i).second;
		}
	}
	m_WeightCached = true;
	artificial = m_Artificial;
	return m_Weight;
}

void Formula::CalculateIsotopicPattern (IsotopicPattern &pattern)
{
	map<int,int>::iterator i, end = Raw.end ();
	i = Raw.begin ();
	if (i == end) // empty formula
		return;
	IsotopicPattern *pat, *pat0;
	pat = Element::GetElement ((*i).first)->GetIsotopicPattern ((*i).second);
	pattern.Copy (*pat);
	pat->Unref ();
	for (i++; i != end; i++) {
		pat = Element::GetElement ((*i).first)->GetIsotopicPattern ((*i).second);
		if (!pat) {
			// no stable isotope known for the element
			pattern.Clear ();
			return;
		}
		pat0 = pattern.Multiply (*pat);
		pat->Unref ();
		pat = pat0->Simplify ();
		pattern.Copy (*pat);
		pat0->Unref ();
		pat->Unref ();
	}
}

bool Formula::BuildConnectivity ()
{
	// FIXME: write this function
	return false;
}

}	//	namespace gcu
