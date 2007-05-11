// -*- C++ -*-

/* 
 * Gnome Crystal
 * application.cc 
 *
 * Copyright (C) 2001-2007 Jean Bréfort <jean.brefort@normalesup.org>
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include "config.h"
#include <gtk/gtk.h>
#include <glade/glade.h>
#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-job.h>
#include <libgnomeprintui/gnome-print-dialog.h>
#include <libgnomeprintui/gnome-print-job-preview.h>
#include <libgnomevfs/gnome-vfs-ops.h>
#include <libgnomevfs/gnome-vfs-utils.h>
#include "application.h"
#include "globals.h"
#include <gcu/filechooser.h>
#include "prefs.h"
#include "window.h"
#include <glib/gi18n.h>

static unsigned short nNewDocs = 1;

gcApplication::gcApplication(): Application ("gcrystal-unstable")
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
	pDoc->SetTitle (buf);
	m_Docs.push_back (pDoc);
	new gcWindow (this, pDoc);
	return pDoc;
}

void gcApplication::OnFileOpen ()
{
	list<string> l;
	l.push_front ("application/x-gcrystal");
	l.push_front ("chemical/x-cif");
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

void gcApplication::OnFilePrint ()
{
	GnomePrintConfig* config = gnome_print_config_default ();
	GnomePrintContext *pc;
	GnomePrintJob *gpj = gnome_print_job_new (config);
	int do_preview = 0, copies = 1, collate = 0;
	GnomePrintDialog *gpd;
	gpd = GNOME_PRINT_DIALOG (gnome_print_dialog_new (gpj, (const guchar*) "Print test", GNOME_PRINT_DIALOG_COPIES));
	gtk_window_set_icon_name (GTK_WINDOW (gpd), "gcrystal");
	gnome_print_dialog_set_copies (gpd, copies, collate);
	switch (gtk_dialog_run (GTK_DIALOG (gpd))) {
	case GNOME_PRINT_DIALOG_RESPONSE_PRINT:
		do_preview = 0;
		break;
	case GNOME_PRINT_DIALOG_RESPONSE_PREVIEW:
		do_preview = 1;
		break;
	case GNOME_PRINT_DIALOG_RESPONSE_CANCEL:
		gtk_widget_destroy (GTK_WIDGET (gpd));
		return;
	}
	gtk_widget_destroy (GTK_WIDGET (gpd));
	pc = gnome_print_job_get_context (gpj);
	gnome_print_beginpage (pc, (const guchar*) "");
	gdouble width, height;
	gnome_print_config_get_double (config, (const guchar*) GNOME_PRINT_KEY_PAPER_WIDTH, &width);
	gnome_print_config_get_double (config, (const guchar*) GNOME_PRINT_KEY_PAPER_HEIGHT, &height);
	m_pActiveDoc->GetActiveView ()->Print (pc, width, height);
	gnome_print_showpage (pc);
	g_object_unref (pc);
	gnome_print_job_close (gpj);
	if (do_preview) {
		GtkWidget *preview = gnome_print_job_preview_new (gpj, (const guchar*)_("Preview"));
		gtk_window_set_icon_name (GTK_WINDOW (preview), "gcrystal");
		gtk_widget_show (preview);
	} else {
		gnome_print_job_print (gpj);
	}
	g_object_unref (gpj);
	gnome_print_config_unref (config);
}

void gcApplication::OnSaveAsImage ()
{
	if (!m_pActiveDoc)
		return;
	list<string> l;
	map<string, GdkPixbufFormat*>::iterator i, end = m_SupportedPixbufFormats.end ();
	for (i = m_SupportedPixbufFormats.begin (); i != end; i++)
		l.push_front ((*i).first.c_str ());
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
	PIXBUF
};

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
		default:
			break;
		}
		if (ext) { 
			int i = strlen (filename) - strlen (ext);
			if ((i <= 0) || (strcmp (filename +i, ext)))
				filename2 += ext;
		}
		GnomeVFSURI *uri = gnome_vfs_uri_new (filename2.c_str ());
		bool err = gnome_vfs_uri_exists (uri);
		gnome_vfs_uri_unref (uri);
		gint result = GTK_RESPONSE_YES;
		if (err) {
			gchar * message = g_strdup_printf (_("File %s\nexists, overwrite?"), filename2.c_str ());
			GtkDialog* Box = GTK_DIALOG (gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, message));
			gtk_window_set_icon_name (GTK_WINDOW (Box), "gcrystal");
			result = gtk_dialog_run (Box);
			gtk_widget_destroy (GTK_WIDGET (Box));
			g_free (message);
			if (result == GTK_RESPONSE_YES)
				gnome_vfs_unlink (filename2.c_str ());
		}
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
		if ((type == GCRYSTAL)? Doc->Import (filename, mime_type):
						Doc->Import (filename, mime_type)) {
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
			char const *title = Doc->GetTitle ();
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
