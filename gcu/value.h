/* 
 * Gnome Chemistry Utils
 * value.h 
 *
 * Copyright (C) 2002-2006 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCU_VALUE_H
#define GCU_VALUE_H

#include "chemistry.h"
#include <string>
#include <map>

using namespace std;

namespace gcu
{

class Value
{
public:
	Value ();
	virtual ~Value ();

	virtual char const *GetAsString ();
	virtual double GetAsDouble ();
};

class SimpleValue: public Value
{
friend class Element;

public:
	SimpleValue ();
	virtual ~SimpleValue ();

	char const *GetAsString ();
	double GetAsDouble ();
	GcuValue const GetValue () {return val;}

private:
	GcuValue val;
	string str;
};

class DimensionalValue: public Value
{
friend class Element;

public:
	DimensionalValue ();
	virtual ~DimensionalValue ();

	char const *GetAsString ();
	double GetAsDouble ();
	GcuDimensionalValue const GetValue () {return val;}

private:
	GcuDimensionalValue val;
	 string str;
};

class StringValue: public Value
{
friend class Element;

public:
	StringValue ();
	virtual ~StringValue ();

	char const *GetAsString ();

private:
	string val;
};

class LocalizedStringValue: public Value
{
friend class Element;

public:
	LocalizedStringValue ();
	virtual ~LocalizedStringValue ();

	char const *GetAsString ();
	char const *GetLocalizedString (char const *lang);

private:
	map <string, string> vals;
};

}	// namespace gcu

#endif	//	GCU_VALUE_H
