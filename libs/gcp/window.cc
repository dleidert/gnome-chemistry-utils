/* 
 * GChemPaint library
 * window.cc
 *
 * Copyright (C) 2006-2007 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "application.h"
#include "document.h"
#include "newfiledlg.h"
#include "window.h"
#include "about.h"
#include "preferences.h"
#include "theme.h"
#include "view.h"
#include "zoomdlg.h"
#include <gcu/filechooser.h>
#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-job.h>
#include <libgnomeprintui/gnome-print-dialog.h>
#include <libgnomeprintui/gnome-print-job-preview.h>
#include <glib/gi18n-lib.h>

static void on_destroy (GtkWidget* widget, gcp::Window* Win)
{
	Win->GetDocument ()->GetView ()->PrepareUnselect ();
	delete Win;
}

static bool on_delete_event (GtkWidget* widget, GdkEvent *event, gcp::Window* Win)
{
	return !Win->VerifySaved ();
}

static void on_file_new(GtkWidget* widget, gcp::Window* Win)
{
	Win->OnFileNew ();
}

static void on_themed_file_new(GtkWidget* widget, gcp::Window* Win)
{
	new gcp::NewFileDlg (Win->GetApplication ());
}

static void on_file_open(GtkWidget* widget, gcp::Window* Win)
{
	Win->OnFileOpen ();
}

void on_file_save_as(GtkWidget* widget, gcp::Window* Win)
{
	Win->GetApplication ()->OnSaveAs();
}

static void on_file_save(GtkWidget* widget, gcp::Window* Win)
{
	Win->OnSave ();
}

static void on_file_save_as_image(GtkWidget* widget, gcp::Window* Win)
{
	Win->GetApplication ()->OnSaveAsImage ();
}

static void on_properties(GtkWidget* widget, gcp::Window* Win)
{
	Win->OnProperties ();
}

void on_print(GtkWidget* widget, gcp::Window* Win)
{
	Win->OnPrint ();
}

static void on_file_close (GtkWidget* widget, gcp::Window *Win)
{
	Win->Close ();
}

static void on_quit (GtkWidget* widget, gcp::Window *Win)
{
	Win->GetApplication ()->CloseAll ();
}

static void on_cut_selection(GtkWidget* widget, gcp::Window* Win)
{
	Win->OnCutSelection ();
}

static void on_copy_selection(GtkWidget* widget, gcp::Window* Win)
{
	Win->OnCopySelection ();
}

static void on_undo(GtkWidget* widget, gcp::Window* Win)
{
	Win->OnUndo ();
}

static void on_redo(GtkWidget* widget, gcp::Window* Win)
{
	Win->OnRedo ();
}

static void on_select_all(GtkWidget* widget, gcp::Window* Win)
{
	Win->OnSelectAll ();
}

static void on_paste_selection(GtkWidget* widget, gcp::Window* Win)
{
	Win->OnPasteSelection ();
}

static void on_delete_selection(GtkWidget* widget, gcp::Window* Win)
{
	Win->OnDeleteSelection ();
}

static void on_preferences (GtkWidget* widget, gcp::Window* Win)
{
	Win->OnPreferences ();
}

static void on_zoom_400 (GtkWidget* widget, gcp::Window* Win)
{
	Win->Zoom (4.);
}

static void on_zoom_300 (GtkWidget* widget, gcp::Window* Win)
{
	Win->Zoom (3.);
}

static void on_zoom_200 (GtkWidget* widget, gcp::Window* Win)
{
	Win->Zoom (2.);
}

static void on_zoom_150 (GtkWidget* widget, gcp::Window* Win)
{
	Win->Zoom (1.5);
}

static void on_zoom_100 (GtkWidget* widget, gcp::Window* Win)
{
	Win->Zoom (1.);
}

static void on_zoom_75 (GtkWidget* widget, gcp::Window* Win)
{
	Win->Zoom (.75);
}

static void on_zoom_50 (GtkWidget* widget, gcp::Window* Win)
{
	Win->Zoom (.5);
}

static void on_zoom_25 (GtkWidget* widget, gcp::Window* Win)
{
	Win->Zoom (.3);
}

static void on_zoom (GtkWidget* widget, gcp::Window* Win)
{
	Win->Zoom (0.);
}

static bool on_focus_in (GtkWidget *widget, GdkEventFocus *event, gcp::Window* Win)
{
	Win->GetDocument ()->GetView ()->ShowCursor (true);
	gcp::Application *App = Win->GetApplication ();
	App->NotifyFocus (true, Win);
	gtk_clipboard_request_contents (gtk_clipboard_get (GDK_SELECTION_CLIPBOARD), gdk_atom_intern ("TARGETS", FALSE),  (GtkClipboardReceivedFunc) gcp::on_receive_targets, App);
	return true;
}

static bool on_focus_out (GtkWidget *widget, GdkEventFocus *event, gcp::Window* Win)
{
	Win->GetDocument ()->GetView ()->ShowCursor (false);
	Win->GetApplication ()->NotifyFocus (false);
	return true;
}

static bool on_state (GtkWidget *widget, GdkEventWindowState *event, gcp::Window* Win)
{
	if (event->changed_mask & GDK_WINDOW_STATE_ICONIFIED)
		Win->GetApplication ()->NotifyIconification (event->new_window_state & GDK_WINDOW_STATE_ICONIFIED);
	return true;
}

static void on_help (GtkWidget *widget, gcp::Window* Win)
{
	Win->GetApplication ()->OnHelp ();
}

static void on_web (GtkWidget *widget, gcp::Window* Win)
{
	Win->GetApplication ()->OnWeb ("gchempaint.nongnu.org");
}

static void on_mail (GtkWidget *widget, gcp::Window* Win)
{
	Win->GetApplication ()->OnMail ("mailto:gchempaint-main@nongnu.org");
}

static void on_bug (GtkWidget *widget, gcp::Window* Win)
{
	Win->GetApplication ()->OnBug ();
}

static void on_show_menu_tip (GtkWidget *proxy, gcp::Window* Win)
{
	GtkAction *action = (GtkAction*) g_object_get_data (G_OBJECT (proxy), "action");
	char *tip;
	g_object_get (action, "tooltip", &tip, NULL);
	if (tip != NULL){
		Win->SetStatusText (tip);
		g_free (tip);
	}
}

static void on_clear_menu_tip (gcp::Window* Win)
{
		Win->ClearStatus ();
}

static void on_connect_proxy (GtkUIManager *ui, GtkAction *action, GtkWidget *proxy, gcp::Window* Win)
{
	/* connect whether there is a tip or not it may change later */
	if (GTK_IS_MENU_ITEM (proxy)) {
		g_object_set_data (G_OBJECT (proxy), "action", action);
		g_object_connect (proxy,
			"signal::select",  G_CALLBACK (on_show_menu_tip), Win,
			"swapped_signal::deselect", G_CALLBACK (on_clear_menu_tip), Win,
			NULL);
	}
}

