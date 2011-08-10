// -*- C++ -*-

/*
 * Gnome Crystal
 * linesdlg.cc
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

class LinesDlgPrivate {
public:
	static void AddRow (LinesDlg *pBox);
	static void DeleteRow (LinesDlg *pBox);
	static void DeleteAll (LinesDlg *pBox);
	static void ValueChanged (LinesDlg *pBox, unsigned row, unsigned column);
	static void RowSelected (LinesDlg *pBox, int row);
	static void EdgesToggled (GtkToggleButton *btn, LinesDlg *pBox);
	static void DiagonalsToggled (GtkToggleButton *btn, LinesDlg *pBox);
	static void MediansToggled (GtkToggleButton *btn, LinesDlg *pBox);
	static bool EdgesRadiusEdited (LinesDlg *pBox);
	static bool DiagsRadiusEdited (LinesDlg *pBox);
	static bool MediansRadiusEdited (LinesDlg *pBox);
	static void EdgesColorSet (GtkColorButton *btn, LinesDlg *pBox);
	static void DiagsColorSet (GtkColorButton *btn, LinesDlg *pBox);
	static void MediansColorSet (GtkColorButton *btn, LinesDlg *pBox);
};

void LinesDlgPrivate::AddRow (LinesDlg *pBox)
{
}

void LinesDlgPrivate::DeleteRow (LinesDlg *pBox)
{
}

void LinesDlgPrivate::DeleteAll (LinesDlg *pBox)
{
	gcr_grid_delete_all (pBox->m_Grid);
	for (unsigned i = 0; i < pBox->m_Lines.size (); i++)
		delete pBox->m_Lines[i];
	pBox->m_Lines.clear ();
	pBox->m_pDoc->GetLineList ()->clear ();
	pBox->m_pDoc->Update ();
	pBox->m_pDoc->SetDirty (true);
	gtk_widget_set_sensitive (pBox->DeleteAllBtn, false);
}

void LinesDlgPrivate::ValueChanged (LinesDlg *pBox, unsigned row, unsigned column)
{
}

void LinesDlgPrivate::RowSelected (LinesDlg *pBox, int row)
{
}

void LinesDlgPrivate::EdgesToggled (GtkToggleButton *btn, LinesDlg *pBox)
{
	bool active = gtk_toggle_button_get_active (btn);
	gtk_widget_set_sensitive (GTK_WIDGET (pBox->EdgesColor), active);
	gtk_widget_set_sensitive (GTK_WIDGET (pBox->EdgesR), active);
	if (active) {
		GdkRGBA rgba;
		double r;
		gtk_color_button_get_rgba (pBox->EdgesColor, &rgba);
		pBox->GetNumber (pBox->EdgesR, &r, gcugtk::Min, 0);
		pBox->Edges = new Line (edges, 0., 0., 0., 0., 0., 0., r, rgba.red, rgba.green, rgba.blue, rgba.alpha);
		pBox->m_pDoc->GetLineList ()->push_front (pBox->Edges);
	} else {
		pBox->m_pDoc->GetLineList ()->remove (pBox->Edges);
		delete pBox->Edges;
		pBox->Edges = NULL;
	}
	pBox->m_pDoc->Update ();
	pBox->m_pDoc->SetDirty (true);
}

void LinesDlgPrivate::DiagonalsToggled (GtkToggleButton *btn, LinesDlg *pBox)
{
	bool active = gtk_toggle_button_get_active (btn);
	gtk_widget_set_sensitive (GTK_WIDGET (pBox->DiagsColor), active);
	gtk_widget_set_sensitive (GTK_WIDGET (pBox->DiagsR), active);
	if (active) {
		GdkRGBA rgba;
		double r;
		gtk_color_button_get_rgba (pBox->DiagsColor, &rgba);
		pBox->GetNumber (pBox->DiagsR, &r, gcugtk::Min, 0);
		pBox->Diagonals = new Line (diagonals, 0., 0., 0., 0., 0., 0., r, rgba.red, rgba.green, rgba.blue, rgba.alpha);
		pBox->m_pDoc->GetLineList ()->push_front (pBox->Diagonals);
	} else {
		pBox->m_pDoc->GetLineList ()->remove (pBox->Diagonals);
		delete pBox->Diagonals;
		pBox->Diagonals = NULL;
	}
	pBox->m_pDoc->Update ();
	pBox->m_pDoc->SetDirty (true);
}

void LinesDlgPrivate::MediansToggled (GtkToggleButton *btn, LinesDlg *pBox)
{
	bool active = gtk_toggle_button_get_active (btn);
	gtk_widget_set_sensitive (GTK_WIDGET (pBox->MediansColor), active);
	gtk_widget_set_sensitive (GTK_WIDGET (pBox->MediansR), active);
	if (active) {
		GdkRGBA rgba;
		double r;
		gtk_color_button_get_rgba (pBox->MediansColor, &rgba);
		pBox->GetNumber (pBox->MediansR, &r, gcugtk::Min, 0);
		pBox->Medians = new Line (medians, 0., 0., 0., 0., 0., 0., r, rgba.red, rgba.green, rgba.blue, rgba.alpha);
		pBox->m_pDoc->GetLineList ()->push_front (pBox->Medians);
	} else {
		pBox->m_pDoc->GetLineList ()->remove (pBox->Medians);
		delete pBox->Medians;
		pBox->Medians = NULL;
	}
	pBox->m_pDoc->Update ();
	pBox->m_pDoc->SetDirty (true);
}

bool LinesDlgPrivate::EdgesRadiusEdited (LinesDlg *pBox)
{
	g_signal_handler_block (pBox->EdgesR, pBox->m_EdgesFocusOutSignalID);
	double r;
	if (pBox->GetNumber (pBox->EdgesR, &r, gcugtk::Min, 0)) {
		pBox->Edges->SetRadius (r);
		pBox->m_pDoc->Update ();
		pBox->m_pDoc->SetDirty (true);
	}
	g_signal_handler_unblock (pBox->EdgesR, pBox->m_EdgesFocusOutSignalID);
	return false;
}

bool LinesDlgPrivate::DiagsRadiusEdited (LinesDlg *pBox)
{
	g_signal_handler_block (pBox->DiagsR, pBox->m_DiagsFocusOutSignalID);
	double r;
	if (pBox->GetNumber (pBox->DiagsR, &r, gcugtk::Min, 0)) {
		pBox->Diagonals->SetRadius (r);
		pBox->m_pDoc->Update ();
		pBox->m_pDoc->SetDirty (true);
	}
	g_signal_handler_unblock (pBox->DiagsR, pBox->m_DiagsFocusOutSignalID);
	return false;
}

bool LinesDlgPrivate::MediansRadiusEdited (LinesDlg *pBox)
{
	g_signal_handler_block (pBox->MediansR, pBox->m_MediansFocusOutSignalID);
	double r;
	if (pBox->GetNumber (pBox->MediansR, &r, gcugtk::Min, 0)) {
		pBox->Medians->SetRadius (r);
		pBox->m_pDoc->Update ();
		pBox->m_pDoc->SetDirty (true);
	}
	g_signal_handler_unblock (pBox->MediansR, pBox->m_MediansFocusOutSignalID);
	return false;
}

void LinesDlgPrivate::EdgesColorSet (GtkColorButton *btn, LinesDlg *pBox)
{
	GdkRGBA rgba;
	gtk_color_button_get_rgba (btn, &rgba);
	pBox->Edges->SetColor (rgba);
	pBox->m_pDoc->Update ();
	pBox->m_pDoc->SetDirty (true);
}

void LinesDlgPrivate::DiagsColorSet (GtkColorButton *btn, LinesDlg *pBox)
{
	GdkRGBA rgba;
	gtk_color_button_get_rgba (btn, &rgba);
	pBox->Diagonals->SetColor (rgba);
	pBox->m_pDoc->Update ();
	pBox->m_pDoc->SetDirty (true);
}

void LinesDlgPrivate::MediansColorSet (GtkColorButton *btn, LinesDlg *pBox)
{
	GdkRGBA rgba;
	gtk_color_button_get_rgba (btn, &rgba);
	pBox->Medians->SetColor (rgba);
	pBox->m_pDoc->Update ();
	pBox->m_pDoc->SetDirty (true);
}

#if 0
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
#endif

LinesDlg::LinesDlg (Application *App, Document* pDoc): gcugtk::Dialog (App, UIDIR"/lines.ui", "lines", GETTEXT_PACKAGE, pDoc)
{
	m_pDoc = pDoc;
	m_Closing = false;
	GtkWidget* button = GetWidget ("add");
	g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (LinesDlgPrivate::AddRow), this);
	DeleteBtn = GetWidget ("delete");
	gtk_widget_set_sensitive (DeleteBtn,0);
	g_signal_connect (G_OBJECT (DeleteBtn), "clicked", G_CALLBACK (LinesDlgPrivate::DeleteRow), this);
	DeleteAllBtn = GetWidget ("delete-all");
	g_signal_connect_swapped (G_OBJECT (DeleteAllBtn), "clicked", G_CALLBACK (LinesDlgPrivate::DeleteAll), this);
	m_Grid = GCR_GRID (gcr_grid_new (_("x1"), G_TYPE_DOUBLE, _("y1"), G_TYPE_DOUBLE, _("z1"), G_TYPE_DOUBLE,
	                                 _("x2"), G_TYPE_DOUBLE, _("y2"), G_TYPE_DOUBLE, _("z2"), G_TYPE_DOUBLE,
	                                 _("Single"), G_TYPE_BOOLEAN, NULL));
	g_object_set (G_OBJECT (m_Grid), "expand", true, NULL);
	gtk_grid_attach (GTK_GRID (GetWidget ("other-grid")), GTK_WIDGET (m_Grid), 0, 1, 4, 5);
	g_signal_connect_swapped (G_OBJECT (m_Grid), "row-selected", G_CALLBACK (LinesDlgPrivate::RowSelected), this);
	g_signal_connect_swapped (G_OBJECT (m_Grid), "value-changed", G_CALLBACK (LinesDlgPrivate::ValueChanged), this);
	EdgesBtn = GTK_CHECK_BUTTON (GetWidget ("edges"));
	EdgesColor = GTK_COLOR_BUTTON (GetWidget ("edges-color"));
	gtk_widget_set_sensitive (GTK_WIDGET (EdgesColor), false);
	EdgesR = GTK_ENTRY (GetWidget ("edges-radius"));
	gtk_entry_set_text (EdgesR, "5");
	gtk_widget_set_sensitive (GTK_WIDGET (EdgesR), false);
	MediansBtn = GTK_CHECK_BUTTON (GetWidget ("medians"));
	MediansColor = GTK_COLOR_BUTTON (GetWidget ("med-color"));
	gtk_widget_set_sensitive (GTK_WIDGET (MediansColor), false);
	MediansR = GTK_ENTRY (GetWidget ("med-radius"));
	gtk_entry_set_text (MediansR, "5");
	gtk_widget_set_sensitive (GTK_WIDGET (MediansR), false);
	DiagsBtn = GTK_CHECK_BUTTON (GetWidget ("diagonals"));
	DiagsColor = GTK_COLOR_BUTTON (GetWidget ("diag-color"));
	gtk_widget_set_sensitive (GTK_WIDGET (DiagsColor), false);
	DiagsR = GTK_ENTRY (GetWidget ("diag-radius"));
	gtk_entry_set_text (DiagsR, "5");
	gtk_widget_set_sensitive (GTK_WIDGET (DiagsR), false);
	m_LineSelected = -1;
	Edges = Diagonals = Medians = NULL;
	gcr::LineList* Lines = m_pDoc->GetLineList ();
	m_Lines.resize ((Lines->size () / 10 + 1) * 10);
	list < gcr::Line * >::iterator i, end = Lines->end ();
	GdkRGBA rgba;
	char *buf;
	for (i = Lines->begin (); i != end; i++) {
		switch ((*i)->Type()) {
		case edges:
			gtk_widget_set_sensitive (GTK_WIDGET (EdgesColor), true);
			gtk_widget_set_sensitive (GTK_WIDGET (EdgesR), true);
			buf = g_strdup_printf ("%g", (*i)->GetRadius ());
			gtk_entry_set_text (EdgesR, buf);
			g_free (buf);
			(*i)->GetColor (rgba);
			gtk_color_button_set_rgba (EdgesColor, &rgba);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (EdgesBtn), true);
			Edges = *i;
			break;
		case diagonals:
			gtk_widget_set_sensitive (GTK_WIDGET (DiagsColor), true);
			gtk_widget_set_sensitive (GTK_WIDGET (DiagsR), true);
			buf = g_strdup_printf ("%g", (*i)->GetRadius ());
			gtk_entry_set_text (DiagsR, buf);
			g_free (buf);
			(*i)->GetColor (rgba);
			gtk_color_button_set_rgba (DiagsColor, &rgba);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (DiagsBtn), true);
			Diagonals = *i;
			break;
		case medians:
			gtk_widget_set_sensitive (GTK_WIDGET (MediansColor), true);
			gtk_widget_set_sensitive (GTK_WIDGET (MediansR), true);
			buf = g_strdup_printf ("%g", (*i)->GetRadius ());
			gtk_entry_set_text (MediansR, buf);
			g_free (buf);
			(*i)->GetColor (rgba);
			gtk_color_button_set_rgba (MediansColor, &rgba);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (MediansBtn), true);
			Medians = *i;
			break;
		case normal:
			m_Lines[gcr_grid_append_row (m_Grid, (*i)->X1 (), (*i)->Y1 (), (*i)->Z1 (),
				                         (*i)->X2 (), (*i)->Y2 (), (*i)->Z2 (), false)] = *i;
			break;
		case unique:
			m_Lines[gcr_grid_append_row (m_Grid, (*i)->X1 (), (*i)->Y1 (), (*i)->Z1 (),
				                         (*i)->X2 (), (*i)->Y2 (), (*i)->Z2 (), true)] = *i;
			break;
		}
	}
	if (!m_Lines.size ())
		gtk_widget_set_sensitive (DeleteAllBtn, false);
	g_signal_connect (G_OBJECT (EdgesBtn), "toggled", G_CALLBACK (LinesDlgPrivate::EdgesToggled), this);
	g_signal_connect (G_OBJECT (MediansBtn), "toggled", G_CALLBACK (LinesDlgPrivate::MediansToggled), this);
	g_signal_connect (G_OBJECT (DiagsBtn), "toggled", G_CALLBACK (LinesDlgPrivate::DiagonalsToggled), this);
	g_signal_connect_swapped (G_OBJECT (EdgesR), "activate", G_CALLBACK (LinesDlgPrivate::EdgesRadiusEdited), this);
	m_EdgesFocusOutSignalID = g_signal_connect_swapped (G_OBJECT (EdgesR), "focus-out-event", G_CALLBACK (LinesDlgPrivate::EdgesRadiusEdited), this);
	g_signal_connect_swapped (G_OBJECT (DiagsR), "activate", G_CALLBACK (LinesDlgPrivate::DiagsRadiusEdited), this);
	m_DiagsFocusOutSignalID = g_signal_connect_swapped (G_OBJECT (DiagsR), "focus-out-event", G_CALLBACK (LinesDlgPrivate::DiagsRadiusEdited), this);
	g_signal_connect_swapped (G_OBJECT (MediansR), "activate", G_CALLBACK (LinesDlgPrivate::MediansRadiusEdited), this);
	m_MediansFocusOutSignalID = g_signal_connect_swapped (G_OBJECT (MediansR), "focus-out-event", G_CALLBACK (LinesDlgPrivate::MediansRadiusEdited), this);
	g_signal_connect (G_OBJECT (EdgesColor), "color-set", G_CALLBACK (LinesDlgPrivate::EdgesColorSet), this);
	g_signal_connect (G_OBJECT (DiagsColor), "color-set", G_CALLBACK (LinesDlgPrivate::DiagsColorSet), this);
	g_signal_connect (G_OBJECT (MediansColor), "color-set", G_CALLBACK (LinesDlgPrivate::MediansColorSet), this);

	gtk_widget_show_all (GTK_WIDGET (dialog));
}

LinesDlg::~LinesDlg ()
{
/*	if (m_Lines)
		g_array_free (m_Lines, false);*/
}

