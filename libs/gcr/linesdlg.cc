// -*- C++ -*-

/* 
 * Gnome Crystal
 * linesdlg.cc 
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
#include "linesdlg.h"
#include "document.h"
#include "application.h"
#include <glib/gi18n.h>

using namespace std;

namespace gcr {
	
enum
{
	COLUMN_X1,
	COLUMN_Y1,
	COLUMN_Z1,
	COLUMN_X2,
	COLUMN_Y2,
	COLUMN_Z2,
	COLUMN_SINGLE
};

struct LineStruct {
	double r, x1, y1, z1, x2, y2, z2;
	double Blue, Red, Green, Alpha;
	bool duplicated;
};

static void on_add (G_GNUC_UNUSED GtkWidget *widget, LinesDlg *pBox)
{
	pBox->LineAdd ();
}

static void on_delete (G_GNUC_UNUSED GtkWidget *widget, LinesDlg *pBox)
{
	pBox->LineDelete ();
}

static void on_delete_all (G_GNUC_UNUSED GtkWidget *widget, LinesDlg *pBox)
{
	pBox->LineDeleteAll ();
}

static void on_select (GtkTreeSelection *Selection, LinesDlg *pBox)
{
	pBox->LineSelect (Selection);
}

static void on_edited (GtkCellRendererText *cell, const gchar *path_string, const gchar *new_text, LinesDlg *pBox)
{
	pBox->OnEdited (cell, path_string, new_text);
}

static void on_toggled (GtkCellRendererToggle *cell, const gchar *path_string, LinesDlg *pBox)
{
	pBox->OnToggled (cell, path_string);
}

static void on_edges_toggled (G_GNUC_UNUSED GtkToggleButton* btn, LinesDlg *pBox)
{
	pBox->OnToggledSpecial (gcr::edges);
}

static void on_diags_toggled (G_GNUC_UNUSED GtkToggleButton* btn, LinesDlg *pBox)
{
	pBox->OnToggledSpecial (gcr::diagonals);
}

static void on_medians_toggled (G_GNUC_UNUSED GtkToggleButton* btn, LinesDlg *pBox)
{
	pBox->OnToggledSpecial (gcr::medians);
}

LinesDlg::LinesDlg (Application *App, Document* pDoc): gcu::Dialog (App, UIDIR"/lines.ui", "lines", GETTEXT_PACKAGE, pDoc)
{
	m_pDoc = pDoc;
	GtkWidget* button = GetWidget ("add");
	g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (on_add), this);
	DeleteBtn = GetWidget ("delete");
	gtk_widget_set_sensitive (DeleteBtn,0);
	g_signal_connect (G_OBJECT (DeleteBtn), "clicked", G_CALLBACK (on_delete), this);
	DeleteAllBtn = GetWidget ("delete_all");
	g_signal_connect (G_OBJECT (DeleteAllBtn), "clicked", G_CALLBACK (on_delete_all), this);
	LineList = gtk_list_store_new (7, G_TYPE_DOUBLE, G_TYPE_DOUBLE, G_TYPE_DOUBLE, G_TYPE_DOUBLE, G_TYPE_DOUBLE, G_TYPE_DOUBLE, G_TYPE_BOOLEAN);
	GtkTreeView* tree = GTK_TREE_VIEW (GetWidget ("lineslist"));
	Selection = gtk_tree_view_get_selection (tree);
	gtk_tree_view_set_model (tree, GTK_TREE_MODEL (LineList));
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	/* column for x1 */
	renderer = gtk_cell_renderer_text_new ();
	g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (on_edited), this);
	g_object_set (G_OBJECT (renderer), "editable", true, NULL);
	g_object_set_data (G_OBJECT (renderer), "column", (gint *) COLUMN_X1);
	column = gtk_tree_view_column_new_with_attributes (_("x1"), renderer, "text", COLUMN_X1, NULL);
	/* set this column to a fixed sizing (of 50 pixels) */
	gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column), GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 50);
	gtk_tree_view_append_column (tree, column);
	/* column for y1 */
	renderer = gtk_cell_renderer_text_new ();
	g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (on_edited), this);
	g_object_set (G_OBJECT (renderer), "editable", true, NULL);
	g_object_set_data (G_OBJECT (renderer), "column", (gint *) COLUMN_Y1);
	column = gtk_tree_view_column_new_with_attributes (_("y1"), renderer, "text", COLUMN_Y1, NULL);
	/* set this column to a fixed sizing (of 50 pixels) */
	gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column), GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 50);
	gtk_tree_view_append_column (tree, column);
	/* column for z1 */
	renderer = gtk_cell_renderer_text_new ();
	g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (on_edited), this);
	g_object_set (G_OBJECT (renderer), "editable", true, NULL);
	g_object_set_data (G_OBJECT (renderer), "column", (gint *) COLUMN_Z1);
	column = gtk_tree_view_column_new_with_attributes (_("z1"), renderer, "text", COLUMN_Z1, NULL);
	/* set this column to a fixed sizing (of 50 pixels) */
	gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column), GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 50);
	gtk_tree_view_append_column (tree, column);
	/* column for x2 */
	renderer = gtk_cell_renderer_text_new ();
	g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (on_edited), this);
	g_object_set(G_OBJECT(renderer), "editable", true, NULL);
	g_object_set_data(G_OBJECT (renderer), "column", (gint *)COLUMN_X2);
	column = gtk_tree_view_column_new_with_attributes (_("x2"), renderer, "text", COLUMN_X2, NULL);
	/* set this column to a fixed sizing (of 50 pixels) */
	gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column), GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 50);
	gtk_tree_view_append_column (tree, column);
	/* column for y2 */
	renderer = gtk_cell_renderer_text_new ();
	g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (on_edited), this);
	g_object_set (G_OBJECT (renderer), "editable", true, NULL);
	g_object_set_data (G_OBJECT (renderer), "column", (gint *) COLUMN_Y2);
	column = gtk_tree_view_column_new_with_attributes (_("y2"), renderer, "text", COLUMN_Y2, NULL);
	/* set this column to a fixed sizing (of 50 pixels) */
	gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column), GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 50);
	gtk_tree_view_append_column (tree, column);
	/* column for z2 */
	renderer = gtk_cell_renderer_text_new ();
	g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (on_edited), this);
	g_object_set (G_OBJECT (renderer), "editable", true, NULL);
	g_object_set_data (G_OBJECT (renderer), "column", (gint *) COLUMN_Z2);
	column = gtk_tree_view_column_new_with_attributes (_("z2"), renderer, "text", COLUMN_Z2, NULL);
	/* set this column to a fixed sizing (of 50 pixels) */
	gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column), GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 50);
	gtk_tree_view_append_column (tree, column);
	/* column for single */
	renderer = gtk_cell_renderer_toggle_new ();
	g_signal_connect (G_OBJECT (renderer), "toggled", G_CALLBACK (on_toggled), this);
	g_object_set_data (G_OBJECT (renderer), "column", (gint *) COLUMN_SINGLE);
	column = gtk_tree_view_column_new_with_attributes (_("Single"), renderer, "active", COLUMN_SINGLE, NULL);
	/* set this column to a fixed sizing (of 50 pixels) */
	gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column), GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN(column), 50);
	gtk_tree_view_append_column (tree, column);
	m_Lines = g_array_sized_new (FALSE, FALSE, sizeof (struct LineStruct), 1);
	LineColor = GTK_COLOR_BUTTON (GetWidget ("color"));
	LineR = GTK_ENTRY (GetWidget ("radius"));
	Edges = GTK_CHECK_BUTTON (GetWidget ("edges"));
	g_signal_connect (G_OBJECT (Edges), "toggled", G_CALLBACK (on_edges_toggled), this);
	EdgesColor = GTK_COLOR_BUTTON (GetWidget ("edges-color"));
	gtk_widget_set_sensitive (GTK_WIDGET (EdgesColor), false);
	EdgesR = GTK_ENTRY (GetWidget ("edges_radius"));
	gtk_widget_set_sensitive (GTK_WIDGET (EdgesR), false);
	Medians = GTK_CHECK_BUTTON (GetWidget ("medians"));
	g_signal_connect (G_OBJECT (Medians), "toggled", G_CALLBACK (on_medians_toggled), this);
	MediansColor = GTK_COLOR_BUTTON (GetWidget ("med-color"));
	gtk_widget_set_sensitive (GTK_WIDGET (MediansColor), false);
	MediansR = GTK_ENTRY (GetWidget ("med_radius"));
	gtk_widget_set_sensitive (GTK_WIDGET (MediansR), false);
	Diags = GTK_CHECK_BUTTON (GetWidget ("diagonals"));
	g_signal_connect (G_OBJECT (Diags), "toggled", G_CALLBACK (on_diags_toggled), this);
	DiagsColor = GTK_COLOR_BUTTON (GetWidget ("diag-color"));
	gtk_widget_set_sensitive (GTK_WIDGET (DiagsColor), false);
	DiagsR = GTK_ENTRY (GetWidget ("diag_radius"));
	gtk_widget_set_sensitive (GTK_WIDGET (DiagsR), false);
	m_LineSelected = -1;
	gcr::LineList* Lines = m_pDoc->GetLineList ();
	gcr::Line* pLine;
	struct LineStruct s;
	GtkTreeIter iter;
	char *tmp;
	GdkColor color;
	for (list<gcr::Line*>::iterator i = Lines->begin(); i != Lines->end(); i++) {
		pLine = *i;
		s.duplicated = false;
		s.r = pLine->GetRadius ();
		pLine->GetColor (&s.Red, &s.Green, &s.Blue, &s.Alpha);
		color.red = (guint16) (s.Red * 65535.);
		color.green = (guint16) (s.Green * 65535.);
		color.blue = (guint16) (s.Blue * 65535.);
		switch (pLine->Type()) {
		case gcr::edges:
			gtk_widget_set_sensitive (GTK_WIDGET (EdgesColor), true);
			gtk_widget_set_sensitive (GTK_WIDGET (EdgesR), true);
			tmp = g_strdup_printf ("%g", s.r);
			gtk_entry_set_text (EdgesR, tmp);
			gtk_color_button_set_color (EdgesColor, &color);
			gtk_color_button_set_alpha (EdgesColor, (guint16) (s.Alpha * 65535.));
			g_free (tmp);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(Edges), true);
			break;
		case gcr::diagonals:
			gtk_widget_set_sensitive (GTK_WIDGET (DiagsColor), true);
			gtk_widget_set_sensitive (GTK_WIDGET (DiagsR), true);
			tmp = g_strdup_printf ("%g", s.r);
			gtk_entry_set_text (DiagsR, tmp);
			gtk_color_button_set_color (DiagsColor, &color);
			gtk_color_button_set_alpha (DiagsColor, (guint16) (s.Alpha * 65535.));
			g_free (tmp);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (Diags), true);
			break;
		case gcr::medians:
			gtk_widget_set_sensitive (GTK_WIDGET (MediansColor), true);
			gtk_widget_set_sensitive (GTK_WIDGET (MediansR), true);
			tmp = g_strdup_printf ("%g", s.r);
			gtk_entry_set_text (MediansR, tmp);
			gtk_color_button_set_color (MediansColor, &color);
			gtk_color_button_set_alpha (MediansColor, (guint16) (s.Alpha * 65535.));
			g_free (tmp);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(Medians), true);
			break;
		case gcr::normal:
			s.duplicated = true;
		case gcr::unique:
			s.x1 = pLine->X1();
			s.y1 = pLine->Y1();
			s.z1 = pLine->Z1();
			s.x2 = pLine->X2();
			s.y2 = pLine->Y2();
			s.z2 = pLine->Z2();
			g_array_append_vals(m_Lines, &s, 1);
			gtk_list_store_append (LineList, &iter);
			gtk_list_store_set (LineList, &iter,
					  0, s.x1,
					  1, s.y1,
					  2, s.z1,
					  3, s.x2,
					  4, s.y2,
					  5, s.z2,
					  6, !s.duplicated, 
					  -1);
			break;
		}
	}
	g_signal_connect (G_OBJECT (Selection), "changed", G_CALLBACK (on_select), this);
	if (!m_Lines->len)
		gtk_widget_set_sensitive (DeleteAllBtn, false);
}

