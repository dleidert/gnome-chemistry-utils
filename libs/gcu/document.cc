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

char* Document::GetNewId (char const *id, bool Cache)
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
	while (snprintf (buf + i, 16, "%d", j++), GetDescendant (buf) != NULL) ;
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
			// Hmm, this might be unsecure if several imported objects have the same Id
			m_PendingTable[buf] = (*it).second;
			m_PendingTable.erase (it);
		}
	}
	return buf;
}

Residue *Document::CreateResidue (G_GNUC_UNUSED char const *name, G_GNUC_UNUSED char const *symbol, G_GNUC_UNUSED Molecule *molecule)
{
	return NULL;
}

Residue const *Document::GetResidue (char const *symbol, bool *ambiguous)
{
	return Residue::GetResidue (symbol, ambiguous);
}

bool Document::SetTarget (char const *id, Object **target, Object *parent, Object *owner, Action action) throw (std::runtime_error)
{
	if (target == NULL)
	    throw std::runtime_error ("Can't set a NULL target.");
	PendingTarget pt;
	pt.target = target;
	pt.parent = parent;
	pt.owner = owner;
	pt.action = action;
	m_PendingTable[id].push_back (pt);
	return false;
}

bool Document::Loaded () throw (LoaderError)
{
	unsigned count = 0;
	std::map <std::string, list <PendingTarget> >::iterator i, end = m_PendingTable.end ();
	std::set <Object *> Deleted;
	std::set <Object *>::iterator new_end = m_NewObjects.end ();
	for (i = m_PendingTable.begin (); i != end; i++) {
		std::string id = (*i).first;
		std::list <PendingTarget> &l = (*i).second;
		std::list <PendingTarget>::iterator j = l.begin (), jend = l.end ();
		Object *obj = (*j).parent->GetDescendant (id.c_str ());
		if (obj == NULL)
			obj = (*j).parent->GetDocument ()->GetDescendant (id.c_str ());
		if (obj && m_NewObjects.find (obj) == new_end)
			obj = NULL;
		if (obj == NULL) {
			switch ((*j).action) {
			case ActionException: {
				m_PendingTable.clear ();
				std::ostringstream str;
				// Note to translators: the two strings are concatenated with the missing id between them.
				str << _("The input contains a reference to object \"") << id << _("\" but no object with this Id is described.");
				throw LoaderError (str.str ());
			}
			case ActionDelete:
				if ((*j).owner) {
					Deleted.insert ((*j).owner);
					delete (*j).owner;
					(*j).owner = NULL;
				}
			case ActionIgnore:
				break;
			}
		} else while (j != jend) {
			std::set <Object *>::iterator d = Deleted.end ();
			if (Deleted.find ((*j).owner) == d) {
				*(*j).target = obj;
				if ((*j).owner)
					(*j).owner->OnLoaded ();
				count++;
			}
				j++;
		}
	}
	m_PendingTable.clear ();
	m_NewObjects.clear ();
	return count > 0;
}

void Document::ObjectLoaded (Object *obj)
{
	m_NewObjects.insert (obj);
}

std::string Document::Name ()
{
	return _("Document");
}

}	//	namespace gcu