bool LinesDlg::Apply ()
{
/*	GdkColor color;
	if (m_LineSelected >= 0) {
		gtk_color_button_get_color (LineColor, &color);
		g_array_index(m_Lines, struct LineStruct, m_LineSelected).Red = color.red / 65535.;
		g_array_index(m_Lines, struct LineStruct, m_LineSelected).Green = color.green / 65535.;
		g_array_index(m_Lines, struct LineStruct, m_LineSelected).Blue = color.blue / 65535.;
		g_array_index(m_Lines, struct LineStruct, m_LineSelected).Alpha = gtk_color_button_get_alpha (LineColor) / 65535.;
		if ((!GetNumber(LineR, &g_array_index (m_Lines, struct LineStruct, m_LineSelected).r, gcugtk::Min, 0)) || (g_array_index (m_Lines, struct LineStruct, m_LineSelected).r == 0.0)) {
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
		GetNumber (EdgesR, &r, gcugtk::Min, 0);
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
		GetNumber (DiagsR, &r, gcugtk::Min, 0);
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
		GetNumber (MediansR, &r, gcugtk::Min, 0);
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
	m_pDoc->SetDirty (true);*/
	return true;
}

void LinesDlg::LineAdd ()
{
	/*GtkTreeIter iter;

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
	gtk_tree_selection_select_iter (Selection, &iter);*/
}

