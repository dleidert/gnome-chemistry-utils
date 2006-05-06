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
#include "view-settings.h"
#include "window.h"
std::list<gcApplication*> Apps;

static unsigned short nNewDocs = 1;
guint TabPos =  0;

//Callbacks
/*
static void on_file_new (GtkWidget *widget, gcApplication *app)
{
	app->OnFileNew ();
}

static void on_file_open (GtkWidget *widget, gcApplication *app)
{
	app->OnFileOpen ();
}

static void on_file_open_new_win (GtkWidget *widget, gcApplication *app)
{
	gcApplication* pApp = new gcApplication ();
	Apps.push_back (pApp);
	pApp->OnFileOpen ();
}

static void on_file_save (GtkWidget *widget, gcApplication *app)
{
	app->OnFileSave ();
}

static void on_file_save_as (GtkWidget *widget, gcApplication *app)
{
	app->OnFileSaveAs ();
}

static void on_file_close (GtkWidget *widget, gcApplication *app)
{
	app->OnFileClose ();
}

static void on_file_print (GtkWidget *widget, gcApplication *app)
{
	app->OnFilePrint ();
}

static void on_export_jpeg (GtkWidget *widget, gcApplication *app)
{
	app->OnExportJPEG ();
}

static void on_export_png (GtkWidget *widget, gcApplication *app)
{
	app->OnExportPNG ();
}

static void on_export_vrml (GtkWidget *widget, gcApplication *app)
{
	app->OnExportVRML ();
}

static void on_view_new (GtkWidget *widget, gcApplication *app)
{
	app->OnViewNew ();
}

static void on_view_close (GtkWidget *widget, gcApplication *app)
{
	app->OnViewClose ();
}

static void on_help (GtkWidget *widget, gcApplication *App)
{
	App->OnHelp ();
}

static void on_about (GtkWidget *widget, void *data)
{
	char * authors[] = {"Jean Bréfort", NULL};
//	char * documentors[] = {NULL};
	char * artists[] = {"Nestor Diaz", NULL};
	char license[] = "This program is free software; you can redistribute it and/or\n" 
		"modify it under the terms of the GNU General Public License as\n"
 		"published by the Free Software Foundation; either version 2 of the\n"
		"License, or (at your option) any later version.\n\n"
		"This program is distributed in the hope that it will be useful,\n"
		"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
		"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
		"GNU General Public License for more details.\n\n"
		"You should have received a copy of the GNU General Public License\n"
		"along with this program; if not, write to the Free Software\n"
		"Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02111-1307\n"
		"USA";*/
/* Note to translators: replace the following string with the appropriate credits for you lang */
/*	char *translator_credits = _("translator_credits");
	gtk_show_about_dialog (NULL,
					"name", _("Gnome Crystal"),
					"authors", authors,
					"artists", artists,
					"comments", _("Gnome Crystal is a lightweight crystal structures viewer for Gnome"),
					"copyright", _("(C) 1999-2006 by Jean Bréfort"),
					"license", license,
					"translator_credits", strcmp (translator_credits, "translator_credits") != 0 ? 
											(const char *)translator_credits : NULL,
					"version", VERSION,
					"website", "http://www.nongnu.org/gchemutils",
					NULL);
}

static void on_view_settings (GtkWidget *widget, gcApplication *app)
{
	if (app)
		app->OnViewSettings ();
}

extern std::list<gcDocument*> Docs;

static bool on_quit (GtkWidget *widget, gcApplication *app)
{
	while (Apps.size()) {
		if (Apps.front ()->OnFileClose ())
			while (gtk_events_pending ())
				gtk_main_iteration ();
		else
			break;
	}
	if (Docs.size ())
		return false;
	delete Apps.front ();
	Apps.pop_front ();
	if  (Apps.size() != 0)
		return false;
	gtk_main_quit ();
	return true;
}

static bool on_delete (GtkWidget* widget, GdkEventAny *event, gcApplication *app)
{
	while (app->OnViewClose ())
		while (gtk_events_pending ())
			gtk_main_iteration ();
	std::list<gcApplication*>::iterator i;
	for (i = Apps.begin (); i != Apps.end (); i++)
	{
		if (app == *i) {
			if (!app->IsEmpty ())
				return true;
			Apps.remove (app);
			delete app;
			return false;
		}
	}
	return false;
}

static void on_destroy (GtkWidget* widget, gcApplication *app)
{
	if (Apps.empty ())
		gtk_main_quit ();
}

static void on_prefs (GtkWidget* widget, gcApplication *app)
{
	new gcPrefsDlg (app);
}

//Helper functions
static bool do_load (const gchar* filename, gcApplication *pApp, gcDocument* pDoc)
{
	pDoc = pApp->GetDoc (filename);
	if (pDoc->GetFileName () && !strcmp (pDoc->GetFileName(), filename)) {
		if (!pDoc->IsDirty ())
			return true;
		else {
			gchar* str = g_strdup_printf (_("\"%s\" has been modified since last saving. Do you wish to come back to saved version?"), pDoc->GetTitle ());
			GtkWidget* mbox = gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, str);
			int res = gtk_dialog_run (GTK_DIALOG (mbox));
			if (res != GTK_RESPONSE_YES)
				return true;
		}
	}
	if (pDoc->Load (filename)) {*/
