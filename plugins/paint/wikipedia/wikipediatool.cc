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
#include <gcu/application.h>
#include <gcu/filechooser.h>
#include <canvas/structs.h>
#include <gio/gio.h>
#include <glib/gi18n-lib.h>
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

bool WikipediaApp::FileProcess (const gchar* filename, const gchar* mime_type, bool bSave, GtkWindow *window, gcu::Document *pDoc)
{
	gcp::Document *Doc = static_cast<gcp::Document*> (pDoc);
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
		gchar * message = g_strdup_printf (_("File %s\nexists, overwrite?"), filename2);
		GtkDialog* Box = GTK_DIALOG (gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, message));
		gtk_window_set_icon_name (GTK_WINDOW (Box), "gchempaint");
		result = gtk_dialog_run (Box);
		gtk_widget_destroy (GTK_WIDGET (Box));
		g_free (message);
	}
	if (result == GTK_RESPONSE_YES) {
		if (err) {
			// destroy the old file
			g_file_delete (file, NULL, &error);
			if (error) {
				gchar * message = g_strdup_printf (_("Error while processing %s:\n%s"), filename2, error->message);
				g_error_free (error);
				GtkDialog* Box = GTK_DIALOG (gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, message));
				gtk_window_set_icon_name (GTK_WINDOW (Box), "gchempaint");
				result = gtk_dialog_run (Box);
				gtk_widget_destroy (GTK_WIDGET (Box));
				g_free (message);
				g_object_unref (file);
				return true;
			}
		}
	} else
		return true;
/*	ArtDRect rect;
	GnomeCanvas *canvas = GNOME_CANVAS (Doc->GetWidget ());
	gcp::WidgetData *pData = static_cast <gcp::WidgetData*> (g_object_get_data (G_OBJECT (canvas), "data"));
	pData->GetObjectBounds (Doc, &rect);
	int x, y, w, h;
	x = static_cast<int> (rect.x0);
	y = static_cast<int> (rect.y0);
	w = static_cast<int> (rect.x1 - rect.x0) + 36;
	h = static_cast<int> (rect.y1 - rect.y0) + 36;
	GdkPixbuf *pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, false, 8, w, h);
	gdk_pixbuf_fill (pixbuf, 0xffffffff);
	GnomeCanvasBuf buf;
	buf.buf = gdk_pixbuf_get_pixels (pixbuf);
	buf.rect.x0 = (int) floor (rect.x0) - 18;
	buf.rect.x1 = (int) ceil (rect.x1);
	buf.rect.y0 = (int) floor (rect.y0) - 18;
	buf.rect.y1 = (int) ceil (rect.y1);
	buf.buf_rowstride = gdk_pixbuf_get_rowstride (pixbuf);
	buf.bg_color = 0xffffff;
	buf.is_buf = 1;
	(* GNOME_CANVAS_ITEM_GET_CLASS (pData->Group)->render) (GNOME_CANVAS_ITEM (pData->Group), &buf);

	GdkPixbuf *alpha = gdk_pixbuf_add_alpha (pixbuf, false, 0, 0, 0);
	g_object_unref (pixbuf);
	// Now make it transparent
	int row, col, rowstride = gdk_pixbuf_get_rowstride (alpha) / 4;;
	guint32 *color, *line;
	line = color = reinterpret_cast<guint32*> (gdk_pixbuf_get_pixels (alpha));
	for (row = 0; row < h; row++) {
		for (col = 0; col < w ; col++) {
			*color = (~*color & 0xff) << 24;
			color++;
		}
		color = line += rowstride;
	}

	map<string, Object*>::iterator i;
	gcp::Molecule *Mol = dynamic_cast<gcp::Molecule*> (pDoc->GetFirstChild (i));
	char const *InChI = Mol->GetInChI ();
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
		gdk_pixbuf_save_to_callbackv (alpha, do_save_image, output, "png", keys.data (), values.data (), &error);
		g_output_stream_close (output, NULL, NULL); // hope there will be no error there
	}
	if (error) {
		cerr << _("Unable to save image file: ") << error->message << endl;
		g_error_free (error);
	}*/
	g_object_unref (file);
//	g_object_unref (alpha);

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
