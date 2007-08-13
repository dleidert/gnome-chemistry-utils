// -*- C++ -*-

/*
 * GChemPaint residues plugin
 * residues-dlg.cc
 *
 * Copyright (C) 2007 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "residues-dlg.h"
#include "pseudo-atom.h"
#include <gcp/application.h>
#include <gcp/document.h>
#include <gcp/residue.h>
#include <gcp/theme.h>
#include <gcp/view.h>
#include <gcp/widgetdata.h>
#include <gdk/gdkkeysyms.h>

static bool on_key_release (GtkWidget* widget, GdkEventKey* ev, gcpResiduesDlg *dlg)
{
	return dlg->OnKeyRelease (ev);
}

static bool on_key_press (GtkWidget* widget, GdkEventKey* ev, gcpResiduesDlg *dlg)
{
	return dlg->OnKeyPress (ev);
}

static void on_page (GtkNotebook *book, GtkNotebookPage *page, int num_page, gcpResiduesDlg *dlg)
{
	dlg->SetPage (num_page);
}

static void on_cur_changed (GtkComboBox *box, gcpResiduesDlg *dlg)
{
}

static void on_save (gcpResiduesDlg *dlg)
{
	dlg->OnSave ();
}

static void on_delete (gcpResiduesDlg *dlg)
{
	dlg->OnDelete ();
}

static void on_symbol_activate (GtkEntry *entry, gcpResiduesDlg *dlg)
{
	dlg->OnSymbolActivate ();
}

static bool on_symbol_focus_out (GtkEntry *entry, GdkEventFocus *event, gcpResiduesDlg *dlg)
{
	on_symbol_activate (entry, dlg);
	return true;
}

static void on_name_activate (GtkEntry *entry, gcpResiduesDlg *dlg)
{
	dlg->OnNameActivate ();
}

static bool on_name_focus_out (GtkEntry *entry, GdkEventFocus *event, gcpResiduesDlg *dlg)
{
	on_name_activate (entry, dlg);
	return true;
}

static void on_generic_toggled (GtkToggleButton *btn, gcpResiduesDlg *dlg)
{
	dlg->SetGeneric (gtk_toggle_button_get_active (btn));
}

gcpResiduesDlg::gcpResiduesDlg (gcp::Application *App):
	Dialog (App, GLADEDIR"/residues.glade", "residues", App),
	gcp::Target (App)
{
	m_Document = new gcp::Document (App, true, NULL);
	GtkWidget *w = m_Document->GetView ()->CreateNewWidget ();
	GtkScrolledWindow* scroll = (GtkScrolledWindow*) gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (scroll, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (scroll, GTK_SHADOW_IN);
	gtk_scrolled_window_add_with_viewport (scroll, w);
	gtk_widget_set_size_request (GTK_WIDGET (scroll), 408, 308);
	gtk_widget_show (GTK_WIDGET (scroll));
	gtk_box_pack_start (GTK_BOX (glade_xml_get_widget (xml, "formula-box")), GTK_WIDGET (scroll), true, true, 0);
	gtk_widget_show_all (GTK_WIDGET (dialog));
	App->SetActiveDocument (m_Document);
	gcp::Theme *theme = m_Document->GetTheme ();
	double x = theme->GetFontSize () / PANGO_SCALE / 2. / theme->GetZoomFactor ();
	m_Atom = new gcpPseudoAtom (x, 150. / theme->GetZoomFactor ());
	gcp::Atom *atom1 = new gcp::Atom (6, x + theme->GetBondLength (), 150. / theme->GetZoomFactor (), 0.);
	m_Document->AddAtom (m_Atom);
	m_Document->AddAtom (atom1);
	gcp::Bond *bond = new gcp::Bond (m_Atom, atom1, 1);
	m_Document->AddBond (bond);
	atom1->Lock (true);
	bond->Lock (true);
	SetWindow (dialog);
	g_signal_connect (dialog, "key-press-event", G_CALLBACK (on_key_press), this);
	g_signal_connect (dialog, "key-release-event", G_CALLBACK (on_key_release), this);
	g_signal_connect (glade_xml_get_widget (xml, "residue-book"), "switch-page", G_CALLBACK (on_page), this);
	m_CurBox = GTK_COMBO_BOX (glade_xml_get_widget (xml, "cur-box"));
	gtk_combo_box_set_active (m_CurBox, 0);
	g_signal_connect (G_OBJECT (m_CurBox), "changed", G_CALLBACK (on_cur_changed), this);
	m_SaveBtn = glade_xml_get_widget (xml, "save");
	g_signal_connect_swapped (G_OBJECT (m_SaveBtn), "clicked", G_CALLBACK (on_save), this);
	m_DeleteBtn = glade_xml_get_widget (xml, "delete");
	g_signal_connect_swapped (G_OBJECT (m_DeleteBtn), "clicked", G_CALLBACK (on_delete), this);
	m_SymbolEntry = GTK_ENTRY (glade_xml_get_widget (xml, "symbol-entry"));
	g_signal_connect (G_OBJECT (m_SymbolEntry), "activate", G_CALLBACK (on_symbol_activate), this);
	g_signal_connect_after (G_OBJECT (m_SymbolEntry), "focus_out_event", G_CALLBACK (on_symbol_focus_out), this);
	m_ValidSymbols = false;
	m_NameEntry = GTK_ENTRY (glade_xml_get_widget (xml, "name-entry"));
	g_signal_connect (G_OBJECT (m_NameEntry), "activate", G_CALLBACK (on_name_activate), this);
	g_signal_connect_after (G_OBJECT (m_NameEntry), "focus_out_event", G_CALLBACK (on_name_focus_out), this);
	m_ValidName = false;
	w = glade_xml_get_widget (xml, "generic-btn");
	g_signal_connect (w, "toggled", G_CALLBACK (on_generic_toggled), this);
	m_Generic = false;
}

gcpResiduesDlg::~gcpResiduesDlg ()
{
}

void gcpResiduesDlg::Add ()
{
}

void gcpResiduesDlg::Remove ()
{
}

bool gcpResiduesDlg::Close ()
{
	Destroy ();
	return true;
}

bool gcpResiduesDlg::OnKeyPress (GdkEventKey *event)
{
	if (m_Page) {
		// Only when editing the residue
		switch (event->keyval) {
		case GDK_Delete:
		case GDK_Clear:
		case GDK_BackSpace: {
			// the molecule must not be deleted
			Object *mol = m_Atom->GetMolecule ();
			gcp::WidgetData *data = reinterpret_cast<gcp::WidgetData*> (g_object_get_data (reinterpret_cast<GObject*> (m_Document->GetWidget ()), "data"));
			if (data->IsSelected (mol)) {
				data->Unselect (mol);
				m_Document->GetView ()->OnDeleteSelection (m_Document->GetWidget ());
				data->SetSelected (mol);
				return false;
			}
			break;
		}
		default:
			break;
		}
		return m_Document->GetView ()->OnKeyPress (m_Document->GetWidget (), event);
	}
	return false;
}

bool gcpResiduesDlg::OnKeyRelease (GdkEventKey *event)
{
	if (m_Page)
		return m_Document->GetView ()->OnKeyRelease (m_Document->GetWidget (), event);
	return false;
}

void gcpResiduesDlg::OnCurChanged (int num)
{
}

void gcpResiduesDlg::OnSave ()
{
	char const *text = gtk_entry_get_text (m_SymbolEntry);
	char **symbols = g_strsplit (text, ";", 0);
	char const *name = gtk_entry_get_text (m_NameEntry);
	gcp::Residue *res = new gcp::Residue (name);
	char **s = symbols;
	while (s) {
		res->AddSymbol (*s);
		s++;
	}
	g_strfreev (symbols);
}

void gcpResiduesDlg::OnDelete ()
{
}

void gcpResiduesDlg::OnSymbolActivate ()
{
	char const *text = gtk_entry_get_text (m_SymbolEntry);
	char **symbols = g_strsplit (text, ";", 0);
	m_ValidSymbols = *symbols;
	g_strfreev (symbols);
	gtk_widget_set_sensitive (m_SaveBtn, m_ValidName && m_ValidSymbols);
}

void gcpResiduesDlg::OnNameActivate ()
{
	char const *text = gtk_entry_get_text (m_NameEntry);
	m_ValidName = strlen (text) > 0;
	gtk_widget_set_sensitive (m_SaveBtn, m_ValidName && m_ValidSymbols);
}
