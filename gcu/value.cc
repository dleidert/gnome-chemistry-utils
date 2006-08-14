/* 
 * Gnome Chemistry Utils
 * value.cc 
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

#include "config.h"
#include "value.h"
#include <stdlib.h>

using namespace gcu;

Value::Value ()
{
}

Value::~Value ()
{
}

char const *Value::GetAsString ()
{
	return "";
}

double Value::GetAsDouble ()
{
	return 0.;
}

SimpleValue::SimpleValue (): Value ()
{
}

SimpleValue::~SimpleValue ()
{
}

char const *SimpleValue::GetAsString ()
{
	if (str.length () == 0) {
		char *buf = gcu_value_get_string (&val);
		str = buf;
		g_free (buf);
	}
	return str.c_str ();
}

double SimpleValue::GetAsDouble ()
{
	return val.value;
}

DimensionalValue::DimensionalValue (): Value ()
{
}

DimensionalValue::~DimensionalValue ()
{
}

char const *DimensionalValue::GetAsString ()
{
	if (str.length () == 0) {
		char *buf = gcu_dimensional_value_get_string (&val);
		str = buf;
		g_free (buf);
	}
	return str.c_str ();
}

double DimensionalValue::GetAsDouble ()
{
	return val.value;
}

StringValue::StringValue ()
{
}

StringValue::~StringValue ()
{
}

char const *StringValue::GetAsString ()
{
	return val.c_str ();
}

LocalizedStringValue::LocalizedStringValue ()
{
}

LocalizedStringValue::~LocalizedStringValue ()
{
	vals.clear ();
}

char const *LocalizedStringValue::GetAsString ()
{
	char *lang = getenv ("LANG");
	string s = vals[lang];
	if (s.length () == 0) {
		lang = g_strdup (lang);
		char *dot = strchr (lang, '.');
		if (dot) {
			*dot = 0;
			s = vals[lang];
			if (s.length () > 0) {
				g_free (lang);
				return s.c_str ();
			}
		}
		if (strlen (lang) > 2) {
			lang[2] = 0;
			s = vals[lang];
			if (s.length () > 0) {
				g_free (lang);
				return s.c_str ();
			}
		}
		g_free (lang);
	} else
		return s.c_str ();
	// if we are there, try "C" or "en" locales
	s = vals["C"];
	if (s.length () > 0)
		return s.c_str ();
	s = vals["en"];
	if (s.length () > 0)
		return s.c_str ();
	// if not found, return first occurence or a void string
	if (vals.size () > 0)
		return (*vals.begin ()).second.c_str ();
	return "";
}

char const *LocalizedStringValue::GetLocalizedString (char const *lang)
{
	string s = vals[lang];
	return (s.length () > 0)? s.c_str (): GetAsString ();
}
