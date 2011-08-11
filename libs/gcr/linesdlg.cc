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
	static bool RadiusEdited (LinesDlg *pBox);
	static void ColorSet (GtkColorButton *btn, LinesDlg *pBox);
};

void LinesDlgPrivate::AddRow (LinesDlg *pBox)
{
	Line *new_line;
	if (pBox->m_LineSelected >= 0)
		new_line = new Line (*pBox->m_Lines[pBox->m_LineSelected]);
	else {
		GdkRGBA rgba;
		gtk_color_button_get_rgba (pBox->LineColor, &rgba);
		double r;
		pBox->GetNumber (pBox->LineR, &r);
		new_line = new Line (normal, 0., 0., 0., 0., 0., 0., r, rgba.red, rgba.green, rgba.blue, rgba.alpha);
	}
	unsigned new_row = gcr_grid_append_row (pBox->m_Grid,
	                                        new_line->X1 (), new_line->Y1 (), new_line->Z1 (),
	                                        new_line->X2 (), new_line->Y2 (), new_line->Z2 (),
	                                        new_line->Type () == unique),
			 max_row = pBox->m_Lines.capacity ();
	if (new_row >= max_row)
		pBox->m_Lines.resize (max_row + 10);
	pBox->m_Lines[new_row] = new_line;
	pBox->m_pDoc->GetLineList ()->push_back (new_line);
	pBox->m_pDoc->Update ();
	pBox->m_pDoc->SetDirty (true);
	gtk_widget_set_sensitive (pBox->DeleteAllBtn, true);
}

void LinesDlgPrivate::DeleteRow (LinesDlg *pBox)
{
	pBox->m_pDoc->GetLineList ()->remove (pBox->m_Lines[pBox->m_LineSelected]);
	delete pBox->m_Lines[pBox->m_LineSelected];
	pBox->m_Lines.erase (pBox->m_Lines.begin () + pBox->m_LineSelected);
	gcr_grid_delete_row (GCR_GRID (pBox->m_Grid), pBox->m_LineSelected);
	pBox->m_pDoc->Update ();
	pBox->m_pDoc->SetDirty (true);
	gtk_widget_set_sensitive (pBox->DeleteAllBtn, !pBox->m_pDoc->GetLineList ()->empty ());
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
	double coord = column != COLUMN_SINGLE? gcr_grid_get_double (pBox->m_Grid, row, column): 0.;
	Line *line = pBox->m_Lines[pBox->m_LineSelected];
	switch (column) {
	case COLUMN_X1:
		line->X1 () = coord;
		break;
	case COLUMN_Y1:
		line->Y1 () = coord;
		break;
	case COLUMN_Z1:
		line->Z1 () = coord;
		break;
	case COLUMN_X2:
		line->X2 () = coord;
		break;
	case COLUMN_Y2:
		line->Y2 () = coord;
		break;
	case COLUMN_Z2:
		line->Z2 () = coord;
		break;
	case COLUMN_SINGLE:
		line->Type () = gcr_grid_get_boolean (pBox->m_Grid, row, column)? unique: normal;
	}
	pBox->m_pDoc->Update ();
	pBox->m_pDoc->SetDirty (true);
}

void LinesDlgPrivate::RowSelected (LinesDlg *pBox, int row)
{
	pBox->m_LineSelected = row;
	gtk_widget_set_sensitive (pBox->DeleteBtn, row >= 0);
	if (row >= 0) {
		g_signal_handler_block (pBox->LineColor, pBox->m_ColorChangedID);
		GdkRGBA rgba;
		pBox->m_Lines[row]->GetColor (rgba);
		gtk_color_button_set_rgba (pBox->LineColor, &rgba);
		g_signal_handler_unblock (pBox->LineColor, pBox->m_ColorChangedID);
		char *buf = g_strdup_printf ("%g", pBox->m_Lines[row]->GetRadius ());
		gtk_entry_set_text (pBox->LineR, buf);
		g_free (buf);
	}
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

bool LinesDlgPrivate::RadiusEdited (LinesDlg *pBox)
{
	g_signal_handler_block (pBox->LineR, pBox->m_LineFocusOutSignalID);
	double r;
	if (pBox->m_LineSelected >= 0 && pBox->GetNumber (pBox->LineR, &r, gcugtk::Min, 0)) {
			pBox->m_Lines[pBox->m_LineSelected]->SetRadius (r);
			pBox->m_pDoc->Update ();
			pBox->m_pDoc->SetDirty (true);
	}
	g_signal_handler_unblock (pBox->LineR, pBox->m_LineFocusOutSignalID);
	return false;
}

void LinesDlgPrivate::ColorSet (GtkColorButton *btn, LinesDlg *pBox)
{
	if (pBox->m_LineSelected >= 0) {
		GdkRGBA rgba;
		gtk_color_button_get_rgba (btn, &rgba);
		pBox->m_Lines[pBox->m_LineSelected]->SetColor (rgba);
		pBox->m_pDoc->Update ();
		pBox->m_pDoc->SetDirty (true);
	}
}

LinesDlg::LinesDlg (Application *App, Document* pDoc): gcugtk::Dialog (App, UIDIR"/lines.ui", "lines", GETTEXT_PACKAGE, pDoc)
{
	m_pDoc = pDoc;
	m_Closing = false;
	GtkWidget* button = GetWidget ("add");
	g_signal_connect_swapped (G_OBJECT (button), "clicked", G_CALLBACK (LinesDlgPrivate::AddRow), this);
	DeleteBtn = GetWidget ("delete");
	gtk_widget_set_sensitive (DeleteBtn,0);
	g_signal_connect_swapped (G_OBJECT (DeleteBtn), "clicked", G_CALLBACK (LinesDlgPrivate::DeleteRow), this);
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
	LineColor = GTK_COLOR_BUTTON (GetWidget ("color"));
	LineR = GTK_ENTRY (GetWidget ("radius"));
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
	if (!Lines->size ())
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
	m_LineFocusOutSignalID = g_signal_connect_swapped (G_OBJECT (LineR), "focus-out-event", G_CALLBACK (LinesDlgPrivate::RadiusEdited), this);
	g_signal_connect_swapped (G_OBJECT (LineR), "activate", G_CALLBACK (LinesDlgPrivate::RadiusEdited), this);
	m_ColorChangedID = g_signal_connect (G_OBJECT (LineColor), "color-set", G_CALLBACK (LinesDlgPrivate::ColorSet), this);

	gtk_widget_show_all (GTK_WIDGET (dialog));
}

LinesDlg::~LinesDlg ()
{
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
