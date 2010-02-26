// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gcu/dialog-owner.cc
 *
 * Copyright (C) 2005-2010 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "dialog-owner.h"
#include "dialog.h"

using namespace std;

namespace gcu
{

DialogOwner::DialogOwner (char const *prop_ui_file)
{
	if (prop_ui_file)
		m_PropUIFile = prop_ui_file;
}

DialogOwner::~DialogOwner ()
{
	ClearDialogs ();
}

Dialog *DialogOwner::GetDialog (string name)
{
	map <string, Dialog *>::iterator i = Dialogs.find (name);
	return (i != Dialogs.end ())? (*i).second: NULL;
}

bool DialogOwner::AddDialog (string name, Dialog *dialog) 
{
	if (Dialogs[name]) {
		Dialogs[name]->Present ();
		return false;
	}
	Dialogs[name] = dialog;
	return true;
}

void DialogOwner::ClearDialogs ()
{
	map <string, Dialog *>::iterator i;
	while (!Dialogs.empty ()) {
		i = Dialogs.begin ();
		if ((*i).second)
			(*i).second->Destroy ();
		else
			Dialogs.erase (i);
	}
}

bool DialogOwner::HasPropertiesDialog () const
{
	return m_PropUIFile.length () > 0;
}

Dialog *DialogOwner::GetPropertiesDialog ()
{
	return NULL;
}

}	//	namespace gcu
