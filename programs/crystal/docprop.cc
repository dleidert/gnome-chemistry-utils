// -*- C++ -*-

/* 
 * GChemPaint library
 * docprop.cc 
 *
 * Copyright (C) 2002-2007 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "docprop.h"
#include "document.h"
#include "application.h"
#include "window.h"
#include <glib/gi18n-lib.h>

using namespace gcu;
using namespace std;

static void on_title_changed (GtkEntry *entry, gcDocPropDlg *dlg)
{
	dlg->OnTitleChanged (gtk_entry_get_text (entry));
}

static bool on_title_focused_out (GtkEntry *entry, GdkEventFocus *event, gcDocPropDlg *dlg)
{
	dlg->OnTitleChanged (gtk_entry_get_text (entry));
	return false;
}

static void on_name_changed (GtkEntry *entry, gcDocPropDlg *dlg)
{
	dlg->OnNameChanged (gtk_entry_get_text (entry));
}

static bool on_name_focused_out (GtkEntry *entry, GdkEventFocus *event, gcDocPropDlg *dlg)
{
	dlg->OnNameChanged (gtk_entry_get_text (entry));
	return false;
}

static void on_mail_changed (GtkEntry *entry, gcDocPropDlg *dlg)
{
	dlg->OnMailChanged (gtk_entry_get_text (entry));
}

static bool on_mail_focused_out (GtkEntry *entry, GdkEventFocus *event, gcDocPropDlg *dlg)
{
	dlg->OnMailChanged (gtk_entry_get_text (entry));
	return false;
}

static void on_comments_changed (GtkTextBuffer *buffer, gcDocPropDlg *dlg)
{
	GtkTextIter start, end;
	gtk_text_buffer_get_bounds (buffer, &start, &end);
	char *text = gtk_text_buffer_get_text (buffer, &start, &end, true);
	dlg->OnCommentsChanged (text);
	g_free (text);
}

gcDocPropDlg::gcDocPropDlg (gcDocument* pDoc):
	Dialog (pDoc->GetApp (), GLADEDIR"/docprop.glade", "properties", pDoc)
{
	if (!xml) {
		delete this;
		return;
	}
	m_pDoc = pDoc;
	Title = GTK_ENTRY (glade_xml_get_widget (xml, "title"));
	const gchar* chn;
	chn = m_pDoc->GetTitle ();
	if (chn)
		gtk_entry_set_text (Title, chn);
	g_signal_connect (G_OBJECT (Title), "activate", G_CALLBACK (on_title_changed), this);
	g_signal_connect (G_OBJECT (Title), "focus-out-event", G_CALLBACK (on_title_focused_out), this);
	Name = GTK_ENTRY (glade_xml_get_widget (xml, "name"));
	chn = m_pDoc->GetAuthor ();
	if (chn)
		gtk_entry_set_text (Name, chn);
	g_signal_connect (G_OBJECT (Name), "activate", G_CALLBACK (on_name_changed), this);
	g_signal_connect (G_OBJECT (Name), "focus-out-event", G_CALLBACK (on_name_focused_out), this);
	Mail = GTK_ENTRY (glade_xml_get_widget (xml, "mail"));
	chn = m_pDoc->GetMail ();
	if (chn)
		gtk_entry_set_text (Mail, chn);
	g_signal_connect (G_OBJECT (Mail), "activate", G_CALLBACK (on_mail_changed), this);
	g_signal_connect (G_OBJECT (Mail), "focus-out-event", G_CALLBACK (on_mail_focused_out), this);
	CreationDate = GTK_LABEL (glade_xml_get_widget(xml, "creation"));
	const GDate* Date = pDoc->GetCreationDate ();
	gchar tmp[64];
	/* The following format prints date as "Monday, July 8, 2002" */
	if (g_date_valid (Date)) {
		g_date_strftime (tmp, sizeof (tmp), _("%A, %B %d, %Y"), Date);
		gtk_label_set_text (CreationDate, tmp);
	}
	RevisionDate = GTK_LABEL (glade_xml_get_widget (xml, "revision"));
	Date = pDoc->GetRevisionDate ();
	if (g_date_valid(Date))
	{
		g_date_strftime (tmp, sizeof (tmp), _("%A, %B %d, %Y"), Date);
		gtk_label_set_text (RevisionDate, tmp);
	}
	Comments = GTK_TEXT_VIEW (glade_xml_get_widget (xml, "comments"));
	GtkTextBuffer *Buffer = gtk_text_view_get_buffer (Comments);
	chn = m_pDoc->GetComment ();
	if(chn)
		gtk_text_buffer_set_text (Buffer, chn , -1);
	g_signal_connect (G_OBJECT (Buffer), "changed", G_CALLBACK (on_comments_changed), this);
	gtk_widget_show_all(GTK_WIDGET (dialog));
}
	
gcDocPropDlg::~gcDocPropDlg ()
{
}

void gcDocPropDlg::OnTitleChanged (char const *title)
{
	m_pDoc->SetTitle (title);
	m_pDoc->RenameViews ();
	m_pDoc->SetDirty ();
}

void gcDocPropDlg::OnNameChanged (char const *name)
{
	m_pDoc->SetAuthor (name);
	m_pDoc->SetDirty ();
}

void gcDocPropDlg::OnMailChanged (char const *mail)
{
	m_pDoc->SetMail (mail);
	m_pDoc->SetDirty ();
}

void gcDocPropDlg::OnCommentsChanged (char const *comment)
{
	m_pDoc->SetComment (comment);
	m_pDoc->SetDirty ();
}