LinesDlg::~LinesDlg ()
{
	if (m_Lines)
		g_array_free (m_Lines, false);
}

bool LinesDlg::Apply ()
{
	GdkColor color;
	if (m_LineSelected >= 0) {
		gtk_color_button_get_color (LineColor, &color);
		g_array_index(m_Lines, struct LineStruct, m_LineSelected).Red = color.red / 65535.;
		g_array_index(m_Lines, struct LineStruct, m_LineSelected).Green = color.green / 65535.;
		g_array_index(m_Lines, struct LineStruct, m_LineSelected).Blue = color.blue / 65535.;
		g_array_index(m_Lines, struct LineStruct, m_LineSelected).Alpha = gtk_color_button_get_alpha (LineColor) / 65535.;
		if ((!GetNumber(LineR, &g_array_index (m_Lines, struct LineStruct, m_LineSelected).r, gcu::Min, 0)) || (g_array_index (m_Lines, struct LineStruct, m_LineSelected).r == 0.0)) {
		}
	}
	gcr::LineList* Lines = m_pDoc->GetLineList ();
	//First, delete old lines
	while (!Lines->empty ())
	{
		delete Lines->front ();
		Lines->pop_front ();
	}
	//Add edged, diagonals and medians
	gcr::Line* pLine;
	double r, Red, Green, Blue, Alpha;
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (Edges))) {
		GetNumber (EdgesR, &r, gcu::Min, 0);
		if (r > 0.0) {
			gtk_color_button_get_color (EdgesColor, &color);
			Red = color.red / 65535.;
			Green = color.green / 65535.;
			Blue = color.blue / 65535.;
			Alpha = gtk_color_button_get_alpha (EdgesColor) / 65535.;
			pLine = new gcr::Line (gcr::edges, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, r, Red, Green, Blue, Alpha);
			Lines->push_back (pLine);
		}
	}
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (Diags))) {
		GetNumber (DiagsR, &r, gcu::Min, 0);
		if (r > 0.0) {
			gtk_color_button_get_color (DiagsColor, &color);
			Red = color.red / 65535.;
			Green = color.green / 65535.;
			Blue = color.blue / 65535.;
			Alpha = gtk_color_button_get_alpha (DiagsColor) / 65535.;
			pLine = new gcr::Line (gcr::diagonals, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, r, Red, Green, Blue, Alpha);
			Lines->push_back (pLine);
		}
	}
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (Medians))) {
		GetNumber (MediansR, &r, gcu::Min, 0);
		if (r > 0.0) {
			gtk_color_button_get_color (MediansColor, &color);
			Red = color.red / 65535.;
			Green = color.green / 65535.;
			Blue = color.blue / 65535.;
			Alpha = gtk_color_button_get_alpha (MediansColor) / 65535.;
			pLine = new gcr::Line (gcr::medians, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, r, Red, Green, Blue, Alpha);
			Lines->push_back (pLine);
		}
	}
	//Add new lines from array
	
	struct LineStruct* s;
	for (unsigned i = 0; i  < m_Lines->len; i++) {
		s = &g_array_index (m_Lines, struct LineStruct, i);
		Lines->push_back (new gcr::Line((s->duplicated)? gcr::normal: gcr::unique, s->x1, s->y1, s->z1, s->x2, s->y2, s->z2, s->r, (float)s->Red, (float)s->Green, (float)s->Blue, (float)s->Alpha));
	}
	m_pDoc->Update ();
	m_pDoc->SetDirty (true);
	return true;
}