static void on_disconnect_proxy (GtkUIManager *ui, GtkAction *action, GtkWidget *proxy, gcp::Window* Win)
{
	if (GTK_IS_MENU_ITEM (proxy)) {
		g_object_set_data (G_OBJECT (proxy), "action", NULL);
		g_object_disconnect (proxy,
			"any_signal::select",  G_CALLBACK (on_show_menu_tip), Win,
			"any_signal::deselect", G_CALLBACK (on_clear_menu_tip), Win,
			NULL);
	}
}

static bool on_key_release(GtkWidget* widget, GdkEventKey* ev, gcp::Window* Win)
{
	return Win->OnKeyReleased(widget, ev);
}

static bool on_key_press(GtkWidget* widget, GdkEventKey* ev, gcp::Window* Win)
{
	return Win->OnKeyPressed(widget, ev);
}

static void on_recent (GtkRecentChooser *widget, gcp::Window *Win)
{
	gcp::Application *App = Win->GetApplication ();
	GtkRecentInfo *info = gtk_recent_chooser_get_current_item (widget);
	gcp::Document *pDoc = Win->GetDocument ();
	App->FileProcess (gtk_recent_info_get_uri (info), gtk_recent_info_get_mime_type (info), false, NULL, (!pDoc->HasChildren () && !pDoc->GetDirty ())? pDoc: NULL);
	gtk_recent_info_unref (info);
}

/*********
 * Menus *
 *********/
