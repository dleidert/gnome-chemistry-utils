// -*- C++ -*-

/*
 * GChemPaint libray
 * newfiledlg.cc
 *
 * Copyright (C) 2007-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "newfiledlg.h"
#include "application.h"
#include "theme.h"

using namespace gcu;
using namespace std;

namespace gcp {

static void on_theme_changed (GtkComboBoxText *box, NewFileDlg *dlg)
{
	dlg->SetTheme (TheThemeManager.GetTheme (gtk_combo_box_text_get_active_text (box)));
}

NewFileDlg::NewFileDlg (Application *App):
	gcugtk::Dialog (App, UIDIR"/newfiledlg.ui", "newfile", GETTEXT_PACKAGE, App),
	Object ()
{
	list <string> names = TheThemeManager.GetThemesNames ();
	list <string>::iterator i = names.begin (), end = names.end ();
	GtkWidget *w = GetWidget ("themes-grid");
	m_Box = GTK_COMBO_BOX_TEXT (gtk_combo_box_text_new ());
	g_object_set (G_OBJECT (m_Box), "hexpand", true, NULL);
	gtk_container_add (GTK_CONTAINER (w), GTK_WIDGET (m_Box));
	Theme *theme;
	m_Theme = TheThemeManager.GetTheme (*i);
	m_Lines = names.size ();
	for (; i != end; i++) {
		gtk_combo_box_text_append_text (m_Box, (*i).c_str ());
		theme = TheThemeManager.GetTheme (*i);
		if (theme)
			theme->AddClient (this);
	}
	gtk_combo_box_set_active (GTK_COMBO_BOX (m_Box), 0);
	m_ChangedSignal = g_signal_connect (G_OBJECT (m_Box), "changed", G_CALLBACK (on_theme_changed), this);
	gtk_widget_show_all (GTK_WIDGET (dialog));
}

NewFileDlg::~NewFileDlg ()
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

bool NewFileDlg::Apply ()
{
	dynamic_cast <Application*> (m_App)->OnFileNew (gtk_combo_box_text_get_active_text (m_Box));
	return true;
}

void NewFileDlg::OnThemeNamesChanged ()
{
	list <string> names = TheThemeManager.GetThemesNames ();
	list <string>::iterator i, end = names.end ();
	int n, nb = gtk_combo_box_get_active (GTK_COMBO_BOX (m_Box));
	g_signal_handler_block (m_Box, m_ChangedSignal);
	while (m_Lines--)
		gtk_combo_box_text_remove (m_Box, 0);
	for (i = names.begin (), n = 0; i != end; i++, n++) {
		gtk_combo_box_text_append_text (m_Box, (*i).c_str ());
		if (m_Theme == TheThemeManager.GetTheme (*i))
			nb = n;
	}
	m_Lines = names.size ();
	gtk_combo_box_set_active (GTK_COMBO_BOX (m_Box), nb);
	g_signal_handler_unblock (m_Box, m_ChangedSignal);
}

}	//	namespace gcp
