// -*- C++ -*-

/* 
 * Gnome Crystal
 * application.cc 
 *
 * Copyright (C) 2001-2008 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "globals.h"
#include "prefs.h"
#include "window.h"
#include <gcu/filechooser.h>
#ifndef GOFFICE_HAS_GLOBAL_HEADER
#   include <goffice/utils/go-image.h>
#   include <gtk/gtk.h>
#endif
#include <gsf/gsf-output-gio.h>
#include <glade/glade.h>
#include <cairo-pdf.h>
#include <cairo-ps.h>
#include <gio/gio.h>
#include <glib/gi18n.h>
#include <cstring>

using namespace gcu;
using namespace std;

static unsigned short nNewDocs = 1;

gcApplication::gcApplication(): Application ("gcrystal")
{
}

gcApplication::~gcApplication ()
{
}

gcDocument *gcApplication::OnFileNew ()
{
	gcDocument* pDoc = new gcDocument (this);
	gchar buf[32];
	g_snprintf (buf, sizeof (buf), _("Untitled%d"), nNewDocs++);
	pDoc->SetLabel (buf);
	m_Docs.push_back (pDoc);
	new gcWindow (this, pDoc);
	return pDoc;
}

void gcApplication::OnFileOpen ()
{
	list<string> l;
	l.push_front ("application/x-gcrystal");
#ifdef HAVE_OPENBABEL_2_2
	l.push_front ("chemical/x-cif");
#endif
	FileChooser (this, false, l);
}

void gcApplication::OnFileSave ()
{
	if (!m_pActiveDoc)
		return;
	if (m_pActiveDoc->GetFileName ())
		m_pActiveDoc->Save ();
	else
		OnFileSaveAs ();
}

void gcApplication::OnFileSaveAs ()
{
	list<string> l;
	l.push_front ("application/x-gcrystal");
	FileChooser (this, true, l, m_pActiveDoc);
}

bool gcApplication::OnFileClose ()
{
	if (!m_pActiveDoc->VerifySaved ())
		return false;
	m_pActiveDoc->RemoveAllViews ();
	return true;
}

void gcApplication::OnSaveAsImage ()
{
	if (!m_pActiveDoc)
		return;
	list<string> l;
	map<string, GdkPixbufFormat*>::iterator i, end = m_SupportedPixbufFormats.end ();
	for (i = m_SupportedPixbufFormats.begin (); i != end; i++)
		l.push_front ((*i).first.c_str ());
	l.push_front ("image/x-eps");
	l.push_front ("application/postscript");
	l.push_front ("application/pdf");
	l.push_front ("model/vrml");
	FileChooser (this, true, l, m_pActiveDoc, _("Save as image"), GetImageSizeWidget ());
}

gcDocument* gcApplication::GetDoc (const char* filename)
{
	gcDocument* pDoc = NULL;
	std::list<gcDocument*>::iterator i, iend = m_Docs.end ();
	for (i = m_Docs.begin (); i != iend; i++) {
		pDoc = *i;
		if (!pDoc->GetFileName ())
			continue;
		if (!strcmp (pDoc->GetFileName (), filename))
			break;
	}
	if (i != iend && pDoc)
		return pDoc;
	if (m_bFileOpening) {
		pDoc = m_Docs.back ();
		if (!pDoc->GetEmpty () || pDoc->GetDirty ())
			pDoc = NULL;
	}
	if (!pDoc) {
		OnFileNew ();
		pDoc = m_Docs.back ();
	}
	nNewDocs--;
	return pDoc;
}

enum {
	GCRYSTAL,
	CIF,
	VRML,
	PDF,
	PS,
	EPS,
	PIXBUF
};

static cairo_status_t cairo_write_func (void *closure, const unsigned char *data, unsigned int length)
{
	gboolean result;
	GsfOutput *output = GSF_OUTPUT (closure);

	result = gsf_output_write (output, length, data);

	return result ? CAIRO_STATUS_SUCCESS : CAIRO_STATUS_WRITE_ERROR;
}

bool gcApplication::FileProcess (const gchar* filename, const gchar* mime_type, bool bSave, GtkWindow *window, Document *pDoc)
{
	gcDocument *Doc = static_cast<gcDocument*> (pDoc);
	int type = GCRYSTAL;
	if (!mime_type)
		mime_type = "application/x-gcrystal";
	string filename2 = filename;
	if (bSave) {
		char const *pixbuf_type = NULL;
		if (!strcmp (mime_type, "chemica/x-cif"))
			type = CIF;
		else if (!strcmp (mime_type, "model/vrml"))
			type = VRML;
		else if (!strcmp (mime_type, "image/x-eps"))
			type = EPS;
		else if (!strcmp (mime_type, "application/postscript"))
			type = PS;
		else if (!strcmp (mime_type, "application/pdf"))
			type = PDF;
		else if ((pixbuf_type = GetPixbufTypeName (filename2, mime_type)))
			type = PIXBUF;
		char const *ext = NULL;
		switch (type) {
		case GCRYSTAL:
			ext = ".gcrystal";
			break;
		case CIF:
			ext = ".cif";
			break;
		case VRML:
			ext = ".wrl";
			break;
		case PDF:
			ext = ".pdf";
			break;
		case PS:
			ext = ".ps";
			break;
		case EPS:
			ext = ".eps";
			break;
		default:
			break;
		}
		if (ext) { 
			int i = strlen (filename) - strlen (ext);
			if ((i <= 0) || (strcmp (filename +i, ext)))
				filename2 += ext;
		}
		GFile *file = g_file_new_for_uri (filename2.c_str ());
		bool err = g_file_query_exists (file, NULL);
		gint result = GTK_RESPONSE_YES;
		if (err) {
			char *unescaped = g_uri_unescape_string (filename2.c_str (), NULL);
			gchar * message = g_strdup_printf (_("File %s\nexists, overwrite?"), unescaped);
			g_free (unescaped);
			GtkDialog* Box = GTK_DIALOG (gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, message));
			gtk_window_set_icon_name (GTK_WINDOW (Box), "gcrystal");
			result = gtk_dialog_run (Box);
			gtk_widget_destroy (GTK_WIDGET (Box));
			g_free (message);
			if (result == GTK_RESPONSE_YES) {
				// destroy the old file if needed
				if (err) {
					GError *error = NULL;
					g_file_delete (file, NULL, &error);
					if (error) {
						char *unescaped = g_uri_unescape_string (filename2.c_str (), NULL);
						gchar * message = g_strdup_printf (_("Error while processing %s:\n%s"), unescaped, error->message);
						g_free (unescaped);
						g_error_free (error);
						GtkDialog* Box = GTK_DIALOG (gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, message));
						gtk_window_set_icon_name (GTK_WINDOW (Box), "gcrystal");
						result = gtk_dialog_run (Box);
						gtk_widget_destroy (GTK_WIDGET (Box));
						g_free (message);
						g_object_unref (file);
						return false;
					}
				}
			}
		}
		g_object_unref (file);
		map <string, string> options; // not used at the moment
		if (result == GTK_RESPONSE_YES)
			switch (type) {
			case GCRYSTAL:
				Doc->SetFileName (filename2);
				Doc->Save ();
				GtkRecentData data;
				data.display_name = (char*) Doc->GetTitle ();
				data.description = NULL;
				data.mime_type = const_cast<char*> ("application/x-gcrystal");
				data.app_name = const_cast<char*> ("gcrystal");
				data.app_exec = const_cast<char*> ("gcrystal %u");
				data.groups = NULL;
				data.is_private =  FALSE;
				gtk_recent_manager_add_full (GetRecentManager (), filename2.c_str (), &data);
				Doc->RenameViews ();
				break;
			case CIF:
				break;
			case VRML:
				Doc->OnExportVRML (filename2);
				break;
			case PIXBUF:
				Doc->SaveAsImage (filename2, pixbuf_type, options);
				break;
			default: {
				char *fnm = go_mime_to_image_format (mime_type);
				GOImageFormat format = go_image_get_format_from_name (fnm);
				GError *error = NULL;
				GsfOutput *output = gsf_output_gio_new_for_uri (filename2.c_str (), &error);
				if (error) {
					gchar * mess = g_strdup_printf (_("Could not create stream!\n%s"), error->message);
					GtkWidget* message = gtk_message_dialog_new (window, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, mess);
					g_free (mess);
					gtk_dialog_run (GTK_DIALOG (message));
					gtk_widget_destroy (message);
					g_error_free (error);
					return true;
				}
				cairo_surface_t *surface = NULL;
				switch (format) {
					case GO_IMAGE_FORMAT_EPS:
						surface = cairo_ps_surface_create_for_stream (cairo_write_func, output, GetImageWidth (), GetImageHeight ());
						cairo_ps_surface_set_eps (surface, TRUE);
						break;
					case GO_IMAGE_FORMAT_PDF:
						surface = cairo_pdf_surface_create_for_stream (cairo_write_func, output, GetImageWidth (), GetImageHeight ());
						break;
					case GO_IMAGE_FORMAT_PS:
						surface = cairo_ps_surface_create_for_stream (cairo_write_func, output, GetImageWidth (), GetImageHeight ());
						break;
					default:
						return true;
				}
				cairo_t *cr = cairo_create (surface);
				cairo_surface_destroy (surface);
				Doc->GetView ()->RenderToCairo (cr, GetImageWidth (), GetImageHeight ());
				cairo_destroy (cr);
				break;
			}
			}
	} else {
		if (!strcmp (mime_type, "application/x-gcrystal"));
		else if (!strcmp (mime_type, "chemical/x-cif"))
			type = CIF;
		else
			return true;
		gcDocument *xDoc = GetDoc (filename);
		if (xDoc)
			Doc = xDoc;
		else if (!pDoc->GetEmpty () || pDoc->GetDirty ())
			Doc = NULL;
		if (!Doc)
			Doc = OnFileNew ();
		if (Doc->GetFileName () && !strcmp (Doc->GetFileName(), filename)) {
			if (!Doc->GetDirty ())
				return true;
			else {
				gchar* str = g_strdup_printf (_("\"%s\" has been modified since last saving. Do you wish to come back to saved version?"), Doc->GetTitle ());
				GtkWidget* mbox = gtk_message_dialog_new (window, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, str);
				int res = gtk_dialog_run (GTK_DIALOG (mbox));
				if (res != GTK_RESPONSE_YES)
					return true;
			}
		}
		if ((type == GCRYSTAL)? Doc->Load (filename):
#ifdef HAVE_OPENBABEL_2_2
						Doc->Import (filename, mime_type)) {
#else
						false) {
#endif
			GtkRecentData data;
			data.display_name = (char*) Doc->GetTitle ();
			data.description = NULL;
			data.mime_type = const_cast<char*> (mime_type);
			data.app_name = const_cast<char*> ("gcrystal");
			data.app_exec = const_cast<char*> ("gcrystal %u");
			data.groups = NULL;
			data.is_private =  FALSE;
			gtk_recent_manager_add_full (GetRecentManager (), filename, &data);
			// change titles in every window and bring to front
			list <CrystalView *> *Views = Doc->GetViews ();
			list <CrystalView *>::iterator i, iend = Views->end ();
			int n = 1, max = Views->size ();
			Doc->RenameViews ();
			char const *title = Doc->GetLabel ();
			for (i = Views->begin (); i != iend; i++) {
				gcWindow *window = dynamic_cast <gcView*> (*i)->GetWindow ();
				GtkWindow *w = window->GetWindow ();
				gtk_window_present (w);
				if (max > 1) {
					char *t = g_strdup_printf ("%s (%i)", title, n++);
					gtk_window_set_title (w, t);
					g_free (t);
				} else
					gtk_window_set_title (w, title);
				window->ActivateActionWidget ("ui/MainMenu/FileMenu/Save", !Doc->GetReadOnly ());
				window->ActivateActionWidget ("ui/MainToolbar/Save", !Doc->GetReadOnly ());
			}
		}
	}
	return false;
}

void gcApplication::RemoveDocument (gcDocument *pDoc)
{
	m_Docs.remove (pDoc);
	if (m_Docs.size () == 0)
		gtk_main_quit ();
}

bool gcApplication::OnQuit ()
{
	while (m_Docs.size () > 0) {
		m_pActiveDoc = m_Docs.front ();
		if (!OnFileClose ())
			return false;
	}
	return true;
}
