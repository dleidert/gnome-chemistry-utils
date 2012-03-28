// -*- C++ -*-

/*
 * Gnome Crystal
 * celldlg.cc
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
 * USA
 */

#include "config.h"
#include "celldlg.h"
#include "document.h"
#include "application.h"
#include <gcu/spacegroup.h>
#include <glib/gi18n.h>

namespace gcr {

class CellDlgPrivate
{
public:
	static void OnSpaceGroupChanged (GtkSpinButton *btn, CellDlg *dlg);
	static void OnAutoSpaceGroupToggled (GtkToggleButton *btn, CellDlg *dlg);
	static void OnTypeChanged (CellDlg *dlg);
	static bool OnAEdited (CellDlg *dlg);
	static bool OnBEdited (CellDlg *dlg);
	static bool OnCEdited (CellDlg *dlg);
	static bool OnAlphaEdited (CellDlg *dlg);
	static bool OnBetaEdited (CellDlg *dlg);
	static bool OnGammaEdited (CellDlg *dlg);
};

#if 0
void on_type_changed (G_GNUC_UNUSED GtkWidget* w, CellDlg *dlg)
{
	dlg->OnTypeChanged ();
}
#endif

CellDlg::CellDlg (Application *App, Document* pDoc): gcugtk::Dialog (App, UIDIR"/cell.ui", "cell", GETTEXT_PACKAGE, pDoc)
{
	m_pDoc = pDoc;
	TypeMenu = GTK_COMBO_BOX (GetWidget ("bravais-box"));
	A = GTK_ENTRY (GetWidget ("a"));
	B = GTK_ENTRY (GetWidget ("b"));
	C = GTK_ENTRY (GetWidget ("c"));
	Alpha = GTK_ENTRY (GetWidget ("alpha"));
	Beta = GTK_ENTRY (GetWidget ("beta"));
	Gamma = GTK_ENTRY (GetWidget ("gamma"));
	AutoSpaceGroup = GTK_TOGGLE_BUTTON (GetWidget ("auto-space-group-btn"));
	g_signal_connect (G_OBJECT (AutoSpaceGroup), "toggled", G_CALLBACK (CellDlgPrivate::OnAutoSpaceGroupToggled), this);
	SpaceGroup = GTK_SPIN_BUTTON (GetWidget ("space-group-btn"));
	gtk_spin_button_set_value (SpaceGroup, m_pDoc->GetSpaceGroup ()->GetId ());
	SpaceGroupSignal = g_signal_connect (G_OBJECT (SpaceGroup), "value-changed", G_CALLBACK (CellDlgPrivate::OnSpaceGroupChanged), this);
	SpaceGroupAdj = gtk_spin_button_get_adjustment (SpaceGroup);
	gcr::Lattice i;
	m_pDoc->GetCell (&i, &m_a, &m_b, &m_c, &m_alpha, &m_beta, &m_gamma);
	snprintf (m_buf, sizeof (m_buf), "%g", m_a);
	gtk_entry_set_text (A, m_buf);
	snprintf (m_buf, sizeof (m_buf), "%g", m_b);
	gtk_entry_set_text (B, m_buf);
	snprintf (m_buf, sizeof (m_buf), "%g", m_c);
	gtk_entry_set_text (C, m_buf);
	snprintf (m_buf, sizeof (m_buf), "%g", m_alpha);
	gtk_entry_set_text (Alpha, m_buf);
	snprintf (m_buf, sizeof (m_buf), "%g", m_beta);
	gtk_entry_set_text (Beta, m_buf);
	snprintf (m_buf, sizeof (m_buf), "%g", m_gamma);
	gtk_entry_set_text (Gamma, m_buf);
	gtk_combo_box_set_active (TypeMenu, i);
	gtk_toggle_button_set_active (AutoSpaceGroup, m_pDoc->GetAutoSpaceGroup ());
	gtk_widget_set_sensitive (GTK_WIDGET (SpaceGroup), !m_pDoc->GetAutoSpaceGroup ());
	CellDlgPrivate::OnTypeChanged (this);
	TypeSignal = g_signal_connect_swapped (G_OBJECT (TypeMenu), "changed", G_CALLBACK (CellDlgPrivate::OnTypeChanged), this);
	gtk_widget_show_all (GTK_WIDGET (dialog));
	// connect entries signals
	g_signal_connect_swapped (G_OBJECT (A), "activate", G_CALLBACK (CellDlgPrivate::OnAEdited), this);
	ASignal = g_signal_connect_swapped (G_OBJECT (A), "focus-out-event", G_CALLBACK (CellDlgPrivate::OnAEdited), this);
	g_signal_connect_swapped (G_OBJECT (B), "activate", G_CALLBACK (CellDlgPrivate::OnBEdited), this);
	BSignal = g_signal_connect_swapped (G_OBJECT (B), "focus-out-event", G_CALLBACK (CellDlgPrivate::OnBEdited), this);
	g_signal_connect_swapped (G_OBJECT (C), "activate", G_CALLBACK (CellDlgPrivate::OnCEdited), this);
	CSignal = g_signal_connect_swapped (G_OBJECT (C), "focus-out-event", G_CALLBACK (CellDlgPrivate::OnCEdited), this);
	g_signal_connect_swapped (G_OBJECT (Alpha), "activate", G_CALLBACK (CellDlgPrivate::OnAlphaEdited), this);
	AlphaSignal = g_signal_connect_swapped (G_OBJECT (Alpha), "focus-out-event", G_CALLBACK (CellDlgPrivate::OnAlphaEdited), this);
	g_signal_connect_swapped (G_OBJECT (Beta), "activate", G_CALLBACK (CellDlgPrivate::OnBetaEdited), this);
	BetaSignal = g_signal_connect_swapped (G_OBJECT (Beta), "focus-out-event", G_CALLBACK (CellDlgPrivate::OnBetaEdited), this);
	g_signal_connect_swapped (G_OBJECT (Gamma), "activate", G_CALLBACK (CellDlgPrivate::OnGammaEdited), this);
	GammaSignal = g_signal_connect_swapped (G_OBJECT (Gamma), "focus-out-event", G_CALLBACK (CellDlgPrivate::OnGammaEdited), this);
}

CellDlg::~CellDlg ()
{
}

#if 0
bool CellDlg::Apply ()
{
	gcr::Lattice i = (gcr::Lattice) gtk_combo_box_get_active (TypeMenu);
	switch (i) {
	case gcr::cubic:
	case gcr::body_centered_cubic:
	case gcr::face_centered_cubic:
		if (!GetNumber (A, &m_a, gcugtk::Min, 0))
			return false;
		m_alpha = m_beta = m_gamma  = 90;
		m_b = m_c = m_a;
		break;
	case gcr::hexagonal:
		if ((!GetNumber (A, &m_a, gcugtk::Min, 0)) ||
			(!GetNumber (C, &m_c, gcugtk::Min, 0)))
			return false;
		m_alpha = m_beta = 90;
		m_gamma  = 120;
		m_b = m_a;
		break;
	case gcr::tetragonal:
	case gcr::body_centered_tetragonal:
		if ((!GetNumber (A, &m_a, gcugtk::Min, 0)) ||
			(!GetNumber (C, &m_c, gcugtk::Min, 0)))
			return false;
		m_alpha = m_beta = m_gamma  = 90;
		m_b = m_a;
		break;
	case gcr::orthorhombic:
	case gcr::base_centered_orthorhombic:
	case gcr::body_centered_orthorhombic:
	case gcr::face_centered_orthorhombic:
		if ((!GetNumber (A, &m_a, gcugtk::Min, 0)) ||
			(!GetNumber (B, &m_b, gcugtk::Min, 0)) ||
			(!GetNumber (C, &m_c, gcugtk::Min, 0)))
			return false;
		m_alpha = m_beta = m_gamma  = 90;
		break;
	case gcr::rhombohedral:
		if ((!GetNumber (A, &m_a, gcugtk::Min, 0)) ||
			(!GetNumber (Alpha, &m_alpha, gcugtk::MinMax, 0, 180)))
			return false;
		m_beta = m_gamma = m_alpha;
		m_b = m_c = m_a;
		break;
	case gcr::monoclinic:
	case gcr::base_centered_monoclinic:
		if ((!GetNumber (A, &m_a, gcugtk::Min, 0)) ||
			(!GetNumber (B, &m_b, gcugtk::Min, 0)) ||
			(!GetNumber (C, &m_c, gcugtk::Min, 0)) ||
			(!GetNumber (Beta, &m_beta, gcugtk::MinMax, 0, 180)))
			return false;
		m_alpha = m_gamma  = 90;
		break;
	case gcr::triclinic:
		if ((!GetNumber (A, &m_a, gcugtk::Min, 0)) ||
			(!GetNumber (B, &m_b, gcugtk::Min, 0)) ||
			(!GetNumber (C, &m_c, gcugtk::Min, 0)) ||
			(!GetNumber (Alpha, &m_alpha, gcugtk::MinMax, 0, 180)) ||
			(!GetNumber (Beta, &m_beta, gcugtk::MinMax, 0, 180)) ||
			(!GetNumber (Gamma, &m_gamma, gcugtk::MinMax, 0, 180)))
			return false;
		break;
	}
	if (m_alpha + m_beta + m_gamma >= 360) {
		GtkDialog* box = GTK_DIALOG (gtk_message_dialog_new (GTK_WINDOW (dialog), GTK_DIALOG_MODAL,
										GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, _("The sum of the three angles must be less than 360°")));
		gtk_dialog_run (box);
	}
	m_pDoc->SetCell (i, m_a, m_b, m_c, m_alpha, m_beta, m_gamma);
	if (gtk_toggle_button_get_active (AutoSpaceGroup)) {
		m_pDoc->SetAutoSpaceGroup (true);
		m_pDoc->SetSpaceGroup (NULL);
	} else {
		m_pDoc->SetAutoSpaceGroup (false);
		m_pDoc->SetSpaceGroup (gcu::SpaceGroup::GetSpaceGroup (gtk_spin_button_get_value (SpaceGroup)));
	}
	m_pDoc->Update ();
	m_pDoc->SetDirty (true);
	return true;
}

#endif

void CellDlgPrivate::OnSpaceGroupChanged (GtkSpinButton *btn, CellDlg *dlg)
{
	g_signal_handler_block (dlg->TypeMenu, dlg->TypeSignal);
	unsigned id = gtk_spin_button_get_value_as_int (btn);
	gcu::SpaceGroup const *sp = gcu::SpaceGroup::GetSpaceGroup (id);
	std::string name = sp->GetHMName ();
	dlg->m_pDoc->SetSpaceGroup (sp);
	if (id > 142 && id < 195) {
		if (id == 146 || id == 148 || id == 155 || id == 160 || id == 161 || id == 166 || id == 167)
			gtk_combo_box_set_active (dlg->TypeMenu, gcr::rhombohedral);
		else
			gtk_combo_box_set_active (dlg->TypeMenu, gcr::hexagonal);
		return;
	}
	switch (name[0]) {
	case 'P':
		if (id > 2) {
			if (id > 16) {
				if (id > 74) {
					if (id > 194) {
						// cubic
						gtk_combo_box_set_active (dlg->TypeMenu, gcr::cubic);
					} else {
						// tetragonal
						gtk_combo_box_set_active (dlg->TypeMenu, gcr::tetragonal);
					}
				} else {
					// orthorhombic
					gtk_combo_box_set_active (dlg->TypeMenu, gcr::orthorhombic);
				}
			} else {
				// monoclinic
				gtk_combo_box_set_active (dlg->TypeMenu, gcr::monoclinic);
			}
		}
		break;
	case 'I':
		if (id > 16) {
			if (id > 74) {
				if (id > 194) {
					// cubic
					gtk_combo_box_set_active (dlg->TypeMenu, gcr::body_centered_cubic);
				} else {
					// tetragonal
					gtk_combo_box_set_active (dlg->TypeMenu, gcr::body_centered_tetragonal);
				}
			} else {
				// orthorhombic
				gtk_combo_box_set_active (dlg->TypeMenu, gcr::body_centered_orthorhombic);
			}
		}
		break;
	case 'F':
		if (id > 16) {
			if (id > 194) {
				// cubic
				gtk_combo_box_set_active (dlg->TypeMenu, gcr::face_centered_cubic);
			} else {
				// orthorhombic
				gtk_combo_box_set_active (dlg->TypeMenu, gcr::face_centered_orthorhombic);
			}
		}
		break;
	default:
		if (id > 2) {
					gtk_combo_box_set_active (dlg->TypeMenu, gcr::rhombohedral);
			if (id > 16) {
				// orthorhombic
				gtk_combo_box_set_active (dlg->TypeMenu, gcr::base_centered_orthorhombic);
			} else {
				// monoclinic
				gtk_combo_box_set_active (dlg->TypeMenu, gcr::base_centered_monoclinic);
			}
		}
		break;
	}
	g_signal_handler_unblock (dlg->TypeMenu, dlg->TypeSignal);
	dlg->m_pDoc->Update ();
	dlg->m_pDoc->SetDirty (true);
}

void CellDlgPrivate::OnAutoSpaceGroupToggled (GtkToggleButton *btn, CellDlg *dlg)
{
	bool autosp = gtk_toggle_button_get_active (btn);
	gtk_widget_set_sensitive (GTK_WIDGET (dlg->SpaceGroup), !autosp);
	dlg->m_pDoc->SetAutoSpaceGroup (autosp);
	dlg->m_pDoc->Update ();
	dlg->m_pDoc->SetDirty (true);
}

void CellDlgPrivate::OnTypeChanged (CellDlg *dlg)
{
	gcr::Lattice i = (Lattice) gtk_combo_box_get_active (dlg->TypeMenu), lattice;
	gcu::SpaceGroup const *spg = dlg->m_pDoc->GetSpaceGroup ();
	std::string name = (spg)? spg->GetHMName (): "";
	unsigned id = gtk_spin_button_get_value (dlg->SpaceGroup);
	double a, b, c, alpha, beta, gamma;
	dlg->m_pDoc->GetCell (&lattice, &a, &b, &c, &alpha, &beta, &gamma);
	switch (i) {
	case gcr::cubic:
		if (!spg || spg->GetId () < 195 || name[0] != 'P')
			id = 195;
		goto cubic_end;
	case gcr::body_centered_cubic:
		if (!spg || spg->GetId () < 195 || name[0] != 'I')
			id = 197;
		goto cubic_end;
	case gcr::face_centered_cubic:
		if (!spg || spg->GetId () < 195 || name[0] != 'F')
			id = 196;
cubic_end:
		gtk_adjustment_set_lower (dlg->SpaceGroupAdj, 195);
		gtk_adjustment_set_upper (dlg->SpaceGroupAdj, 232);
		c = b = a;
		alpha = beta = gamma = 90;
		gtk_entry_set_text (dlg->B, gtk_entry_get_text (dlg->A));
		gtk_entry_set_text (dlg->C, gtk_entry_get_text (dlg->A));
		gtk_entry_set_text (dlg->Alpha, "90");
		gtk_entry_set_text (dlg->Beta, "90");
		gtk_entry_set_text (dlg->Gamma, "90");
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->Alpha), false);
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->Beta), false);
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->Gamma), false);
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->B), false);
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->C), false);
		break;
	case gcr::hexagonal:
		if (!spg || spg->GetId () < 143 || spg->GetId () > 194)
			id = 168;
		gtk_adjustment_set_lower (dlg->SpaceGroupAdj, 143);
		gtk_adjustment_set_upper (dlg->SpaceGroupAdj, 194);
		b = a;
		alpha = beta = 90;
		gamma = 120;
		gtk_entry_set_text (dlg->B, gtk_entry_get_text (dlg->A));
		gtk_entry_set_text (dlg->Alpha, "90");
		gtk_entry_set_text (dlg->Beta, "90");
		gtk_entry_set_text (dlg->Gamma, "120");
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->Alpha), false);
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->Beta), false);
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->Gamma), false);
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->B), false);
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->C), true);
		break;
	case gcr::tetragonal:
		if (!spg || spg->GetId () < 75 || spg->GetId () > 142 || name[0] != 'P')
			id = 75;
		goto tetragonal_end;
	case gcr::body_centered_tetragonal:
		if (!spg || spg->GetId () < 75 || spg->GetId () > 142 || name[0] != 'I')
			id = 79;
