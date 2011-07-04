// -*- C++ -*-

/*
 * GChemPaint library
 * Hposdlg.cc
 *
 * Copyright (C) 2005-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "Hposdlg.h"
#include "application.h"
#include "atom.h"
#include "document.h"
#include "view.h"

using namespace gcu;

namespace gcp {

static void on_pos_changed (HPosDlg *dlg)
{
	dlg->OnPosChanged ();
}

HPosDlg::HPosDlg (Document *pDoc, Atom *pAtom):
	gcugtk::Dialog (pDoc->GetApplication (), UIDIR"/H-pos.ui", "Hposdlg", GETTEXT_PACKAGE, pAtom),
	m_Atom (pAtom)
{
	box = GTK_COMBO_BOX (GetWidget ("H-pos"));
	gtk_combo_box_set_active (box, (int) m_Atom->GetHPosStyle ());
	g_signal_connect_swapped (G_OBJECT (box), "changed", G_CALLBACK (on_pos_changed), this);
	m_View = pDoc->GetView ();
	gtk_widget_show_all (GTK_WIDGET (dialog));
}

HPosDlg::~HPosDlg ()
{
}

void HPosDlg::OnPosChanged ()
{
	Document *Doc = m_View->GetDoc ();
	Operation *Op = Doc->GetNewOperation (GCP_MODIFY_OPERATION);
	Object *Obj = m_Atom->GetGroup ();
	Op->AddObject (Obj, 0);
	m_Atom->SetHPosStyle ((HPos) gtk_combo_box_get_active (box));
	m_Atom->Update ();
	m_Atom->ForceChanged ();
	m_View->Update (m_Atom);
	m_Atom->EmitSignal (OnChangedSignal);
	Op->AddObject (Obj, 1);
	Doc->FinishOperation ();
}

}	//	namespace gcp
