// -*- C++ -*-

/*
 * GChemPaint library
 * docprop.cc
 *
 * Copyright (C) 2002-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "docprop.h"
#include "document.h"
#include "application.h"
#include "theme.h"
#include "view.h"
#include "window.h"
#include <glib/gi18n-lib.h>

using namespace gcu;
using namespace std;

namespace gcp {

class DocPropPrivate
{
public:
	static void OnColors (GtkToggleButton *btn, Document *doc);
};

void DocPropPrivate::OnColors (GtkToggleButton *btn, Document *doc)
{
	doc->SetUseAtomColors (gtk_toggle_button_get_active (btn));
	doc->GetView ()->Update (doc);
	doc->SetDirty ();
}

static void on_theme_changed (GtkComboBoxText *box, DocPropDlg *dlg)
{
	dlg->OnThemeChanged (TheThemeManager.GetTheme (gtk_combo_box_text_get_active_text (box)));
}

static void on_title_changed (GtkEntry *entry, DocPropDlg *dlg)
{
	dlg->OnTitleChanged (gtk_entry_get_text (entry));
}

static bool on_title_focused_out (GtkEntry *entry, G_GNUC_UNUSED GdkEventFocus *event, DocPropDlg *dlg)
{
	dlg->OnTitleChanged (gtk_entry_get_text (entry));
	return false;
}

static void on_name_changed (GtkEntry *entry, DocPropDlg *dlg)
{
	dlg->OnNameChanged (gtk_entry_get_text (entry));
}

static bool on_name_focused_out (GtkEntry *entry, G_GNUC_UNUSED GdkEventFocus *event, DocPropDlg *dlg)
{
	dlg->OnNameChanged (gtk_entry_get_text (entry));
	return false;
}

static void on_mail_changed (GtkEntry *entry, DocPropDlg *dlg)
{
	dlg->OnMailChanged (gtk_entry_get_text (entry));
}

static bool on_mail_focused_out (GtkEntry *entry, G_GNUC_UNUSED GdkEventFocus *event, DocPropDlg *dlg)
{
	dlg->OnMailChanged (gtk_entry_get_text (entry));
	return false;
}

static void on_comments_changed (GtkTextBuffer *buffer, DocPropDlg *dlg)
{
	GtkTextIter start, end;
	gtk_text_buffer_get_bounds (buffer, &start, &end);
	char *text = gtk_text_buffer_get_text (buffer, &start, &end, true);
	dlg->OnCommentsChanged (text);
	g_free (text);
}

DocPropDlg::DocPropDlg (Document* pDoc):
	gcugtk::Dialog (pDoc->GetApplication (), UIDIR"/docprop.ui", "properties", GETTEXT_PACKAGE, pDoc),
	Object ()
{
	m_pDoc = pDoc;
	Title = GTK_ENTRY (GetWidget ("title"));
	char const *chn;
	chn = m_pDoc->GetTitle ();
	if (chn)
		gtk_entry_set_text (Title, chn);
	g_signal_connect (G_OBJECT (Title), "activate", G_CALLBACK (on_title_changed), this);
	g_signal_connect (G_OBJECT (Title), "focus-out-event", G_CALLBACK (on_title_focused_out), this);
	NameEntry = GTK_ENTRY (GetWidget ("name"));
	chn = m_pDoc->GetAuthor ();
	if (chn)
		gtk_entry_set_text (NameEntry, chn);
	g_signal_connect (G_OBJECT (NameEntry), "activate", G_CALLBACK (on_name_changed), this);
	g_signal_connect (G_OBJECT (NameEntry), "focus-out-event", G_CALLBACK (on_name_focused_out), this);
	Mail = GTK_ENTRY (GetWidget ("mail"));
	chn = m_pDoc->GetMail ();
	if (chn)
		gtk_entry_set_text (Mail, chn);
	g_signal_connect (G_OBJECT (Mail), "activate", G_CALLBACK (on_mail_changed), this);
	g_signal_connect (G_OBJECT (Mail), "focus-out-event", G_CALLBACK (on_mail_focused_out), this);
	CreationDate = GTK_LABEL (GetWidget ("creation"));
	const GDate* Date = pDoc->GetCreationDate ();
	char tmp[64];
	/* The following format prints date as "Monday, July 8, 2002" */
	if (g_date_valid (Date)) {
		g_date_strftime (tmp, sizeof (tmp), _("%A, %B %d, %Y"), Date);
		gtk_label_set_text (CreationDate, tmp);
	}
	RevisionDate = GTK_LABEL (GetWidget ("revision"));
	Date = pDoc->GetRevisionDate ();
	if (g_date_valid(Date))
	{
		g_date_strftime (tmp, sizeof (tmp), _("%A, %B %d, %Y"), Date);
		gtk_label_set_text (RevisionDate, tmp);
	}
	Comments = GTK_TEXT_VIEW (GetWidget ("comments"));
	Buffer = gtk_text_view_get_buffer (Comments);
	chn = m_pDoc->GetComment ().c_str ();
	if(chn)
		gtk_text_buffer_set_text (Buffer, chn , -1);
	g_signal_connect (G_OBJECT (Buffer), "changed", G_CALLBACK (on_comments_changed), this);
	GtkWidget *w = GetWidget ("props-grid");
	m_Box = GTK_COMBO_BOX_TEXT (gtk_combo_box_text_new ());
	gtk_grid_attach (GTK_GRID (w), GTK_WIDGET (m_Box), 1, 8, 1, 1);
	list <string> names = TheThemeManager.GetThemesNames ();
	list <string>::iterator i, end = names.end ();
	Theme *theme;
	m_Lines = names.size ();
	int nb = 0, n;
	for (i = names.begin (), n = 0; i != end; i++, n++) {
		gtk_combo_box_text_append_text (m_Box, (*i).c_str ());
		theme = TheThemeManager.GetTheme (*i);
		if (theme) {
			theme->AddClient (this);
			if (theme ==  m_pDoc->GetTheme ())
				nb = n;
		}
	}
	gtk_combo_box_set_active (GTK_COMBO_BOX (m_Box), nb);
	m_ChangedSignal = g_signal_connect (G_OBJECT (m_Box), "changed", G_CALLBACK (on_theme_changed), this);
	w = GetWidget ("atom-colors");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w), pDoc->GetUseAtomColors ());
	g_signal_connect (G_OBJECT (w), "toggled", G_CALLBACK (DocPropPrivate::OnColors), pDoc);
	gtk_widget_show_all (GTK_WIDGET (dialog));
}