void LinesDlg::LineAdd ()
{
	GtkTreeIter iter;
	
	struct LineStruct s;
	s.x1 = s.y1 =s.z1 = s.x2 = s.y2 =s.z2 = 0.0;
	GdkColor color;
	gtk_color_button_get_color (LineColor, &color);
	s.Red = color.red / 65535.;
	s.Green = color.green / 65535.;
	s.Blue = color.blue / 65535.;
	s.Alpha = gtk_color_button_get_alpha (LineColor) / 65535.;
	GetNumber (LineR, &s.r);
	g_array_append_vals (m_Lines, &s, 1);
	gtk_list_store_append (LineList, &iter);
	gtk_list_store_set (LineList, &iter,
		      0, 0.0,
		      1, 0.0,
		      2, 0.0,
		      3, 0.0,
		      4, 0.0,
		      5, 0.0,
		      6, false,
		      -1);
	gtk_widget_set_sensitive (DeleteAllBtn, true);
	gtk_tree_selection_select_iter (Selection, &iter);
}

void LinesDlg::LineDelete ()
{
	GtkTreeModel* model = GTK_TREE_MODEL (LineList);

	if (gtk_tree_selection_get_selected (Selection, &model, &m_Iter)) {
		gint i;
		GtkTreePath *path;

		path = gtk_tree_model_get_path (model, &m_Iter);
		i = gtk_tree_path_get_indices (path)[0];
		gtk_list_store_remove (GTK_LIST_STORE (model), &m_Iter);

		g_array_remove_index (m_Lines, i);

		gtk_tree_path_free (path);
    }
	if (!m_Lines->len)
		gtk_widget_set_sensitive (DeleteAllBtn, false);
}