static GtkActionEntry entries[] = {
  { "FileMenu", NULL, N_("_File") },
	  { "New", GTK_STOCK_NEW, N_("_New File"), NULL,
		  N_("Create a new file"), G_CALLBACK (on_file_new) },
	  { "NewThemed", GTK_STOCK_NEW, N_("Ne_w File with Theme..."), "<shift><control>N",
		  N_("Create a new file using a theme"), G_CALLBACK (on_themed_file_new) },
	  { "Open", GTK_STOCK_OPEN, N_("_Open..."), "<control>O",
		  N_("Open a file"), G_CALLBACK (on_file_open) },
	  { "Save", GTK_STOCK_SAVE, N_("_Save"), "<control>S",
		  N_("Save the current file"), G_CALLBACK (on_file_save) },
	  { "SaveAs", GTK_STOCK_SAVE_AS, N_("Save _As..."), "<shift><control>S",
		  N_("Save the current file with a different name"), G_CALLBACK (on_file_save_as) },
	  { "SaveAsImage", GTK_STOCK_SAVE_AS, N_("Save As _Image..."), "<control>I",
		  N_("Save the current file as an image"), G_CALLBACK (on_file_save_as_image) },
	  { "Print", GTK_STOCK_PRINT, N_("_Print..."), "<control>P",
		  N_("Print the current file"), G_CALLBACK (on_print) },
	  { "Properties", GTK_STOCK_PROPERTIES, N_("Prope_rties..."), NULL,
		  N_("Modify the file's properties"), G_CALLBACK (on_properties) },
	  { "Close", GTK_STOCK_CLOSE, N_("_Close"), "<control>W",
		  N_("Close the current file"), G_CALLBACK (on_file_close) },
	  { "Quit", GTK_STOCK_QUIT, N_("_Quit"), "<control>Q",
		  N_("Quit GChemPaint"), G_CALLBACK (on_quit) },
  { "EditMenu", NULL, N_("_Edit") },
	  { "Undo", GTK_STOCK_UNDO, N_("_Undo"), "<control>Z",
		  N_("Undo the last action"), G_CALLBACK (on_undo) },
	  { "Redo", GTK_STOCK_REDO, N_("_Redo"), "<shift><control>Z",
		  N_("Redo the undone action"), G_CALLBACK (on_redo) },
	  { "Cut", GTK_STOCK_CUT, N_("Cu_t"), "<control>X",
		  N_("Cut the selection"), G_CALLBACK (on_cut_selection) },
	  { "Copy", GTK_STOCK_COPY, N_("_Copy"), "<control>C",
		  N_("Copy the selection"), G_CALLBACK (on_copy_selection) },
	  { "Paste", GTK_STOCK_PASTE, N_("_Paste"), "<control>V",
		  N_("Paste the clipboard"), G_CALLBACK (on_paste_selection) },
	  { "Erase", GTK_STOCK_CLEAR, N_("C_lear"), NULL,
		  N_("Clear the selection"), G_CALLBACK (on_delete_selection) },
	  { "SelectAll", NULL, N_("Select _All"), "<control>A",
		  N_("Select everything"), G_CALLBACK (on_select_all) },
	  { "Preferences", GTK_STOCK_PREFERENCES, N_("Pr_eferences..."), NULL,
		  N_("Configure the application"), G_CALLBACK (on_preferences) },
  { "ViewMenu", NULL, N_("_View") },
	  { "ZoomMenu", NULL, N_("_Zoom") },
		  { "400%", NULL, N_("_400%"), "<control>4",
			  N_("Zoom to 400%"), G_CALLBACK (on_zoom_400) },
		  { "300%", NULL, N_("_300%"), "<control>3",
			  N_("Zoom to 300%"), G_CALLBACK (on_zoom_300) },
		  { "200%", NULL, N_("_200%"), "<control>2",
			  N_("Zoom to 200%"), G_CALLBACK (on_zoom_200) },
		  { "150%", NULL, N_("150%"), "<control>6",
			  N_("Zoom to 150%"), G_CALLBACK (on_zoom_150) },
		  { "100%", NULL, N_("_100%"), "<control>1",
			  N_("Zoom to 100%"), G_CALLBACK (on_zoom_100) },
 		  { "75%", NULL, N_("_75%"), "<control>7",
			  N_("Zoom to 75%"), G_CALLBACK (on_zoom_75) },
		  { "50%", NULL, N_("_50%"), "<control>5",
			  N_("Zoom to 50%"), G_CALLBACK (on_zoom_50) },
		  { "25%", NULL, N_("25%"),  "<control>8",
			  N_("Zoom to 25%"), G_CALLBACK (on_zoom_25) },
		  { "Zoom", NULL, N_("_Zoom to...%"), "<control>M",
			  N_("Open Zoom Dialog Box"), G_CALLBACK (on_zoom) },
  { "ToolsMenu", NULL, N_("_Tools") },
  { "WindowsMenu", NULL, N_("_Windows") },
  { "HelpMenu", NULL, N_("_Help") },
	  { "Help", GTK_STOCK_HELP, N_("_Contents"), "F1",
		  N_("View help for GChemPaint"), G_CALLBACK (on_help) },
	  { "Web", NULL, N_("GChemPaint on the _web"), NULL,
		  N_("Browse GChemPaint's web site"), G_CALLBACK (on_web) },
	  { "Mail", NULL, N_("_Ask a question"), NULL,
		  N_("Ask a question about GChemPaint"), G_CALLBACK (on_mail) },
	  { "Bug", NULL, N_("Report _Bugs"), NULL,
		  N_("Submit a bug report for GChemPaint"), G_CALLBACK (on_bug) },
	  { "About", NULL, N_("_About"), NULL,
		  N_("About GChemPaint"), G_CALLBACK (on_about) }
};

