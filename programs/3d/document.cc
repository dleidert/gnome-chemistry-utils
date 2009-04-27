// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * programs/3d/document.cc
 *
 * Copyright (C) 2006 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "view.h"
#include "window.h"
#include <glib/gutils.h>

gc3dDocument::gc3dDocument (gc3dApplication *App): Chem3dDoc (App, new gc3dView (this))
{
}

gc3dDocument::~gc3dDocument ()
{
}

void gc3dDocument::Load (char const *uri, char const *mime_type)
{
	Chem3dDoc::Load (uri, mime_type);
	char *title = g_strdup (GetTitle ().c_str ());
	if (!*title)
		title = g_path_get_basename (uri);
	char *buf = g_uri_unescape_string (title, NULL);
	dynamic_cast <gc3dView *> (m_View)->GetWindow ()->SetTitle (buf);
	g_free (buf);
	g_free (title);
	char *dirname = g_path_get_dirname (uri);
	m_App->SetCurDir (dirname);
	g_free (dirname);
}