void LinesDlg::LineDeleteAll ()
{
	g_array_free (m_Lines, false);
	m_Lines = g_array_sized_new (FALSE, FALSE, sizeof (struct LineStruct), 1);
	gtk_list_store_clear (LineList);
	gtk_widget_set_sensitive (DeleteAllBtn, false);
}

void LinesDlg::LineSelect (GtkTreeSelection *Selection)
{
	GdkColor color;
	if (m_LineSelected >= 0) {
		gtk_color_button_get_color (LineColor, &color);
		g_array_index(m_Lines, struct LineStruct, m_LineSelected).Red = color.red / 65535.;
		g_array_index(m_Lines, struct LineStruct, m_LineSelected).Green = color.green / 65535.;
		g_array_index(m_Lines, struct LineStruct, m_LineSelected).Blue = color.blue / 65535.;
		g_array_index(m_Lines, struct LineStruct, m_LineSelected).Alpha = gtk_color_button_get_alpha (LineColor) / 65535.;
		if ((!GetNumber (LineR, &g_array_index(m_Lines, struct LineStruct, m_LineSelected).r, gcu::Min, 0)) || (g_array_index (m_Lines, struct LineStruct, m_LineSelected).r == 0.0)) {
		}
	}
	GtkTreeModel* model = GTK_TREE_MODEL (LineList);
	GtkTreePath *path;
	if (gtk_tree_selection_get_selected (Selection, &model, &m_Iter)) {
		gtk_widget_set_sensitive (DeleteBtn, true);
		path = gtk_tree_model_get_path (model, &m_Iter);
		m_LineSelected = gtk_tree_path_get_indices (path)[0];
		color.red = (guint16) (g_array_index(m_Lines, struct LineStruct, m_LineSelected).Red * 65535.);
		color.green = (guint16) (g_array_index(m_Lines, struct LineStruct, m_LineSelected).Green * 65535.);
		color.blue = (guint16) (g_array_index(m_Lines, struct LineStruct, m_LineSelected).Blue * 65535.);
		gtk_color_button_set_color (LineColor, &color);
		gtk_color_button_set_alpha (LineColor, (guint16) (g_array_index(m_Lines, struct LineStruct, m_LineSelected).Alpha * 65535.));
		char *tmp;
		tmp = g_strdup_printf ("%g", g_array_index (m_Lines, struct LineStruct, m_LineSelected).r);
		gtk_entry_set_text (LineR, tmp);
		g_free (tmp);
		gtk_tree_path_free (path);
	} else {
		gtk_widget_set_sensitive (DeleteBtn, false);
		if (!m_Lines->len) gtk_widget_set_sensitive (DeleteAllBtn, false);
		m_LineSelected = -1;
	}
}

