// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * programs/3d/application.cc 
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
#include "application.h"
#include "document.h"
#include "view.h"
#include "window.h"
#include <gcu/filechooser.h>
#include <glib/gi18n.h>

gc3dApplication::gc3dApplication (): Application (_("GChem3D Viewer"))
{
}

gc3dApplication::~gc3dApplication ()
{
}

gc3dDocument *gc3dApplication::OnFileNew ()
{
	gc3dDocument* Doc = new gc3dDocument (this);
	Doc->SetTitle (_("GChem3D Viewer"));
	new gc3dWindow (this, Doc);
	return Doc;
}

void gc3dApplication::OnFileOpen (gc3dDocument *Doc)
{
	list<char const*> l;
	l.push_front ("chemical/x-cml");
	l.push_front ("chemical/x-mdl-molfile");
	l.push_front ("chemical/x-pdb");
	l.push_front ("chemical/x-xyz");
	FileChooser (this, false, l, Doc);
}

void gc3dApplication::OnQuit ()
{
	gc3dDocument *Doc;
	while (m_Docs.size () > 0) {
		Doc = dynamic_cast <gc3dDocument *> (*m_Docs.begin ());
		dynamic_cast <gc3dView *> (Doc->GetView ())->GetWindow ()->OnFileClose ();
	}
}

bool gc3dApplication::FileProcess (const gchar* filename, const gchar* mime_type, bool bSave, GtkWindow *window, Document *Doc)
{
	if (Doc && !dynamic_cast <gc3dDocument *> (Doc)->IsEmpty ())
			Doc = NULL;
	if (!Doc)
		Doc = OnFileNew ();
	dynamic_cast <gc3dDocument *> (Doc)->Load (filename, mime_type);
	return false;
}