DocPropDlg::~DocPropDlg ()
{
	list <string> names = TheThemeManager.GetThemesNames ();
	list <string>::iterator i, end = names.end ();
	Theme *theme;
	for (i = names.begin (); i != end; i++) {
		theme = TheThemeManager.GetTheme (*i);
		if (theme)
			theme->RemoveClient (this);
	}
}

void DocPropDlg::OnThemeNamesChanged ()
{
	list <string> names = TheThemeManager.GetThemesNames ();
	list <string>::iterator i, end = names.end ();
	int n, nb = gtk_combo_box_get_active (GTK_COMBO_BOX (m_Box));
	g_signal_handler_block (m_Box, m_ChangedSignal);
	while (m_Lines--)
		gtk_combo_box_text_remove (m_Box, 0);
	for (i = names.begin (), n = 0; i != end; i++, n++) {
		gtk_combo_box_text_append_text (m_Box, (*i).c_str ());
		if (m_pDoc->GetTheme () == TheThemeManager.GetTheme (*i))
			nb = n;
	}
	m_Lines = names.size ();
	gtk_combo_box_set_active (GTK_COMBO_BOX (m_Box), nb);
	g_signal_handler_unblock (m_Box, m_ChangedSignal);
}

void DocPropDlg::OnThemeChanged (Theme *theme)
{
	m_pDoc->SetTheme (theme);
}

void DocPropDlg::OnTitleChanged (char const *title)
{
	m_pDoc->SetTitle (title);
	Window *window = static_cast < Window * > (m_pDoc->GetWindow ());
	if (window)
		window->SetTitle (m_pDoc->GetTitle ());
}

void DocPropDlg::OnNameChanged (char const *name)
{
	m_pDoc->SetAuthor (name);
}

void DocPropDlg::OnMailChanged (char const *mail)
{
	m_pDoc->SetMail (mail);
}

void DocPropDlg::OnCommentsChanged (char const *comment)
{
	m_pDoc->SetComment (comment);
}

}	//	namespace gcp
