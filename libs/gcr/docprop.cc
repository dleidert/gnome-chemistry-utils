// -*- C++ -*-

/*
 * Gnome Crystal library
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
#include "window.h"
#include <glib/gi18n-lib.h>

namespace gcr {

class DocPropDlgPrivate
{
public:
	static void OnTitleChanged (GtkEntry *entry, Document *doc);
	static bool OnTitleFocusedOut (GtkEntry *entry, GdkEventFocus *event, Document *doc);
	static void OnNameChanged (GtkEntry *entry, Document *doc);
	static bool OnNameFocusedOut (GtkEntry *entry, GdkEventFocus *event, Document *doc);
	static void OnMailChanged (GtkEntry *entry, Document *doc);
	static bool OnMailFocusedOut (GtkEntry *entry, GdkEventFocus *event, Document *doc);
	static void OnCommentsChanged (GtkTextBuffer *buffer, Document *doc);
};

void DocPropDlgPrivate::OnTitleChanged (GtkEntry *entry, Document *doc)
{
	char const *txt = gtk_entry_get_text (entry);
	if (txt && *txt == 0)
		txt = NULL;
	doc->SetTitle (txt);
	doc->RenameViews ();
	doc->SetDirty ();
}

bool DocPropDlgPrivate::OnTitleFocusedOut (GtkEntry *entry, G_GNUC_UNUSED GdkEventFocus *event, Document *doc)
{
	char const *txt = gtk_entry_get_text (entry);
	if (txt && *txt == 0)
		txt = NULL;
	doc->SetTitle (txt);
	doc->RenameViews ();
	doc->SetDirty ();
	return false;
}

void DocPropDlgPrivate::OnNameChanged (GtkEntry *entry, Document *doc)
{
	doc->SetAuthor (gtk_entry_get_text (entry));
	doc->SetDirty ();
}

bool DocPropDlgPrivate::OnNameFocusedOut (GtkEntry *entry, G_GNUC_UNUSED GdkEventFocus *event, Document *doc)
{
	doc->SetAuthor (gtk_entry_get_text (entry));
	doc->SetDirty ();
	return false;
}

void DocPropDlgPrivate::OnMailChanged (GtkEntry *entry, Document *doc)
{
	doc->SetMail (gtk_entry_get_text (entry));
	doc->SetDirty ();
}

bool DocPropDlgPrivate::OnMailFocusedOut (GtkEntry *entry, G_GNUC_UNUSED GdkEventFocus *event, Document *doc)
{
	doc->SetMail (gtk_entry_get_text (entry));
	doc->SetDirty ();
	return false;
}

void DocPropDlgPrivate::OnCommentsChanged (GtkTextBuffer *buffer, Document *doc)
{
	GtkTextIter start, end;
	gtk_text_buffer_get_bounds (buffer, &start, &end);
	char *text = gtk_text_buffer_get_text (buffer, &start, &end, true);
	doc->SetComment (text);
	g_free (text);
	doc->SetDirty ();
}

DocPropDlg::DocPropDlg (Document* pDoc):
	gcugtk::Dialog (static_cast < gcugtk::Application * > (pDoc->GetApp ()), UIDIR"/docprop.ui", "properties", GETTEXT_PACKAGE, pDoc)
{
	m_pDoc = pDoc;
	Title = GTK_ENTRY (GetWidget ("title"));
	char const *chn;
	chn = m_pDoc->GetTitle ();
	if (chn)
		gtk_entry_set_text (Title, chn);
	g_signal_connect (G_OBJECT (Title), "activate", G_CALLBACK (DocPropDlgPrivate::OnTitleChanged), pDoc);
	g_signal_connect (G_OBJECT (Title), "focus-out-event", G_CALLBACK (DocPropDlgPrivate::OnTitleFocusedOut), pDoc);
	Name = GTK_ENTRY (GetWidget ("name"));
	chn = m_pDoc->GetAuthor ();
	if (chn)
		gtk_entry_set_text (Name, chn);
	g_signal_connect (G_OBJECT (Name), "activate", G_CALLBACK (DocPropDlgPrivate::OnNameChanged), pDoc);
	g_signal_connect (G_OBJECT (Name), "focus-out-event", G_CALLBACK (DocPropDlgPrivate::OnNameFocusedOut), pDoc);
	Mail = GTK_ENTRY (GetWidget ("mail"));
	chn = m_pDoc->GetMail ();
	if (chn)
		gtk_entry_set_text (Mail, chn);
	g_signal_connect (G_OBJECT (Mail), "activate", G_CALLBACK (DocPropDlgPrivate::OnMailChanged), pDoc);
	g_signal_connect (G_OBJECT (Mail), "focus-out-event", G_CALLBACK (DocPropDlgPrivate::OnMailFocusedOut), pDoc);
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
	GtkTextBuffer *Buffer = gtk_text_view_get_buffer (Comments);
	chn = m_pDoc->GetComment ();
	if(chn)
		gtk_text_buffer_set_text (Buffer, chn , -1);
	g_signal_connect (G_OBJECT (Buffer), "changed", G_CALLBACK (DocPropDlgPrivate::OnCommentsChanged), pDoc);
	gtk_widget_show_all(GTK_WIDGET (dialog));
}

DocPropDlg::~DocPropDlg ()
{
}

}	//	namespace gcr