tetragonal_end:
		gtk_adjustment_set_lower (dlg->SpaceGroupAdj, 75);
		gtk_adjustment_set_upper (dlg->SpaceGroupAdj, 142);
		b = a;
		alpha = beta = gamma = 90;
		gtk_entry_set_text (dlg->B, gtk_entry_get_text (dlg->A));
		gtk_entry_set_text (dlg->Alpha, "90");
		gtk_entry_set_text (dlg->Beta, "90");
		gtk_entry_set_text (dlg->Gamma, "90");
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->Alpha), false);
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->Beta), false);
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->Gamma), false);
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->B), false);
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->C), true);
		break;
	case gcr::base_centered_orthorhombic:
		if (!spg || spg->GetId () < 16 || spg->GetId () > 74 || (name[0] != 'C' && name[0] != 'B' && name[0] != 'A'))
			id = 20;
		goto orthorhombic_end;
	case gcr::orthorhombic:
		if (!spg || spg->GetId () < 16 || spg->GetId () > 74 || name[0] != 'P')
			id = 16;
		goto orthorhombic_end;
	case gcr::body_centered_orthorhombic:
		if (!spg || spg->GetId () < 16 || spg->GetId () > 74 || name[0] != 'I')
			id = 23;
		goto orthorhombic_end;
	case gcr::face_centered_orthorhombic:
		if (!spg || spg->GetId () < 16 || spg->GetId () > 74 || name[0] != 'F')
			id = 22;