/*		gtk_label_set_text (pView->GetLabel (), pDoc->GetTitle ());
		GtkLabel *pLabel = pView->GetMenuLabel ();
		if (pLabel)
			gtk_label_set_text (pLabel, pDoc->GetTitle ());*/
/*		return true;
	}
	nNewDocs++;
	return false;
}

void do_save_as (const gchar* filename, gcDocument *pDoc)
{
	pDoc->SetFileName (filename);
	pDoc->Save ();
}

void do_export_jpeg (const gchar *filename, gcView* pView)
{
	if (!pView)
		return;
	pView->ExportJPG (filename);
}

void do_export_png(const gchar *filename, gcView* pView)
{
	if (!pView)
		return;
	pView->ExportPNG (filename);
}

void do_export_vrml (const gchar *filename, gcView* pView)
{
	gcDocument *pDoc = pView->GetDocument ();
	if (pDoc)
		pDoc->OnExportVRML (filename, pView);;
}

bool RequestApp (gcView* pView)
{
	gcDocument* pDoc = pView->GetDocument ();
	gcApplication* pApp = NULL;
	std::list<gcApplication*>::iterator i;
	for (i = Apps.begin(); i!= Apps.end(); i++) {
		if (!(*i)->HasDocument (pDoc)) {
			pApp = *i;
			break;
		}
	}
	if (!pApp) {
		pApp = new gcApplication ();
		Apps.push_back (pApp);
	}
	if (pApp) {
		pApp->AddView (pView);
		gtk_label_set_text (pView->GetLabel (), pDoc->GetTitle ());
		GtkLabel *pLabel = pView->GetMenuLabel ();
		if (pLabel)
			gtk_label_set_text (pLabel, pDoc->GetTitle ());
	}
	return (pApp != NULL);
}*/

