// -*- C++ -*-

/* 
 * Gnome Crystal
 * application.cc 
 *
 * Copyright (C) 2001-2005
 *
 * Developed by Jean Bréfort <jean.brefort@normalesup.org>
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
#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#include <glade/glade.h>
#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-job.h>
#include <libgnomeprintui/gnome-print-dialog.h>
#include <libgnomeprintui/gnome-print-job-preview.h>
#include "application.h"
#include "globals.h"
#include "filesel.h"
#include "prefs.h"
#include "view-settings.h"
std::list<gcApplication*> Apps;

static unsigned short nNewDocs = 1;
guint TabPos =  0;

//Callbacks
static void on_file_new(GtkWidget *widget, gcApplication *app)
{
	app->OnFileNew();
}

static void on_file_open(GtkWidget *widget, gcApplication *app)
{
	app->OnFileOpen();
}

static void on_file_open_new_win(GtkWidget *widget, gcApplication *app)
{
	gcApplication* pApp = new gcApplication();
	Apps.push_back(pApp);
	pApp->OnFileOpen();
}

static void on_file_save(GtkWidget *widget, gcApplication *app)
{
	app->OnFileSave();
}

static void on_file_save_as(GtkWidget *widget, gcApplication *app)
{
	app->OnFileSaveAs();
}

static void on_file_close(GtkWidget *widget, gcApplication *app)
{
	app->OnFileClose();
}

static void on_file_print(GtkWidget *widget, gcApplication *app)
{
	app->OnFilePrint();
}

static void on_export_jpeg(GtkWidget *widget, gcApplication *app)
{
	app->OnExportJPEG();
}

static void on_export_png(GtkWidget *widget, gcApplication *app)
{
	app->OnExportPNG();
}

static void on_export_vrml(GtkWidget *widget, gcApplication *app)
{
	app->OnExportVRML();
}

static void on_view_new(GtkWidget *widget, gcApplication *app)
{
	app->OnViewNew();
}

static void on_view_close(GtkWidget *widget, gcApplication *app)
{
	app->OnViewClose();
}

void on_about()
{
	char * authors[] = {"Jean Bréfort", NULL};
	char * artists[] = {"Nestor Diaz", NULL};
	char * documentors[] = {NULL};
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
		"Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307\n"
		"USA";
/* Note to translators: replace the following string with the appropriate credits for you lang */
	char *translator_credits = _("translator_credits");
	GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file ( DATADIR"/pixmaps/gnome-crystal-logo.png", NULL);
	gtk_show_about_dialog (NULL,
					"name", "GChemPaint",
					"authors", authors,
					"artists", artists,
					"comments", _("GChemPaint is a 2D chemical structures editor for Gnome"),
					"copyright", _("(C) 2001-2005 by Jean Bréfort"),
					"license", license,
					"logo", pixbuf,
					"icon-name", "gchempaint",
					"translator_credits", strcmp (translator_credits, "translator_credits") != 0 ? 
											(const char *)translator_credits : NULL,
					"version", VERSION,
					"website", "http://www.nongnu.org/gchempaint",
					NULL);
	if (pixbuf != NULL)
		g_object_unref (pixbuf);
}

static void on_lattice(GtkWidget *widget, gcApplication *app)
{
	if (app) app->OnCrystalDefine(0);
}

static void on_atoms(GtkWidget *widget, gcApplication *app)
{
	if (app) app->OnCrystalDefine(1);
}

static void on_lines(GtkWidget *widget, gcApplication *app)
{
	if (app) app->OnCrystalDefine(2);
}

static void on_size(GtkWidget *widget, gcApplication *app)
{
	if (app) app->OnCrystalDefine(3);
}

static void on_cleavages(GtkWidget *widget, gcApplication *app)
{
	if (app) app->OnCrystalDefine(4);
}

static void on_view_settings(GtkWidget *widget, gcApplication *app)
{
	if (app) app->OnViewSettings();
}

