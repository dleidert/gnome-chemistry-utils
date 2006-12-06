// -*- C++ -*-

/* 
 * Gnome Crystal
 * view-settings.cc
 *
 * Copyright (C) 2001-2006 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "view-settings.h"
#include "document.h"
#include "application.h"
#include <glade/glade.h>

gcViewSettingsDlg::gcViewSettingsDlg (gcView* pView): Dialog (pView->GetDoc ()->GetApp (), GLADEDIR"/view-settings.glade", "view_settings")
{
	m_pView = pView;
	m_pView->NotifyDialog (this);
	FoV = GTK_SPIN_BUTTON (glade_xml_get_widget (xml, "fov"));
	Psi = (GtkEntry *) glade_xml_get_widget (xml, "psi");
	Theta = (GtkEntry *) glade_xml_get_widget (xml, "theta");
	Phi = (GtkEntry *) glade_xml_get_widget (xml, "phi");
	Background = (GtkColorButton *) glade_xml_get_widget (xml, "color");
	double x0, x1, x2, x3;
	GdkColor color;
	m_pView->GetBackgroundColor (&x0, &x1, &x2, &x3);
	color.red = (guint16) (x0 * 65535.);
	color.green = (guint16) (x1 * 65535.);
	color.blue = (guint16) (x2 * 65535.);
	gtk_color_button_set_color (Background, &color);
	gtk_color_button_set_alpha (Background, (guint16) (x3 * 65535.));
	m_pView->GetRotation (&x0, &x1, &x2);
	char m_buf[32];
	snprintf (m_buf, sizeof (m_buf) - 1, "%g", x0);
	gtk_entry_set_text (Psi, m_buf);
	snprintf (m_buf, sizeof (m_buf) - 1, "%g", x1);
	gtk_entry_set_text (Theta, m_buf);
	snprintf (m_buf, sizeof (m_buf) - 1, "%g", x2);
	gtk_entry_set_text (Phi, m_buf);
	gtk_spin_button_set_value (FoV, (int) (m_pView->GetFoV ()));
	
	gtk_widget_show_all (GTK_WIDGET (dialog));
}

gcViewSettingsDlg::~gcViewSettingsDlg ()
{
	m_pView->RemoveDialog (this);
}

bool gcViewSettingsDlg::Apply()
{
	double x0, x1, x2;
	if (!GetNumber (Psi, &x0, MinEqMax, -180, 180))
		return false;
	if (!GetNumber (Theta, &x1, MinEqMaxEq, 0, 180))
		return false;
	if (!GetNumber (Phi, &x2, MinEqMax, -180, 180))
		return false;
	m_pView->SetRotation (x0, x1, x2);
	m_pView->GetFoV () = gtk_spin_button_get_value (FoV);
	GdkColor color;
	gtk_color_button_get_color (Background, &color);
	m_pView->SetBackgroundColor (color.red / 65535., color.green / 65535., color.blue / 65535., gtk_color_button_get_alpha (Background) / 65535.);
	m_pView->Update ();
	dynamic_cast <gcDocument *> (m_pView->GetDoc ())->SetDirty (true);
	return true;	
}
