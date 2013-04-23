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

static void on_role_changed (GtkComboBox *box, ReactionProp *prop)
{
	prop->SetRole (gtk_combo_box_get_active (box));
}

static void on_focus (ReactionProp *prop)
{
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
	if (arrow->GetChildrenNumber () < 2) {
		gtk_spin_button_set_range (m_StepBtn, 1., 1.);
		gtk_widget_set_sensitive (GTK_WIDGET (m_StepBtn), false);
		gtk_spin_button_set_range (m_LineBtn, 1., 1.);
		gtk_widget_set_sensitive (GTK_WIDGET (m_LineBtn), false);
		gtk_spin_button_set_range (m_PosBtn, 1., 1.);
		gtk_widget_set_sensitive (GTK_WIDGET (m_PosBtn), false);
	} else {
		unsigned step, line, max;
		gtk_spin_button_set_range (m_StepBtn, 1, arrow->GetLastStep () + 1);
		gtk_spin_button_set_value (m_StepBtn, step = prop->GetStep ());
		gtk_spin_button_set_range (m_LineBtn, 1, arrow->GetLastLine (step) + 1);
		gtk_spin_button_set_value (m_LineBtn, line = prop->GetLine ());
		max = arrow->GetLastPos (step, line);
		gtk_spin_button_set_range (m_PosBtn, 1, max);
		gtk_spin_button_set_value (m_PosBtn, prop->GetRank ());
		if (max == 1)
			gtk_widget_set_sensitive (GTK_WIDGET (m_PosBtn), false);
	}
	gtk_widget_show (GTK_WIDGET (dialog));
	g_signal_connect_swapped(G_OBJECT (dialog), "focus-in-event", G_CALLBACK (on_focus), prop);
}

ReactionPropDlg::~ReactionPropDlg ()
{
}

}