/* Toggle items */
static GtkToggleActionEntry toggle_entries[] = {
};

static const char *ui_description =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu action='FileMenu'>"
"      <menuitem action='New'/>"
"      <menuitem action='NewThemed'/>"
"      <menuitem action='Open'/>"
"      <menuitem action='Save'/>"
"      <menuitem action='SaveAs'/>"
"      <menuitem action='SaveAsImage'/>"
"      <separator name='file-sep1'/>"
"      <menuitem action='Print'/>"
"      <separator name='file-sep2'/>"
"      <menuitem action='Properties'/>"
"      <separator name='file-sep3'/>"
"      <menuitem action='Close'/>"
"	   <placeholder name='file1'/>"
"    </menu>"
"    <menu action='EditMenu'>"
"      <menuitem action='Undo'/>"
"      <menuitem action='Redo'/>"
"      <separator name='edit-sep1'/>"
"      <menuitem action='Cut'/>"
"      <menuitem action='Copy'/>"
"      <menuitem action='Paste'/>"
"      <menuitem action='Erase'/>"
"      <separator name='edit-sep2'/>"
"      <menuitem action='SelectAll'/>"
"      <separator name='edit-sep3'/>"
"      <menuitem action='Preferences'/>"
"    </menu>"
"    <menu action='ViewMenu'>"
"      <menu action='ZoomMenu'>"
"        <menuitem action='400%'/>"
"        <menuitem action='300%'/>"
"        <menuitem action='200%'/>"
"        <menuitem action='150%'/>"
"        <menuitem action='100%'/>"
"        <menuitem action='75%'/>"
"        <menuitem action='50%'/>"
"        <menuitem action='25%'/>"
"        <menuitem action='Zoom'/>"
"      </menu>"
"    </menu>"
"    <menu action='ToolsMenu'>"
"	   <placeholder name='tools1'/>"
"      <separator name='tools-sep1'/>"
"      <placeholder name='tools2'/>"
"    </menu>"
"    <menu action='WindowsMenu'>"
"	   <placeholder name='windows1'/>"
"      <separator name='windows-sep1'/>"
"      <placeholder name='windows'/>"
"    </menu>"
"    <menu action='HelpMenu'>"
"      <menuitem action='Help'/>"
"      <placeholder name='web'/>"
"      <placeholder name='mail'/>"
"      <placeholder name='bug'/>"
"      <menuitem action='About'/>"
"    </menu>"
"  </menubar>"
"  <toolbar name='MainToolbar'>"
"    <toolitem action='New'/>"
"    <toolitem action='Open'/>"
"    <toolitem action='Save'/>"
"    <toolitem action='Print'/>"
"	 <placeholder name='tools1'/>"
"  </toolbar>"
"</ui>";

static const char *ui_mail_description =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu action='HelpMenu'>"
"      <placeholder name='mail'>"
"        <menuitem action='Mail'/>"
"      </placeholder>"
"    </menu>"
"  </menubar>"
"</ui>";

static const char *ui_web_description =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu action='HelpMenu'>"
"      <placeholder name='web'>"
"        <menuitem action='Web'/>"
"      </placeholder>"
"      <placeholder name='bug'>"
"        <menuitem action='Bug'/>"
"      </placeholder>"
"    </menu>"
"  </menubar>"
"</ui>";

