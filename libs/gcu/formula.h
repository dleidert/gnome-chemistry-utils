// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * formula.h 
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

#ifndef GCU_FORMULA_H
#define GCU_FORMULA_H

#include <string>
#include <map>
#include <list>
#include <stdexcept>
#include "isotope.h"
#include "macros.h"
#include "value.h"

namespace gcu
{

typedef enum {
	GCU_FORMULA_PARSE_GUESS,
	GCU_FORMULA_PARSE_ATOM,
	GCU_FORMULA_PARSE_RESIDUE,
	GCU_FORMULA_PARSE_ASK,
} FormulaParseMode;

/*!\class parse_error gcu/formula.h
Exception class derived from std::exception used for errors encountered
when parsing a formula.
*/

class parse_error: public std::exception
{
public:
/*! Takes a character string describing the error and two integers
* indicating where the error occured.
*/
    explicit 
    parse_error (const std::string&  __arg, int start, int length);

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

private:
	std::string m_msg;
	int m_start, m_length;

};

class FormulaElt
{
public:
	FormulaElt ();
	virtual ~FormulaElt ();
	virtual std::string Markup ();
	virtual void BuildRawFormula (std::map<int, int> &raw) = 0;
	virtual int GetValence () = 0;
	int stoich;
};

class FormulaAtom: public FormulaElt
{
public:
	FormulaAtom (int Z);
	virtual ~FormulaAtom ();
	std::string Markup ();
	void BuildRawFormula (std::map<int, int> &raw);
	int GetValence ();
	int elt;
};

class FormulaBlock: public FormulaElt
{
public:
	FormulaBlock ();
	virtual ~FormulaBlock ();
	std::string Markup ();
	void BuildRawFormula (std::map<int, int> &raw);
	std::list<FormulaElt *> children;
	int GetValence ();
	int parenthesis;
};

class Residue;

class FormulaResidue: public FormulaElt
{
public:
	FormulaResidue (Residue const *res, char const *symbol, int Z);
	virtual ~FormulaResidue ();
	std::string Markup ();
	void BuildRawFormula (std::map<int, int> &raw);
	int GetValence ();
	Residue const *residue;
	std::string Symbol;
GCU_RO_PROP (int, Z);
};


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
	Formula (std::string entry, FormulaParseMode mode = GCU_FORMULA_PARSE_GUESS) throw (parse_error);
	virtual ~Formula ();

/*!
@return the original formula as a pango markup.
*/
	char const *GetMarkup ();
/*!
@return the raw formula as a map of atoms numbers indexed by atomic number Z.
*/
	std::map<int,int> &GetRawFormula ();
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
	void SetFormula (std::string entry) throw (parse_error);
/*!
Clears all data.
*/
	void Clear ();
/*!
@param artificial: will be true if the formula contains an artificial element (with
no natural isotope).
@returns the molecular weight corresponding to the formula.
*/
	DimensionalValue GetMolecularWeight (bool &artificial);
/*!
@param pattern: the IsotopicPattern to be filled
This method evaluates the isotopic pattern and fills the pattern parameter
with the calculated data.
*/
	void CalculateIsotopicPattern (IsotopicPattern &pattern);

	bool BuildConnectivity ();
	std::list<FormulaElt *> const &GetElements () {return Details;}

private:
	void Parse (std::string &formula, std::list<FormulaElt *>&result) throw (parse_error);

private:
	std::string Entry, Markup, RawMarkup;
	std::map<int,int> Raw;
	std::list<FormulaElt *> Details;
	DimensionalValue m_Weight;
	bool m_WeightCached;
	bool m_Artificial;
	bool m_ConnectivityCached;
GCU_PROP (FormulaParseMode, ParseMode);
};
	
}

#endif // GCU_FORMULA_H
