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
	gc3dDocument *pDoc = dynamic_cast <gc3dDocument *> (Doc);
	if(bSave) {
		map <string, string> options; // not used at the moment
		if (!strcmp (mime_type, "image/png"))
			pDoc->GetView ()->SaveAsImage (filename, "png", options, GetImageResolution ());
		else if (!strcmp (mime_type, "image/jpeg"))
			pDoc->GetView ()->SaveAsImage (filename, "jpeg", options, GetImageResolution ());
		else if (!strcmp (mime_type, "model/vrml"))
			;
		else
			;
			
	} else {
	if (pDoc && !pDoc->IsEmpty ())
			pDoc = NULL;
		if (!pDoc)
			pDoc = OnFileNew ();
		pDoc->Load (filename, mime_type);
		GtkRecentData data;
		data.display_name = (char*) pDoc->GetTitle ();
		data.description = NULL;
		data.mime_type = (char*) mime_type;
		data.app_name = "gchem3d-viewer";
		data.app_exec = "gchem3d-viewer %u";
		data.groups = NULL;
		data.is_private =  FALSE;
		gtk_recent_manager_add_full (GetRecentManager (), filename, &data);
	}
	return false;
}

void gc3dApplication::OnSaveAsImage (gc3dDocument *Doc)
{
	if (!Doc)
		return;
	list<char const*> l;
	l.push_front ("image/jpeg");
	l.push_front ("image/png");
//	l.push_front ("model/vrml");
	FileChooser (this, true, l, Doc, _("Save as image"), GetImageResolutionWidget ());
}