extern std::list<gcDocument*> Docs;

static bool on_quit(GtkWidget *widget, gcApplication *app)
{
	while (Apps.size())
	{
		if (Apps.front()->OnFileClose())
			while (gtk_events_pending()) gtk_main_iteration();
		else break;
	}
	if (Docs.size()) return false;
	delete Apps.front();
	Apps.pop_front();
	if (Apps.size() != 0) return false;
	gtk_main_quit();
	return true;
}

static void on_select(GtkWidget* widget, gcApplication *app)
{
	GtkWidget* w = GTK_WIDGET(g_object_get_data(G_OBJECT(widget), "page"));
	app->OnSelectView(w);
}

static void on_change_page(GtkWidget* widget, GtkNotebook* book, int page, gcApplication *app)
{
	app->OnChangePage(page);
}

static bool on_delete(GtkWidget* widget, GdkEventAny *event, gcApplication *app)
{
	while (app->OnViewClose()) while (gtk_events_pending()) gtk_main_iteration();
	std::list<gcApplication*>::iterator i;
	for (i = Apps.begin(); i != Apps.end(); i++)
	{
		if (app == *i)
		{
			if (!app->IsEmpty()) return true;
			Apps.remove(app);
			delete app;
			return false;
		}
	}
	return false;
}

static void on_destroy(GtkWidget* widget, gcApplication *app)
{
	if (Apps.empty()) gtk_main_quit();
}

static void on_prefs(GtkWidget* widget, gcApplication *app)
{
	gcPrefsDlg* box = new gcPrefsDlg();
}

//Helper functions
static bool do_load(const gchar* filename, gcView* pView)
{
	gcApplication* pApp = pView->GetApp();
	pView = pApp->GetDocView(filename);
	gcDocument* pDoc = pView->GetDocument();
	pApp = pView->GetApp();
	if (pDoc->GetFileName() && !strcmp(pDoc->GetFileName(), filename))
	{
		pApp->SelectView(pView);
		if (!pDoc->IsDirty()) return true;
		else
		{
			gchar* str = g_strdup_printf(_("\"%s\" has been modified since last saving. Do you wish to come back to saved version?"), pDoc->GetTitle());
			GtkWidget* mbox = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, str);
			int res = gtk_dialog_run(GTK_DIALOG(mbox));
			if (res != GTK_RESPONSE_YES) return true;
		}
	}
	if (pDoc->Load(filename))
	{
		gtk_label_set_text(pView->GetLabel(), pDoc->GetTitle());
		GtkLabel *pLabel = pView->GetMenuLabel();
		if (pLabel) gtk_label_set_text(pLabel, pDoc->GetTitle());
		return true;
	}
	nNewDocs++;
	return false;
}

void do_save_as(const gchar* filename, gcView* pView)
{
	gcDocument *pDoc = pView->GetDocument();
	pDoc->SetFileName(filename);
	pDoc->Save();
}

void do_export_jpeg(const gchar *filename, gcView* pView)
{
	if (!pView) return;
	pView->ExportJPG(filename);
}

void do_export_png(const gchar *filename, gcView* pView)
{
	if (!pView) return;
	pView->ExportPNG(filename);
}

void do_export_vrml(const gchar *filename, gcView* pView)
{
	gcDocument *pDoc = pView->GetDocument();
	if (pDoc) pDoc->OnExportVRML(filename, pView);;
}

bool RequestApp(gcView* pView)
{
	gcDocument* pDoc = pView->GetDocument();
	gcApplication* pApp = NULL;
	std::list<gcApplication*>::iterator i;
	for (i = Apps.begin(); i!= Apps.end(); i++)
	{
		if (!(*i)->HasDocument(pDoc))
		{
			pApp = *i;
			break;
		}
	}
	if (!pApp)
	{
		pApp = new gcApplication();
		Apps.push_back(pApp);
	}
	if (pApp)
	{
		pApp->AddView(pView);
		gtk_label_set_text(pView->GetLabel(), pDoc->GetTitle());
		GtkLabel *pLabel = pView->GetMenuLabel();
		if (pLabel) gtk_label_set_text(pLabel, pDoc->GetTitle());
	}
	return (pApp != NULL);
}

