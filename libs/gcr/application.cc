// -*- C++ -*-

/*
 * GCrystal library
 * application.cc
 *
 * Copyright (C) 2010-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "globals.h"
#include "view.h"
#include "window.h"
#include <gcugtk/filechooser.h>
#include <gcu/loader.h>
#include <gcu/macros.h>
#include <gsf/gsf-output-gio.h>
#include <cairo-pdf.h>
#include <cairo-ps.h>
#include <glib/gi18n-lib.h>
#include <list>
#include <map>
#include <string>

namespace gcr {

// defines used for GCU_GCONF_GET
#define ROOTDIR	"/apps/gchemutils/crystal/"

static void on_config_changed (GOConfNode *node, gchar const *name, G_GNUC_UNUSED gpointer user_data)
{
	GCU_UPDATE_KEY ("printing/resolution", int, PrintResolution, {})
	GCU_UPDATE_KEY ("view/fov", int, FoV, {})
	GCU_UPDATE_KEY ("view/psi", float, Psi, {})
	GCU_UPDATE_KEY ("view/theta", float, Theta, {})
	GCU_UPDATE_KEY ("view/phi", float, Phi, {})
	GCU_UPDATE_KEY ("view/red", float, Red, {})
	GCU_UPDATE_KEY ("view/green", float, Green, {})
	GCU_UPDATE_KEY ("view/blue", float, Blue, {})
}

Application::Application (): gcugtk::Application ("gcrystal")
{
//Configuration loading
	m_ConfNode = go_conf_get_node (Application::GetConfDir (), "crystal");
	GCU_GCONF_GET ("printing/resolution", int, PrintResolution, 300)
	FoV = go_conf_load_int (m_ConfNode, "views/fov", 0, 45, 10);
	Psi = go_conf_load_double (m_ConfNode, "views/psi", -180., 180., 70.);
	Theta = go_conf_load_double (m_ConfNode, "views/theta", 0., 180., 10.);
	Phi = go_conf_load_double (m_ConfNode, "views/phi", -180., 180., -90.);
	Red = go_conf_load_double (m_ConfNode, "views/red", 0., 1., 1.);
	Green = go_conf_load_double (m_ConfNode, "views/green", 0., 1., 1.);
	Blue = go_conf_load_double (m_ConfNode, "views/blue", 0., 1., 1.);
	m_NotificationId = go_conf_add_monitor (m_ConfNode, NULL, (GOConfMonitorFunc) on_config_changed, NULL);
	gcu::Loader::Init (this);
	m_SupportedMimeTypes.push_back ("application/x-gcrystal");
	m_WriteableMimeTypes.push_back ("application/x-gcrystal");
	// browse available loaders
	std::map < std::string, gcu::LoaderStruct>::iterator it;
	bool found = gcu::Loader::GetFirstLoader (it);
	while (found) {
		if ((*it).second.supportsCrystals) {
			if ((*it).second.read)
				AddMimeType (m_SupportedMimeTypes, (*it).first);
			// FIXME: add write support for cif and cml
			if ((*it).second.write)
				AddMimeType (m_WriteableMimeTypes, (*it).first);
		}
		found = gcu::Loader::GetNextLoader (it);
	}
	m_pActiveDoc = NULL;
}

Application::~Application ()
{

	go_conf_remove_monitor (m_NotificationId);
	go_conf_free_node (m_ConfNode);
}

void Application::OnFileOpen ()
{
	gcugtk::FileChooser (this, false, m_SupportedMimeTypes);
}

void Application::OnFileSave ()
{
	if (!m_pActiveDoc)
		return;
	if (m_pActiveDoc->GetFileName ())
		m_pActiveDoc->Save ();
	else
		OnFileSaveAs ();
}

void Application::OnFileSaveAs ()
{
	gcugtk::FileChooser (this, true, m_WriteableMimeTypes, m_pActiveDoc);
}

bool Application::OnFileClose ()
{
	if (!m_pActiveDoc->VerifySaved ())
		return false;
	m_pActiveDoc->RemoveAllViews ();
	return true;
}

void Application::OnSaveAsImage ()
{
	if (!m_pActiveDoc)
		return;
	std::list < std::string > l;
	std::map < std::string, GdkPixbufFormat * >::iterator i, end = m_SupportedPixbufFormats.end ();
	for (i = m_SupportedPixbufFormats.begin (); i != end; i++)
		l.push_front ((*i).first.c_str ());
	l.push_front ("image/x-eps");
	l.push_front ("application/postscript");
	l.push_front ("application/pdf");
	l.push_front ("model/vrml");
	gcugtk::FileChooser (this, true, l, m_pActiveDoc, _("Save as image"), GetImageSizeWidget ());
}

bool Application::OnQuit ()
{
	while (m_Docs.size () > 0) {
		m_pActiveDoc = static_cast < gcr::Document * > (*m_Docs.begin ());
		if (!OnFileClose ())
			return false;
	}
	return true;
}

enum {
	GCRYSTAL,
	CIF,
	CML,
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

bool Application::FileProcess (const gchar* filename, const gchar* mime_type, bool bSave, GtkWindow *window, gcu::Document *pDoc)
{
	Document *Doc = static_cast < Document * > (pDoc);
	int type = GCRYSTAL;
	if (!mime_type)
		mime_type = "application/x-gcrystal";
	std::string filename2 = filename;
	if (bSave) {
		char const *pixbuf_type = NULL;
		if (!strcmp (mime_type, "chemical/x-cif"))
			type = CIF;
		if (!strcmp (mime_type, "chemical/x-cml"))
			type = CML;
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
		case CML:
			ext = ".cml";
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
			GtkDialog* Box = GTK_DIALOG (gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, _("File %s\nexists, overwrite?"), unescaped));
			g_free (unescaped);
			gtk_window_set_icon_name (GTK_WINDOW (Box), "gcrystal");
			result = gtk_dialog_run (Box);
			gtk_widget_destroy (GTK_WIDGET (Box));
			if (result == GTK_RESPONSE_YES) {
				// destroy the old file if needed
				if (err) {
					GError *error = NULL;
					g_file_delete (file, NULL, &error);
					if (error) {
						char *unescaped = g_uri_unescape_string (filename2.c_str (), NULL);
						GtkDialog* Box = GTK_DIALOG (gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, _("Error while processing %s:\n%s"), unescaped, error->message));
						g_free (unescaped);
						g_error_free (error);
						gtk_window_set_icon_name (GTK_WINDOW (Box), "gcrystal");
						result = gtk_dialog_run (Box);
						gtk_widget_destroy (GTK_WIDGET (Box));
						g_object_unref (file);
						return false;
					}
				}
			}
		}
		g_object_unref (file);
		std::map < std::string, std::string> options; // not used at the moment
		if (result == GTK_RESPONSE_YES)
			switch (type) {
			case GCRYSTAL: {
				Doc->SetFileName (filename2);
				Doc->Save ();
				GtkRecentData data;
				data.display_name = const_cast <char * > (Doc->GetTitle ());
				data.description = NULL;
				data.mime_type = const_cast<char*> ("application/x-gcrystal");
				data.app_name = const_cast<char*> ("gcrystal");
				data.app_exec = const_cast<char*> ("gcrystal %u");
				data.groups = NULL;
				data.is_private =  FALSE;
				gtk_recent_manager_add_full (GetRecentManager (), filename2.c_str (), &data);
				Doc->RenameViews ();
				break;
			}
			case CIF:
			case CML: {
				Save (filename2, mime_type, Doc, gcu::ContentTypeCrystal);
				GtkRecentData data;
				data.display_name = (char*) Doc->GetTitle ();
				data.description = NULL;
				data.mime_type = const_cast <char*> (mime_type);
				data.app_name = const_cast <char*> ("gcrystal");
				data.app_exec = const_cast <char*> ("gcrystal %u");
				data.groups = NULL;
				data.is_private =  FALSE;
				gtk_recent_manager_add_full (GetRecentManager (), filename2.c_str (), &data);
				break;
			}
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
					GtkWidget* message = gtk_message_dialog_new (window, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, _("Could not create stream!\n%s"), error->message);
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
				Doc->GetView ()->RenderToCairo (cr, GetImageWidth (), GetImageHeight (), !GetTransparentBackground ());
				cairo_destroy (cr);
				break;
			}
			}
	} else {
		if (!strcmp (mime_type, "application/x-gcrystal"));
		else if (!strcmp (mime_type, "chemical/x-cif"))
			type = CIF;
		else if (!strcmp (mime_type, "chemical/x-cml"))
			type = CML;
		else
			return true;
		m_bFileOpening = true; // force creation of new document if current one is not empty or is dirty
		Document *xDoc = GetDocument (filename);
		m_bFileOpening = false;
		if (xDoc)
			Doc = xDoc;
		else if (!pDoc->GetEmpty () || pDoc->GetDirty ())
			Doc = NULL;
		if (!Doc)
			Doc = OnFileNew ();
		m_pActiveDoc = Doc;
		if (Doc->GetFileName () && !strcmp (Doc->GetFileName(), filename)) {
			if (!Doc->GetDirty ())
				return true;
			else {
				GtkWidget* mbox = gtk_message_dialog_new (window, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, _("\"%s\" has been modified since last saving. Do you wish to come back to saved version?"), Doc->GetTitle ());
				int res = gtk_dialog_run (GTK_DIALOG (mbox));
				if (res != GTK_RESPONSE_YES)
					return true;
			}
		}
		gcu::ContentType ctype = Load (filename, mime_type, Doc);
		if (ctype == gcu::ContentTypeCrystal) {
			Doc->Loaded ();
			Doc->SetReadOnly (true);
			Doc->UpdateAllViews ();
			GtkRecentData data;
			data.display_name = (char*) Doc->GetTitle ();
			if (!(*data.display_name))
				data.display_name = (char*) Doc->GetLabel ();
			data.description = NULL;
			data.mime_type = const_cast<char*> (mime_type);
			data.app_name = const_cast<char*> ("gcrystal");
			data.app_exec = const_cast<char*> ("gcrystal %u");
			data.groups = NULL;
			data.is_private =  FALSE;
			gtk_recent_manager_add_full (GetRecentManager (), filename, &data);
			goto normal_exit;
		} else if (ctype != gcu::ContentTypeUnknown) {
			// FIXME: open using the appropriate program.
			return false;
		}
		if ((type == GCRYSTAL)? Doc->Load (filename): false) {
normal_exit:
			GtkRecentData data;
			data.display_name = (char*) Doc->GetTitle ();
			if (!(*data.display_name))
				data.display_name = (char*) Doc->GetLabel ();
			data.description = NULL;
			data.mime_type = const_cast<char*> (mime_type);
			data.app_name = const_cast<char*> ("gcrystal");
			data.app_exec = const_cast<char*> ("gcrystal %u");
			data.groups = NULL;
			data.is_private =  FALSE;
			gtk_recent_manager_add_full (GetRecentManager (), filename, &data);
			// change titles in every window and bring to front
			std::list <gcr::View *> *Views = Doc->GetViews ();
			std::list <gcr::View *>::iterator i, iend = Views->end ();
			int n = 1, max = Views->size ();
			Doc->RenameViews ();
			char const *title = Doc->GetLabel ();
			for (i = Views->begin (); i != iend; i++) {
				Window *window = dynamic_cast <View*> (*i)->GetWindow ();
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

void Application::AddMimeType (std::list < std::string > &l, std::string const& mime_type)
{
	std::list < std::string >::iterator i, iend = l.end ();
	for (i = l.begin (); i != iend; i++)
		if (*i == mime_type)
			break;
	if (i == iend)
		l.push_back (mime_type);
	else
		g_warning ("Duplicate mime type: %s", mime_type.c_str ());
}

char const *Application::GetFirstSupportedMimeType (std::list < std::string >::iterator &it)
{
	it = m_SupportedMimeTypes.begin ();
	return (it == m_SupportedMimeTypes.end ())? NULL: (*it).c_str ();
}

char const *Application::GetNextSupportedMimeType (std::list<std::string>::iterator &it)
{
	it++;
	return (it == m_SupportedMimeTypes.end ())? NULL: (*it).c_str ();
}

Document* Application::GetDocument (const char* filename)
{
	Document* pDoc = NULL;
	std::set < gcu::Document * >::iterator i, iend = m_Docs.end ();
	for (i = m_Docs.begin (); i != iend; i++) {
		pDoc = static_cast < gcr::Document * > (*i);
		if (!pDoc->GetFileName ())
			continue;
		if (!strcmp (pDoc->GetFileName (), filename))
			break;
	}
	if (i != iend && pDoc)
		return pDoc;
	if (m_bFileOpening) {
		pDoc = m_pActiveDoc;
		if (pDoc && (!pDoc->GetEmpty () || pDoc->GetDirty ())){
			pDoc = NULL;
		}
	}
	if (!pDoc) {
		OnFileNew ();
		pDoc = m_pActiveDoc;
	}
	return pDoc;
}

Window *Application::CreateNewWindow (Document *doc)
{
	return new Window (this, doc);
}


}	//	namespace gcr
