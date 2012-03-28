// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gcugtk/stringdlg.cc
 *
 * Copyright (C) 2005-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "application.h"
#include "stringinputdlg.h"
#include <gcu/document.h>

namespace gcugtk {

StringInputDlg::StringInputDlg (gcu::Document *doc, StringInputCB cb, char const *title):
	gcugtk::Dialog (static_cast < Application * > (doc->GetApplication ()), UIDIR"/stringinputdlg.ui", "string-input", GETTEXT_PACKAGE, doc),
	m_Doc (doc),
	m_CB (cb)
{
	GtkWidget *w = GTK_WIDGET (gtk_builder_get_object (GetBuilder (), "string-input"));
	gtk_window_set_title (GTK_WINDOW (w), title);
	gtk_widget_show_all (w);
}

StringInputDlg::~StringInputDlg ()
{
}

bool StringInputDlg::Apply ()
{
	GtkEntry *entry = GTK_ENTRY (gtk_builder_get_object (GetBuilder (), "result"));
	if (entry)
		m_CB (m_Doc, gtk_entry_get_text (entry));
	return true;
}

}	//	namespace gcugtk
