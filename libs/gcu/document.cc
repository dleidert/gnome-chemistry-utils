// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * libs/gcu/document.cc
 *
 * Copyright (C) 2004-2007 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "document.h"
#include "application.h"
#include "dialog.h"

using namespace gcu;

Document::Document (Application *App): Object (DocumentType),
m_Dirty (false),
m_Empty (true)
{
	m_App = App;
	if (m_App)
		m_App->AddDocument (this);
}

Document::~Document ()
{
	if (m_App)
		m_App->RemoveDocument (this);
}

gchar* Document::GetNewId (gchar* id, bool Cache)
{
	gchar *Id = g_strdup (id);
	int i = 0;
	while ((Id[i] < '0') || (Id[i] > '9'))
		i++;
	gchar *buf = new gchar[i + 16];
	strncpy (buf, Id, i);
	buf[i] = 0;
	g_free (Id);
	int j = 1;
	string s = m_TranslationTable[buf];
	if (s.size ())
		j = atoi (s.c_str ());
	char* key = g_strdup (buf);
	while (snprintf (buf + i, 16, "%d", j++), GetDescendant (buf) != NULL);
	Id = g_strdup_printf ("%d", j);
	if (Cache) {
		m_TranslationTable[key] = Id;
		m_TranslationTable[id] = buf;
	}
	g_free (Id);
	g_free (key);
	return buf;
}