gcApplication::gcApplication(): Application ("gcrystal-unstable", DATADIR)
{
	// get default programs
	GConfClient* cli = gconf_client_get_default ();
	if (cli) {
		const char *value;
/*		GConfEntry* entry = gconf_client_get_entry (cli, "/desktop/gnome/applications/help_viewer/exec", NULL, true, NULL);
		if (entry) {
			value = gconf_value_get_string (gconf_entry_get_value (entry));
			if (value) HelpBrowser = value;
		}*/
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
//	m_bFileOpening = false;
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
	if (!m_pActiveDoc)
		return;
	list<char const*> l;
	l.push_front ("application/x-gcrystal");
	FileChooser (this, true, l, m_pActiveDoc);
}

bool gcApplication::OnFileClose ()
{
/*	if (m_pActiveView == NULL) {
		if (m_Views.empty () && (Apps.size () > 1)) {
			Apps.remove (this);
			gtk_widget_destroy (GTK_WIDGET (m_Window));
			return true;
		}
		else
			return false;
	}*/
/*	if (m_pActiveView->IsLocked ())
		return false;
	gcDocument* pDoc = m_pActiveView->GetDocument ();
	if (!pDoc->VerifySaved ())
		return false;
	std::list<gcApplication*>::iterator i, iend = Apps.end ();
	std::list<gcView*>::iterator j, jend;
	bool bEnded;
	do {
		bEnded = true;
		for (i = Apps.begin(); i != iend; i++) {
			if (*i == this)
				continue;
			jend = (*i)->m_Views.end ();
			for (j = (*i)->m_Views.begin (); j != jend; j++)
				if ((*j)->GetDocument () == pDoc) {
					(*i)->OnViewClose (*j);
					bEnded = false;
					break;
				}
			if (!bEnded)
				break;
		}
	} while (!bEnded);
	OnViewClose (m_pActiveView);
	m_Docs.remove (pDoc);*/
	return true;
}

void gcApplication::OnViewSettings ()
{
/*	if (!m_pActiveView)
		return;
	new gcViewSettingsDlg (m_pActiveView);*/
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
//	m_pActiveView->Print (pc, width, height);
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

void gcApplication::OnViewNew ()
{
	if (!m_pActiveDoc)
		return;
/*	gcView* pView = new gcView (m_pActiveView);
	gcDocument* pDoc = m_pActiveView->GetDocument ();
	//Try to add the new view to an existing gcApplication
	gcApplication* pApp = NULL;
	std::list<gcApplication*>::iterator i, iend = Apps.end ();
	for (i = Apps.begin (); i!= iend; i++) {
		if (*i == this)
			continue;
		if (!(*i)->HasDocument (pDoc)) {
			pApp = *i;
			break;
		}
	}
	if (!pApp) {
		pApp = new gcApplication ();
		Apps.push_back (pApp);
	}
	if (pApp) {
		pApp->AddView (pView);
		pDoc->SetDirty ();
		gtk_label_set_text (pView->GetLabel (), pDoc->GetTitle ());
		GtkLabel *pLabel = pView->GetMenuLabel ();
		if (pLabel)
			gtk_label_set_text (pLabel, pDoc->GetTitle ());
	}
	else
		delete pView;*/
}

bool gcApplication::OnViewClose (gcView* pView)
{
/*	if (pView == NULL)
		pView = m_pActiveView;
	if (pView == NULL)
		return false;
	gcDocument* pDoc = pView->GetDocument ();
	gint i = 0;
	GtkWidget* w;
	if (pView == m_pActiveView)
		i = gtk_notebook_get_current_page(m_Notebook);
	else
		do {
		w = gtk_notebook_get_nth_page (m_Notebook, i);
		if (w == NULL)
			return false; //should not occur
		if (pView == ((gcView*) g_object_get_data (G_OBJECT (w), "view")))
			break;
		else
			i++;
	} while (w);
	if (!pDoc->RemoveView (pView))
		return false;
	m_Views.remove (pView);
	gtk_notebook_remove_page (m_Notebook, i);
	delete pView;
	if (pView == m_pActiveView) {
		i = gtk_notebook_get_current_page (m_Notebook);
		w = gtk_notebook_get_nth_page (m_Notebook, i);
		m_pActiveView = (w)? (gcView*) g_object_get_data(G_OBJECT(w), "view"): NULL;
	}
	if (m_Views.empty () && (Apps.size () > 1)) {
		Apps.remove(this);
		gtk_widget_destroy (GTK_WIDGET (m_Window));
		delete this;
	}*/
	return true;
}

bool gcApplication::HasDocument (gcDocument* pDoc)
{
	std::list<gcView*>::iterator i, iend = m_Views.end ();
	for (i = m_Views.begin(); i != iend; i++)
		if (pDoc == (*i)->GetDocument ())
			return true;
	return false;
}

void gcApplication::AddView (gcView* pView)
{
/*	m_Views.push_back (pView);
	m_pActiveView = pView;*/
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
		int i = strlen(filename) - strlen (ext);
		if ((i > 0) && (!strcmp(filename +i, ext)))
			filename2 = g_strdup(filename);
		else
			filename2 = g_strdup_printf("%s%s", filename, ext);
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
		}
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
				break;
			case PNG:
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