namespace gcp {

Window::Window (gcp::Application *App, char const *Theme, char const *extra_ui)
{
	GtkWidget *vbox;
	GtkWidget *bar;
	GtkActionGroup *action_group;
	GtkAccelGroup *accel_group;
	GError *error;

	m_App = App;
	m_Window = GTK_WINDOW (gtk_window_new (GTK_WINDOW_TOPLEVEL));
	g_object_set (G_OBJECT (m_Window), "urgency-hint", false, NULL);
	g_object_set_data (G_OBJECT (m_Window), "gcp-role", (void*) 1);
	g_signal_connect (G_OBJECT (m_Window), "destroy", G_CALLBACK (on_destroy), this);
	g_signal_connect (G_OBJECT (m_Window), "delete-event", G_CALLBACK (on_delete_event), this);
	g_signal_connect (G_OBJECT (m_Window), "focus_in_event", G_CALLBACK (on_focus_in), this);
	g_signal_connect (G_OBJECT (m_Window), "focus_out_event", G_CALLBACK (on_focus_out), this);
	g_signal_connect (G_OBJECT (m_Window), "window-state-event", G_CALLBACK (on_state), this);
	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (m_Window), vbox);
	action_group = gtk_action_group_new ("MenuActions");
	gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);
	gtk_action_group_add_actions (action_group, entries, G_N_ELEMENTS (entries), this);
	gtk_action_group_add_toggle_actions (action_group, toggle_entries, G_N_ELEMENTS (toggle_entries), this);

	m_UIManager = gtk_ui_manager_new ();
	g_object_connect (m_UIManager,
		"signal::connect_proxy",    G_CALLBACK (on_connect_proxy), this,
		"signal::disconnect_proxy", G_CALLBACK (on_disconnect_proxy), this,
		NULL);
	gtk_ui_manager_insert_action_group (m_UIManager, action_group, 0);
	g_object_unref (action_group);

	accel_group = gtk_ui_manager_get_accel_group (m_UIManager);
	gtk_window_add_accel_group (GTK_WINDOW (m_Window), accel_group);
	
	error = NULL;
	if (!gtk_ui_manager_add_ui_from_string (m_UIManager, ui_description, -1, &error)) {
		g_message ("building menus failed: %s", error->message);
		g_error_free (error);
		exit (EXIT_FAILURE);
	}
	if (App->HasWebBrowser () && !gtk_ui_manager_add_ui_from_string (m_UIManager, ui_web_description, -1, &error)) {
		g_message ("building menus failed: %s", error->message);
		g_error_free (error);
	}
	if (App->HasMailAgent () && !gtk_ui_manager_add_ui_from_string (m_UIManager, ui_mail_description, -1, &error)) {
		g_message ("building menus failed: %s", error->message);
		g_error_free (error);
	}
	if (extra_ui && !gtk_ui_manager_add_ui_from_string (m_UIManager, extra_ui, -1, &error)) {
		g_message ("building menus failed: %s", error->message);
		g_error_free (error);
	}

	//  now add entries registered by plugins.
	App->BuildMenu (m_UIManager);

	GtkWidget *menu = gtk_ui_manager_get_widget (m_UIManager, "/MainMenu/FileMenu/Open");
	GtkWidget *w = gtk_recent_chooser_menu_new_for_manager (App->GetRecentManager ());
	GtkRecentFilter *filter = gtk_recent_filter_new ();
	gtk_recent_filter_add_mime_type (filter, "application/x-gchempaint");
	gtk_recent_filter_add_mime_type (filter, "chemical/x-chemdraw");
	gtk_recent_filter_add_mime_type (filter, "chemical/x-cml");
	gtk_recent_filter_add_mime_type (filter, "chemical/x-mdl-molfile");
	gtk_recent_filter_add_mime_type (filter, "chemical/x-pdb");
	gtk_recent_chooser_add_filter (GTK_RECENT_CHOOSER (w), filter);
	g_signal_connect (G_OBJECT (w), "item-activated", G_CALLBACK (on_recent), this);
	GtkWidget *item = gtk_menu_item_new_with_mnemonic (_("Open _recent"));
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), w);
	gtk_widget_show_all (item);
	gtk_menu_shell_insert (GTK_MENU_SHELL (gtk_widget_get_parent (menu)), item, 3);

	bar = gtk_ui_manager_get_widget (m_UIManager, "/MainMenu");
	gtk_box_pack_start (GTK_BOX (vbox), bar, false, false, 0);
	bar = gtk_ui_manager_get_widget (m_UIManager, "/MainToolbar");
	gtk_toolbar_set_tooltips(GTK_TOOLBAR(bar), true);
	gtk_box_pack_start (GTK_BOX (vbox), bar, false, false, 0);
	m_Doc = new Document (App, true, this);
	if (Theme)
		m_Doc->SetTheme (TheThemeManager.GetTheme (Theme));
	gtk_window_set_title (m_Window, m_Doc->GetTitle ());
	w = m_Doc->GetView ()->CreateNewWidget ();
	GtkScrolledWindow* scroll = (GtkScrolledWindow*)gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(scroll, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (scroll, GTK_SHADOW_IN);
	gtk_scrolled_window_add_with_viewport(scroll, w);
	gtk_widget_set_size_request(GTK_WIDGET(scroll), 408, 308);
	gtk_widget_show(GTK_WIDGET(scroll));
	gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (scroll), true, true, 0);
	m_Bar = gtk_statusbar_new ();
	m_statusId = gtk_statusbar_get_context_id (GTK_STATUSBAR (m_Bar), "status");
	gtk_statusbar_push (GTK_STATUSBAR (m_Bar), m_statusId, _("Ready"));
	m_MessageId = 0;
	gtk_box_pack_start (GTK_BOX (vbox), m_Bar, false, false, 0);
		
	g_signal_connect(GTK_OBJECT(m_Window), "key_press_event", (GCallback)on_key_press, this);
	g_signal_connect(GTK_OBJECT(m_Window), "key_release_event", (GCallback)on_key_release, this);

	gtk_widget_set_sensitive (gtk_ui_manager_get_widget (m_UIManager, "/MainMenu/EditMenu/Copy"), false);
	gtk_widget_set_sensitive (gtk_ui_manager_get_widget (m_UIManager, "/MainMenu/EditMenu/Cut"), false);
	gtk_widget_set_sensitive (gtk_ui_manager_get_widget (m_UIManager, "/MainMenu/EditMenu/Erase"), false);
	gtk_widget_show_all (GTK_WIDGET (m_Window));
	App->SetActiveDocument (m_Doc);
}

