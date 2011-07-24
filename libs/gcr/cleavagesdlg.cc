// -*- C++ -*-

/*
 * Gnome Crystal
 * cleavagesdlg.cc
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
#include "application.h"
#include "cleavagesdlg.h"
#include "document.h"
#include <gcu/application.h>
#include <glib/gi18n.h>

using namespace std;

namespace gcr {

enum
{
	COLUMN_H,
	COLUMN_K,
	COLUMN_L,
	COLUMN_PLANES
};

class CleavagesDlgPrivate {
public:
	static void AddRow (CleavagesDlg *pBox);
	static void DeleteRow (CleavagesDlg *pBox);
	static void DeleteAll (CleavagesDlg *pBox);
	static void ValueChanged (CleavagesDlg *pBox, unsigned row, unsigned column);
	static void RowSelected (CleavagesDlg *pBox, int row);
	static void FixedSizeChanged (CleavagesDlg *pBox, GtkToggleButton *btn);
};

void CleavagesDlgPrivate::AddRow (G_GNUC_UNUSED CleavagesDlg *pBox)
{
	Cleavage *c = new Cleavage ();
	c->h () = 1;
	c->k () = 1;
	c->l () = 1;
	c->Planes() = 1;
	unsigned new_row = gcr_grid_append_row (GCR_GRID (pBox->m_Grid), 1, 1, 1, 1),max_row = pBox->m_Cleavages.capacity ();
	if (new_row >= max_row)
		pBox->m_Cleavages.resize (max_row + 5);
	pBox->m_Cleavages[new_row] = c;
	pBox->m_pDoc->GetCleavageList ()->push_back (c);
	pBox->m_pDoc->Update ();
	pBox->m_pDoc->SetDirty (true);
}

void CleavagesDlgPrivate::DeleteRow (G_GNUC_UNUSED CleavagesDlg *pBox)
{
	pBox->m_pDoc->GetCleavageList ()->remove (pBox->m_Cleavages[pBox->m_CurRow]);
	delete pBox->m_Cleavages[pBox->m_CurRow];
	pBox->m_Cleavages.erase (pBox->m_Cleavages.begin () + pBox->m_CurRow);
	gcr_grid_delete_row (GCR_GRID (pBox->m_Grid), pBox->m_CurRow);
	pBox->m_pDoc->Update ();
	pBox->m_pDoc->SetDirty (true);
}

void CleavagesDlgPrivate::DeleteAll (G_GNUC_UNUSED CleavagesDlg *pBox)
{
	gcr_grid_delete_all (GCR_GRID (pBox->m_Grid));
	for (unsigned i = 0; i < pBox->m_Cleavages.size (); i++)
		delete pBox->m_Cleavages[i];
	pBox->m_Cleavages.clear ();
	pBox->m_pDoc->GetCleavageList ()->clear ();
	pBox->m_pDoc->Update ();
	pBox->m_pDoc->SetDirty (true);
}

void CleavagesDlgPrivate::ValueChanged (CleavagesDlg *pBox, G_GNUC_UNUSED unsigned row, G_GNUC_UNUSED unsigned column)
{
	switch (column) {
	case COLUMN_H: // h changed
		pBox->m_Cleavages[row]->h () = gcr_grid_get_int (GCR_GRID (pBox->m_Grid), row, column);
		break;
	case COLUMN_K: // k changed
		pBox->m_Cleavages[row]->k () = gcr_grid_get_int (GCR_GRID (pBox->m_Grid), row, column);
		break;
	case COLUMN_L: // l changed
		pBox->m_Cleavages[row]->l () = gcr_grid_get_int (GCR_GRID (pBox->m_Grid), row, column);
		break;
	case COLUMN_PLANES: // planes number changed
		pBox->m_Cleavages[row]->Planes () = gcr_grid_get_uint (GCR_GRID (pBox->m_Grid), row, column);
		break;
	}
	pBox->m_pDoc->Update ();
	pBox->m_pDoc->SetDirty (true);
}

void CleavagesDlgPrivate::RowSelected (CleavagesDlg *pBox, int row)
{
	pBox->m_CurRow = row;
	gtk_widget_set_sensitive (pBox->DeleteBtn, row >= 0);
}

void CleavagesDlgPrivate::FixedSizeChanged (CleavagesDlg *pBox, GtkToggleButton *btn)
{
	pBox->m_pDoc->SetFixedSize (gtk_toggle_button_get_active (btn));
	pBox->m_pDoc->Update ();
	pBox->m_pDoc->SetDirty (true);
}

CleavagesDlg::CleavagesDlg (gcr::Application *App, gcr::Document* pDoc): gcugtk::Dialog (App, UIDIR"/cleavages.ui", "cleavages", GETTEXT_PACKAGE, static_cast < gcu::DialogOwner * > (pDoc))
{
	m_pDoc = pDoc;
	m_Closing = false;
	GtkWidget* button = GetWidget ("add");
	g_signal_connect_swapped (G_OBJECT (button), "clicked", G_CALLBACK (CleavagesDlgPrivate::AddRow), this);
	DeleteBtn = GetWidget ("delete");
	gtk_widget_set_sensitive(DeleteBtn,0);
	g_signal_connect_swapped (G_OBJECT (DeleteBtn), "clicked", G_CALLBACK (CleavagesDlgPrivate::DeleteRow), this);
	DeleteAllBtn = GetWidget ("delete_all");
	g_signal_connect_swapped (G_OBJECT (DeleteAllBtn), "clicked", G_CALLBACK (CleavagesDlgPrivate::DeleteAll), this);
	button = GetWidget ("fixed");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), m_pDoc->GetFixedSize ());
	g_signal_connect_swapped (G_OBJECT (button), "toggled", G_CALLBACK (CleavagesDlgPrivate::FixedSizeChanged), this);
	m_Grid = gcr_grid_new ("h", G_TYPE_INT, "k", G_TYPE_INT, "l", G_TYPE_INT, _("Planes cleaved"), G_TYPE_UINT, NULL);
	g_object_set (G_OBJECT (m_Grid), "expand", true, NULL);
	GtkWidget *align = GetWidget ("cleavages-grid");
	gtk_grid_attach (GTK_GRID (align), m_Grid, 0, 0, 1, 4);
	gcr::CleavageList* Cleavages = m_pDoc->GetCleavageList ();
	m_Cleavages.resize ((Cleavages->size () / 5 + 1) * 5);
	for (list < gcr::Cleavage * >::iterator i = Cleavages->begin (); i != Cleavages->end (); i++)
		m_Cleavages[gcr_grid_append_row (GCR_GRID (m_Grid), (*i)->h (), (*i)->k (), (*i)->l (), (*i)->Planes ())] = *i;
	g_signal_connect_swapped (G_OBJECT (m_Grid), "row-selected", G_CALLBACK (CleavagesDlgPrivate::RowSelected), this);
	g_signal_connect_swapped (G_OBJECT (m_Grid), "value-changed", G_CALLBACK (CleavagesDlgPrivate::ValueChanged), this);
	if (!m_Cleavages.size ())
		gtk_widget_set_sensitive (DeleteAllBtn, false);
	gtk_widget_show_all (GTK_WIDGET (dialog));
}

CleavagesDlg::~CleavagesDlg()
{
}

void CleavagesDlg::Closed ()
{
	// check if the cleavages list is coherent
	m_Closing = true;
	m_pDoc->CheckCleavages ();
}

void CleavagesDlg::ReloadData ()
{
	if (m_Closing)
		return;
	gcr_grid_delete_all (GCR_GRID (m_Grid));
	m_Cleavages.clear ();
	gcr::CleavageList* Cleavages = m_pDoc->GetCleavageList ();
	for (list < gcr::Cleavage * >::iterator i = Cleavages->begin (); i != Cleavages->end (); i++)
		m_Cleavages[gcr_grid_append_row (GCR_GRID (m_Grid), (*i)->h (), (*i)->k (), (*i)->l (), (*i)->Planes ())] = *i;
	if (!m_Cleavages.size ())
		gtk_widget_set_sensitive (DeleteAllBtn, false);
}

}	//	namespace gcr

