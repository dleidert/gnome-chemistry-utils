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

enum {
	UNKNOWN,
	VRML,
	JPEG,
	PNG
};

bool gc3dApplication::FileProcess (const gchar* filename, const gchar* mime_type, bool bSave, GtkWindow *window, Document *Doc)
{
	gc3dDocument *pDoc = dynamic_cast <gc3dDocument *> (Doc);
	if(bSave) {
		if (!mime_type)
			mime_type = "";
		int type = UNKNOWN;
		if (!strcmp (mime_type, "image/png"))
			type = PNG;
		else if (!strcmp (mime_type, "image/jpeg"))
			type = JPEG;
		else if (!strcmp (mime_type, "model/vrml"))
			type = VRML;
		if (type == UNKNOWN) {
			GtkWidget* message = gtk_message_dialog_new (window, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, 
														_("Sorry, format not supported!"));
			gtk_dialog_run (GTK_DIALOG (message));
			gtk_widget_destroy (message);
			return true;
		}
		char *filename2, *ext = "";
		switch (type) {
		case VRML:
			ext = ".wrl";
			break;
		case JPEG:
			ext = ".jpg";
			break;
		case PNG:
			ext = ".png";
			break;
		}
		int i = strlen (filename) - strlen (ext);
		if ((i > 0) && (!strcmp (filename +i, ext)))
			filename2 = g_strdup (filename);
		else
			filename2 = g_strdup_printf ("%s%s", filename, ext);
		GnomeVFSURI *uri = gnome_vfs_uri_new (filename2);
		bool err = gnome_vfs_uri_exists (uri);
		gnome_vfs_uri_unref (uri);
		gint result = GTK_RESPONSE_YES;
		if (err) {
			gchar * message = g_strdup_printf (_("File %s\nexists, overwrite?"), filename2);
			GtkDialog* Box = GTK_DIALOG (gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, message));
			result = gtk_dialog_run (Box);
			gtk_widget_destroy (GTK_WIDGET (Box));
			g_free (message);
			if (result == GTK_RESPONSE_YES)
				gnome_vfs_unlink (filename2);
			else {
				g_free (filename2);
				return true;
			}
		}
		map <string, string> options; // not used at the moment
		if (result == GTK_RESPONSE_YES)
			switch (type) {
			case VRML:
				pDoc->OnExportVRML (filename2);
				break;
			case JPEG:
				pDoc->GetView ()->SaveAsImage (filename, "jpeg", options, GetImageResolution ());
				break;
			case PNG:
				pDoc->GetView ()->SaveAsImage (filename, "png", options, GetImageResolution ());
				break;
			}
		g_free (filename2);
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
	l.push_front ("model/vrml");
	FileChooser (this, true, l, Doc, _("Save as image"), GetImageResolutionWidget ());
}
