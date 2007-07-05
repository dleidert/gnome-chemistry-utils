// -*- C++ -*-

/*
 * GChemPaint library
 * residues-dlg.cc
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
#include "residues-dlg.h"
#include <gcp/application.h>
#include <gcp/document.h>
#include <gcp/view.h>

gcpResiduesDlg::gcpResiduesDlg (gcp::Application *App):
	Dialog (App, GLADEDIR"/residues.glade", "residues", App),
	gcp::Target (App)
{
	m_Document = new gcp::Document (App, true, NULL);
	GtkWidget *w = m_Document->GetView ()->CreateNewWidget ();
	GtkScrolledWindow* scroll = (GtkScrolledWindow*) gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (scroll, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (scroll, GTK_SHADOW_IN);
	gtk_scrolled_window_add_with_viewport (scroll, w);
	gtk_widget_set_size_request (GTK_WIDGET (scroll), 408, 308);
	gtk_widget_show (GTK_WIDGET (scroll));
	gtk_box_pack_start (GTK_BOX (glade_xml_get_widget (xml, "residues-box")), GTK_WIDGET (scroll), true, true, 0);
	gtk_widget_show_all (GTK_WIDGET (dialog));
	App->SetActiveDocument (m_Document);
	SetWindow (dialog);
}

gcpResiduesDlg::~gcpResiduesDlg ()
{
}

void gcpResiduesDlg::Add ()
{
}

void gcpResiduesDlg::Remove ()
{
}

bool gcpResiduesDlg::Close ()
{
	Destroy ();
	return true;
}