void LinesDlg::OnEdited (GtkCellRendererText *cell, const gchar *path_string, const gchar *new_text)
{
	GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
	GtkTreeIter iter;

	gtk_tree_model_get_iter (GTK_TREE_MODEL (LineList), &iter, path);

	long j  = (long) g_object_get_data (G_OBJECT (cell), "column");
	double x = atof (new_text);
	gtk_list_store_set (LineList, &iter, j, x, -1);
	switch (j) {
	case COLUMN_X1:
		g_array_index (m_Lines, struct LineStruct, m_LineSelected).x1 = x;
		break;
	case COLUMN_Y1:
		g_array_index (m_Lines, struct LineStruct, m_LineSelected).y1 = x;
		break;
	case COLUMN_Z1:
		g_array_index (m_Lines, struct LineStruct, m_LineSelected).z1 = x;
		break;
	case COLUMN_X2:
		g_array_index (m_Lines, struct LineStruct, m_LineSelected).x2 = x;
		break;
	case COLUMN_Y2:
		g_array_index (m_Lines, struct LineStruct, m_LineSelected).y2 = x;
		break;
	case COLUMN_Z2:
		g_array_index (m_Lines, struct LineStruct, m_LineSelected).z2 = x;
		break;
	}
	gtk_tree_path_free (path);
}

