// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gcu/dialog.cc
 *
 * Copyright (C) 2001-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "dialog.h"
#include "application.h"
#include <glib/gi18n-lib.h>

namespace gcu
{

Dialog::Dialog (Application* App, const char* windowname, DialogOwner *owner) throw (std::runtime_error)
{
	m_App = App;
	m_Owner = owner;
	if (owner && !owner->AddDialog (windowname, this)) {
		// do we need to set m_Owner to NULL?
		throw std::runtime_error (_("Could not reference the new dialog."));
	}
}

Dialog::~Dialog()
{
	if (m_Owner)
		m_Owner->RemoveDialog (m_windowname);
}

void Dialog::SetRealName (char const *name, DialogOwner *owner) throw (std::runtime_error)
{
	if (m_Owner)
		m_Owner->RemoveDialog (m_windowname);
	if (owner)
		m_Owner = owner;
	m_windowname = name;
	if (m_Owner && !m_Owner->AddDialog (name, this)) {
		// do we need to set m_Owner to NULL?
		throw std::runtime_error (_("Could not reference the new dialog."));
	}
}

}	//	namespace gcu
