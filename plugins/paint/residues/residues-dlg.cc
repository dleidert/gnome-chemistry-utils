// -*- C++ -*-

/*
 * GChemPaint residues plugin
 * residues-dlg.cc
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
#include "residues-dlg.h"
#include "pseudo-atom.h"
#include <gcp/application.h>
#include <gcp/bond.h>
#include <gcp/document.h>
#include <gcp/molecule.h>
#include <gcp/residue.h>
#include <gcp/theme.h>
#include <gcp/view.h>
#include <gcp/widgetdata.h>
#include <gdk/gdkkeysyms.h>
#include <glib/gi18n-lib.h>
#include <sstream>

extern xmlDocPtr user_residues;
set<xmlDocPtr> docs;


static bool on_key_release (G_GNUC_UNUSED GtkWidget* widget, GdkEventKey* ev, gcpResiduesDlg *dlg)
{
	return dlg->OnKeyRelease (ev);
}

static bool on_key_press (G_GNUC_UNUSED GtkWidget* widget, GdkEventKey* ev, gcpResiduesDlg *dlg)
{
	return dlg->OnKeyPress (ev);
}

static void on_page (G_GNUC_UNUSED GtkNotebook *book, G_GNUC_UNUSED void *page, int num_page, gcpResiduesDlg *dlg)
{
	dlg->SetPage (num_page);
}

static void on_cur_changed (G_GNUC_UNUSED GtkComboBox *box, gcpResiduesDlg *dlg)
{
	dlg->OnCurChanged ();
}

static void on_save (gcpResiduesDlg *dlg)
{
	dlg->Add ();
}

static void on_delete (gcpResiduesDlg *dlg)
{
	dlg->Remove ();
}

static void on_symbol_activate (G_GNUC_UNUSED GtkEntry *entry, gcpResiduesDlg *dlg)
{
	dlg->OnSymbolActivate ();
}

static bool on_symbol_focus_out (GtkEntry *entry, G_GNUC_UNUSED GdkEventFocus *event, gcpResiduesDlg *dlg)
{
	on_symbol_activate (entry, dlg);
	return true;
}

static void on_name_activate (G_GNUC_UNUSED GtkEntry *entry, gcpResiduesDlg *dlg)
{
	dlg->OnNameActivate ();
}

static bool on_name_focus_out (GtkEntry *entry, G_GNUC_UNUSED GdkEventFocus *event, gcpResiduesDlg *dlg)
{
	on_name_activate (entry, dlg);
	return true;
}

static void on_generic_toggled (GtkToggleButton *btn, gcpResiduesDlg *dlg)
{
	dlg->SetGeneric (gtk_toggle_button_get_active (btn));
}

static void on_page_changed (GtkNotebook *book, gcpResiduesDlg *dlg)
{
	dlg->SetPage (gtk_notebook_get_current_page (book));
}

static int insert_symbol (GtkComboBox *box, char const *str)
{
	GtkTreeModel *model = gtk_combo_box_get_model (box);
	GtkTreeIter iter;
	int i = 1;
	if (!gtk_tree_model_get_iter_from_string (model, &iter, "1")) {
		gtk_list_store_append (GTK_LIST_STORE (model), &iter);
		gtk_list_store_set (GTK_LIST_STORE (model), &iter, 0, str, -1);
		return i;
	}
	char* text;
	gtk_tree_model_get (model, &iter, 0, &text, -1);
	while (strcmp (text, str) < 0) {
		if (gtk_tree_model_iter_next (model, &iter))
			gtk_tree_model_get (model, &iter, 0, &text, -1);
		else {
			gtk_list_store_append (GTK_LIST_STORE (model), &iter);
			gtk_list_store_set (GTK_LIST_STORE (model), &iter, 0, str, -1);
			return i + 1;
		}
		i++;
	}
		gtk_list_store_insert (GTK_LIST_STORE (model), &iter, i);
		gtk_list_store_set (GTK_LIST_STORE (model), &iter, 0, str, -1);
	return i;
}

static void delete_symbol (GtkComboBox *box, char const *str)
{
	GtkTreeModel *model = gtk_combo_box_get_model (box);
	GtkTreeIter iter;
	if (!gtk_tree_model_get_iter_from_string (model, &iter, "1"))
		return;
	char* text;
	gtk_tree_model_get (model, &iter, 0, &text, -1);
	while (strcmp (text, str) < 0) {
		if (gtk_tree_model_iter_next (model, &iter))
			gtk_tree_model_get (model, &iter, 0, &text, -1);
		else
			return;
	}
	gtk_list_store_remove (GTK_LIST_STORE (model), &iter);
}

gcpResiduesDlg::gcpResiduesDlg (gcp::Application *App):
	gcugtk::Dialog (App, UIDIR"/residues.ui", "residues", GETTEXT_PACKAGE, App),
	gcp::Target (App)
{
	m_Document = new gcp::Document (App, true, NULL);
	m_Document->SetAllowClipboard (false);
	GtkWidget *w = m_Document->GetView ()->CreateNewWidget ();
	GtkScrolledWindow* scroll = (GtkScrolledWindow*) gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (scroll, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (scroll, GTK_SHADOW_IN);
	gtk_scrolled_window_add_with_viewport (scroll, w);
	gtk_widget_set_size_request (GTK_WIDGET (scroll), 408, 308);
	g_object_set (G_OBJECT (scroll), "expand", true, NULL);
	gtk_widget_show (GTK_WIDGET (scroll));
	gtk_grid_attach(GTK_GRID (GetWidget ("formula-grid")), GTK_WIDGET (scroll), 0, 0, 1, 1);
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
	g_signal_connect (GetWidget ("residue-book"), "switch-page", G_CALLBACK (on_page), this);
	m_CurBox = GTK_COMBO_BOX (GetWidget ("cur-box"));
	ResidueIterator i;
	string const *s = Residue::GetFirstResidueSymbol (i);
	GtkListStore *list = GTK_LIST_STORE (gtk_combo_box_get_model (m_CurBox));
	GtkTreeIter iter;
	while (s) {
		gtk_list_store_append (list, &iter);
		gtk_list_store_set (list, &iter, 0, s->c_str (), -1);
		s = Residue::GetNextResidueSymbol (i);
	}
	gtk_combo_box_set_active (m_CurBox, 0);
	g_signal_connect (G_OBJECT (m_CurBox), "changed", G_CALLBACK (on_cur_changed), this);
	m_SaveBtn = GetWidget ("save");
	g_signal_connect_swapped (G_OBJECT (m_SaveBtn), "clicked", G_CALLBACK (on_save), this);
	m_DeleteBtn = GetWidget ("delete");
	g_signal_connect_swapped (G_OBJECT (m_DeleteBtn), "clicked", G_CALLBACK (on_delete), this);
	m_SymbolEntry = GTK_ENTRY  (GetWidget ("symbol-entry"));
	g_signal_connect (G_OBJECT (m_SymbolEntry), "activate", G_CALLBACK (on_symbol_activate), this);
	g_signal_connect_after (G_OBJECT (m_SymbolEntry), "focus_out_event", G_CALLBACK (on_symbol_focus_out), this);
	m_ValidSymbols = false;
	m_NameEntry = GTK_ENTRY (GetWidget ("name-entry"));
	g_signal_connect (G_OBJECT (m_NameEntry), "activate", G_CALLBACK (on_name_activate), this);
	g_signal_connect_after (G_OBJECT (m_NameEntry), "focus_out_event", G_CALLBACK (on_name_focus_out), this);
	m_ValidName = false;
	m_GenericBtn = GetWidget ("generic-btn");
	g_signal_connect (m_GenericBtn, "toggled", G_CALLBACK (on_generic_toggled), this);
	m_Generic = false;
	m_Residue = NULL;
	m_Page = 0;
	g_signal_connect (GetWidget ("residue-book"), "change-current-page", G_CALLBACK (on_page_changed), this);
}

gcpResiduesDlg::~gcpResiduesDlg ()
{
}

void gcpResiduesDlg::Add ()
{
	xmlDocPtr xml;
	xmlNodePtr node, child;
	if (m_Document->GetChildrenNumber () != 1) {
		GtkDialog* box = GTK_DIALOG (gtk_message_dialog_new (GTK_WINDOW(dialog), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, _("Please, provide only one molecule.")));
		gtk_window_set_icon_name (GTK_WINDOW (box), m_App->GetName ().c_str ());
		if (gtk_dialog_run (box) != GTK_RESPONSE_NONE)
			gtk_widget_destroy (GTK_WIDGET (box));
		return;
	}
	if (!user_residues) {
		user_residues = xmlNewDoc ((xmlChar*) "1.0");
		docs.insert (user_residues);
		xmlDocSetRootElement (user_residues,  xmlNewDocNode (user_residues, NULL, (xmlChar*) "residues", NULL));
		char* filename = g_strconcat (getenv ("HOME"), "/.gchemutils/residues.xml", NULL);
		user_residues->URL = xmlStrdup ((xmlChar*) filename);
		g_free (filename);
	}
	char const *name = gtk_entry_get_text (m_NameEntry);
	if (*name == 0) {
		GtkDialog* box = GTK_DIALOG (gtk_message_dialog_new (GTK_WINDOW(dialog), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, _("Please, provide a name for the residue")));
		gtk_window_set_icon_name (GTK_WINDOW (box), m_App->GetName ().c_str ());
		if (gtk_dialog_run (box) != GTK_RESPONSE_NONE)
			gtk_widget_destroy (GTK_WIDGET (box));
		return;
	}
	gcp::Residue const *r = static_cast<gcp::Residue const *> (gcp::Residue::GetResiduebyName (name));
	if (r && r != m_Residue) {

	}
	char const *symbols = gtk_entry_get_text (m_SymbolEntry);
	std::istringstream s(symbols);
	std::list<string> sl;
	char buf[10];
	while (!s.eof ()) {
		s.getline(buf, 10, ';');
		if (strlen (buf) > 8) {
			// Symbols longer than 8 chars are not currently allowed
		GtkDialog* box = GTK_DIALOG (gtk_message_dialog_new (GTK_WINDOW(dialog), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, _("Symbols with more than eight characters are not allowed.")));
		gtk_window_set_icon_name (GTK_WINDOW (box), m_App->GetName ().c_str ());
		if (gtk_dialog_run (box) != GTK_RESPONSE_NONE)
			gtk_widget_destroy (GTK_WIDGET (box));
		return;
		} else if (!strcmp (buf, _("New"))) {
			GtkDialog* box = GTK_DIALOG (gtk_message_dialog_new (GTK_WINDOW(dialog), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, _("\"New\" is not a valid symbol")));
			gtk_window_set_icon_name (GTK_WINDOW (box), m_App->GetName ().c_str ());
			if (gtk_dialog_run (box) != GTK_RESPONSE_NONE)
				gtk_widget_destroy (GTK_WIDGET (box));
			return;
		} else
			sl.push_back (buf);
	}
	if (sl.size () == 0) {
		GtkDialog* box = GTK_DIALOG (gtk_message_dialog_new (GTK_WINDOW(dialog), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, _("Please provide at least one symbol")));
		gtk_window_set_icon_name (GTK_WINDOW (box), m_App->GetName ().c_str ());
		if (gtk_dialog_run (box) != GTK_RESPONSE_NONE)
			gtk_widget_destroy (GTK_WIDGET (box));
		return;
	}
	std::list<string>::reverse_iterator i, iend = sl.rend ();
	for (i = sl.rbegin (); i != iend; i++) {
		r = static_cast<gcp::Residue const *> (gcp::Residue::GetResidue ((*i).c_str ()));
		if (r && r != m_Residue) {
			char *mess = g_strdup_printf (_("%s is already used by another residue."), (*i).c_str ());
			GtkDialog* box = GTK_DIALOG (gtk_message_dialog_new (GTK_WINDOW(dialog), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,mess));
			gtk_window_set_icon_name (GTK_WINDOW (box), m_App->GetName ().c_str ());
			if (gtk_dialog_run (box) != GTK_RESPONSE_NONE)
				gtk_widget_destroy (GTK_WIDGET (box));
			g_free (mess);
			return;
		}
	}
	gcp::Molecule *mol = dynamic_cast<gcp::Molecule*> (m_Atom->GetMolecule ());
	string raw = mol->GetRawFormula ();
	if (raw.length () == 0) {
		GtkDialog* box = GTK_DIALOG (gtk_message_dialog_new (GTK_WINDOW(dialog), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, _("Empty formula, this should never happen.\nPlease file a bug report")));
		gtk_window_set_icon_name (GTK_WINDOW (box), m_App->GetName ().c_str ());
		if (gtk_dialog_run (box) != GTK_RESPONSE_NONE)
			gtk_widget_destroy (GTK_WIDGET (box));
		return;
	}
	// If we are there, everything is OK.
	char *new_name = g_strdup (name), *new_symbols = g_strdup (symbols);;
	Remove (); // remove the old version if any
	m_Residue = new gcp::Residue (new_name);
	int n = -1;
	for (i = sl.rbegin (); i != iend; i++) {
		m_Residue->AddSymbol ((*i).c_str ());
		n = insert_symbol (m_CurBox, (*i).c_str ());
	}
	node = xmlNewDocNode (user_residues, NULL, (xmlChar const *) "residue", NULL);
	if (m_Generic)
		xmlNewProp (node, (xmlChar const *) "generic", (xmlChar const *) "true");
	xmlNewProp (node, (xmlChar const *) "raw", (xmlChar const *) raw.c_str ());
	m_Residue->SetGeneric (m_Generic);
	child = xmlNewDocNode (user_residues, NULL, (xmlChar const *) "symbols", (xmlChar const *) new_symbols);
	g_free (new_symbols);
	xmlAddChild (node, child);
	child = xmlNewDocNode (user_residues, NULL, (xmlChar const *) "name", (xmlChar const *) new_name);
	g_free (new_name);
	xmlAddChild (node, child);
	xml = m_Document->BuildXMLTree ();
	child = xml->children->children;
	while (strcmp ((char const *) child->name, "molecule"))
		child = child->next;
	xmlUnlinkNode (child);
	xmlAddChild (node, child);
	xmlAddChild (user_residues->children, node);
	xmlIndentTreeOutput = true;
	xmlKeepBlanksDefault (0);
	xmlSaveFormatFile ((char*) user_residues->URL, user_residues, true);
	xmlFreeDoc (xml);
	m_Residue->Load (node, false, m_App);
	gtk_combo_box_set_active (m_CurBox, n);
}

void gcpResiduesDlg::Remove ()
{
	if (m_Residue == NULL)
		return;
	gcp::Residue *residue = m_Residue;
	gtk_combo_box_set_active (m_CurBox, 0);
	xmlUnlinkNode (residue->GetNode ());
	xmlFreeNode (residue->GetNode ());
	map<string, bool> const &symbols = residue->GetSymbols ();
	map<string, bool>::const_iterator i, end = symbols.end ();
	for (i = symbols.begin (); i != end; i++)
		delete_symbol (m_CurBox, (*i).first.c_str ());
	delete residue;
	xmlIndentTreeOutput = true;
	xmlKeepBlanksDefault (0);
	xmlSaveFormatFile ((char*) user_residues->URL, user_residues, true);
}

bool gcpResiduesDlg::Close ()
{
	Dialog::Destroy ();
	return true;
}

bool gcpResiduesDlg::OnKeyPress (GdkEventKey *event)
{
	if (m_Page) {
// Next lines are commented out because they just do not work properly: FIXME if possible
		if (event->state & GDK_CONTROL_MASK) {
			switch (event->keyval) {
			case GDK_KEY_Z:
				m_Document->OnRedo ();
				break;
			case GDK_KEY_z:
				m_Document->OnUndo ();
				break;
			}
			return false;
		}
		// Only when editing the residue
		switch (event->keyval) {
		case GDK_KEY_Delete:
		case GDK_KEY_Clear:
		case GDK_KEY_BackSpace: {
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

void gcpResiduesDlg::OnCurChanged ()
{
	char *symbol;
	GtkTreeIter iter;
	GtkTreePath *path = gtk_tree_path_new_from_indices (gtk_combo_box_get_active (m_CurBox), -1);
	GtkTreeModel *model = gtk_combo_box_get_model (m_CurBox);
	gtk_tree_model_get_iter (model, &iter, path);
	gtk_tree_path_free (path);
	gtk_tree_model_get (model, &iter, 0, &symbol, -1);
	if (!strcmp (symbol, _("New"))) {
		m_Residue = NULL;
		gtk_entry_set_text (m_NameEntry, "");
		gtk_entry_set_text (m_SymbolEntry, "");
		gtk_widget_set_sensitive (reinterpret_cast<GtkWidget*> (m_NameEntry), true);
		gtk_widget_set_sensitive (reinterpret_cast<GtkWidget*> (m_SymbolEntry), true);
		gtk_widget_set_sensitive (m_SaveBtn, false);
		gtk_widget_set_sensitive (m_DeleteBtn, false);
		gtk_widget_set_sensitive (m_GenericBtn, true);
		m_Document->SetEditable (true);
		g_free (symbol);
		return;
	}
	m_Residue = const_cast <gcp::Residue *> (static_cast<gcp::Residue const*> (Residue::GetResidue (symbol)));
	if (m_Residue->GetReadOnly ()) {
		gtk_widget_set_sensitive (reinterpret_cast<GtkWidget*> (m_NameEntry), false);
		gtk_widget_set_sensitive (reinterpret_cast<GtkWidget*> (m_SymbolEntry), false);
		gtk_widget_set_sensitive (m_SaveBtn, false);
		gtk_widget_set_sensitive (m_DeleteBtn, false);
		gtk_widget_set_sensitive (m_GenericBtn, false);
		m_Document->SetEditable (false);
	} else {
		gtk_widget_set_sensitive (reinterpret_cast<GtkWidget*> (m_NameEntry), true);
		gtk_widget_set_sensitive (reinterpret_cast<GtkWidget*> (m_SymbolEntry), true);
		gtk_widget_set_sensitive (m_SaveBtn, true);
		gtk_widget_set_sensitive (m_DeleteBtn, m_Residue->GetRefs () == 0);
		gtk_widget_set_sensitive (m_GenericBtn, true);
		m_Document->SetEditable (true);
	}
	gtk_entry_set_text (m_NameEntry, m_Residue->GetName ());
	map<string, bool> const &symbols = m_Residue->GetSymbols ();
	map<string, bool>::const_iterator i = symbols.begin (), end = symbols.end ();
	string sy;
	if (i != symbols.end ())
		sy = (*i).first;
	for (i++; i != end; i++)
		sy += string(";") + (*i).first;
	gtk_entry_set_text (m_SymbolEntry, sy.c_str ());
	m_Document->Clear ();
	m_Document->LoadObjects (m_Residue->GetMolNode ());
	double r =  m_Document->GetTheme ()->GetBondLength () / m_Document->GetMedianBondLength ();
	if (fabs (r - 1.) > .0001) {
		Matrix2D m (r, 0., 0., r);
		m_Document->Transform2D (m, 0., 0.);
		m_Document->GetView ()->Update (m_Document);
		m_Document->GetView ()->EnsureSize ();
	}
	m_Document->GetView ()->EnsureSize ();
	m_Atom = dynamic_cast <gcpPseudoAtom *> (m_Document->GetDescendant ("a1"));
	map<gcu::Atom*, gcu::Bond*>::iterator j;
	m_Atom->GetFirstBond (j);
	static_cast <gcp::Atom *>((*j).first)->Lock ();
	static_cast <gcp::Bond *>((*j).second)->Lock ();
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (m_GenericBtn), m_Residue->GetGeneric ());
	g_free (symbol);
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

void gcpResiduesDlg::OnNewResidue (gcp::Residue *res)
{
	if (res) {
		map<string, bool> const &symbols = res->GetSymbols ();
		map<string, bool>::const_iterator i = symbols.begin (), end = symbols.end ();
		GtkListStore *list = GTK_LIST_STORE (gtk_combo_box_get_model (m_CurBox));
		GtkTreeIter iter;
		for (i = symbols.begin (); i != end; i++) {
			gtk_list_store_append (list, &iter);
			gtk_list_store_set (list, &iter, 0, (*i).first.c_str (), -1);
		}
	} else if (m_Residue && !m_Residue->GetReadOnly ())
		gtk_widget_set_sensitive (m_DeleteBtn, m_Residue->GetRefs () == 0);
}
