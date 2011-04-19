// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * programs/spectra/application.cc
 *
 * Copyright (C) 2007-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include <gcu/loader.h>
#include <glib/gi18n.h>
#include <clocale>
#include <map>

using namespace gcu;
using namespace std;

gsvApplication::gsvApplication (): gcugtk::Application (_("GSpectrum"), DATADIR, "gspectrum")
{
	// First, initialize plugins manager
	go_plugins_init (NULL, NULL, NULL, NULL, TRUE, GO_TYPE_PLUGIN_LOADER_MODULE);
	gcu::Loader::Init (this);
	m_SupportedMimeTypes.push_back ("chemical/x-jcamp-dx");
	map<string, LoaderStruct>::iterator it;
	bool found = Loader::GetFirstLoader (it);
	while (found) {
		if ((*it).second.supportsSpectra) {
			if ((*it).second.read)
				AddMimeType (m_SupportedMimeTypes, (*it).first);
			if ((*it).second.write)
				AddMimeType (m_WriteableMimeTypes, (*it).first);
		}
		found = Loader::GetNextLoader (it);
	}
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
	FileChooser (this, false, m_SupportedMimeTypes, Doc);
}

void gsvApplication::OnQuit ()
{
	gsvDocument *Doc;
	while (m_Docs.size () > 0) {
		Doc = dynamic_cast <gsvDocument *> (*m_Docs.begin ());
		dynamic_cast <gsvView *> (Doc->GetView ())->GetWindow ()->OnFileClose ();
	}
}

bool gsvApplication::FileProcess (const gchar* filename, const gchar* mime_type, bool bSave, G_GNUC_UNUSED GtkWindow *window, Document *Doc)
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
		if (pDoc && !pDoc->GetEmpty ())
			pDoc = NULL;
		if (!pDoc)
			pDoc = OnFileNew ();
		if (!strcmp (mime_type, "chemical/x-jcamp-dx"))
			pDoc->Load (filename, mime_type);
		else {
			ContentType ctype = Load (filename, mime_type, pDoc);
			if (ctype != gcu::ContentTypeSpectrum)
				return false;
		}
		GtkRecentData data;
		data.display_name = g_strdup (pDoc->GetTitle ().c_str ());
		if (*data.display_name == 0) {
			char *title = g_path_get_basename (filename);
			char *buf = g_uri_unescape_string (title, NULL);
			g_free (title);
			data.display_name = buf;
			pDoc->SetTitle (data.display_name);
			g_free (buf);
		}
		char *dirname = g_path_get_dirname (filename);
		SetCurDir (dirname);
		g_free (dirname);
		data.description = NULL;
		data.mime_type = (char*) mime_type;
		data.app_name = const_cast<char*> ("gspectrum");
		data.app_exec = const_cast<char*> ("gspectrum %u");
		data.groups = NULL;
		data.is_private =  FALSE;
		gtk_recent_manager_add_full (GetRecentManager (), filename, &data);
		g_free (data.display_name);
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

void gsvApplication::AddMimeType (list<string> &l, string const& mime_type)
{
	list<string>::iterator i, iend = l.end ();
	for (i = l.begin (); i != iend; i++)
		if (*i == mime_type)
			break;
	if (i == iend)
		l.push_back (mime_type);
	else
		g_warning ("Duplicate mime type: %s", mime_type.c_str ());
}
