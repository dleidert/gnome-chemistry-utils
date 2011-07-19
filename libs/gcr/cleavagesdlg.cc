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
	static void RowAdded (CleavagesDlg *pBox, unsigned row);
	static void RowDeleted (CleavagesDlg *pBox, unsigned row);
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

void CleavagesDlgPrivate::RowAdded (G_GNUC_UNUSED CleavagesDlg *pBox, G_GNUC_UNUSED unsigned new_row)
{
}

void CleavagesDlgPrivate::RowDeleted (G_GNUC_UNUSED CleavagesDlg *pBox, G_GNUC_UNUSED unsigned row)
{
}

void CleavagesDlgPrivate::ValueChanged (G_GNUC_UNUSED CleavagesDlg *pBox, G_GNUC_UNUSED unsigned row, G_GNUC_UNUSED unsigned column)
{
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
	GtkWidget* button = GetWidget ("add");
	g_signal_connect_swapped (G_OBJECT (button), "clicked", G_CALLBACK (CleavagesDlgPrivate::AddRow), this);
	DeleteBtn = GetWidget ("delete");
	gtk_widget_set_sensitive(DeleteBtn,0);
	g_signal_connect_swapped (G_OBJECT (DeleteBtn), "clicked", G_CALLBACK (CleavagesDlgPrivate::DeleteRow), this);
	DeleteAllBtn = GetWidget ("delete_all");
	g_signal_connect_swapped (G_OBJECT (DeleteAllBtn), "clicked", G_CALLBACK (CleavagesDlgPrivate::DeleteAll), this);
	FixedBtn = GTK_TOGGLE_BUTTON (GetWidget ("fixed"));
	gtk_toggle_button_set_active (FixedBtn, m_pDoc->GetFixedSize ());
	g_signal_connect_swapped (G_OBJECT (FixedBtn), "toggled", G_CALLBACK (CleavagesDlgPrivate::FixedSizeChanged), this);
	m_Grid = gcr_grid_new ("h", G_TYPE_INT, "k", G_TYPE_INT, "l", G_TYPE_INT, _("Planes cleaved"), G_TYPE_INT, NULL);
	g_object_set (G_OBJECT (m_Grid), "expand", true, NULL);
	GtkWidget *align = GetWidget ("cleavages-grid");
	gtk_grid_attach (GTK_GRID (align), m_Grid, 0, 0, 1, 4);
	gcr::CleavageList* Cleavages = m_pDoc->GetCleavageList ();
	m_Cleavages.resize ((Cleavages->size () / 5 + 1) * 5);
	for (list < gcr::Cleavage * >::iterator i = Cleavages->begin (); i != Cleavages->end (); i++)
		m_Cleavages[gcr_grid_append_row (GCR_GRID (m_Grid), (*i)->h (), (*i)->k (), (*i)->l (), (*i)->Planes ())] = *i;
	g_signal_connect_swapped (G_OBJECT (m_Grid), "row-selected", G_CALLBACK (CleavagesDlgPrivate::RowSelected), this);
	if (!m_Cleavages.size ())
		gtk_widget_set_sensitive (DeleteAllBtn, false);
	gtk_widget_show_all (GTK_WIDGET (dialog));
}

CleavagesDlg::~CleavagesDlg()
{
}

bool CleavagesDlg::Apply()
{
#if 0
	gcr::CleavageList* Cleavages = m_pDoc->GetCleavageList();
	//First, delete old Cleavages
	while (!Cleavages->empty())
	{
		delete Cleavages->front();
		Cleavages->pop_front();
	}
	//Add new Cleavages from array

	gcr::Cleavage* c;
	struct CleavageStruct* s;
	for (unsigned i = 0; i  < m_Cleavages->len; i++)
	{
		s = &g_array_index(m_Cleavages, struct CleavageStruct, i);
		if (!s->planes || ((!s->h) && (!s->k) && (!s->l))) continue;
		c = new gcr::Cleavage();
		c->h() = s->h;
		c->k() = s->k;
		c->l() = s->l;
		c->Planes() = s->planes;
		Cleavages->push_back(c);
	}
	m_pDoc->SetFixedSize (gtk_toggle_button_get_active (FixedBtn));
	m_pDoc->Update ();
	m_pDoc->SetDirty (true);
#endif
	return true;
}

void CleavagesDlg::OnEdited(GtkCellRendererText *cell, const gchar *path_string, const gchar *new_text)
{
#if 0
	GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
	GtkTreeIter iter;

	gtk_tree_model_get_iter (GTK_TREE_MODEL(CleavageList), &iter, path);

	long i = gtk_tree_path_get_indices (path)[0], j  = (long) g_object_get_data (G_OBJECT (cell), "column");
	int x = atol(new_text);
	gtk_list_store_set(CleavageList, &iter, j, x, -1);
	switch (j)
	{
		case COLUMN_H:
			g_array_index(m_Cleavages, struct CleavageStruct, i).h = x;
			break;
		case COLUMN_K:
			g_array_index(m_Cleavages, struct CleavageStruct, i).k = x;
			break;
		case COLUMN_L:
			g_array_index(m_Cleavages, struct CleavageStruct, i).l = x;
			break;
		case COLUMN_PLANES:
			g_array_index(m_Cleavages, struct CleavageStruct, i).planes = x;
			break;
	}
	gtk_tree_path_free (path);
#endif
}

}	//	namespace gcr

