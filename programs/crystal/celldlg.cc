// -*- C++ -*-

/* 
 * Gnome Crystal
 * sizedlg.cc 
 *
 * Copyright (C) 2002-2005
 *
 * Developed by Jean Bréfort <jean.brefort@normalesup.org>
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
#include "celldlg.h"
#include "document.h"
#include "application.h"

void on_type_changed (GtkWidget* w, gcCellDlg *pBox)
{
	pBox->OnTypeChanged ();
}

gcCellDlg::gcCellDlg (gcApplication *App, gcDocument* pDoc): Dialog (App, DATADIR"/gchemutils-unstable/glade/cell.glade", "cell")
{
	m_pDoc = pDoc;
	pDoc->NotifyDialog (this);
	TypeMenu = GTK_COMBO_BOX (glade_xml_get_widget (xml, "lattice_type"));
	g_signal_connect (G_OBJECT (TypeMenu), "changed", G_CALLBACK (on_type_changed), this);
	A = (GtkEntry*) glade_xml_get_widget (xml, "a");
	B = (GtkEntry*) glade_xml_get_widget (xml, "b");
	C = (GtkEntry*) glade_xml_get_widget (xml, "c");
	Alpha = (GtkEntry*) glade_xml_get_widget (xml, "alpha");
	Beta = (GtkEntry*) glade_xml_get_widget (xml, "beta");
	Gamma = (GtkEntry*) glade_xml_get_widget (xml, "gamma");
	gcLattices i;
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
	OnTypeChanged ();
}

gcCellDlg::~gcCellDlg ()
{
	m_pDoc->RemoveDialog (this);
}

bool gcCellDlg::Apply ()
{
	gcLattices i = (gcLattices) gtk_combo_box_get_active (TypeMenu);
	switch (i)
	{
	case cubic:
	case body_centered_cubic:
	case face_centered_cubic:
		if (!GetNumber (A, &m_a, Min, 0))
			return false;
		m_alpha = m_beta = m_gamma  = 90;
		m_b = m_c = m_a;
		break;
	case hexagonal:
		if ((!GetNumber (A, &m_a, Min, 0)) ||
			(!GetNumber (C, &m_c, Min, 0)))
			return false;
		m_alpha = m_beta = 90;
		m_gamma  = 120;
		m_b = m_a;
		break;
	case tetragonal:
	case body_centered_tetragonal:
		if ((!GetNumber (A, &m_a, Min, 0)) ||
			(!GetNumber (C, &m_c, Min, 0)))
			return false;
		m_alpha = m_beta = m_gamma  = 90;
		m_b = m_a;
		break;
	case orthorhombic:
	case base_centered_orthorhombic:
	case body_centered_orthorhombic:
	case face_centered_orthorhombic:
		if ((!GetNumber (A, &m_a, Min, 0)) ||
			(!GetNumber (B, &m_b, Min, 0)) ||
			(!GetNumber (C, &m_c, Min, 0)))
			return false;
		m_alpha = m_beta = m_gamma  = 90;
		break;
	case rhombohedral:
		if ((!GetNumber (A, &m_a, Min, 0)) ||
			(!GetNumber (Alpha, &m_alpha, MinMax, 0, 180)))
			return false;
		m_beta = m_gamma = m_alpha;
		m_b = m_c = m_a;
		break;
	case monoclinic:
	case base_centered_monoclinic:
		if ((!GetNumber (A, &m_a, Min, 0)) ||
			(!GetNumber (B, &m_b, Min, 0)) ||
			(!GetNumber (C, &m_c, Min, 0)) ||
			(!GetNumber (Beta, &m_beta, MinMax, 0, 180)))
			return false;
		m_alpha = m_gamma  = 90;
		break;
	case triclinic:
		if ((!GetNumber (A, &m_a, Min, 0)) ||
			(!GetNumber (B, &m_b, Min, 0)) ||
			(!GetNumber (C, &m_c, Min, 0)) ||
			(!GetNumber (Alpha, &m_alpha, MinMax, 0, 180)) ||
			(!GetNumber (Beta, &m_beta, MinMax, 0, 180)) ||
			(!GetNumber (Gamma, &m_gamma, MinMax, 0, 180)))
			return false;
		break;
	}
	if (m_alpha + m_beta + m_gamma >= 360)
	{
		GtkDialog* box = GTK_DIALOG (gtk_message_dialog_new (GTK_WINDOW (dialog), GTK_DIALOG_MODAL,
										GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, _("The sum of the three angles must be less than 360°")));
		gtk_dialog_run (box);
	}
	m_pDoc->SetCell (i, m_a, m_b, m_c, m_alpha, m_beta, m_gamma);
	m_pDoc->Update ();
	m_pDoc->SetDirty ();
	return true;
}

void gcCellDlg::OnTypeChanged ()
{
	gcLattices i = (gcLattices) gtk_combo_box_get_active (TypeMenu);
	switch(i)
	{
		case cubic:
		case body_centered_cubic:
		case face_centered_cubic:
			gtk_entry_set_text (Alpha, "90");
			gtk_entry_set_text (Beta, "90");
			gtk_entry_set_text (Gamma, "90");
			gtk_widget_set_sensitive (GTK_WIDGET (Alpha),0);
			gtk_widget_set_sensitive (GTK_WIDGET (Beta),0);
			gtk_widget_set_sensitive (GTK_WIDGET (Gamma),0);
			gtk_widget_set_sensitive (GTK_WIDGET (B),0);
			gtk_widget_set_sensitive (GTK_WIDGET (C),0);
			break;
		case hexagonal:
			gtk_entry_set_text (Alpha, "90");
			gtk_entry_set_text (Beta, "90");
			gtk_entry_set_text (Gamma, "120");
			gtk_widget_set_sensitive (GTK_WIDGET (Alpha),0);
			gtk_widget_set_sensitive (GTK_WIDGET (Beta),0);
			gtk_widget_set_sensitive (GTK_WIDGET (Gamma),0);
			gtk_widget_set_sensitive (GTK_WIDGET (B),0);
			gtk_widget_set_sensitive (GTK_WIDGET (C),1);
			break;
		case tetragonal:
		case body_centered_tetragonal:
			gtk_entry_set_text (Alpha, "90");
			gtk_entry_set_text (Beta, "90");
			gtk_entry_set_text (Gamma, "90");
			gtk_widget_set_sensitive (GTK_WIDGET (Alpha),0);
			gtk_widget_set_sensitive (GTK_WIDGET (Beta),0);
			gtk_widget_set_sensitive (GTK_WIDGET (Gamma),0);
			gtk_widget_set_sensitive (GTK_WIDGET (B),0);
			gtk_widget_set_sensitive (GTK_WIDGET (C),1);
			break;
		case orthorhombic:
		case base_centered_orthorhombic:
		case body_centered_orthorhombic:
		case face_centered_orthorhombic:
			gtk_entry_set_text (Alpha, "90");
			gtk_entry_set_text (Beta, "90");
			gtk_entry_set_text (Gamma, "90");
			gtk_widget_set_sensitive (GTK_WIDGET (Alpha),0);
			gtk_widget_set_sensitive (GTK_WIDGET (Beta),0);
			gtk_widget_set_sensitive (GTK_WIDGET (Gamma),0);
			gtk_widget_set_sensitive (GTK_WIDGET (B),1);
			gtk_widget_set_sensitive (GTK_WIDGET (C),1);
			break;
		case rhombohedral:
			gtk_widget_set_sensitive (GTK_WIDGET (Alpha),1);
			gtk_widget_set_sensitive (GTK_WIDGET (Beta),0);
			gtk_widget_set_sensitive (GTK_WIDGET (Gamma),0);
			gtk_widget_set_sensitive (GTK_WIDGET (B),0);
			gtk_widget_set_sensitive (GTK_WIDGET (C),0);
			break;
		case monoclinic:
		case base_centered_monoclinic:
			gtk_entry_set_text (Alpha, "90");
			gtk_entry_set_text (Gamma, "90");
			gtk_widget_set_sensitive (GTK_WIDGET (Alpha),0);
			gtk_widget_set_sensitive (GTK_WIDGET (Beta),1);
			gtk_widget_set_sensitive (GTK_WIDGET (Gamma),0);
			gtk_widget_set_sensitive (GTK_WIDGET (B),1);
			gtk_widget_set_sensitive (GTK_WIDGET (C),1);
			break;
		case triclinic:
			gtk_widget_set_sensitive (GTK_WIDGET (Alpha),1);
			gtk_widget_set_sensitive (GTK_WIDGET (Beta),1);
			gtk_widget_set_sensitive (GTK_WIDGET (Gamma),1);
			gtk_widget_set_sensitive (GTK_WIDGET (B),1);
			gtk_widget_set_sensitive (GTK_WIDGET (C),1);
			break;
	}
}
