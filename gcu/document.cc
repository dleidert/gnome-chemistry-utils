// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * chemistry/document.cc
 *
 * Copyright (C) 2004
 *
 * Developed by Jean Bréfort <jean.brefort@ac-dijon.fr>
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

#include "document.h"

using namespace gcu;

Document::Document (): Object (DocumentType)
{
}

Document::~Document ()
{
}

gchar* Document::GetNewId (gchar* id)
{
	gchar *Id = g_strdup(id);
	int i = 0;
	while ((Id[i] < '0') || (Id[i] > '9')) i++;
	gchar *buf = new gchar[i + 16];
	strncpy(buf, Id, i);
	g_free(Id);
	int j = 1;
	string s = m_TranslationTable[buf];
	if (s.size()) j = atoi(s.c_str());
	char* key = g_strdup (buf);
	while (snprintf(buf + i, 16, "%d", j++), GetDescendant(buf) != NULL);
	Id = g_strdup_printf ("%d", j);
	m_TranslationTable[key] = Id;
	g_free (Id);
	g_free (key);
	m_TranslationTable[id] = buf;
	return buf;
}
