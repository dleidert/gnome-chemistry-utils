// -*- C++ -*-

/*
 * Gnome Crystal library
 * prefs.cc
 *
 * Copyright (C) 2001-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "prefs.h"
#include "globals.h"
#include "application.h"

namespace gcr {

guint PrintResolution = 300;
class PrefsDlgPrivate {
public:
	static void OnPrintResolution (PrefsDlg *dlg);
	static void OnCustomPrintResolution (PrefsDlg *dlg);
	static void OnFoVChanged (PrefsDlg *dlg);
	static bool OnPsiChanged (PrefsDlg *dlg);
	static bool OnThetaChanged (PrefsDlg *dlg);
	static bool OnPhiChanged (PrefsDlg *dlg);
	static void OnBackgroundChanged (PrefsDlg *dlg);
};

void PrefsDlgPrivate::OnPrintResolution (PrefsDlg *dlg)
{
	int PrintIndex = gtk_combo_box_get_active (dlg->PrintResMenu);
	switch (PrintIndex)
	{
	case 0:
		PrintResolution = 300;
		break;
	case 1:
		PrintResolution = 360;
		break;
	case 2:
		PrintResolution = 600;
		break;
	case 3:
		PrintResolution = 720;
		break;
	case 4:
		PrintResolution = 1200;
		break;
	case 5:
		PrintResolution = 1440;
		break;
	case 6:
		PrintResolution = 2400;
		break;
	case 7:
		PrintResolution = 2880;
		break;
	default:
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->PrintResBtn), true);
		return; // no change in that case
	}
	g_signal_handler_block (dlg->PrintResBtn, dlg->PrintResChanged);
	gtk_spin_button_set_value (dlg->PrintResBtn, PrintResolution);
	gtk_widget_set_sensitive (GTK_WIDGET (dlg->PrintResBtn), false);
	g_signal_handler_unblock (dlg->PrintResBtn, dlg->PrintResChanged);
	go_conf_set_int (static_cast <Application * > (dlg->GetApp ())->GetConfNode (), "printing/resolution", PrintResolution);
}

void PrefsDlgPrivate::OnCustomPrintResolution (PrefsDlg *dlg)
{
	PrintResolution = gtk_spin_button_get_value_as_int (dlg->PrintResBtn);
	go_conf_set_int (static_cast <Application * > (dlg->GetApp ())->GetConfNode (), "printing/resolution", PrintResolution);
}

void PrefsDlgPrivate::OnFoVChanged (PrefsDlg *dlg)
{
	FoV = gtk_spin_button_get_value_as_int (dlg->FoVBtn);
	go_conf_set_int (static_cast <Application * > (dlg->GetApp ())->GetConfNode (), "views/fov", FoV);
}

bool PrefsDlgPrivate::OnPsiChanged (PrefsDlg *dlg)
{
	g_signal_handler_block (dlg->PsiEnt, dlg->PsiSignal);
	double value;
	if (dlg->GetNumber (dlg->PsiEnt, &value, gcugtk::MinEqMax, -180, 180)) {
		Psi = value;
		go_conf_set_double (static_cast <Application * > (dlg->GetApp ())->GetConfNode (), "views/psi", Psi);
	}
	g_signal_handler_unblock (dlg->PsiEnt, dlg->PsiSignal);
	return false;
}

bool PrefsDlgPrivate::OnThetaChanged (PrefsDlg *dlg)
{
	g_signal_handler_block (dlg->ThetaEnt, dlg->ThetaSignal);
	double value;
	if (dlg->GetNumber (dlg->ThetaEnt, &value, gcugtk::MinEqMaxEq, 0, 180)) {
		Theta = value;
		go_conf_set_double (static_cast <Application * > (dlg->GetApp ())->GetConfNode (), "views/theta", Theta);
	}
	g_signal_handler_unblock (dlg->ThetaEnt, dlg->ThetaSignal);
	return false;
}

bool PrefsDlgPrivate::OnPhiChanged (PrefsDlg *dlg)
{
	g_signal_handler_block (dlg->PhiEnt, dlg->PhiSignal);
	double value;
	if (dlg->GetNumber (dlg->PhiEnt, &value, gcugtk::MinEqMax, -180, 180)) {
		Phi = value;
		go_conf_set_double (static_cast <Application * > (dlg->GetApp ())->GetConfNode (), "views/phi", Phi);
	}
	g_signal_handler_unblock (dlg->PhiEnt, dlg->PhiSignal);
	return false;
}

void PrefsDlgPrivate::OnBackgroundChanged (PrefsDlg *dlg)
{
	GdkRGBA rgba;
	gtk_color_button_get_rgba (dlg->BackgroundBtn, &rgba);
	Red = rgba.red;
	go_conf_set_double (static_cast <Application * > (dlg->GetApp ())->GetConfNode (), "views/red", Red);
	Green = rgba.green;
	go_conf_set_double (static_cast <Application * > (dlg->GetApp ())->GetConfNode (), "views/green", Green);
	Blue = rgba.blue;
	go_conf_set_double (static_cast <Application * > (dlg->GetApp ())->GetConfNode (), "views/blue", Blue);
}

PrefsDlg::PrefsDlg (Application *App): gcugtk::Dialog (App, UIDIR"/prefs.ui", "prefs", GETTEXT_PACKAGE, App)
{
	PrintResMenu = GTK_COMBO_BOX (GetWidget ("printres"));
	PrintResBtn = GTK_SPIN_BUTTON (GetWidget ("printresbtn"));
	int PrintIndex;
	bool active = false;
	switch (PrintResolution) {
	case 300:
		PrintIndex = 0;
		break;
	case 360:
		PrintIndex = 1;
		break;
	case 600:
		PrintIndex = 2;
		break;
	case 720:
		PrintIndex = 3;
		break;
	case 1200:
		PrintIndex = 4;
		break;
	case 1440:
		PrintIndex = 5;
		break;
	case 2400:
		PrintIndex = 6;
		break;
	case 2880:
		PrintIndex = 7;
		break;
	default:
		PrintIndex = 8;
		active = true;
		break;
	}
	gtk_spin_button_set_value (PrintResBtn, PrintResolution);
	PrintResChanged = g_signal_connect_swapped (PrintResBtn, "value-changed", G_CALLBACK (PrefsDlgPrivate::OnCustomPrintResolution), this);
	gtk_widget_set_sensitive (GTK_WIDGET (PrintResBtn), active);
	gtk_combo_box_set_active (PrintResMenu, PrintIndex);
	g_signal_connect_swapped (PrintResMenu, "changed", G_CALLBACK (PrefsDlgPrivate::OnPrintResolution), this);
	FoVBtn = GTK_SPIN_BUTTON (GetWidget ("fov"));
	gtk_spin_button_set_value (FoVBtn, FoV);
	g_signal_connect_swapped (FoVBtn, "value-changed", G_CALLBACK (PrefsDlgPrivate::OnFoVChanged), this);
	PsiEnt = GTK_ENTRY (GetWidget ("psi"));
	ThetaEnt = GTK_ENTRY (GetWidget ("theta"));
	PhiEnt = GTK_ENTRY (GetWidget ("phi"));
	char m_buf[32];
	snprintf (m_buf, sizeof (m_buf) - 1, "%g", Psi);
	gtk_entry_set_text (PsiEnt, m_buf);
	snprintf (m_buf, sizeof (m_buf) - 1, "%g", Theta);
	gtk_entry_set_text (ThetaEnt, m_buf);
	snprintf (m_buf, sizeof (m_buf) - 1, "%g", Phi);
	gtk_entry_set_text (PhiEnt, m_buf);
	g_signal_connect_swapped (PsiEnt, "activate", G_CALLBACK (PrefsDlgPrivate::OnPsiChanged), this);
	PsiSignal = g_signal_connect_swapped (PsiEnt, "focus-out-event", G_CALLBACK (PrefsDlgPrivate::OnPsiChanged), this);
	g_signal_connect_swapped (ThetaEnt, "activate", G_CALLBACK (PrefsDlgPrivate::OnThetaChanged), this);
	ThetaSignal = g_signal_connect_swapped (ThetaEnt, "focus-out-event", G_CALLBACK (PrefsDlgPrivate::OnThetaChanged), this);
	g_signal_connect_swapped (PhiEnt, "activate", G_CALLBACK (PrefsDlgPrivate::OnPhiChanged), this);
	PhiSignal = g_signal_connect_swapped (PhiEnt, "focus-out-event", G_CALLBACK (PrefsDlgPrivate::OnPhiChanged), this);
	BackgroundBtn = GTK_COLOR_BUTTON (GetWidget ("color"));
	GdkRGBA rgba;
	rgba.red = Red;
	rgba.green = Green;
	rgba.blue = Blue;
	rgba.alpha = 1.;
	gtk_color_button_set_rgba (BackgroundBtn, &rgba);
	g_signal_connect_swapped (BackgroundBtn, "color-set", G_CALLBACK (PrefsDlgPrivate::OnBackgroundChanged), this);
	gtk_widget_show_all (GTK_WIDGET (dialog));
}

PrefsDlg::~PrefsDlg()
{
}

}	//	namespace gcr
