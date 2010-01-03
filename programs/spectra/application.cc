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
#ifndef GOFFICE_HAS_GLOBAL_HEADER
#   include <goffice/utils/go-image.h>
#   include <gio/gio.h>
#endif
#include <glib/gi18n.h>
#include <clocale>
#include <map>

using namespace gcu;
using namespace std;

gsvApplication::gsvApplication (): Application (_("GSpectrum"), DATADIR, "gspectrum")
{
	SetImageWidth (600);
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
		GFile *file = g_file_new_for_uri (filename);
		bool err = g_file_query_exists (file, NULL);
		gint result = GTK_RESPONSE_YES;
		if (err) {
			char *unescaped = g_uri_unescape_string (filename, NULL);
			gchar * message = g_strdup_printf (_("File %s\nexists, overwrite?"), unescaped);
			g_free (unescaped);
			GtkDialog* Box = GTK_DIALOG (gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, message));
			gtk_window_set_icon_name (GTK_WINDOW (Box), "gspectrum");
			result = gtk_dialog_run (Box);
			gtk_widget_destroy (GTK_WIDGET (Box));
			g_free (message);
		}
		if (result == GTK_RESPONSE_YES) {
			g_file_delete (file, NULL, NULL);
			dynamic_cast <gsvView *> (pDoc->GetView ())->SaveAsImage (filename, mime_type, GetImageWidth (), GetImageHeight ());
		}
		g_object_unref (file);
	} else {
		char *old_locale = g_strdup (setlocale (LC_NUMERIC, NULL));
		setlocale (LC_NUMERIC, "C");
		if (pDoc && !pDoc->GetEmpty ())
			pDoc = NULL;
		if (!pDoc)
			pDoc = OnFileNew ();
		pDoc->Load (filename, mime_type);
		setlocale (LC_NUMERIC, old_locale);
		g_free (old_locale);
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
	char const *mime;
	map<string, GdkPixbufFormat*>::iterator i, end = m_SupportedPixbufFormats.end ();
	for (i = m_SupportedPixbufFormats.begin (); i != end; i++)
		l.push_front ((*i).first.c_str ());
	if (go_image_get_format_from_name ("eps") != GO_IMAGE_FORMAT_UNKNOWN) {
		mime = go_image_format_to_mime ("eps");
		if (mime)
			l.push_front (mime);
	}
	l.push_front ("application/postscript");
	l.push_front ("application/pdf");
	l.push_front ("image/svg+xml");
	FileChooser (this, true, l, Doc, _("Save as image"), GetImageSizeWidget ());
}
