// -*- C++ -*-

/* 
 * Gnome Crystal
 * sizedlg.cc 
 *
 * Copyright (C) 2002-2006 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "sizedlg.h"
#include "document.h"
#include "application.h"

gcSizeDlg::gcSizeDlg (gcApplication *App, gcDocument* pDoc): Dialog (App, DATADIR"/gchemutils-unstable/glade/crystal/size.glade", "size")
{
	m_pDoc = pDoc;
	pDoc->NotifyDialog(this);
	MinX = (GtkEntry*) glade_xml_get_widget(xml, "xmin");
	MaxX = (GtkEntry*) glade_xml_get_widget(xml, "xmax");
	MinY = (GtkEntry*) glade_xml_get_widget(xml, "ymin");
	MaxY = (GtkEntry*) glade_xml_get_widget(xml, "ymax");
	MinZ = (GtkEntry*) glade_xml_get_widget(xml, "zmin");
	MaxZ = (GtkEntry*) glade_xml_get_widget(xml, "zmax");
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

gcSizeDlg::~gcSizeDlg()
{
	m_pDoc->RemoveDialog(this);
}

bool gcSizeDlg::Apply ()
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
	m_pDoc->SetDirty ();
	return true;
}
