// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * programs/spectra/application.cc
 *
 * Copyright (C) 2007 Jean Bréfort <jean.brefort@normalesup.org>
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

using namespace gcu;

gsvApplication::gsvApplication (): Application (_("GSpectrum"), DATADIR, "gspectrum-unstable")
{
}

gsvApplication::~gsvApplication ()
{
}

gsvDocument *gsvApplication::OnFileNew ()
{
	gsvDocument* Doc = new gsvDocument (this);
	Doc->SetTitle (_("GSpectrum"));
	new gsvWindow (this, Doc);
	return Doc;
}

void gsvApplication::OnFileOpen (gsvDocument *Doc)
{
	list<string> l;
	l.push_front ("chemical/x-jcamp-dx");
	FileChooser (this, false, l, Doc);
}

void gsvApplication::OnQuit ()
{
	gsvDocument *Doc;
	while (m_Docs.size () > 0) {
		Doc = dynamic_cast <gsvDocument *> (*m_Docs.begin ());
		dynamic_cast <gsvView *> (Doc->GetView ())->GetWindow ()->OnFileClose ();
	}
}

bool gsvApplication::FileProcess (const gchar* filename, const gchar* mime_type, bool bSave, GtkWindow *window, Document *Doc)
{
	gsvDocument *pDoc = dynamic_cast <gsvDocument *> (Doc);
	if(bSave) {
	} else {
		if (pDoc && !pDoc->GetEmpty ())
			pDoc = NULL;
		if (!pDoc)
			pDoc = OnFileNew ();
		pDoc->Load (filename, mime_type);
		GtkRecentData data;
		data.display_name = (char*) pDoc->GetTitle ().c_str ();
		data.description = NULL;
		data.mime_type = (char*) mime_type;
		data.app_name = const_cast<char*> ("gspectrum");
		data.app_exec = const_cast<char*> ("gspectrum %u");
		data.groups = NULL;
		data.is_private =  FALSE;
		gtk_recent_manager_add_full (GetRecentManager (), filename, &data);
	}
	return false;
}

void gsvApplication::OnSaveAsImage (gsvDocument *Doc)
{
	if (!Doc)
		return;
	list<string> l;
	map<string, GdkPixbufFormat*>::iterator i, end = m_SupportedPixbufFormats.end ();
	for (i = m_SupportedPixbufFormats.begin (); i != end; i++)
		l.push_front ((*i).first.c_str ());
	l.push_front ("image/x-eps");
	l.push_front ("application/postscript");
	l.push_front ("application/pdf");
	l.push_front ("image/svg+xml");
	FileChooser (this, true, l, Doc, _("Save as image"), GetImageSizeWidget ());
}
