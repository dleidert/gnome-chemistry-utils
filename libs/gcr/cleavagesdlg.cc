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
	void OnAdd (CleavagesDlg *pBox);
	void OnDelete (CleavagesDlg *pBox);
	void OnDeleteAll (CleavagesDlg *pBox);
	void RowAdded (CleavagesDlg *pBox, unsigned row);
	void RowDeleted (CleavagesDlg *pBox, unsigned row);
	void ValueChanged (CleavagesDlg *pBox, unsigned row, unsigned column);
};

void CleavagesDlgPrivate::OnAdd (G_GNUC_UNUSED CleavagesDlg *pBox)
{
}

void CleavagesDlgPrivate::OnDelete (G_GNUC_UNUSED CleavagesDlg *pBox)
{
}

void CleavagesDlgPrivate::OnDeleteAll (G_GNUC_UNUSED CleavagesDlg *pBox)
{
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

static void on_add(G_GNUC_UNUSED GtkWidget *widget, CleavagesDlg *pBox)
{
	pBox->CleavageAdd();
}

static void on_delete(G_GNUC_UNUSED GtkWidget *widget, CleavagesDlg *pBox)
{
	pBox->CleavageDelete();
}

static void on_delete_all(G_GNUC_UNUSED GtkWidget *widget, CleavagesDlg *pBox)
{
	pBox->CleavageDeleteAll();
}

static void on_select(GtkTreeSelection *Selection, CleavagesDlg *pBox)
{
	pBox->CleavageSelect(Selection);
}

static void on_edited(GtkCellRendererText *cell, const gchar *path_string, const gchar *new_text, CleavagesDlg *pBox)
{
	pBox->OnEdited(cell, path_string, new_text);
}

CleavagesDlg::CleavagesDlg (gcr::Application *App, gcr::Document* pDoc): gcugtk::Dialog (App, UIDIR"/cleavages.ui", "cleavages", GETTEXT_PACKAGE, static_cast < gcu::DialogOwner * > (pDoc))
{
	m_pDoc = pDoc;
	GtkWidget* button = GetWidget ("add");
	g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (on_add), this);
	DeleteBtn = GetWidget ("delete");
	gtk_widget_set_sensitive(DeleteBtn,0);
	g_signal_connect (G_OBJECT (DeleteBtn), "clicked", G_CALLBACK (on_delete), this);
	DeleteAllBtn = GetWidget ("delete_all");
	g_signal_connect (G_OBJECT (DeleteAllBtn), "clicked", G_CALLBACK (on_delete_all), this);
	FixedBtn = GTK_TOGGLE_BUTTON (GetWidget ("fixed"));
	m_Grid = gcr_grid_new ("h", G_TYPE_INT, "k", G_TYPE_INT, "l", G_TYPE_INT, _("Planes cleaved"), G_TYPE_INT, NULL);
	g_object_set (G_OBJECT (m_Grid), "vexpand", true, NULL);
	GtkWidget *align = GetWidget ("cleavages-grid");
	gtk_grid_attach (GTK_GRID (align), m_Grid, 0, 0, 3, 1);
	gcr::CleavageList* Cleavages = m_pDoc->GetCleavageList ();
	m_Cleavages.resize ((Cleavages->size () / 5 + 1) * 5);
	for (list < gcr::Cleavage * >::iterator i = Cleavages->begin (); i != Cleavages->end (); i++)
		m_Cleavages[gcr_grid_append_row (GCR_GRID (m_Grid), (*i)->h (), (*i)->k (), (*i)->l (), (*i)->Planes ())] = *i;
//	g_signal_connect (G_OBJECT (Selection), "changed", G_CALLBACK (on_select), this);
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

void CleavagesDlg::CleavageAdd()
{
#if 0
	GtkTreeIter iter;

	struct CleavageStruct s;
	s.l = s.h = s.k = 1;
	s.planes = 1;
	g_array_append_vals(m_Cleavages, &s, 1);
	gtk_list_store_append (CleavageList, &iter);
	gtk_list_store_set (CleavageList, &iter,
		      0, 1,
		      1, 1,
		      2, 1,
		      3, 1,
		      -1);
	gtk_widget_set_sensitive(DeleteAllBtn, true);
	gtk_tree_selection_select_iter(Selection, &iter);
#endif
}

void CleavagesDlg::CleavageDelete()
{
#if 0
	GtkTreeModel* model = GTK_TREE_MODEL(CleavageList);
	GtkTreeIter iter;

	if(gtk_tree_selection_get_selected(Selection, &model, &iter))
	{
		gint i;
		GtkTreePath *path;

		path = gtk_tree_model_get_path (model, &iter);
		i = gtk_tree_path_get_indices (path)[0];
		gtk_list_store_remove(GTK_LIST_STORE(model), &iter);

		g_array_remove_index(m_Cleavages, i);

		gtk_tree_path_free(path);
    }
	if (!m_Cleavages->len)gtk_widget_set_sensitive(DeleteAllBtn, false);
#endif
}

void CleavagesDlg::CleavageDeleteAll()
{
#if 0
	g_array_free(m_Cleavages, false);
	m_Cleavages = g_array_sized_new (FALSE, FALSE, sizeof (struct CleavageStruct), 1);
	gtk_list_store_clear(CleavageList);
	gtk_widget_set_sensitive(DeleteAllBtn, false);
#endif
}

void CleavagesDlg::CleavageSelect(GtkTreeSelection *Selection)
{
#if 0
	GtkTreeModel* model = GTK_TREE_MODEL (CleavageList);
	GtkTreeIter iter;
	if (gtk_tree_selection_get_selected (Selection, &model, &iter)) {
		gtk_widget_set_sensitive (DeleteBtn, true);
	} else {
		gtk_widget_set_sensitive (DeleteBtn, false);
		if (!m_Cleavages->len)
			gtk_widget_set_sensitive (DeleteAllBtn, false);
	}
#endif
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