Window::~Window ()
{
	m_App->DeleteWindow (this);
}

void Window::OnFileNew()
{
	m_App->OnFileNew ();
}

void Window::OnSave()
{
	if (m_Doc->GetFileName ())
		m_Doc->Save();
	else
		m_App->OnSaveAs ();
}

void Window::OnFileOpen()
{
	FileChooser (m_App, false, m_App->GetSupportedMimeTypes (), (!m_Doc->HasChildren () && !m_Doc->GetDirty ())? m_Doc: NULL);
}

void Window::OnProperties()
{
	m_Doc->OnProperties ();
}

void Window::OnPrint()
{
	GnomePrintConfig* config = gnome_print_config_default ();
	GnomePrintContext *pc;// = gnome_print_context_new (config);
	GnomePrintJob *gpj = gnome_print_job_new (config);
	int do_preview = 0, copies = 1, collate = 0;
	GnomePrintDialog *gpd;
	gpd = GNOME_PRINT_DIALOG (gnome_print_dialog_new(gpj, (const guchar*)_("Print"), GNOME_PRINT_DIALOG_COPIES));
	gtk_window_set_icon_name (GTK_WINDOW (gpd), "gchempaint");
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
	gnome_print_beginpage (pc, (const guchar*)"");
	gdouble width, height;
	gnome_print_config_get_double (config, (guchar const*) GNOME_PRINT_KEY_PAPER_WIDTH, &width);
	gnome_print_config_get_double (config, (guchar const*) GNOME_PRINT_KEY_PAPER_HEIGHT, &height);
	m_Doc->Print (pc, width, height);
	gnome_print_showpage (pc);
	g_object_unref (pc);
	gnome_print_job_close (gpj);
	if (do_preview) {
		GtkWidget *preview = gnome_print_job_preview_new (gpj, (const guchar*)_("Preview"));
		gtk_window_set_icon_name (GTK_WINDOW (preview), "gchempaint");
		gtk_widget_show (preview);
	} else {
		gnome_print_job_print (gpj);
	}
	g_object_unref (gpj);
	gnome_print_config_unref (config);
}

void Window::SetActive (gcp::Document* pDoc, GtkWidget* w)
{
}

void Window::OnUndo()
{
	m_Doc->OnUndo();
}

void Window::OnRedo()
{
	m_Doc->OnRedo();
}