//Implementation of gcApplication class

static GnomeUIInfo export_menu [] = {
	GNOMEUIINFO_ITEM_NONE(N_("VRML"), N_("Export to VRML"), on_export_vrml),
	GNOMEUIINFO_ITEM_NONE(N_("PNG"), N_("Export view to png file"), on_export_png),
	GNOMEUIINFO_ITEM_NONE(N_("Jpeg"), N_("Export view to jpeg file"), on_export_jpeg),
	GNOMEUIINFO_END
};

static GnomeUIInfo file_menu [] = {
	GNOMEUIINFO_MENU_NEW_ITEM(N_("_New File"), N_("Create a new file"), on_file_new, NULL),
	GNOMEUIINFO_MENU_OPEN_ITEM(on_file_open, NULL),
	{ GNOME_APP_UI_ITEM, N_("O_pen in a new window..."),
		N_("Open a file in a new window"), (gpointer)on_file_save_as, NULL, NULL, \
		GNOME_APP_PIXMAP_NONE, NULL, 'o', (GdkModifierType)(GDK_CONTROL_MASK | GDK_SHIFT_MASK), NULL },
	GNOMEUIINFO_MENU_SAVE_ITEM(on_file_save, NULL),
	GNOMEUIINFO_MENU_SAVE_AS_ITEM(on_file_save_as, NULL),
	GNOMEUIINFO_SEPARATOR,
	GNOMEUIINFO_SUBTREE_HINT(N_("_Export"), N_("Export data or graphics"), export_menu),
	GNOMEUIINFO_SEPARATOR,
	GNOMEUIINFO_MENU_PRINT_ITEM(on_file_print, NULL),
	GNOMEUIINFO_SEPARATOR,
	GNOMEUIINFO_MENU_CLOSE_ITEM(on_file_close, NULL),
	GNOMEUIINFO_MENU_EXIT_ITEM(on_quit, NULL),
	GNOMEUIINFO_END
};
 
static GnomeUIInfo edit_menu [] = {
	GNOMEUIINFO_MENU_PREFERENCES_ITEM(on_prefs, NULL),
	GNOMEUIINFO_END
};

static GnomeUIInfo crystal_menu [] = {
	GNOMEUIINFO_ITEM_NONE(N_("_Lattice..."), N_("Define the lattice"), on_lattice),
	GNOMEUIINFO_ITEM_NONE(N_("_Atoms..."), N_("Define the lattice"), on_atoms),
	GNOMEUIINFO_ITEM_NONE(N_("_Bonds and lines..."), N_("Add bonds and lines"), on_lines),
	GNOMEUIINFO_ITEM_NONE(N_("_Size..."), N_("Define size"), on_size),
	GNOMEUIINFO_ITEM_NONE(N_("_Cleavages..."), N_("Add cleavages to remove some planes"), on_cleavages),
	GNOMEUIINFO_END
};

static GnomeUIInfo view_menu [] = {
	GNOMEUIINFO_ITEM_NONE(N_("View _settings..."), N_("Choose background color and model position"), on_view_settings),
	GNOMEUIINFO_END
};
static GnomeUIInfo windows_menu [] = {
	GNOMEUIINFO_MENU_NEW_WINDOW_ITEM(on_view_new, NULL),
	GNOMEUIINFO_MENU_CLOSE_WINDOW_ITEM(on_view_close, NULL),
	GNOMEUIINFO_SEPARATOR,
	GNOMEUIINFO_END
};
 
static GnomeUIInfo help_menu [] = {
	GNOMEUIINFO_HELP((void*)"gnome-crystal"),
	GNOMEUIINFO_MENU_ABOUT_ITEM(on_about, NULL),
	GNOMEUIINFO_END
};