orthorhombic_end:
		gtk_adjustment_set_lower (dlg->SpaceGroupAdj, 16);
		gtk_adjustment_set_upper (dlg->SpaceGroupAdj, 74);
		alpha = beta = gamma = 90;
		gtk_entry_set_text (dlg->Alpha, "90");
		gtk_entry_set_text (dlg->Beta, "90");
		gtk_entry_set_text (dlg->Gamma, "90");
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->Alpha), false);
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->Beta), false);
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->Gamma), false);
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->B), true);
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->C), true);
		break;
	case gcr::rhombohedral:
		if (!spg || spg->GetId () < 143 || spg->GetId () > 194)
			id = 146;
		gtk_adjustment_set_lower (dlg->SpaceGroupAdj, 143);
		gtk_adjustment_set_upper (dlg->SpaceGroupAdj, 194);
		gamma = beta = alpha;
		gtk_entry_set_text (dlg->Beta, gtk_entry_get_text (dlg->Alpha));
		gtk_entry_set_text (dlg->Gamma, gtk_entry_get_text (dlg->Alpha));
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->Alpha), true);
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->Beta), false);
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->Gamma), false);
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->B), false);
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->C), false);
		break;
	case gcr::monoclinic:
		if (!spg || spg->GetId () < 3 || spg->GetId () > 16 || name[0] != 'P')
			id = 3;
		goto monoclinic_end;
	case gcr::base_centered_monoclinic:
		if (!spg || spg->GetId () < 3 || spg->GetId () > 16 || name[0] == 'P')
			id = 5;
