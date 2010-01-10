// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * libs/gcu/document.cc
 *
 * Copyright (C) 2004-2010 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "residue.h"
#include "dialog.h"
#include <glib/gi18n-lib.h>
#include <cstring>
#include <sstream>

using namespace std;

namespace gcu
{

Document::Document (Application *App): Object (DocumentType),
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
	if (m_PendingTable.size () > 0) {
		std::map <std::string, list <PendingTarget> >::iterator it, end = m_PendingTable.end ();
		if ((it = m_PendingTable.find (id)) != end) {
			m_PendingTable[buf] = (*it).second;
			m_PendingTable.erase (it);
		}
	}
	return buf;
}

Residue *Document::CreateResidue (char const *name, char const *symbol, Molecule *molecule)
{
	return NULL;
}

Residue const *Document::GetResidue (char const *symbol, bool *ambiguous)
{
	return Residue::GetResidue (symbol, ambiguous);
}

bool Document::SetTarget (char const *id, Object **target, Object *parent, Object *owner) throw (std::runtime_error)
{
	if (target == NULL)
	    throw std::runtime_error ("Can't set a NULL target.");
	*target = parent->GetDescendant (id);
	if (*target)
		return true;
	PendingTarget pt;
	pt.target = target;
	pt.parent = parent;
	pt.owner = owner;
	m_PendingTable[id].push_back (pt);
	return false;
}

bool Document::Loaded () throw (LoaderError)
{
	unsigned count = 0;
	std::map <std::string, list <PendingTarget> >::iterator i, end = m_PendingTable.end ();
	for (i = m_PendingTable.begin (); i != end; i++) {
		std::string id = (*i).first;
		std::list <PendingTarget> &l = (*i).second;
		std::list <PendingTarget>::iterator j = l.begin (), jend = l.end ();
		Object *obj = (*j).parent->GetDescendant (id.c_str ());
		if (obj == NULL) {
			m_PendingTable.clear ();
			std::ostringstream str;
			// Note to translators: the two strings are concatenated with the missing id between them.
			str << _("The input contains a reference to object \"") << id << _("\" but no object with this Id is described.");
			throw LoaderError (str.str ());
		} else while (j != jend) {
			*(*j).target = obj;
			if ((*j).owner)
				(*j).owner->OnLoaded ();
			count++;
			j++;
		}
	}
	m_PendingTable.clear ();
	return count > 0;
}

}	//	namespace gcu
