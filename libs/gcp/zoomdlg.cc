// -*- C++ -*-

/* 
 * GChemPaint library
 * zoomdlg.cc 
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
#include "application.h"
#include "document.h"
#include "view.h"
#include "widgetdata.h"
#include "zoomdlg.h"
#include <libxml/tree.h>

using namespace gcu;

namespace gcp {
	
static void on_focus_in (ZoomDlg *dlg)
{
	dlg->OnFocusIn ();
}
	
static void on_zoom_changed (GtkSpinButton *btn, gcp::Document *pDoc)
{
	View *pView = pDoc->GetView ();
	if (pView)
		pView->Zoom (gtk_spin_button_get_value (btn) / 100.);
}

ZoomDlg::ZoomDlg (gcp::Document *pDoc):
	gcugtk::Dialog (pDoc->GetApplication (), UIDIR"/zoom.ui", "zoomdlg", GETTEXT_PACKAGE, pDoc)
{
	g_signal_connect_swapped (G_OBJECT (dialog), "focus_in_event", G_CALLBACK (on_focus_in), this);
	btn = GTK_SPIN_BUTTON (GetWidget ("zoom"));
	m_ZoomSignal = g_signal_connect (G_OBJECT (btn), "value-changed", G_CALLBACK (on_zoom_changed), pDoc);
	m_Doc = pDoc;
}

ZoomDlg::~ZoomDlg ()
{
}
	
void ZoomDlg::OnFocusIn ()
{
	if (!m_Doc)
		return;
	WidgetData *pData = (WidgetData*) g_object_get_data (G_OBJECT (m_Doc->GetWidget ()), "data");
	if (!pData)
		return;
	g_signal_handler_block (btn, m_ZoomSignal);
	gtk_spin_button_set_value (btn, pData->Zoom * 100.);
	g_signal_handler_unblock (btn, m_ZoomSignal);
}

}	// namespace gcp
