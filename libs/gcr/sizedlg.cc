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
}

SizeDlg::~SizeDlg()
{
}

bool SizeDlg::Apply ()
{
	double xmin, xmax, ymin, ymax, zmin, zmax, x;
	if ((!GetNumber (MinX, &xmin)) ||
		(!GetNumber (MaxX, &xmax)) ||
		(!GetNumber (MinY, &ymin)) ||
		(!GetNumber (MaxY, &ymax)) ||
		(!GetNumber (MinZ, &zmin)) ||
		(!GetNumber (MaxZ, &zmax))) {
		return false;
	}
	if (xmin > xmax) {
		x = xmin;
		xmin = xmax;
		xmax = x;
	}
	if (ymin > ymax) {
		x = ymin;
		ymin = ymax;
		ymax = x;
	}
	if (zmin > zmax) {
		x = zmin;
		zmin = zmax;
		zmax = x;
	}
	m_pDoc->SetSize (xmin, xmax, ymin, ymax, zmin, zmax);
	m_pDoc->Update ();
	m_pDoc->SetDirty (true);
	return true;
}

}	//	namespace gcr