monoclinic_end:
		gtk_adjustment_set_lower (dlg->SpaceGroupAdj, 3);
		gtk_adjustment_set_upper (dlg->SpaceGroupAdj, 15);
		alpha = gamma = 90;
		gtk_entry_set_text (dlg->Alpha, "90");
		gtk_entry_set_text (dlg->Gamma, "90");
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->Alpha), false);
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->Beta), true);
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->Gamma), false);
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->B), true);
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->C), true);
		break;
	case gcr::triclinic:
		if (!spg || spg->GetId () > 2)
			id = 1;
		gtk_adjustment_set_lower (dlg->SpaceGroupAdj, 1);
		gtk_adjustment_set_upper (dlg->SpaceGroupAdj, 2);
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->Alpha), true);
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->Beta), true);
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->Gamma), true);
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->B), true);
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->C), true);
		break;
	}
	g_signal_handler_block (dlg->SpaceGroup, dlg->SpaceGroupSignal);
	if (!spg || id != spg->GetId ()) {
		spg = NULL;
		dlg->m_pDoc->SetSpaceGroup (gcu::SpaceGroup::GetSpaceGroup (id));
	}
	dlg->m_pDoc->SetCell (i, a, b, c, alpha, beta, gamma);
	gtk_spin_button_set_value (dlg->SpaceGroup, (spg)? spg->GetId (): id);
	g_signal_handler_unblock (dlg->SpaceGroup, dlg->SpaceGroupSignal);
	dlg->m_pDoc->Update ();
	dlg->m_pDoc->SetDirty (true);
}

