// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * formula.cc 
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

#include "formula.h"

using namespace gcu;

class FormulaElt
{
	string Markup () = 0;
};

Formula::Formula (string entry)
{
	Entry = entry;
	Parse ();
}

Formula::~Formula ()
{
}

char const *Formula::GetMarkup ()
{
	return Markup->c_str ();
}

map<int,int> &Formula::GetRawFormula ()
{
	return Raw->c_str ();
}

char const *Formula::GetRawMarkup ()
{
	return RawMarkup->c_str ();
}

void Formula::SetFormula (string entry)
{
	Entry = entry;
	Parse ();
}

void Formula::Parse ()
{
}
