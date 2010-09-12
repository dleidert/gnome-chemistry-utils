// -*- C++ -*-

/* 
 * Gnome Crystal
 * celldlg.cc 
 *
 * Copyright (C) 2002-2010 Jean Bréfort <jean.brefort@normalesup.org>
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
	// TODO move OnTypeChanged there
};

void on_type_changed (G_GNUC_UNUSED GtkWidget* w, CellDlg *pBox)
{
	pBox->OnTypeChanged ();
}

CellDlg::CellDlg (Application *App, Document* pDoc): gcu::Dialog (App, UIDIR"/cell.ui", "cell", GETTEXT_PACKAGE, pDoc)
{
	m_pDoc = pDoc;
	TypeMenu = GTK_COMBO_BOX (GetWidget ("lattice-type"));
	TypeSignal = g_signal_connect (G_OBJECT (TypeMenu), "changed", G_CALLBACK (on_type_changed), this);
	A = GTK_ENTRY (GetWidget ("a"));
	B = GTK_ENTRY (GetWidget ("b"));
	C = GTK_ENTRY (GetWidget ("c"));
	Alpha = GTK_ENTRY (GetWidget ("alpha"));
	Beta = GTK_ENTRY (GetWidget ("beta"));
	Gamma = GTK_ENTRY (GetWidget ("gamma"));
	AutoSpaceGroup = GTK_TOGGLE_BUTTON (GetWidget ("auto-space-group-btn"));
	g_signal_connect (G_OBJECT (AutoSpaceGroup), "toggled", G_CALLBACK (CellDlgPrivate::OnAutoSpaceGroupToggled), this);
	SpaceGroup = GTK_SPIN_BUTTON (GetWidget ("space-group-btn"));
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
	OnTypeChanged ();
}

CellDlg::~CellDlg ()
{
}

bool CellDlg::Apply ()
{
	gcr::Lattice i = (gcr::Lattice) gtk_combo_box_get_active (TypeMenu);
	switch (i) {
	case gcr::cubic:
	case gcr::body_centered_cubic:
	case gcr::face_centered_cubic:
		if (!GetNumber (A, &m_a, gcu::Min, 0))
			return false;
		m_alpha = m_beta = m_gamma  = 90;
		m_b = m_c = m_a;
		break;
	case gcr::hexagonal:
		if ((!GetNumber (A, &m_a, gcu::Min, 0)) ||
			(!GetNumber (C, &m_c, gcu::Min, 0)))
			return false;
		m_alpha = m_beta = 90;
		m_gamma  = 120;
		m_b = m_a;
		break;
	case gcr::tetragonal:
	case gcr::body_centered_tetragonal:
		if ((!GetNumber (A, &m_a, gcu::Min, 0)) ||
			(!GetNumber (C, &m_c, gcu::Min, 0)))
			return false;
		m_alpha = m_beta = m_gamma  = 90;
		m_b = m_a;
		break;
	case gcr::orthorhombic:
	case gcr::base_centered_orthorhombic:
	case gcr::body_centered_orthorhombic:
	case gcr::face_centered_orthorhombic:
		if ((!GetNumber (A, &m_a, gcu::Min, 0)) ||
			(!GetNumber (B, &m_b, gcu::Min, 0)) ||
			(!GetNumber (C, &m_c, gcu::Min, 0)))
			return false;
		m_alpha = m_beta = m_gamma  = 90;
		break;
	case gcr::rhombohedral:
		if ((!GetNumber (A, &m_a, gcu::Min, 0)) ||
			(!GetNumber (Alpha, &m_alpha, gcu::MinMax, 0, 180)))
			return false;
		m_beta = m_gamma = m_alpha;
		m_b = m_c = m_a;
		break;
	case gcr::monoclinic:
	case gcr::base_centered_monoclinic:
		if ((!GetNumber (A, &m_a, gcu::Min, 0)) ||
			(!GetNumber (B, &m_b, gcu::Min, 0)) ||
			(!GetNumber (C, &m_c, gcu::Min, 0)) ||
			(!GetNumber (Beta, &m_beta, gcu::MinMax, 0, 180)))
			return false;
		m_alpha = m_gamma  = 90;
		break;
	case gcr::triclinic:
		if ((!GetNumber (A, &m_a, gcu::Min, 0)) ||
			(!GetNumber (B, &m_b, gcu::Min, 0)) ||
			(!GetNumber (C, &m_c, gcu::Min, 0)) ||
			(!GetNumber (Alpha, &m_alpha, gcu::MinMax, 0, 180)) ||
			(!GetNumber (Beta, &m_beta, gcu::MinMax, 0, 180)) ||
			(!GetNumber (Gamma, &m_gamma, gcu::MinMax, 0, 180)))
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

void CellDlg::OnTypeChanged ()
{
	gcr::Lattice i = (gcr::Lattice) gtk_combo_box_get_active (TypeMenu);
	gcu::SpaceGroup const *spg = m_pDoc->GetSpaceGroup ();
	std::string name = (spg)? spg->GetHMName (): "";
	unsigned id = gtk_spin_button_get_value (SpaceGroup);
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
		gtk_adjustment_set_lower (SpaceGroupAdj, 195);
		gtk_adjustment_set_upper (SpaceGroupAdj, 232);
		gtk_entry_set_text (Alpha, "90");
		gtk_entry_set_text (Beta, "90");
		gtk_entry_set_text (Gamma, "90");
		gtk_widget_set_sensitive (GTK_WIDGET (Alpha), false);
		gtk_widget_set_sensitive (GTK_WIDGET (Beta), false);
		gtk_widget_set_sensitive (GTK_WIDGET (Gamma), false);
		gtk_widget_set_sensitive (GTK_WIDGET (B), false);
		gtk_widget_set_sensitive (GTK_WIDGET (C), false);
		break;
	case gcr::hexagonal:
		if (!spg || spg->GetId () < 143 || spg->GetId () > 194)
			id = 168;
		gtk_adjustment_set_lower (SpaceGroupAdj, 143);
		gtk_adjustment_set_upper (SpaceGroupAdj, 194);
		gtk_entry_set_text (Alpha, "90");
		gtk_entry_set_text (Beta, "90");
		gtk_entry_set_text (Gamma, "120");
		gtk_widget_set_sensitive (GTK_WIDGET (Alpha), false);
		gtk_widget_set_sensitive (GTK_WIDGET (Beta), false);
		gtk_widget_set_sensitive (GTK_WIDGET (Gamma), false);
		gtk_widget_set_sensitive (GTK_WIDGET (B), false);
		gtk_widget_set_sensitive (GTK_WIDGET (C), true);
		break;
	case gcr::tetragonal:
		if (!spg || spg->GetId () < 75 || spg->GetId () > 142 || name[0] != 'P')
			id = 75;
		goto tetragonal_end;
	case gcr::body_centered_tetragonal:
		if (!spg || spg->GetId () < 75 || spg->GetId () > 142 || name[0] != 'I')
			id = 79;
tetragonal_end:
		gtk_adjustment_set_lower (SpaceGroupAdj, 75);
		gtk_adjustment_set_upper (SpaceGroupAdj, 142);
		gtk_entry_set_text (Alpha, "90");
		gtk_entry_set_text (Beta, "90");
		gtk_entry_set_text (Gamma, "90");
		gtk_widget_set_sensitive (GTK_WIDGET (Alpha), false);
		gtk_widget_set_sensitive (GTK_WIDGET (Beta), false);
		gtk_widget_set_sensitive (GTK_WIDGET (Gamma), false);
		gtk_widget_set_sensitive (GTK_WIDGET (B), false);
		gtk_widget_set_sensitive (GTK_WIDGET (C), true);
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
		gtk_adjustment_set_lower (SpaceGroupAdj, 16);
		gtk_adjustment_set_upper (SpaceGroupAdj, 74);
		gtk_entry_set_text (Alpha, "90");
		gtk_entry_set_text (Beta, "90");
		gtk_entry_set_text (Gamma, "90");
		gtk_widget_set_sensitive (GTK_WIDGET (Alpha), false);
		gtk_widget_set_sensitive (GTK_WIDGET (Beta), false);
		gtk_widget_set_sensitive (GTK_WIDGET (Gamma), false);
		gtk_widget_set_sensitive (GTK_WIDGET (B), true);
		gtk_widget_set_sensitive (GTK_WIDGET (C), true);
		break;
	case gcr::rhombohedral:
		if (!spg || spg->GetId () < 143 || spg->GetId () > 194)
			id = 146;
		gtk_adjustment_set_lower (SpaceGroupAdj, 143);
		gtk_adjustment_set_upper (SpaceGroupAdj, 194);
		gtk_widget_set_sensitive (GTK_WIDGET (Alpha), true);
		gtk_widget_set_sensitive (GTK_WIDGET (Beta), false);
		gtk_widget_set_sensitive (GTK_WIDGET (Gamma), false);
		gtk_widget_set_sensitive (GTK_WIDGET (B), false);
		gtk_widget_set_sensitive (GTK_WIDGET (C), false);
		break;
	case gcr::monoclinic:
		if (!spg || spg->GetId () < 3 || spg->GetId () > 16 || name[0] != 'P')
			id = 3;
		goto monoclinic_end;
	case gcr::base_centered_monoclinic:
		if (!spg || spg->GetId () < 3 || spg->GetId () > 16 || name[0] == 'P')
			id = 5;
monoclinic_end:
		gtk_adjustment_set_lower (SpaceGroupAdj, 3);
		gtk_adjustment_set_upper (SpaceGroupAdj, 15);
		gtk_entry_set_text (Alpha, "90");
		gtk_entry_set_text (Gamma, "90");
		gtk_widget_set_sensitive (GTK_WIDGET (Alpha), false);
		gtk_widget_set_sensitive (GTK_WIDGET (Beta), true);
		gtk_widget_set_sensitive (GTK_WIDGET (Gamma), false);
		gtk_widget_set_sensitive (GTK_WIDGET (B), true);
		gtk_widget_set_sensitive (GTK_WIDGET (C), true);
		break;
	case gcr::triclinic:
		if (!spg || spg->GetId () > 2)
			id = 1;
		gtk_adjustment_set_lower (SpaceGroupAdj, 1);
		gtk_adjustment_set_upper (SpaceGroupAdj, 2);
		gtk_widget_set_sensitive (GTK_WIDGET (Alpha), true);
		gtk_widget_set_sensitive (GTK_WIDGET (Beta), true);
		gtk_widget_set_sensitive (GTK_WIDGET (Gamma), true);
		gtk_widget_set_sensitive (GTK_WIDGET (B), true);
		gtk_widget_set_sensitive (GTK_WIDGET (C), true);
		break;
	}
	g_signal_handler_block (SpaceGroup, SpaceGroupSignal);
	if (id != spg->GetId ())
		spg = NULL;
	gtk_spin_button_set_value (SpaceGroup, (spg)? spg->GetId (): id);
	g_signal_handler_unblock (SpaceGroup, SpaceGroupSignal);
}

void CellDlgPrivate::OnSpaceGroupChanged (GtkSpinButton *btn, CellDlg *dlg)
{
	g_signal_handler_block (dlg->TypeMenu, dlg->TypeSignal);
	unsigned id = gtk_spin_button_get_value_as_int (btn);
	std::string name = gcu::SpaceGroup::GetSpaceGroup (id)->GetHMName ();
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
}

void CellDlgPrivate::OnAutoSpaceGroupToggled (GtkToggleButton *btn, CellDlg *dlg)
{
	gtk_widget_set_sensitive (GTK_WIDGET (dlg->SpaceGroup), !gtk_toggle_button_get_active (btn));
}

}	//	namespace gcr
