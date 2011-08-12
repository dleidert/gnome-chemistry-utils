// -*- C++ -*-

/*
 * Gnome Crystal
 * sizedlg.cc
 *
 * Copyright (C) 2002-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "sizedlg.h"
#include "document.h"
#include "application.h"

namespace gcr {

class SizeDlgPrivate
{
public:
	static bool MinXEdited (SizeDlg *pBox);
	static bool MaxXEdited (SizeDlg *pBox);
	static bool MinYEdited (SizeDlg *pBox);
	static bool MaxYEdited (SizeDlg *pBox);
	static bool MinZEdited (SizeDlg *pBox);
	static bool MaxZEdited (SizeDlg *pBox);
};

bool SizeDlgPrivate::MinXEdited (SizeDlg *pBox)
{
	g_signal_handler_block (pBox->MinX, pBox->m_MinXFocusOutSignalID);
	double xmin, xmax, ymin, ymax, zmin, zmax, value;
	pBox->m_pDoc->GetSize (&xmin, &xmax, &ymin, &ymax, &zmin, &zmax);
	if (pBox->GetNumber (pBox->MinX, &value, gcugtk::Max, 0, xmax) && xmin != value) {
		pBox->m_pDoc->SetSize (value, xmax, ymin, ymax, zmin, zmax);
		pBox->m_pDoc->Update ();
		pBox->m_pDoc->SetDirty (true);
	}
	g_signal_handler_unblock (pBox->MinX, pBox->m_MinXFocusOutSignalID);
	return false;
}

bool SizeDlgPrivate::MaxXEdited (SizeDlg *pBox)
{
	g_signal_handler_block (pBox->MaxX, pBox->m_MaxXFocusOutSignalID);
	double xmin, xmax, ymin, ymax, zmin, zmax, value;
	pBox->m_pDoc->GetSize (&xmin, &xmax, &ymin, &ymax, &zmin, &zmax);
	if (pBox->GetNumber (pBox->MaxX, &value, gcugtk::Min, xmin) && xmin != value) {
		pBox->m_pDoc->SetSize (xmin, value, ymin, ymax, zmin, zmax);
		pBox->m_pDoc->Update ();
		pBox->m_pDoc->SetDirty (true);
	}
	g_signal_handler_unblock (pBox->MaxX, pBox->m_MaxXFocusOutSignalID);
	return false;
}

bool SizeDlgPrivate::MinYEdited (SizeDlg *pBox)
{
	g_signal_handler_block (pBox->MinY, pBox->m_MinYFocusOutSignalID);
	double xmin, xmax, ymin, ymax, zmin, zmax, value;
	pBox->m_pDoc->GetSize (&xmin, &xmax, &ymin, &ymax, &zmin, &zmax);
	if (pBox->GetNumber (pBox->MinY, &value, gcugtk::Max, 0, ymax) && ymin != value) {
		pBox->m_pDoc->SetSize (xmin, xmax, value, ymax, zmin, zmax);
		pBox->m_pDoc->Update ();
		pBox->m_pDoc->SetDirty (true);
	}
	g_signal_handler_unblock (pBox->MinY, pBox->m_MinYFocusOutSignalID);
	return false;
}

bool SizeDlgPrivate::MaxYEdited (SizeDlg *pBox)
{
	g_signal_handler_block (pBox->MaxY, pBox->m_MaxYFocusOutSignalID);
	double xmin, xmax, ymin, ymax, zmin, zmax, value;
	pBox->m_pDoc->GetSize (&xmin, &xmax, &ymin, &ymax, &zmin, &zmax);
	if (pBox->GetNumber (pBox->MaxY, &value, gcugtk::Min, ymin) && ymax != value) {
		pBox->m_pDoc->SetSize (xmin, xmax, ymin, value, zmin, zmax);
		pBox->m_pDoc->Update ();
		pBox->m_pDoc->SetDirty (true);
	}
	g_signal_handler_unblock (pBox->MaxY, pBox->m_MaxYFocusOutSignalID);
	return false;
}

bool SizeDlgPrivate::MinZEdited (SizeDlg *pBox)
{
	g_signal_handler_block (pBox->MinZ, pBox->m_MinZFocusOutSignalID);
	double xmin, xmax, ymin, ymax, zmin, zmax, value;
	pBox->m_pDoc->GetSize (&xmin, &xmax, &ymin, &ymax, &zmin, &zmax);
	if (pBox->GetNumber (pBox->MinZ, &value, gcugtk::Max, 0, zmax) && zmin != value) {
		pBox->m_pDoc->SetSize (xmin, xmax, ymin, ymax, value, zmax);
		pBox->m_pDoc->Update ();
		pBox->m_pDoc->SetDirty (true);
	}
	g_signal_handler_unblock (pBox->MinZ, pBox->m_MinZFocusOutSignalID);
	return false;
}

bool SizeDlgPrivate::MaxZEdited (SizeDlg *pBox)
{
	g_signal_handler_block (pBox->MaxZ, pBox->m_MaxZFocusOutSignalID);
	double xmin, xmax, ymin, ymax, zmin, zmax, value;
	pBox->m_pDoc->GetSize (&xmin, &xmax, &ymin, &ymax, &zmin, &zmax);
	if (pBox->GetNumber (pBox->MaxZ, &value, gcugtk::Min, zmin) && zmax != value) {
		pBox->m_pDoc->SetSize (xmin, xmax, ymin, ymax, zmin, value);
		pBox->m_pDoc->Update ();
		pBox->m_pDoc->SetDirty (true);
	}
	g_signal_handler_unblock (pBox->MaxZ, pBox->m_MaxZFocusOutSignalID);
	return false;
}

SizeDlg::SizeDlg (Application *App, Document* pDoc): gcugtk::Dialog (App, UIDIR"/size.ui", "size", GETTEXT_PACKAGE, pDoc)
{
	m_pDoc = pDoc;
	MinX = GTK_ENTRY (GetWidget ("xmin"));
	MaxX = GTK_ENTRY (GetWidget ("xmax"));
	MinY = GTK_ENTRY (GetWidget ("ymin"));
	MaxY = GTK_ENTRY (GetWidget ("ymax"));
	MinZ = GTK_ENTRY (GetWidget ("zmin"));
	MaxZ = GTK_ENTRY (GetWidget ("zmax"));
	double xmin, xmax, ymin, ymax, zmin, zmax;
	pDoc->GetSize(&xmin, &xmax, &ymin, &ymax, &zmin, &zmax);
	snprintf(m_buf, sizeof(m_buf), "%g", xmin);
	gtk_entry_set_text(MinX, m_buf);
	snprintf(m_buf, sizeof(m_buf), "%g", xmax);
	gtk_entry_set_text(MaxX, m_buf);
	snprintf(m_buf, sizeof(m_buf), "%g", ymin);
	gtk_entry_set_text(MinY, m_buf);
	snprintf(m_buf, sizeof(m_buf), "%g", ymax);
	gtk_entry_set_text(MaxY, m_buf);
	snprintf(m_buf, sizeof(m_buf), "%g", zmin);
	gtk_entry_set_text(MinZ, m_buf);
	snprintf(m_buf, sizeof(m_buf), "%g", zmax);
	gtk_entry_set_text(MaxZ, m_buf);
	gtk_widget_show_all (GTK_WIDGET (dialog));
	g_signal_connect_swapped (G_OBJECT (MinX), "activate", G_CALLBACK (SizeDlgPrivate::MinXEdited), this);
	m_MinXFocusOutSignalID = g_signal_connect_swapped (G_OBJECT (MinX), "focus-out-event", G_CALLBACK (SizeDlgPrivate::MinXEdited), this);
	g_signal_connect_swapped (G_OBJECT (MaxX), "activate", G_CALLBACK (SizeDlgPrivate::MaxXEdited), this);
	m_MaxXFocusOutSignalID = g_signal_connect_swapped (G_OBJECT (MaxX), "focus-out-event", G_CALLBACK (SizeDlgPrivate::MaxXEdited), this);
	g_signal_connect_swapped (G_OBJECT (MinY), "activate", G_CALLBACK (SizeDlgPrivate::MinYEdited), this);
	m_MinYFocusOutSignalID = g_signal_connect_swapped (G_OBJECT (MinY), "focus-out-event", G_CALLBACK (SizeDlgPrivate::MinYEdited), this);
	g_signal_connect_swapped (G_OBJECT (MaxY), "activate", G_CALLBACK (SizeDlgPrivate::MaxYEdited), this);
	m_MaxYFocusOutSignalID = g_signal_connect_swapped (G_OBJECT (MaxY), "focus-out-event", G_CALLBACK (SizeDlgPrivate::MaxYEdited), this);
	g_signal_connect_swapped (G_OBJECT (MinZ), "activate", G_CALLBACK (SizeDlgPrivate::MinZEdited), this);
	m_MinZFocusOutSignalID = g_signal_connect_swapped (G_OBJECT (MinZ), "focus-out-event", G_CALLBACK (SizeDlgPrivate::MinZEdited), this);
	g_signal_connect_swapped (G_OBJECT (MaxZ), "activate", G_CALLBACK (SizeDlgPrivate::MaxZEdited), this);
	m_MaxZFocusOutSignalID = g_signal_connect_swapped (G_OBJECT (MaxZ), "focus-out-event", G_CALLBACK (SizeDlgPrivate::MaxZEdited), this);
}

SizeDlg::~SizeDlg()
{
}

}	//	namespace gcr
