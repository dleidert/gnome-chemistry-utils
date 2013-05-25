// -*- C++ -*-

/*
 * Gnome Crystal library
 * view-settings.cc
 *
 * Copyright (C) 2001-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "view-settings.h"
#include "document.h"
#include "application.h"
#include "view.h"

namespace gcr {

class ViewSettingsDlgPrivate
{
public:
	static void OnFoVChanged (ViewSettingsDlg *dlg);
	static bool OnPsiChanged (ViewSettingsDlg *dlg);
	static bool OnThetaChanged (ViewSettingsDlg *dlg);
	static bool OnPhiChanged (ViewSettingsDlg *dlg);
	static void OnBackgroundChanged (ViewSettingsDlg *dlg);
};

void ViewSettingsDlgPrivate::OnFoVChanged (ViewSettingsDlg *dlg)
{
	dlg->m_pView->GetFoV () = gtk_spin_button_get_value_as_int (dlg->FoV);
	dlg->m_pView->Update ();
	dynamic_cast < Document * > (dlg->m_pView->GetDoc ())->SetDirty (true);
}

bool ViewSettingsDlgPrivate::OnPsiChanged (ViewSettingsDlg *dlg)
{
	g_signal_handler_block (dlg->Psi, dlg->PsiSignal);
	double value;
	if (dlg->GetNumber (dlg->Psi, &value, gcugtk::MinEqMax, -180, 180)) {
		dlg->m_pView->SetRotation (value, dlg->m_pView->GetTheta (), dlg->m_pView->GetPhi ());
		dlg->m_pView->Update ();
		dynamic_cast < Document * > (dlg->m_pView->GetDoc ())->SetDirty (true);
	}
	g_signal_handler_unblock (dlg->Psi, dlg->PsiSignal);
	return false;
}

bool ViewSettingsDlgPrivate::OnThetaChanged (ViewSettingsDlg *dlg)
{
	g_signal_handler_block (dlg->Theta, dlg->ThetaSignal);
	double value;
	if (dlg->GetNumber (dlg->Theta, &value, gcugtk::MinEqMaxEq, 0, 180)) {
		dlg->m_pView->SetRotation (dlg->m_pView->GetPsi (), value, dlg->m_pView->GetPhi ());
		dlg->m_pView->Update ();
		dynamic_cast < Document * > (dlg->m_pView->GetDoc ())->SetDirty (true);
	}
	g_signal_handler_unblock (dlg->Theta, dlg->ThetaSignal);
	return false;
}

bool ViewSettingsDlgPrivate::OnPhiChanged (ViewSettingsDlg *dlg)
{
	g_signal_handler_block (dlg->Phi, dlg->PhiSignal);
	double value;
	if (dlg->GetNumber (dlg->Phi, &value, gcugtk::MinEqMax, -180, 180)) {
		dlg->m_pView->SetRotation (dlg->m_pView->GetPsi (), dlg->m_pView->GetTheta (), value);
		dlg->m_pView->Update ();
		dynamic_cast < Document * > (dlg->m_pView->GetDoc ())->SetDirty (true);
	}
	g_signal_handler_unblock (dlg->Phi, dlg->PhiSignal);
	return false;
}

void ViewSettingsDlgPrivate::OnBackgroundChanged (ViewSettingsDlg *dlg)
{
	GdkRGBA rgba;
	gtk_color_chooser_get_rgba (dlg->Background, &rgba);
	dlg->m_pView->SetRed (rgba.red);
	dlg->m_pView->SetGreen (rgba.green);
	dlg->m_pView->SetBlue (rgba.blue);
	dlg->m_pView->SetAlpha (rgba.alpha);
	dlg->m_pView->Update ();
	dynamic_cast < Document * > (dlg->m_pView->GetDoc ())->SetDirty (true);
}

ViewSettingsDlg::ViewSettingsDlg (View* pView): gcugtk::Dialog (static_cast < gcugtk::Application * > (pView->GetDoc ()->GetApp ()), UIDIR"/view-settings.ui", "view-settings", GETTEXT_PACKAGE, pView)
{
	m_pView = pView;
	FoV = GTK_SPIN_BUTTON (GetWidget ("fov"));
	Psi = GTK_ENTRY (GetWidget ("psi"));
	Theta = GTK_ENTRY (GetWidget ("theta"));
	Phi = GTK_ENTRY (GetWidget ("phi"));
	Background = GTK_COLOR_CHOOSER (GetWidget ("color"));
	GdkRGBA rgba;
	m_pView->GetBackgroundColor (&rgba.red, &rgba.green, &rgba.blue, &rgba.alpha);
	gtk_color_chooser_set_rgba (Background, &rgba);
	g_signal_connect_swapped (Background, "color-set", G_CALLBACK (ViewSettingsDlgPrivate::OnBackgroundChanged), this);
	double x0, x1, x2;
	m_pView->GetRotation (&x0, &x1, &x2);
	char m_buf[32];
	snprintf (m_buf, sizeof (m_buf) - 1, "%g", x0);
	gtk_entry_set_text (Psi, m_buf);
	snprintf (m_buf, sizeof (m_buf) - 1, "%g", x1);
	gtk_entry_set_text (Theta, m_buf);
	snprintf (m_buf, sizeof (m_buf) - 1, "%g", x2);
	gtk_entry_set_text (Phi, m_buf);
	gtk_spin_button_set_value (FoV, (int) (m_pView->GetFoV ()));
	g_signal_connect_swapped (G_OBJECT (Psi), "activate", G_CALLBACK (ViewSettingsDlgPrivate::OnPsiChanged), this);
	PsiSignal = g_signal_connect_swapped (G_OBJECT (Psi), "focus-out-event", G_CALLBACK (ViewSettingsDlgPrivate::OnPsiChanged), this);
	g_signal_connect_swapped (G_OBJECT (Theta), "activate", G_CALLBACK (ViewSettingsDlgPrivate::OnThetaChanged), this);
	ThetaSignal = g_signal_connect_swapped (G_OBJECT (Theta), "focus-out-event", G_CALLBACK (ViewSettingsDlgPrivate::OnThetaChanged), this);
	g_signal_connect_swapped (G_OBJECT (Phi), "activate", G_CALLBACK (ViewSettingsDlgPrivate::OnPhiChanged), this);
	PhiSignal = g_signal_connect_swapped (G_OBJECT (Phi), "focus-out-event", G_CALLBACK (ViewSettingsDlgPrivate::OnPhiChanged), this);
	g_signal_connect_swapped (FoV, "value-changed", G_CALLBACK (ViewSettingsDlgPrivate::OnFoVChanged), this);

	gtk_widget_show_all (GTK_WIDGET (dialog));
}

ViewSettingsDlg::~ViewSettingsDlg ()
{
}

}	//	namespace gcr