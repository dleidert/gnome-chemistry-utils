// -*- C++ -*-

/* 
 * Gnome Crystal
 * cleavagesdlg.cc 
 *
 * Copyright (C) 2002-2010 Jean Bréfort <jean.brefort@normalesup.org>
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

struct CleavageStruct {
	int h, k, l;
	unsigned planes;
};

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

CleavagesDlg::CleavagesDlg (gcu::Application *App, gcr::Document* pDoc): Dialog (App, UIDIR"/cleavages.ui", "cleavages", GETTEXT_PACKAGE, pDoc)
{
	m_pDoc = pDoc;
	GtkWidget* button = GetWidget ("add");
	g_signal_connect (G_OBJECT(button), "clicked", G_CALLBACK(on_add), this);
	DeleteBtn = GetWidget ("delete");
	gtk_widget_set_sensitive(DeleteBtn,0);
	g_signal_connect (G_OBJECT (DeleteBtn), "clicked", G_CALLBACK (on_delete), this);
	DeleteAllBtn = GetWidget ("delete_all");
	g_signal_connect (G_OBJECT (DeleteAllBtn), "clicked", G_CALLBACK (on_delete_all), this);
	FixedBtn = GTK_TOGGLE_BUTTON (GetWidget ("fixed"));
	m_Grid = gcr_grid_new ("h", "k", "l", _("Planes cleaved"), NULL);
	gtk_widget_show_all (m_Grid);
	GtkWidget *align = GetWidget ("cleavages-align");
	gtk_container_add (GTK_CONTAINER (align), m_Grid);
#if 0
	GtkTreeView* tree = GTK_TREE_VIEW (GetWidget ("cleavageslist"));
	Selection = gtk_tree_view_get_selection (tree);
	CleavageList = gtk_list_store_new (5, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_UINT, G_TYPE_BOOLEAN);
	gtk_tree_view_set_model (tree, GTK_TREE_MODEL (CleavageList));
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	/* column for h */
	renderer = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT (renderer), "editable", true, NULL);
	g_object_set_data (G_OBJECT (renderer), "column", (gint *) COLUMN_H);
	g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (on_edited), this);
	column = gtk_tree_view_column_new_with_attributes(_("h"), renderer, "text", 0, NULL);
	/* set this column to a fixed sizing (of 50 pixels) */
	gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column), GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 50);
	gtk_tree_view_append_column (tree, column);
	/* column for k */
	renderer = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT (renderer), "editable", true, NULL);
	g_object_set_data (G_OBJECT (renderer), "column", (gint *) COLUMN_K);
	g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (on_edited), this);
	column = gtk_tree_view_column_new_with_attributes (_("k"), renderer, "text", 1, NULL);
	/* set this column to a fixed sizing (of 50 pixels) */
	gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column), GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 50);
	gtk_tree_view_append_column (tree, column);
	/* column for l */
	renderer = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT (renderer), "editable", true, NULL);
	g_object_set_data (G_OBJECT (renderer), "column", (gint *) COLUMN_L);
	g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (on_edited), this);
	column = gtk_tree_view_column_new_with_attributes(_("l"), renderer, "text", 2, NULL);
	/* set this column to a fixed sizing (of 50 pixels) */
	gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column), GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 50);
	gtk_tree_view_append_column (tree, column);
	/* column for planes number */
	renderer = gtk_cell_renderer_text_new ();
	g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (on_edited), this);
	g_object_set (G_OBJECT (renderer), "editable", true, NULL);
	g_object_set_data(G_OBJECT (renderer), "column", (gint *) COLUMN_PLANES);
	column = gtk_tree_view_column_new_with_attributes (_("Planes cleaved"), renderer, "text", 3, NULL);
	/* set this column to a fixed sizing (of 75 pixels) */
	gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column), GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 75);
	gtk_tree_view_append_column (tree, column);
#endif
	m_Cleavages = g_array_sized_new (FALSE, FALSE, sizeof (struct CleavageStruct), 1);
	gcr::CleavageList* Cleavages = m_pDoc->GetCleavageList ();
	gcr::Cleavage* pCleavage;
	struct CleavageStruct s;
//	GtkTreeIter iter;
	for (list<gcr::Cleavage*>::iterator i = Cleavages->begin (); i != Cleavages->end (); i++)
	{
		pCleavage = *i;
		s.h = pCleavage->h ();
		s.k = pCleavage->k ();
		s.l = pCleavage->l ();
		s.planes = pCleavage->Planes ();
		g_array_append_vals (m_Cleavages, &s, 1);
/*		gtk_list_store_append (CleavageList, &iter);
		gtk_list_store_set (CleavageList, &iter,
				  0, s.h,
				  1, s.k,
				  2, s.l,
				  3, s.planes,
				  -1);*/
	}
//	g_signal_connect (G_OBJECT (Selection), "changed", G_CALLBACK (on_select), this);
	if (!m_Cleavages->len)
		gtk_widget_set_sensitive (DeleteAllBtn, false);
}

CleavagesDlg::~CleavagesDlg()
{
	if (m_Cleavages)
		g_array_free (m_Cleavages, false);
}

bool CleavagesDlg::Apply()
{
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
	return true;
}

void CleavagesDlg::CleavageAdd()
{
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
}

void CleavagesDlg::CleavageDelete()
{
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
}

void CleavagesDlg::CleavageDeleteAll()
{
	g_array_free(m_Cleavages, false);
	m_Cleavages = g_array_sized_new (FALSE, FALSE, sizeof (struct CleavageStruct), 1);
	gtk_list_store_clear(CleavageList);
	gtk_widget_set_sensitive(DeleteAllBtn, false);
}

void CleavagesDlg::CleavageSelect(GtkTreeSelection *Selection)
{
	GtkTreeModel* model = GTK_TREE_MODEL (CleavageList);
	GtkTreeIter iter;
	if (gtk_tree_selection_get_selected (Selection, &model, &iter)) {
		gtk_widget_set_sensitive (DeleteBtn, true);
	} else {
		gtk_widget_set_sensitive (DeleteBtn, false);
		if (!m_Cleavages->len)
			gtk_widget_set_sensitive (DeleteAllBtn, false);
	}
}

void CleavagesDlg::OnEdited(GtkCellRendererText *cell, const gchar *path_string, const gchar *new_text)
{
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
}

}	//	namespace gcr

