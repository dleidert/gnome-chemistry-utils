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

using namespace std;

namespace gcu
{

class FormulaElt;

class Formula
{
public:
	Formula (string entry);
	virtual ~Formula ();

	char const *GetMarkup ();
	map<int,int> &GetRawFormula ();
	char const *GetRawMarkup ();
	void SetFormula (string entry);

private:
	void Parse ();

private:
	string Entry, Markup, RawMarkup;
	map<int,int> Raw;
	list<FormulaElt *> Details;
};
	
}

#endif // GCU_FORMULA_H