void LinesDlg::LineDelete ()
{
/*	GtkTreeModel* model = GTK_TREE_MODEL (LineList);

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
		gtk_widget_set_sensitive (DeleteAllBtn, false);*/
}

void LinesDlg::LineDeleteAll ()
{
/*	g_array_free (m_Lines, false);
	m_Lines = g_array_sized_new (FALSE, FALSE, sizeof (struct LineStruct), 1);
	gtk_list_store_clear (LineList);
	gtk_widget_set_sensitive (DeleteAllBtn, false);*/
}

void LinesDlg::LineSelect (GtkTreeSelection *Selection)
{
/*	GdkColor color;
	if (m_LineSelected >= 0) {
		gtk_color_button_get_color (LineColor, &color);
		g_array_index(m_Lines, struct LineStruct, m_LineSelected).Red = color.red / 65535.;
		g_array_index(m_Lines, struct LineStruct, m_LineSelected).Green = color.green / 65535.;
		g_array_index(m_Lines, struct LineStruct, m_LineSelected).Blue = color.blue / 65535.;
		g_array_index(m_Lines, struct LineStruct, m_LineSelected).Alpha = gtk_color_button_get_alpha (LineColor) / 65535.;
		if ((!GetNumber (LineR, &g_array_index(m_Lines, struct LineStruct, m_LineSelected).r, gcugtk::Min, 0)) || (g_array_index (m_Lines, struct LineStruct, m_LineSelected).r == 0.0)) {
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
	}*/
}