static GnomeUIInfo main_menu [] = {
	GNOMEUIINFO_MENU_FILE_TREE(&file_menu),
	GNOMEUIINFO_MENU_EDIT_TREE(&edit_menu),
	GNOMEUIINFO_SUBTREE(N_("_Crystal"), &crystal_menu),
	GNOMEUIINFO_MENU_VIEW_TREE(&view_menu),
	GNOMEUIINFO_MENU_WINDOWS_TREE(&windows_menu),
	GNOMEUIINFO_MENU_HELP_TREE(&help_menu),
	GNOMEUIINFO_END
};

static GnomeUIInfo tool_bar [] = {
	GNOMEUIINFO_ITEM_STOCK(N_("New"), N_("Create a new file"), on_file_new, GTK_STOCK_NEW),
	GNOMEUIINFO_ITEM_STOCK(N_("Open"), N_("Open a file"), on_file_open, GTK_STOCK_OPEN),
	GNOMEUIINFO_ITEM_STOCK(N_("Save"), N_("Save active file"), on_file_save, GTK_STOCK_SAVE),
	GNOMEUIINFO_ITEM_STOCK(N_("Print"), N_("Print active view"), on_file_print, GTK_STOCK_PRINT),
	GNOMEUIINFO_ITEM_STOCK(N_("Close"), N_("Close active file"), on_file_close, GTK_STOCK_CLOSE),
	GNOMEUIINFO_ITEM_STOCK(N_("Quit"), N_("Quit Gnome Crystal"), on_quit, GTK_STOCK_QUIT),
	GNOMEUIINFO_END
};

gcApplication::gcApplication()
{
	m_App = gnome_app_new(_("Gnome Crystal"), _("Gnome Crystal"));
	gnome_app_create_menus_with_data((GnomeApp*)m_App, main_menu, this);
	gnome_app_create_toolbar_with_data((GnomeApp*)m_App, tool_bar, this);
	m_WindowsMenu = GTK_MENU(windows_menu[0].widget->parent);
	GtkWidget *bar = gnome_appbar_new(true, true, GNOME_PREFERENCES_ALWAYS);
	gnome_appbar_set_default(GNOME_APPBAR(bar), _("Ready."));
	gnome_app_set_statusbar((GnomeApp*)m_App, bar);
	gnome_app_install_appbar_menu_hints(GNOME_APPBAR(bar), main_menu);
	g_signal_connect(G_OBJECT(m_App), "delete_event", (GtkSignalFunc)on_delete, this);
	g_signal_connect(G_OBJECT(m_App), "destroy", G_CALLBACK(on_destroy), NULL);
	m_Notebook = GTK_NOTEBOOK(gtk_notebook_new());
	gtk_widget_set_size_request(GTK_WIDGET(m_Notebook), 300, 300);
	gnome_app_set_contents((GnomeApp*)m_App, GTK_WIDGET(m_Notebook));
	UpdateConfig();
	gtk_widget_show_all(m_App);
}

gcApplication::~gcApplication()
{
}

void gcApplication::OnFileNew()
{
	gcDocument* pDoc = GetNewDocument();
	gchar tmp[32];
	g_snprintf(tmp, sizeof(tmp), _("Untitled%d"), nNewDocs++);
	pDoc->SetTitle(tmp);
	gcView* pView = (gcView*)pDoc->GetView();
	AddView(pView);
	m_bFileOpening = false;
}

void gcApplication::OnFileOpen()
{
	gcView* pView;
	gcDocument* pDoc;
	if (m_Views.empty())
	{
		OnFileNew();
		m_bFileOpening = true;
	}
	pView = m_Views.back();
	gcFileSel* FileSel = new gcFileSel(_("Load a model..."), (void (*)(const    gchar*, gcView*))do_load, false, ".gcrystal", pView);
}

