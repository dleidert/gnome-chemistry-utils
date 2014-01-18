// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gcugtk/chem3dapplication.cc
 *
 * Copyright (C) 2011 Jean Bréfort <jean.brefort@normalesup.org>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include "config.h"
#include "chem3dapplication.h"
#include "chem3ddoc.h"
#include "chem3dview.h"
#include "chem3dwindow.h"
#include "message.h"
#include "molecule.h"
#include <gcugtk/filechooser.h>
#include <gsf/gsf-output-gio.h>
#include <cairo-pdf.h>
#include <cairo-ps.h>
#include <glib/gi18n-lib.h>

namespace gcugtk {

Chem3dApplication::Chem3dApplication (gcu::Display3DMode display3d, char const *bg):
	gcugtk::Application (_("GChem3D Viewer"), DATADIR, "gchem3d"),
	m_Display3D (display3d)
{
	if (!bg)
		bg = "black";
	if (!strcmp (bg, "black")) {
		m_Red = 0.;
		m_Green = 0.;
		m_Blue = 0.;
	} else if (!strcmp (bg, "white")) {
		m_Red = 1.;
		m_Green = 1.;
		m_Blue = 1.;
	} else {
		if ((strlen (bg) != 7) || (*bg != '#')) {
			g_warning ("Unrecognized color: %s\n", bg);
			return;
		}
		int r, g, b;
		r = strtoul (bg + 1, NULL, 16);
		b = r & 0xff;
		m_Blue = (float) b / 255.;
		r >>= 8;
		g = r & 0xff;
		m_Green = (float) g / 255.;
		r >>=8;
		m_Red = (float) r / 255;
	}
}

Chem3dApplication::~Chem3dApplication ()
{
}

void Chem3dApplication::OnFileOpen (Chem3dDoc *doc)
{
	std::list < std::string > l;
	l.push_front ("chemical/x-cml");
	l.push_front ("chemical/x-mdl-molfile");
	l.push_front ("chemical/x-pdb");
	l.push_front ("chemical/x-xyz");
	gcugtk::FileChooser (this, false, l, doc);
}

void Chem3dApplication::OnSaveAsImage (Chem3dDoc *Doc)
{
	if (!Doc)
		return;
	std::list < std::string > l;
	std::map < std::string, GdkPixbufFormat * >::iterator i, end = m_SupportedPixbufFormats.end ();
	for (i = m_SupportedPixbufFormats.begin (); i != end; i++)
		l.push_front ((*i).first.c_str ());
	l.push_front ("image/x-eps");
	l.push_front ("application/postscript");
	l.push_front ("application/pdf");
	l.push_front ("model/vrml");
	gcugtk::FileChooser (this, true, l, Doc, _("Save as image"), GetImageSizeWidget ());
}

void Chem3dApplication::OnQuit ()
{
	Chem3dDoc *doc;
	Chem3dView *view;
	Chem3dWindow *window;
	while (!m_Docs.empty ()) {
		doc = static_cast < Chem3dDoc * > (*m_Docs.begin ());
		view = static_cast < Chem3dView * > (doc->GetView ());
		window = static_cast < Chem3dWindow * > (view->GetWindow ());
		gtk_widget_destroy (GTK_WIDGET (window->GetWindow ()));
		delete window;
	}
}

static cairo_status_t cairo_write_func (void *closure, const unsigned char *data, unsigned int length)
{
	gboolean result;
	GsfOutput *output = GSF_OUTPUT (closure);

	result = gsf_output_write (output, length, data);

	return result ? CAIRO_STATUS_SUCCESS : CAIRO_STATUS_WRITE_ERROR;
}

bool Chem3dApplication::FileProcess (const gchar* filename, const gchar* mime_type, bool bSave, GtkWindow *window, gcu::Document *Doc)
{
	Chem3dDoc *pDoc = dynamic_cast < Chem3dDoc * > (Doc);
	if(bSave) {
		bool supported = false, vrml = false, use_cairo = false;
		std::string filename2 = filename;
		char const *pixbuf_type = NULL;
		GOImageFormat format = GO_IMAGE_FORMAT_UNKNOWN;
		int i;
		cairo_t *cr = NULL;

		if (mime_type) {
			char *fnm = go_mime_to_image_format (mime_type);
			if (fnm) {
				format = go_image_get_format_from_name (fnm);
				switch (format) {
				case GO_IMAGE_FORMAT_EPS:
				case GO_IMAGE_FORMAT_PDF:
				case GO_IMAGE_FORMAT_PS: {
					supported = true;
					use_cairo = true;
					break;
				}
				default:
					pixbuf_type = GetPixbufTypeName (filename2, mime_type);
					supported = (pixbuf_type);
					break;
				}
			} else if (!strcmp (mime_type, "model/vrml")) {
				supported = true;
				vrml = true;
				i = strlen (filename) - 4;
				if ((i <= 0) || (strcmp (filename + i, ".wrl")))
					filename2 += ".wrl";
			}
		}
		if (!supported) {
			GtkWidget* message = gtk_message_dialog_new (window, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
														_("Sorry, format not supported!"));
			gtk_dialog_run (GTK_DIALOG (message));
			gtk_widget_destroy (message);
			return true;
		}
		GVfs *vfs = g_vfs_get_default ();
		GFile *file = g_vfs_get_file_for_uri (vfs, filename2.c_str ());
		GError *error = NULL;
		bool err = g_file_query_exists (file, NULL);
		gint result = GTK_RESPONSE_YES;
		if (err) {
			char *unescaped = g_uri_unescape_string (filename2.c_str (), NULL);
			gchar * message = g_strdup_printf (_("File %s\nexists, overwrite?"), unescaped);
			g_free (unescaped);
			Message *box = new Message (this, message, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, window);
			result = box->Run ();
			g_free (message);
			if (result == GTK_RESPONSE_YES) {
				g_file_delete (file, NULL, &error);
				g_object_unref (file);
			} else {
				g_object_unref (file);
				return true;
			}
		}
		std::map < std::string, std::string > options; // not used at the moment
		if (result == GTK_RESPONSE_YES) {
			if (vrml)
				pDoc->OnExportVRML (filename2);
			else if (use_cairo) {
				GsfOutput *output = gsf_output_gio_new_for_uri (filename, &error);
				if (error) {
					gchar * mess = g_strdup_printf (_("Could not create stream!\n%s"), error->message);
					Message *box = new Message (this, mess, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, window);
					g_free (mess);
					box->Run ();
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
						break;
				}
				cr = cairo_create (surface);
				cairo_surface_destroy (surface);
				pDoc->GetView ()->RenderToCairo (cr, GetImageWidth (), GetImageHeight (), !GetTransparentBackground ());
				cairo_destroy (cr);
			}
			else
				pDoc->GetView ()->SaveAsImage (filename2, pixbuf_type, options, GetImageWidth (), GetImageHeight (), !GetTransparentBackground ());
		}
	} else {
		if (pDoc && !pDoc->IsEmpty ())
			pDoc = NULL;
		if (!pDoc)
			pDoc = OnFileNew ();
		pDoc->Load (filename, mime_type);
		Molecule *mol = static_cast < gcugtk::Molecule * > (pDoc->GetMol ());
		if (mol && mol->GetChildrenNumber ())
			static_cast < gcugtk::Chem3dWindow * > (pDoc->GetWindow ())->AddMoleculeMenus (mol);

		GtkRecentData data;
		data.display_name = const_cast <char*> (pDoc->GetTitle ().c_str ());
		if (*data.display_name == 0)
			data.display_name = NULL;
		data.description = NULL;
		data.mime_type = (char*) mime_type;
		data.app_name = const_cast <char*> ("gchem3d");
		data.app_exec = const_cast <char*> ("gchem3d %u");
		data.groups = NULL;
		data.is_private =  FALSE;
		gtk_recent_manager_add_full (GetRecentManager (), filename, &data);
	}
	return false;
}

}	// namespace gcugtk
