// -*- C++ -*-

/* 
 * GChemPaint library
 * reaction-prop-dlg.cc 
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
#include "reaction-prop-dlg.h"
#include "reaction-arrow.h"
#include "reaction-prop.h"
#include <gcu/document.h>

namespace gcp {

static void on_role_changed (GtkComboBox *box, ReactionProp *prop)
{
	prop->SetRole (gtk_combo_box_get_active (box));
}

ReactionPropDlg::ReactionPropDlg (ReactionArrow *arrow, ReactionProp *prop):
	Dialog (arrow->GetDocument ()->GetApp (), GLADEDIR"/arrow-object.glade", "reaction-prop", prop),
	m_Arrow (arrow),
	m_Prop (prop)
{
	GtkComboBox *box = (GtkComboBox *) glade_xml_get_widget (xml, "role-combo");
	int max = (prop->GetObject ()->GetType () == MoleculeType)?
				REACTION_PROP_MAX_MOL: REACTION_PROP_MAX;
	for (int i = REACTION_PROP_UNKNOWN; i < max; i++)
		gtk_combo_box_append_text (box, ReactionPropRoles[i]);
	gtk_combo_box_set_active (box, prop->GetRole ());
	g_signal_connect (G_OBJECT (box), "changed", G_CALLBACK (on_role_changed), prop);
	gtk_widget_show (GTK_WIDGET(dialog));
}

ReactionPropDlg::~ReactionPropDlg ()
{
}

}