void gcApplication::OnFileSave()
{
	if (!m_pActiveView) return;
	gcDocument* pDoc = m_pActiveView->GetDocument();
	if (!pDoc) return;
	if (pDoc->GetFileName()) pDoc->Save();
	else OnFileSaveAs();
}

void gcApplication::OnFileSaveAs()
{
	if (!m_pActiveView) return;
	gcDocument* pDoc = m_pActiveView->GetDocument();
	if (!pDoc) return;
	gcFileSel* FileSel = new gcFileSel(_("Save model as..."), do_save_as, true, ".gcrystal", m_pActiveView);
}

bool gcApplication::OnFileClose()
{
	if (m_pActiveView == NULL)
	{
		if (m_Views.empty() && (Apps.size() > 1))
		{
			Apps.remove(this);
			gtk_widget_destroy(m_App);
			return true;
		}
		else return false;
	}
	if (m_pActiveView->IsLocked()) return false;
	gcDocument* pDoc = m_pActiveView->GetDocument();
	if (!pDoc->VerifySaved()) return false;
	std::list<gcApplication*>::iterator i;
	std::list<gcView*>::iterator j;
	gcView *pView;
	bool bEnded;
	do
	{
		bEnded = true;
		for (i = Apps.begin(); i != Apps.end(); i++)
		{
			if (*i == this) continue;
			for (j = (*i)->m_Views.begin(); j != (*i)->m_Views.end(); j++)
				if ((*j)->GetDocument() == pDoc)
				{
					(*i)->OnViewClose(*j);
					bEnded = false;
					break;
				}
			if (!bEnded) break;
		}
	}
	while (!bEnded);
	OnViewClose(m_pActiveView);
	return true;
}

void gcApplication::OnSelectView(GtkWidget* widget)
{
	gint i;
	i = gtk_notebook_page_num(m_Notebook, widget);
	if(i != gtk_notebook_get_current_page(m_Notebook))
		gtk_notebook_set_current_page(m_Notebook, i);
}

void gcApplication::OnCrystalDefine(int page)
{
	gcDocument* pDoc = m_pActiveView->GetDocument();
	if (pDoc) pDoc->Define(page);
}

void gcApplication::OnViewSettings()
{
	if (!m_pActiveView) return;
	gcViewSettingsDlg* pDlg = new gcViewSettingsDlg(m_pActiveView);
}

void gcApplication::OnFilePrint()
{
	GnomePrintConfig* config = gnome_print_config_default();
	GnomePrintContext *pc;
	GnomePrintJob *gpj = gnome_print_job_new(config);
	int do_preview, copies = 1, collate = 0;
	GnomePrintDialog *gpd;
	gpd = GNOME_PRINT_DIALOG (gnome_print_dialog_new(gpj, (const guchar*)"Print test", GNOME_PRINT_DIALOG_COPIES));
	gtk_window_set_icon_name (GTK_WINDOW (gpd), "gcrystal");
	gnome_print_dialog_set_copies(gpd, copies, collate);
	switch (gtk_dialog_run(GTK_DIALOG(gpd)))
	{
	case GNOME_PRINT_DIALOG_RESPONSE_PRINT:
		do_preview = 0;
		break;
	case GNOME_PRINT_DIALOG_RESPONSE_PREVIEW:
		do_preview = 1;
		break;
	case GNOME_PRINT_DIALOG_RESPONSE_CANCEL:
		gtk_widget_destroy (GTK_WIDGET(gpd));
		return;
	}
	gtk_widget_destroy(GTK_WIDGET(gpd));
	pc = gnome_print_job_get_context (gpj);
	gnome_print_beginpage(pc, (const guchar*)"");
	gdouble width, height;
	gnome_print_job_get_page_size_from_config(config, &width, &height);
	m_pActiveView->Print(pc, width, height);
	gnome_print_showpage(pc);
	g_object_unref(pc);
	gnome_print_job_close(gpj);
	if (do_preview)
	{
		GtkWidget *preview = gnome_print_job_preview_new (gpj, (const guchar*)_("Preview"));
		gtk_window_set_icon_name (GTK_WINDOW (preview), "gcrystal");
		gtk_widget_show (preview);
	}
	else
	{
		gnome_print_job_print(gpj);
	}
	g_object_unref (gpj);
	gnome_print_config_unref(config);
}