void LinesDlg::OnToggled (G_GNUC_UNUSED GtkCellRendererToggle *cell, const gchar *path_string)
{
	GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
	GtkTreeIter iter;

	gtk_tree_model_get_iter (GTK_TREE_MODEL(LineList), &iter, path);
	gboolean single;
	gtk_tree_model_get (GTK_TREE_MODEL (LineList), &iter, COLUMN_SINGLE, &single, -1);
	single ^= 1;
	gtk_list_store_set (LineList, &iter, COLUMN_SINGLE, single, -1);
	g_array_index (m_Lines, struct LineStruct, gtk_tree_path_get_indices (path)[0]).duplicated = !single;
	gtk_tree_path_free (path);
}

void LinesDlg::OnToggledSpecial (int Type)
{
	switch (Type) {
	case gcr::edges:
		gtk_widget_set_sensitive (GTK_WIDGET (EdgesColor), gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (Edges)));
		gtk_widget_set_sensitive (GTK_WIDGET (EdgesR), gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (Edges)));
		break;
	case gcr::diagonals:
		gtk_widget_set_sensitive (GTK_WIDGET (DiagsColor), gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (Diags)));
		gtk_widget_set_sensitive (GTK_WIDGET (DiagsR), gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (Diags)));
		break;
	case gcr::medians:
		gtk_widget_set_sensitive (GTK_WIDGET (MediansColor), gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (Medians)));
		gtk_widget_set_sensitive (GTK_WIDGET (MediansR), gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (Medians)));
		break;
	}
}

}	//	namespace gcr