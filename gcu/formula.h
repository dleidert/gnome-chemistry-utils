// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * formula.h 
 *
 * Copyright (C) 2005-2006 Jean Bréfort <jean.brefort@normalesup.org>
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
/*!\class parse_error gcu/formula.h
Exception class derived from std::exception used for errors encountered
when parsing a formula.
*/

class parse_error: public exception
{
	string m_msg;
	int m_start, m_length;

public:
/*! Takes a character string describing the error and two integers
* indicating where the error occured.
*/
    explicit 
    parse_error (const string&  __arg, int start, int length);

    virtual 
    ~parse_error () throw ();

/*! Returns a C-style character string describing the general cause of
 *  the current error (the same string passed to the constructor).
*/
    virtual const char* 
    what () const throw ();
/*! Returns a C-style character string describing the general cause of
 *  the current error (the same string passed to the constructor).
*/
    const char* 
    what (int& start, int& length) const throw ();

/*! Adds an offset to the start of the error. This is used by the Formula
class when an exception occurs while parsing a substring.
*/
	void add_offset (int offset) {m_start += offset;}

};

class FormulaElt;

/*!\class Formula gcu/formula.h
This class interprets a chemical formula provided as a string and make
some calculations using it. Currently, it is able to calculate the raw formula,
the molecular weight and the isotopic pattern.
*/
class Formula
{
public:
/*!
@param entry: the formula to parse as a string.
The constructor will emit a parse_error exception.
if it cannot parse the given formula.
*/
	Formula (string entry) throw (parse_error);
	virtual ~Formula ();

/*!
@return the original formula as a pango markup.
*/
	char const *GetMarkup ();
/*!
@return the raw formula as a map of atoms numbers indexed by atomic number Z.
*/
	map<int,int> &GetRawFormula ();
/*!
@return the raw formula as a pango markup.
*/
	char const *GetRawMarkup ();
/*!
@param entry: the formula to parse as a string.
Calls Formula::Clear before parsing the new formula.
The method will emit a parse_error exception
if it cannot parse the given formula.
*/
	void SetFormula (string entry) throw (parse_error);
/*!
Clears all data.
*/
	void Clear ();
/*!
@param prec: will be filled with the precision (number of significative decimal figures).
@param artificial: will be true if the formula contains an artificial element (with
no natural isotope).
@returns the molecular weight corresponding to the formula.
*/
	double GetMolecularWeight (int &prec, bool &artificial);
/*!
@param pattern: the IsotopicPattern to be filled
This method evaluates the isotopic pattern and fills the pattern parameter
with the calculated data.
*/
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