void gcApplication::OnExportJPEG()
{
	if (!m_pActiveView) return;
	gcFileSel* FileSel = new gcFileSel(_("Export model as a Jpeg file..."), do_export_jpeg, true, ".jpg", m_pActiveView);
}

void gcApplication::OnExportPNG()
{
	if (!m_pActiveView) return;
	gcFileSel* FileSel = new gcFileSel(_("Export model as a PNG file..."), do_export_png, true, ".png", m_pActiveView);
}

void gcApplication::OnExportVRML()
{
	if (!m_pActiveView) return;
	gcFileSel* FileSel = new gcFileSel(_("Export model as a VRML file..."), do_export_vrml, true, ".wrl", m_pActiveView);
}

void gcApplication::OnViewNew()
{
	if (!m_pActiveView) return;
	gcView* pView = new gcView(m_pActiveView);
	gcDocument* pDoc = m_pActiveView->GetDocument();
	//Try to add the new view to an existing gcApplication
	gcApplication* pApp= NULL;
	std::list<gcApplication*>::iterator i;
	for (i = Apps.begin(); i!= Apps.end(); i++)
	{
		if (*i == this) continue;
		if (!(*i)->HasDocument(pDoc))
		{
			pApp = *i;
			break;
		}
	}
	if (!pApp)
	{
		pApp = new gcApplication();
		Apps.push_back(pApp);
	}
	if (pApp)
	{
		pApp->AddView(pView);
		pDoc->SetDirty();
		gtk_label_set_text(pView->GetLabel(), pDoc->GetTitle());
		GtkLabel *pLabel = pView->GetMenuLabel();
		if (pLabel) gtk_label_set_text(pLabel, pDoc->GetTitle());
	}
	else delete pView;
}

bool gcApplication::OnViewClose(gcView* pView)
{
	if (pView == NULL) pView = m_pActiveView;
	if (pView == NULL) return false;
	gcDocument* pDoc = pView->GetDocument();
	gint i = 0;
	GtkWidget* w;
	if (pView == m_pActiveView) i = gtk_notebook_get_current_page(m_Notebook);
	else do
	{
		w = gtk_notebook_get_nth_page(m_Notebook, i);
		if (w == NULL) return false; //should not occur
		if (pView == ((gcView*) g_object_get_data(G_OBJECT(w), "view"))) break;
		else i++;
	}
	while (w);
	if (!pDoc->RemoveView(pView)) return false;
	m_Views.remove(pView);
	gtk_notebook_remove_page(m_Notebook, i);
	delete pView;
	if (pView == m_pActiveView)
	{
		i = gtk_notebook_get_current_page(m_Notebook);
		w = gtk_notebook_get_nth_page(m_Notebook, i);
		m_pActiveView = (w) ? (gcView*) g_object_get_data(G_OBJECT(w), "view") : NULL;
	}
	if (m_Views.empty() && (Apps.size() > 1))
	{
		Apps.remove(this);
		gtk_widget_destroy(m_App);
		delete this;
	}
	return true;
}

bool gcApplication::HasDocument(gcDocument* pDoc)
{
	std::list<gcView*>::iterator i;
	for (i = m_Views.begin(); i != m_Views.end(); i++)
		if (pDoc == (*i)->GetDocument()) return true;
	return false;
}

