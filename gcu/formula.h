// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * formula.h 
 *
 * Copyright (C) 2005
 *
 * Developed by Jean Bréfort <jean.brefort@normalesup.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA  02111-1307, USA.
 */

#ifndef GCU_FORMULA_H
#define GCU_FORMULA_H

#include <string>
#include <map>
#include <list>
#include <stdexcept>
#include "isotope.h"

using namespace std;

namespace gcu
{

class parse_error: public exception
{
	string m_msg;
	int m_start, m_length;

public:
	/** Takes a character string describing the error and two integers
	* indicating where the error occured. */
    explicit 
    parse_error (const string&  __arg, int start, int length);

    virtual 
    ~parse_error () throw ();

    /** Returns a C-style character string describing the general cause of
     *  the current error (the same string passed to the ctor).  */
    virtual const char* 
    what () const throw ();
	/** Returns a C-style character string describing the general cause of
     *  the current error (the same string passed to the ctor).  */
    const char* 
    what (int& start, int& length) const throw ();

	void add_offset (int offset) {m_start += offset;}

};

class FormulaElt;

class Formula
{
public:
	Formula (string entry) throw (parse_error);
	virtual ~Formula ();

	char const *GetMarkup ();
	map<int,int> &GetRawFormula ();
	char const *GetRawMarkup ();
	void SetFormula (string entry) throw (parse_error);
	void Clear ();
	double GetMolecularWeight (int &prec, bool &artificial);
	void CalculateIsotopicPattern (IsotopicPattern &pattern);

private:
	void Parse (string &formula, list<FormulaElt *>&result) throw (parse_error);

private:
	string Entry, Markup, RawMarkup;
	map<int,int> Raw;
	list<FormulaElt *> Details;
	double m_Weight;
	int m_WeightPrec;
	bool m_WeightCached;
	bool m_Artificial;
};
	
}

#endif // GCU_FORMULA_H