bool CellDlgPrivate::OnAEdited (CellDlg *dlg)
{
	g_signal_handler_block (dlg->A, dlg->ASignal);
	Lattice lattice;
	double val, a, b, c, alpha, beta, gamma;
	dlg->m_pDoc->GetCell (&lattice, &a, &b, &c, &alpha, &beta, &gamma);
	if (dlg->GetNumber (dlg->A, &val, gcugtk::Min, 0) && val != a) {
		switch (lattice) {
		case cubic:
		case body_centered_cubic:
		case face_centered_cubic:
		case rhombohedral:
			c = val;
			gtk_entry_set_text (dlg->C, gtk_entry_get_text (dlg->A));
		case hexagonal:
		case tetragonal:
		case body_centered_tetragonal:
			b = val;
			gtk_entry_set_text (dlg->B, gtk_entry_get_text (dlg->A));
			break;
		default:
			break;
		}
		dlg->m_pDoc->SetCell (lattice, val, b, c, alpha, beta, gamma);
		dlg->m_pDoc->Update ();
		dlg->m_pDoc->SetDirty (true);
	}
	g_signal_handler_unblock (dlg->A, dlg->ASignal);
	return false;
}

bool CellDlgPrivate::OnBEdited (CellDlg *dlg)
{
	g_signal_handler_block (dlg->B, dlg->BSignal);
	Lattice lattice;
	double val, a, b, c, alpha, beta, gamma;
	dlg->m_pDoc->GetCell (&lattice, &a, &b, &c, &alpha, &beta, &gamma);
	if (dlg->GetNumber (dlg->B, &val, gcugtk::Min, 0) && val != b) {
		dlg->m_pDoc->SetCell (lattice, a, val, c, alpha, beta, gamma);
		dlg->m_pDoc->Update ();
		dlg->m_pDoc->SetDirty (true);
	}
	g_signal_handler_unblock (dlg->B, dlg->BSignal);
	return false;
}