void Window::OnSelectAll()
{
	if (m_Doc->GetEditable ())
		m_Doc->GetView ()->OnSelectAll ();
}

void Window::OnPasteSelection()
{
	if (m_Doc->GetEditable ()) {
		GtkClipboard* clipboard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
		m_Doc->GetView ()->OnPasteSelection (m_Doc->GetWidget (), clipboard);
	}
}

void Window::OnCutSelection()
{
	if (m_Doc->GetEditable ()) {
		GtkClipboard* clipboard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
		m_Doc->GetView ()->OnCutSelection (m_Doc->GetWidget (), clipboard);
	}
}

void Window::OnCopySelection()
{
	if (m_Doc->GetEditable ()) {
		GtkClipboard* clipboard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
		m_Doc->GetView ()->OnCopySelection (m_Doc->GetWidget (), clipboard);
	}
}

void Window::OnDeleteSelection()
{
	if (m_Doc->GetEditable ())
		m_Doc->GetView ()->OnDeleteSelection (m_Doc->GetWidget ());
}

void Window::OnPreferences ()
{
	new PrefsDlg (GetApplication ());
}

void Window::Zoom (double zoom)
{
	View *pView = m_Doc->GetView ();
	// authorized zooms: 20% to 800% all other values will open the zoom dialog.
	if (zoom >= 0.2 && zoom <= 8.)
		pView->Zoom (zoom);
	else {
		Dialog *pDialog = m_Doc->GetDialog ("Zoom");
		if (pDialog)
			gtk_window_present (pDialog->GetWindow ()); 
		else
			new ZoomDlg (m_Doc);
	}
}

void Window::ClearStatus()
{
	if (m_MessageId) {
		gtk_statusbar_pop (GTK_STATUSBAR (m_Bar), m_statusId);
		m_MessageId = 0;
	}
}

void Window::SetStatusText(const char* text)
{
	if (m_MessageId)
		gtk_statusbar_pop (GTK_STATUSBAR (m_Bar), m_statusId);
	m_MessageId = gtk_statusbar_push (GTK_STATUSBAR (m_Bar), m_statusId, text);
}

void Window::Destroy ()
{
	gtk_widget_destroy (GTK_WIDGET (m_Window));
}

void Window::SetTitle (char const *title)
{
	gtk_window_set_title (m_Window, ((title)? title: GetDefaultTitle ()));
}

void Window::Show ()
{
	gdk_window_raise (GTK_WIDGET (m_Window)->window);
}

bool Window::OnKeyPressed(GtkWidget* widget, GdkEventKey* ev)
{
	return (m_Doc->GetEditable ())?
		m_Doc->GetView ()->OnKeyPress (m_Doc->GetWidget (), ev):
		false;
}

bool Window::OnKeyReleased(GtkWidget* widget, GdkEventKey* ev)
{
	return (m_Doc->GetEditable ())?
		m_Doc->GetView ()->OnKeyRelease (m_Doc->GetWidget (), ev):
		false;
}

bool Window::Close ()
{
	if (VerifySaved ()) {
		m_Doc->GetView ()->PrepareUnselect ();
		gtk_widget_destroy (GTK_WIDGET (m_Window));
		return true;
	}
	return false;
}


char const *Window::GetDefaultTitle ()
{
	return _("GChemPaint");
}

void Window::ActivateActionWidget (char const *path, bool activate)
{
	GtkWidget *w = gtk_ui_manager_get_widget (m_UIManager, path);
	if (w)
		gtk_widget_set_sensitive (w, activate);
}

bool Window::VerifySaved ()
{
	if (!m_Doc->GetDirty ())
		return true;
	gchar* str = g_strdup_printf(_("\"%s\" has been modified.  Do you wish to save it?"), m_Doc->GetTitle ());
	GtkWidget* mbox;
	int res;
	do {
		mbox = gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, str);
		gtk_dialog_add_button (GTK_DIALOG (mbox),  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
		res = gtk_dialog_run (GTK_DIALOG (mbox));
		gtk_widget_destroy (mbox);
		if (res == GTK_RESPONSE_YES)
			OnSave ();
	} while ((res == GTK_RESPONSE_YES) && (m_Doc->GetFileName () == NULL));
	if (res == GTK_RESPONSE_NO)
		m_Doc->SetDirty (false);
	g_free(str);
	return (res != GTK_RESPONSE_CANCEL);
}

} // namespace gcp