void LinesDlg::OnEdited (GtkCellRendererText *cell, const gchar *path_string, const gchar *new_text)
{
/*	GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
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
	gtk_tree_path_free (path);*/
}

void LinesDlg::OnToggled (G_GNUC_UNUSED GtkCellRendererToggle *cell, const gchar *path_string)
{
/*	GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
	GtkTreeIter iter;

	gtk_tree_model_get_iter (GTK_TREE_MODEL(LineList), &iter, path);
	gboolean single;
	gtk_tree_model_get (GTK_TREE_MODEL (LineList), &iter, COLUMN_SINGLE, &single, -1);
	single ^= 1;
	gtk_list_store_set (LineList, &iter, COLUMN_SINGLE, single, -1);
	g_array_index (m_Lines, struct LineStruct, gtk_tree_path_get_indices (path)[0]).duplicated = !single;
	gtk_tree_path_free (path);*/
}

void LinesDlg::OnToggledSpecial (int Type)
{
/*	switch (Type) {
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
	}*/
}

void LinesDlg::Closed ()
{
	// check if the cleavages list is coherent
	m_Closing = true;
	m_pDoc->CheckLines ();
}

void LinesDlg::ReloadData ()
{
	if (m_Closing)
		return;
	gcr_grid_delete_all (GCR_GRID (m_Grid));
	m_Lines.clear ();
	gcr::LineList* Lines = m_pDoc->GetLineList ();
	for (list < Line * >::iterator i = Lines->begin (); i != Lines->end (); i++)
		m_Lines[gcr_grid_append_row (GCR_GRID (m_Grid)/*FIXME*/)] = *i;
	if (!m_Lines.size ())
		gtk_widget_set_sensitive (DeleteAllBtn, false);
}

}	//	namespace gcr
