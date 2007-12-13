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
#include <libgnomevfs/gnome-vfs-ops.h>
#include <libgnomevfs/gnome-vfs-utils.h>
#include <glib/gi18n.h>
#include <cstring>

gc3dApplication::gc3dApplication (Display3DMode display3d, char const *bg):
	Application (_("GChem3D Viewer"), DATADIR, "gchem3d-viewer"),
	m_Display3D (display3d)
{
	if (bg) {
		if (!strcmp (bg, "black")) {
			m_Red = 0.;
			m_Green = 0.;
			m_Blue = 0.;
		} else if (!strcmp (bg, "white")) {
			m_Red = 0.;
			m_Green = 0.;
			m_Blue = 0.;
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
}

gc3dApplication::~gc3dApplication ()
{
}

gc3dDocument *gc3dApplication::OnFileNew ()
{
	gc3dDocument* Doc = new gc3dDocument (this);
	Doc->SetTitle (_("GChem3D Viewer"));
	Doc->SetDisplay3D (m_Display3D);
	GLView *view = Doc->GetView ();
	view->SetRed (m_Red);
	view->SetGreen (m_Green);
	view->SetBlue (m_Blue);
	new gc3dWindow (this, Doc);
	return Doc;
}

void gc3dApplication::OnFileOpen (gc3dDocument *Doc)
{
	list<string> l;
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
		bool supported = false, vrml = false;
		string filename2 = filename;
		char const *pixbuf_type = NULL;
		int i;
		if (mime_type) {
			if (!strcmp (mime_type, "model/vrml")) {
				supported = true;
				vrml = true;
				i = strlen (filename) - 4;
				if ((i <= 0) || (strcmp (filename + i, ".wrl")))
					filename2 += ".wrl";
			} else {
				pixbuf_type = GetPixbufTypeName (filename2, mime_type);
				supported = (pixbuf_type);
			}
		}
		if (!supported) {
			GtkWidget* message = gtk_message_dialog_new (window, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, 
														_("Sorry, format not supported!"));
			gtk_dialog_run (GTK_DIALOG (message));
			gtk_widget_destroy (message);
			return true;
		}
		GnomeVFSURI *uri = gnome_vfs_uri_new (filename2.c_str ());
		bool err = gnome_vfs_uri_exists (uri);
		gnome_vfs_uri_unref (uri);
		gint result = GTK_RESPONSE_YES;
		if (err) {
			gchar * message = g_strdup_printf (_("File %s\nexists, overwrite?"), filename2.c_str ());
			GtkDialog* Box = GTK_DIALOG (gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, message));
			result = gtk_dialog_run (Box);
			gtk_widget_destroy (GTK_WIDGET (Box));
			g_free (message);
			if (result == GTK_RESPONSE_YES)
				gnome_vfs_unlink (filename2.c_str ());
			else
				return true;
		}
		map <string, string> options; // not used at the moment
		if (result == GTK_RESPONSE_YES) {
			if (vrml)
				pDoc->OnExportVRML (filename2);
			else
				pDoc->GetView ()->SaveAsImage (filename2, pixbuf_type, options, GetImageWidth (), GetImageHeight ());
		}
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
		data.app_name = (char*) "gchem3d-viewer";
		data.app_exec = (char*) "gchem3d-viewer %u";
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
	list<string> l;
	map<string, GdkPixbufFormat*>::iterator i, end = m_SupportedPixbufFormats.end ();
	for (i = m_SupportedPixbufFormats.begin (); i != end; i++)
		l.push_front ((*i).first.c_str ());
	l.push_front ("model/vrml");
	FileChooser (this, true, l, Doc, _("Save as image"), GetImageSizeWidget ());
}