void gcApplication::AddView(gcView* pView)
{
	pView->SetApp(this);
	GtkWidget* widget = pView->CreateNewWidget();
	m_Views.push_back(pView);
	m_pActiveView = pView;
	gchar* tmp = pView->GetDocument()->GetTitle();
	GtkWidget *label = gtk_label_new(tmp);
	pView->SetLabel(GTK_LABEL(label));
	GtkWidget* item = gtk_menu_item_new_with_label(tmp);
	pView->SetMenu(GTK_MENU_ITEM(item));
	g_object_set_data(G_OBJECT(item), "page", widget);
	g_object_set_data(G_OBJECT(widget), "menu", item);
	g_object_set_data(G_OBJECT(widget), "view", pView);
	gtk_widget_show(item);
	gtk_menu_shell_append((GtkMenuShell*)m_WindowsMenu, item);
	gtk_notebook_append_page(m_Notebook, widget, label);
	gtk_notebook_set_current_page(m_Notebook, g_list_length(m_Notebook->children) - 1);
	g_signal_connect(G_OBJECT(item), "activate", (GtkSignalFunc)on_select, this); 
}

bool gcApplication::LoadFile(char* filename)
{
	return do_load(filename, m_pActiveView);
}

void gcApplication::UpdateConfig()
{
	if (TabPos == 0) gtk_notebook_set_show_tabs(m_Notebook, false);
	else
	{
		gtk_notebook_set_show_tabs(m_Notebook, true);
		gtk_notebook_set_tab_pos(m_Notebook, (GtkPositionType)(TabPos - 1));
	}
}

gcView* gcApplication::GetDocView(const char* filename)
{
	gcDocument* pDoc;
	gcView *pView = NULL;
	std::list<gcView*>::iterator i;
	for (i = m_Views.begin(); i != m_Views.end(); i++)
	{
		pDoc = (*i)->GetDocument();
		if (!pDoc->GetFileName()) continue;
		if (!strcmp(pDoc->GetFileName(), filename))
		{
			pView = *i;
			break;
		}
	}
	if (!pView) //search doc in other apps
	{
		std::list<gcApplication*>::iterator j;
		for (j = Apps.begin(); j != Apps.end() && !pView; j++)
		{
			if (*j == this) continue;
			for (i = (*j)->m_Views.begin(); i != (*j)->m_Views.end(); i++)
			{
				pDoc = (*i)->GetDocument();
				if (!pDoc->GetFileName()) continue;
				if (!strcmp(pDoc->GetFileName(), filename))
				{
					pView = *i;
					GdkWindow *win = GTK_WIDGET((*j)->m_App)->window;
					gdk_window_hide(win);
					gdk_window_show(win);
					gdk_window_raise(win);
					break;
				}
			}
		}
	}
	if (pView) return pView;
	if (m_bFileOpening)
	{
		pView = m_Views.back();
		pDoc = pView->GetDocument();
		if (!pDoc->IsEmpty() || pDoc->IsDirty()) pView = NULL;
	}
	if (!pView)
	{
		OnFileNew();
		pView = m_Views.back();
	}
	nNewDocs--;
	return pView;
}

gcView* gcApplication::GetView(gcDocument* pDoc)
{
	std::list<gcView*>::iterator i;
	for (i = m_Views.begin(); i != m_Views.end(); i++)
		if (pDoc == (*i)->GetDocument()) return *i;
	return NULL;
}

void gcApplication::SelectView(gcView* pView)
{
	GtkWidget *w;
	guint i = 0;
	do
	{
		w = gtk_notebook_get_nth_page(m_Notebook, i);
		if (w == NULL) return; //should not occur
		if (pView == ((gcView*) g_object_get_data(G_OBJECT(w), "view"))) break;
		else i++;
	}
	while (w);
	gtk_notebook_set_current_page(m_Notebook, i);
}

void gcApplication::OnChangePage(int i)
{
	GtkWidget *w = gtk_notebook_get_nth_page(m_Notebook, i);
	m_pActiveView = (gcView*)g_object_get_data(G_OBJECT(w), "view");
}
