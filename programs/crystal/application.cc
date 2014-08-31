// -*- C++ -*-

/*
 * Gnome Crystal
 * application.cc
 *
 * Copyright (C) 2001-2011 Jean Bréfort <jean.brefort@normalesup.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
 * USA
 */

#include "config.h"
#include "application.h"
#include "window.h"
#include <glib/gi18n.h>
#include <cstring>

using namespace std;

static unsigned short nNewDocs = 1;

static gcu::Object *CreateAtom ()
{
	return new gcAtom ();
}

gcApplication::gcApplication(): gcr::Application ()
{
		AddType ("atom", CreateAtom, gcu::AtomType);
}

gcApplication::~gcApplication ()
{
}

gcr::Document *gcApplication::OnFileNew ()
{
	gcDocument* pDoc = new gcDocument (this);
	char buf[32];
	g_snprintf (buf, sizeof (buf), _("Untitled%d"), nNewDocs++);
	pDoc->SetLabel (buf);
	new gcWindow (this, pDoc);
	m_pActiveDoc = pDoc;
	return pDoc;
}

gcr::Window *gcApplication::CreateNewWindow (gcr::Document *doc)
{
	return new gcWindow (this, static_cast < gcDocument * > (doc));
}
