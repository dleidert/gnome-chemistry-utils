// -*- C++ -*-

/* 
 * Gnome Crystal
 * prefs.cc 
 *
 * Copyright (C) 2001-2007 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "prefs.h"
#include "globals.h"
#include "application.h"

guint PrintResolution = 300;

static void on_print_resolution (GtkWidget *widget, gcPrefsDlg * dialog)
{
	dialog->UpdatePrinting ();
}

gcPrefsDlg::gcPrefsDlg (gcApplication *App): Dialog (App, GLADEDIR"/prefs.glade", "prefs", App)
{
	if (!xml) {
		delete this;
		return;
	}
	PrintResMenu = (GtkComboBox *) glade_xml_get_widget (xml, "printres");
	PrintResBtn = (GtkSpinButton *) glade_xml_get_widget (xml, "printresbtn");
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
	gtk_widget_set_sensitive (GTK_WIDGET (PrintResBtn), active);
	gtk_combo_box_set_active (PrintResMenu, PrintIndex);
	g_signal_connect (PrintResMenu, "changed", G_CALLBACK (on_print_resolution), this);
	FoVBtn = GTK_SPIN_BUTTON (glade_xml_get_widget (xml, "fov"));
	gtk_spin_button_set_value (FoVBtn, FoV);
	PsiEnt = (GtkEntry *) glade_xml_get_widget (xml, "psi");
	ThetaEnt = (GtkEntry *) glade_xml_get_widget (xml, "theta");
	PhiEnt = (GtkEntry *) glade_xml_get_widget (xml, "phi");
	char m_buf[32];
	snprintf (m_buf, sizeof (m_buf) - 1, "%g", Psi);
	gtk_entry_set_text (PsiEnt, m_buf);
	snprintf (m_buf, sizeof (m_buf) - 1, "%g", Theta);
	gtk_entry_set_text (ThetaEnt, m_buf);
	snprintf (m_buf, sizeof (m_buf) - 1, "%g", Phi);
	gtk_entry_set_text (PhiEnt, m_buf);
	BackgroundBtn = (GtkColorButton *) glade_xml_get_widget (xml, "color");
	GdkColor color;
	color.red = (guint16) (Red * 65535.);
	color.green = (guint16) (Green * 65535.);
	color.blue = (guint16) (Blue * 65535.);
	gtk_color_button_set_color (BackgroundBtn, &color);
	gtk_widget_show_all (GTK_WIDGET (dialog));
}

gcPrefsDlg::~gcPrefsDlg()
{
}

bool gcPrefsDlg::Apply()
{
	double x0, x1, x2;
	if (!GetNumber (PsiEnt, &x0, MinEqMax, -180, 180))
		return false;
	if (!GetNumber (ThetaEnt, &x1, MinEqMaxEq, 0, 180))
		return false;
	if (!GetNumber (PhiEnt, &x2, MinEqMax, -180, 180))
		return false;
	PrintResolution = gtk_spin_button_get_value_as_int (PrintResBtn);
	Psi = x0;
	Theta= x1;
	Phi = x2;
	FoV = gtk_spin_button_get_value_as_int(FoVBtn);
	GdkColor color;
	gtk_color_button_get_color (BackgroundBtn, &color);
	Red = color.red / 65535.;
	Green = color.green / 65535.;
	Blue = color.blue / 65535.;
	gconf_client_set_int (conf_client, "/apps/gchemutils/crystal/printing/resolution", PrintResolution, NULL);
	gconf_client_set_int (conf_client, "/apps/gchemutils/crystal/views/fov", FoV, NULL);
	gconf_client_set_float (conf_client, "/apps/gchemutils/crystal/views/psi", Psi, NULL);
	gconf_client_set_float (conf_client, "/apps/gchemutils/crystal/views/theta", Theta, NULL);
	gconf_client_set_float (conf_client, "/apps/gchemutils/crystal/views/phi", Phi, NULL);
	gconf_client_set_float (conf_client, "/apps/gchemutils/crystal/views/red", Red, NULL);
	gconf_client_set_float (conf_client, "/apps/gchemutils/crystal/views/green", Green, NULL);
	gconf_client_set_float (conf_client, "/apps/gchemutils/crystal/views/blue", Blue, NULL);
	return true;
}

void gcPrefsDlg::UpdatePrinting ()
{
	int PrintRes = PrintResolution ;
	int PrintIndex = gtk_combo_box_get_active (PrintResMenu);
	switch (PrintIndex)
	{
	case 0:
		PrintRes = 300;
		break;
	case 1:
		PrintRes = 360;
		break;
	case 2:
		PrintRes = 600;
		break;
	case 3:
		PrintRes = 720;
		break;
	case 4:
		PrintRes = 1200;
		break;
	case 5:
		PrintRes = 1440;
		break;
	case 6:
		PrintRes = 2400;
		break;
	case 7:
		PrintRes = 2880;
		break;
	}
	gtk_spin_button_set_value (PrintResBtn, PrintRes);
	gtk_widget_set_sensitive (GTK_WIDGET (PrintResBtn), (PrintIndex < 8)? false : true);
}