bool CellDlgPrivate::OnCEdited (CellDlg *dlg)
{
	g_signal_handler_block (dlg->C, dlg->CSignal);
	Lattice lattice;
	double val, a, b, c, alpha, beta, gamma;
	dlg->m_pDoc->GetCell (&lattice, &a, &b, &c, &alpha, &beta, &gamma);
	if (dlg->GetNumber (dlg->C, &val, gcugtk::Min, 0) && val != c) {
		dlg->m_pDoc->SetCell (lattice, a, b, val, alpha, beta, gamma);
		dlg->m_pDoc->Update ();
		dlg->m_pDoc->SetDirty (true);
	}
	g_signal_handler_unblock (dlg->C, dlg->CSignal);
	return false;
}

bool CellDlgPrivate::OnAlphaEdited (CellDlg *dlg)
{
	g_signal_handler_block (dlg->Alpha, dlg->AlphaSignal);
	Lattice lattice;
	double val, a, b, c, alpha, beta, gamma;
	dlg->m_pDoc->GetCell (&lattice, &a, &b, &c, &alpha, &beta, &gamma);
	if (dlg->GetNumber (dlg->Alpha, &val, gcugtk::Min, 0) && val != alpha) {
		if (lattice == rhombohedral) {
			gamma = beta = val;
			gtk_entry_set_text (dlg->Beta, gtk_entry_get_text (dlg->Alpha));
			gtk_entry_set_text (dlg->Gamma, gtk_entry_get_text (dlg->Alpha));
		}
		dlg->m_pDoc->SetCell (lattice, a, b, c, val, beta, gamma);
		dlg->m_pDoc->Update ();
		dlg->m_pDoc->SetDirty (true);
	}
	g_signal_handler_unblock (dlg->Alpha, dlg->AlphaSignal);
	return false;
}

bool CellDlgPrivate::OnBetaEdited (CellDlg *dlg)
{
	g_signal_handler_block (dlg->Beta, dlg->BetaSignal);
	Lattice lattice;
	double val, a, b, c, alpha, beta, gamma;
	dlg->m_pDoc->GetCell (&lattice, &a, &b, &c, &alpha, &beta, &gamma);
	if (dlg->GetNumber (dlg->Beta, &val, gcugtk::Min, 0) && val != beta) {
		dlg->m_pDoc->SetCell (lattice, a, b, c, alpha, val, gamma);
		dlg->m_pDoc->Update ();
		dlg->m_pDoc->SetDirty (true);
	}
	g_signal_handler_unblock (dlg->Beta, dlg->BetaSignal);
	return false;
}

bool CellDlgPrivate::OnGammaEdited (CellDlg *dlg)
{
	g_signal_handler_block (dlg->Gamma, dlg->GammaSignal);
	Lattice lattice;
	double val, a, b, c, alpha, beta, gamma;
	dlg->m_pDoc->GetCell (&lattice, &a, &b, &c, &alpha, &beta, &gamma);
	if (dlg->GetNumber (dlg->Gamma, &val, gcugtk::Min, 0) && val != gamma) {
		dlg->m_pDoc->SetCell (lattice, a, b, c, alpha, beta, val);
		dlg->m_pDoc->Update ();
		dlg->m_pDoc->SetDirty (true);
	}
	g_signal_handler_unblock (dlg->Gamma, dlg->GammaSignal);
	return false;
}

}	//	namespace gcr
