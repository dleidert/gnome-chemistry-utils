// -*- C++ -*-

/*
 * GChemPaint library
 * reaction-prop-dlg.cc
 *
 * Copyright (C) 2007-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "reaction-prop-dlg.h"
#include "reaction-arrow.h"
#include "reaction-prop.h"
#include <gcu/document.h>

using namespace gcu;

namespace gcp {

class ReactionPropDlgPrivate
{
public:
	static void OnPosChanged (GtkSpinButton *btn, ReactionPropDlg *dlg);
};

void ReactionPropDlgPrivate::OnPosChanged (GtkSpinButton *btn, ReactionPropDlg *dlg)
{
	unsigned step, line, pos;
	step = dlg->m_Prop->GetStep ();
	line = dlg->m_Prop->GetLine ();
	pos = dlg->m_Prop->GetRank ();
	if (btn == dlg->m_StepBtn)
		step = gtk_spin_button_get_value_as_int (btn);
	else if (btn == dlg->m_LineBtn)
		line = gtk_spin_button_get_value_as_int (btn);
	else if (btn == dlg->m_PosBtn)
		pos = gtk_spin_button_get_value_as_int (btn);
	else {
		// should not occur
		g_critical ("Should not occur");
		return;
	}
	dlg->m_Arrow->SetChildPos (dlg->m_Prop, step, line, pos);
}

static void on_role_changed (GtkComboBox *box, ReactionProp *prop)
{
	prop->SetRole (gtk_combo_box_get_active (box));
}

ReactionPropDlg::ReactionPropDlg (ReactionArrow *arrow, ReactionProp *prop):
	gcugtk::Dialog (static_cast < gcugtk::Application * > (arrow->GetDocument ()->GetApp ()), UIDIR"/arrow-object.ui", "reaction-prop", GETTEXT_PACKAGE, prop),
	m_Arrow (arrow),
	m_Prop (prop)
{
	GtkComboBoxText *box = GTK_COMBO_BOX_TEXT (GetWidget ("role-combo"));
	int max = (prop->GetObject ()->GetType () == MoleculeType)?
				REACTION_PROP_MAX_MOL: REACTION_PROP_MAX;
	for (int i = REACTION_PROP_UNKNOWN; i < max; i++)
		gtk_combo_box_text_append_text (box, ReactionPropRoles[i]);
	gtk_combo_box_set_active (GTK_COMBO_BOX (box), prop->GetRole ());
	g_signal_connect (G_OBJECT (box), "changed", G_CALLBACK (on_role_changed), prop);
	m_StepBtn = GTK_SPIN_BUTTON (GetWidget ("step-btn"));
	m_LineBtn = GTK_SPIN_BUTTON (GetWidget ("line-btn"));
	m_PosBtn = GTK_SPIN_BUTTON (GetWidget ("pos-btn"));
	m_StepSgn = g_signal_connect (m_StepBtn, "value-changed", G_CALLBACK (ReactionPropDlgPrivate::OnPosChanged), this);
	m_LineSgn = g_signal_connect (m_LineBtn, "value-changed", G_CALLBACK (ReactionPropDlgPrivate::OnPosChanged), this);
	m_PosSgn = g_signal_connect (m_PosBtn, "value-changed", G_CALLBACK (ReactionPropDlgPrivate::OnPosChanged), this);
	Update ();
	gtk_widget_show (GTK_WIDGET (dialog));
}

ReactionPropDlg::~ReactionPropDlg ()
{
}

void ReactionPropDlg::Update ()
{
	g_signal_handler_block (m_StepBtn, m_StepSgn);
	g_signal_handler_block (m_LineBtn, m_LineSgn);
	g_signal_handler_block (m_PosBtn, m_PosSgn);
	if (m_Arrow->GetChildrenNumber () < 2) {
		gtk_spin_button_set_range (m_StepBtn, 1., 1.);
		gtk_widget_set_sensitive (GTK_WIDGET (m_StepBtn), false);
		gtk_spin_button_set_range (m_LineBtn, 1., 1.);
		gtk_widget_set_sensitive (GTK_WIDGET (m_LineBtn), false);
		gtk_spin_button_set_range (m_PosBtn, 1., 1.);
		gtk_widget_set_sensitive (GTK_WIDGET (m_PosBtn), false);
	} else {
		unsigned step, line, rank, last_step, last_line, last_rank;
		step = m_Prop->GetStep ();
		last_step = m_Arrow->GetLastStep ();
		last_line = m_Arrow->GetLastLine (step);
		line = m_Prop->GetLine ();
		rank = m_Prop->GetRank ();
		last_rank = m_Arrow->GetLastPos (step, line);
		if (step != last_step || last_line > 1 ||last_rank > 1)
			last_step++;
		if (line != last_line || last_rank > 2)
			last_line++;
		gtk_spin_button_set_range (m_StepBtn, 1, last_step);
		gtk_spin_button_set_value (m_StepBtn, step);
		gtk_spin_button_set_range (m_LineBtn, 1, last_line);
		gtk_spin_button_set_value (m_LineBtn, line = m_Prop->GetLine ());
		gtk_spin_button_set_range (m_PosBtn, 1, last_rank);
		gtk_spin_button_set_value (m_PosBtn, rank);
		gtk_widget_set_sensitive (GTK_WIDGET (m_PosBtn), last_rank > 1);
	}
	g_signal_handler_unblock (m_StepBtn, m_StepSgn);
	g_signal_handler_unblock (m_LineBtn, m_LineSgn);
	g_signal_handler_unblock (m_PosBtn, m_PosSgn);
}

}
