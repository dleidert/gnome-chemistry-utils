// -*- C++ -*-

/* 
 * Gnome Crystal
 * application.cc 
 *
 * Copyright (C) 2001-2006 Jean Bréfort <jean.brefort@normalesup.org>
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

static unsigned short nNewDocs = 1;

gcApplication::gcApplication(): Application ("gcrystal-unstable", DATADIR)
{
	// get default programs
	GConfClient* cli = gconf_client_get_default ();
	if (cli) {
		const char *value;
		GConfEntry* entry = gconf_client_get_entry (cli, "/desktop/gnome/applications/browser/exec", NULL, true, NULL);
		if (entry) {
			value = gconf_value_get_string (gconf_entry_get_value (entry));
			if (value) WebBrowser = value;
		}
		entry = gconf_client_get_entry (cli, "/desktop/gnome/url-handlers/mailto/command", NULL, true, NULL);
		if (entry) {
			value = gconf_value_get_string (gconf_entry_get_value (entry));
			if (value) {
				MailAgent = value;
				int i = MailAgent.find (" %s");
				if (i)
					MailAgent.erase (i, MailAgent.size ());
			}
		}
	}
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
	list<char const*> l;
	l.push_front ("application/x-gcrystal");
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
	list<char const*> l;
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

void gcApplication::OnExportJPEG ()
{
	if (!m_pActiveDoc)
		return;
	list<char const*> l;
	l.push_front ("image/jpeg");
	FileChooser (this, true, l, m_pActiveDoc, _("Export model as a Jpeg file..."));
}

void gcApplication::OnExportPNG ()
{
	if (!m_pActiveDoc)
		return;
	list<char const*> l;
	l.push_front ("image/png");
	FileChooser (this, true, l, m_pActiveDoc, _("Export model as a PNG file..."));
}

void gcApplication::OnExportVRML ()
{
	if (!m_pActiveDoc)
		return;
	list<char const*> l;
	l.push_front ("image/vrml");
	FileChooser (this, true, l, m_pActiveDoc, _("Export model as a VRML file..."));
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
		if (!pDoc->IsEmpty () || pDoc->IsDirty ())
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
	VRML,
	JPEG,
	PNG
};

bool gcApplication::FileProcess (const gchar* filename, const gchar* mime_type, bool bSave, GtkWindow *window, Document *pDoc)
{
	gcDocument *Doc = static_cast<gcDocument*> (pDoc);
	if (!mime_type)
		mime_type = "application/x-gcrystal";
	if (bSave) {
		int type = GCRYSTAL;
		if (!strcmp (mime_type, "image/png"))
			type = PNG;
		else if (!strcmp (mime_type, "image/jpeg"))
			type = JPEG;
		else if (!strcmp (mime_type, "image/vrml"))
			type = VRML;
		char *filename2, *ext = "";
		switch (type) {
		case GCRYSTAL:
			ext = ".gcrystal";
			break;
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
			gtk_window_set_icon_name (GTK_WINDOW (Box), "gcrystal");
			result = gtk_dialog_run (Box);
			gtk_widget_destroy (GTK_WIDGET (Box));
			g_free (message);
			if (result == GTK_RESPONSE_YES)
				gnome_vfs_unlink (filename2);
		}
		map <string, string> options; // not used at the moment
		if (result == GTK_RESPONSE_YES)
			switch (type) {
			case GCRYSTAL:
				Doc->SetFileName (filename2);
				Doc->Save ();
				break;
			case VRML:
				Doc->OnExportVRML (filename2);
				break;
			case JPEG:
				Doc->SaveAsImage (filename2, "jpeg", options);
				break;
			case PNG:
				Doc->SaveAsImage (filename2, "png", options);
				break;
			}
		g_free (filename2);
	} else {
		if (strcmp (mime_type, "application/x-gcrystal"))
			return true;
		if (!Doc)
			Doc = GetDoc (filename);
		if (!Doc)
			Doc = OnFileNew ();
		if (Doc->GetFileName () && !strcmp (Doc->GetFileName(), filename)) {
			if (!Doc->IsDirty ())
				return true;
			else {
				gchar* str = g_strdup_printf (_("\"%s\" has been modified since last saving. Do you wish to come back to saved version?"), Doc->GetTitle ());
				GtkWidget* mbox = gtk_message_dialog_new (window, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, str);
				int res = gtk_dialog_run (GTK_DIALOG (mbox));
				if (res != GTK_RESPONSE_YES)
					return true;
			}
		}
		if (Doc->Load (filename)) {
			// TODO: change titles in every window
	/*		gtk_label_set_text (pView->GetLabel (), pDoc->GetTitle ());
			GtkLabel *pLabel = pView->GetMenuLabel ();
			if (pLabel)
				gtk_label_set_text (pLabel, pDoc->GetTitle ());*/
		}
	}
	return false;
}

void gcApplication::OnBug ()
{
	if (!WebBrowser.size ())
		return;
	char *argv[3] = {NULL, PACKAGE_BUGREPORT, NULL};
	argv[0] = (char*) WebBrowser.c_str();
	g_spawn_async (NULL, argv, NULL, G_SPAWN_SEARCH_PATH,
		NULL, NULL, NULL, NULL);
}

void gcApplication::ShowURI (string& uri)
{
	if (!WebBrowser.size ())
		return;
	char *argv[3] = {NULL, NULL, NULL};
	argv[0] = (char*) WebBrowser.c_str();
	argv[1] = (char*) uri.c_str ();
	g_spawn_async (NULL, argv, NULL, G_SPAWN_SEARCH_PATH,
		NULL, NULL, NULL, NULL);
}

void gcApplication::OnWeb ()
{
	if (!WebBrowser.size ())
		return;
	char *argv[3] = {NULL, "http://gchemutils.nongnu.org/", NULL};
	argv[0] = (char*) WebBrowser.c_str();
	g_spawn_async (NULL, argv, NULL, G_SPAWN_SEARCH_PATH,
		NULL, NULL, NULL, NULL);
}

void gcApplication::OnMail ()
{
	if (!MailAgent.size ())
		return;
	char *argv[3] = {NULL, "mailto:gchemutils-main@nongnu.org", NULL};
	argv[0] = (char*) MailAgent.c_str();
	g_spawn_async (NULL, argv, NULL, G_SPAWN_SEARCH_PATH,
		NULL, NULL, NULL, NULL);
}

void gcApplication::RemoveDocument (gcDocument *pDoc)
{
	m_Docs.remove (pDoc);
	if (m_Docs.size () == 0)
		gtk_main_quit ();
}

bool gcApplication::OnQuit ()
{
	while (m_Docs.size () > 1) {
		m_pActiveDoc = m_Docs.front ();
		if (!OnFileClose ())
			return false;
	}
	return true;
}
