// -*- C++ -*-

/* 
 * GChemPaint Wikipedia plugin
 * wikipediatool.cc 
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
#include "wikipediatool.h"
#include <gcp/application.h>
#include <gcp/document.h>
#include <gcp/molecule.h>
#include <gcp/theme.h>
#include <gcp/view.h>
#include <gcp/widgetdata.h>
#include <gcu/application.h>
#include <gcu/filechooser.h>
#include <gccv/canvas.h>
#include <gccv/structs.h>
#include <gio/gio.h>
#include <glib/gi18n-lib.h>
#include <iostream>
#include <sstream>
#include <vector>

using namespace gcu;
using namespace std;

class WikipediaApp: public gcu::Application
{
public:
	WikipediaApp (gcu::Application *App);
	virtual ~WikipediaApp ();

	bool FileProcess (const gchar* filename, const gchar* mime_type, bool bSave, GtkWindow *window, Document *pDoc = NULL);
	GtkWindow *GetWindow () {return m_pApp->GetWindow ();}

private:
	gcu::Application *m_pApp;
};

WikipediaApp::WikipediaApp (gcu::Application *App): gcu::Application ("gchempaint")
{
	m_pApp = App;
}

WikipediaApp::~WikipediaApp ()
{
}

// FIXME: make this a public function in libgcu
static gboolean do_save_image (const gchar *buf, gsize count, GError **error, gpointer data)
{
	GOutputStream *output = (GOutputStream *) data;
	while (count) {
		count -= g_output_stream_write (output, buf, count, NULL, error);
		if (*error)
			return false;
	}
	return true;
}

static void destroy_surface (G_GNUC_UNUSED guchar *pixels, gpointer data)
{
	cairo_surface_destroy (reinterpret_cast <cairo_surface_t *> (data));
}

#define ZOOM 1.

bool WikipediaApp::FileProcess (char const *filename, G_GNUC_UNUSED char const *mime_type, G_GNUC_UNUSED bool bSave, GtkWindow *window, gcu::Document *pDoc)
{
	gcp::Document *Doc = static_cast<gcp::Document *> (pDoc);
	if (!filename || !strlen( filename) || filename[strlen( filename) - 1] == '/') {
		GtkWidget* message = gtk_message_dialog_new (window, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, 
															_("Please enter a file name,\nnot a directory"));
		gtk_dialog_run (GTK_DIALOG (message));
		gtk_widget_destroy (message);
		return true;
	}
	char *filename2 = (strcmp (filename + strlen (filename) - 4, ".png"))?
						g_strconcat (filename, ".png", NULL):
						g_strdup (filename);
	GFile *file = g_vfs_get_file_for_uri (g_vfs_get_default (), filename2);
	GError *error = NULL;
	bool err = g_file_query_exists (file, NULL);
	gint result = GTK_RESPONSE_YES;
	if (err) {
		char *buf = g_uri_unescape_string (filename2, NULL);
		GtkDialog* Box = GTK_DIALOG (gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, _("File %s\nexists, overwrite?"), buf));
		g_free (buf);
		gtk_window_set_icon_name (GTK_WINDOW (Box), "gchempaint");
		result = gtk_dialog_run (Box);
		gtk_widget_destroy (GTK_WIDGET (Box));
	}
	if (result == GTK_RESPONSE_YES) {
		if (err) {
			// destroy the old file
			g_file_delete (file, NULL, &error);
			if (error) {
				char *buf = g_uri_unescape_string (filename2, NULL);
				GtkDialog* Box = GTK_DIALOG (gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, _("Error while processing %s:\n%s"), buf, error->message));
				g_free (buf);
				g_error_free (error);
				gtk_window_set_icon_name (GTK_WINDOW (Box), "gchempaint");
				result = gtk_dialog_run (Box);
				gtk_widget_destroy (GTK_WIDGET (Box));
				g_object_unref (file);
				return true;
			}
		}
	} else
		return true;
	gccv::Rect rect;
	gcp::WidgetData *Data = Doc->GetView ()->GetData ();
	Data->GetObjectBounds (Doc, &rect);
	int w = (int) (ceil (rect.x1) - floor (rect.x0)), h = (int) (ceil (rect.y1) - floor (rect.y0));
	w = (int) rint (w + 12) * ZOOM;
	h = (int) rint (h + 12) * ZOOM;
	cairo_surface_t *surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, w, h);
	cairo_t *cr = cairo_create (surface);
	cairo_scale (cr, ZOOM, ZOOM);
	cairo_translate (cr, -floor (rect.x0) + 6., -floor (rect.y0) + 6.);
	Doc->GetView ()->GetCanvas ()->Render (cr, false);
	int rowstride = cairo_image_surface_get_stride (surface);
	unsigned char *data = cairo_image_surface_get_data (surface);
	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_data (data, GDK_COLORSPACE_RGB, TRUE, 8, w, h, rowstride, destroy_surface, surface);
	go_cairo_convert_data_to_pixbuf (data, NULL, w, h, rowstride);
	cairo_destroy (cr);

	map<string, Object*>::iterator i;
	gcp::Molecule *Mol = dynamic_cast<gcp::Molecule*> (pDoc->GetFirstChild (i));
	char const *InChI = Mol->GetInChI ().c_str ();
	GOutputStream *output = G_OUTPUT_STREAM (g_file_create (file, G_FILE_CREATE_NONE, NULL, &error));
	if (!error) {
		vector<char*> keys, values;
		char const *author = static_cast<gcp::Document*> (pDoc)->GetAuthor ();
		// We need to be sure that the author name can be converted to latin-1
		// otherwise, pixbuf export will fail, but it still must be passed as UTF-8
		char *latin1_author = g_convert (author, strlen (author), "ISO-8859-1", "UTF-8", NULL, NULL, NULL);
		if (latin1_author) {
			keys.push_back (const_cast<char*> ("tEXt::Author"));
			values.push_back (const_cast<char*> (author));
			g_free (latin1_author);
		}
		keys.push_back (const_cast<char*> ("tEXt::Copyright"));
		values.push_back (const_cast<char*> ("Public domain"));
		keys.push_back (const_cast<char*> ("tEXt::InChI"));
		values.push_back (const_cast<char*> (InChI));
		keys.push_back (reinterpret_cast<char*> (NULL));
		values.push_back (reinterpret_cast<char*> (NULL));
		gdk_pixbuf_save_to_callbackv (pixbuf, do_save_image, output, "png", &keys[0], &values[0], &error);
		g_output_stream_close (output, NULL, NULL); // hope there will be no error there
	}
	if (error) {
		cerr << _("Unable to save image file: ") << error->message << endl;
		g_error_free (error);
	}
	g_object_unref (file);
	g_object_unref (pixbuf);

	g_free (filename2);
	return false;
}

static WikipediaApp *pApp;

gcpWikipediaTool::gcpWikipediaTool (gcp::Application *App): gcp::Tool (App, "Wikipedia")
{
	pApp = new WikipediaApp (App);
}

gcpWikipediaTool::~gcpWikipediaTool ()
{
	delete pApp;
}

bool gcpWikipediaTool::OnClicked ()
{
	gcp::Molecule *pMol = dynamic_cast<gcp::Molecule*> (m_pObject->GetMolecule ());
	if (!pMol)
		return false;
	gcp::Document *pDoc = new gcp::Document (NULL, true),
		*OrigDoc = static_cast<gcp::Document*> (pMol->GetDocument ());
	pDoc->GetView ()->CreateNewWidget ();
	gcp::Theme *pTheme = gcp::TheThemeManager.GetTheme ("Wikipedia");
	pDoc->SetTheme (pTheme);
	xmlDocPtr xml = xmlNewDoc ((xmlChar*) "1.0");
	if (!xml)
		return false; // should not happen, but we need something better 
	xmlDocSetRootElement (xml, xmlNewDocNode (xml, NULL, (xmlChar*) "chemistry", NULL));
	xmlNsPtr ns = xmlNewNs (xml->children, (xmlChar*) "http://www.nongnu.org/gchempaint", (xmlChar*) "gcp");
	xmlSetNs (xml->children, ns);
	//FIXME: implement exception handling
	xmlNodePtr child = pMol->Save (xml);
	if (child)
		xmlAddChild (xml->children, child);

	pDoc->Load (xml->children);
	xmlFree (xml);
	pDoc->SetAuthor (OrigDoc->GetAuthor ());

	list<string> mimes;
	mimes.push_front ("image/png");
	new FileChooser (pApp, true, mimes, pDoc, NULL, NULL);
	delete pDoc;
	return false;
}
