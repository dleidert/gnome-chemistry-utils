// -*- C++ -*-

/* 
 * Gnome Crystal
 * atomsdlg.cc 
 *
 * Copyright (C) 2002-2007 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "atom.h"
#include "atomsdlg.h"
#include "document.h"
#include "application.h"
#include <gcu/element.h>
#include <list>
#include <string>
#include <glib/gi18n.h>

using namespace std;
using namespace gcu;

enum
{
	COLUMN_ELT,
	COLUMN_X,
	COLUMN_Y,
	COLUMN_Z
};

struct AtomStruct {
	unsigned short Elt;
	double x, y, z;
	GcuAtomicRadius Radius;
	bool CustomColor;
	double Blue, Red, Green, Alpha;
};

static void on_add (GtkWidget *widget, gcAtomsDlg *pBox)
{
	pBox->AtomAdd ();
}

static void on_delete (GtkWidget *widget, gcAtomsDlg *pBox)
{
	pBox->AtomDelete ();
}

static void on_delete_all (GtkWidget *widget, gcAtomsDlg *pBox)
{
	pBox->AtomDeleteAll ();
}

static void on_select (GtkTreeSelection *Selection, gcAtomsDlg *pBox)
{
	pBox->AtomSelect (Selection);
}

static void on_element (GtkWidget *widget, guint Z, gcAtomsDlg *pBox)
{
	pBox->OnElement (Z);
}

static void on_edited (GtkCellRendererText *cell, const gchar *path_string, const gchar *new_text, gcAtomsDlg *pBox)
{
	pBox->OnEdited (cell, path_string, new_text);
}

static void on_toggled_color (GtkToggleButton *button, gcAtomsDlg *pBox)
{
	pBox->SetCustomColor (gtk_toggle_button_get_active (button));
}

static void on_radius_type_changed (GtkComboBox *menu, gcAtomsDlg *pBox)
{
	pBox->SetRadiusType (gtk_combo_box_get_active (menu));
}

static void on_radius_index_changed(GtkComboBox *menu, gcAtomsDlg *pBox)
{
	pBox->SetRadiusIndex (gtk_combo_box_get_active (menu));
}

static void on_charge_changed (GtkSpinButton *btn, gcAtomsDlg *pBox)
{
	pBox->SetCharge (gtk_spin_button_get_value_as_int (btn));
}

gcAtomsDlg::gcAtomsDlg (gcApplication *App, gcDocument* pDoc): Dialog (App, GLADEDIR"/atoms.glade", "atoms", pDoc)
{
	if (!xml) {
		m_Atoms = NULL;
		delete this;
		return;
	}
	m_pDoc = pDoc;
	GtkWidget *frame = glade_xml_get_widget (xml, "mendeleiev");
	periodic = (GtkPeriodic*) gtk_periodic_new ();
	g_signal_connect (G_OBJECT (periodic), "element_changed", G_CALLBACK (on_element), this);
	g_object_set (G_OBJECT (periodic), "can_unselect", TRUE, "color-style", GTK_PERIODIC_COLOR_DEFAULT, NULL);
	gtk_container_add (GTK_CONTAINER (frame), (GtkWidget *) periodic);
	gtk_widget_show_all (frame);
	GtkWidget *button = glade_xml_get_widget (xml, "add");
	g_signal_connect (G_OBJECT (button), "clicked", GTK_SIGNAL_FUNC (on_add), this);
	DeleteBtn = glade_xml_get_widget (xml, "delete");
	gtk_widget_set_sensitive (DeleteBtn,0);
	g_signal_connect (G_OBJECT (DeleteBtn), "clicked", G_CALLBACK (on_delete), this);
	DeleteAllBtn = glade_xml_get_widget (xml, "delete_all");
	g_signal_connect (G_OBJECT (DeleteAllBtn), "clicked", GTK_SIGNAL_FUNC (on_delete_all), this);
	AtomList = gtk_list_store_new (4, G_TYPE_STRING, G_TYPE_DOUBLE, G_TYPE_DOUBLE, G_TYPE_DOUBLE);
	GtkTreeView *tree = (GtkTreeView *) glade_xml_get_widget (xml, "atomslist");
	Selection = gtk_tree_view_get_selection (tree);
	gtk_tree_view_set_model (tree, GTK_TREE_MODEL (AtomList));
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	/* column for element */
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Atom"), renderer, "text", 0, NULL);
	/* set this column to a minimum sizing (of 80 pixels) */
	gtk_tree_view_column_set_sizing(GTK_TREE_VIEW_COLUMN (column), GTK_TREE_VIEW_COLUMN_GROW_ONLY);
	gtk_tree_view_column_set_min_width(GTK_TREE_VIEW_COLUMN (column), 80);
	gtk_tree_view_append_column (tree, column);
	/* column for x */
	renderer = gtk_cell_renderer_text_new ();
	g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (on_edited), this);
	g_object_set (G_OBJECT (renderer), "editable", true, NULL);
	g_object_set_data (G_OBJECT (renderer), "column", (gint *) COLUMN_X);
	column = gtk_tree_view_column_new_with_attributes (_("x"), renderer, "text", 1, NULL);
	/* set this column to a fixed sizing (of 70 pixels) */
	gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column), GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 70);
	gtk_tree_view_append_column (tree, column);
	/* column for y */
	renderer = gtk_cell_renderer_text_new ();
	g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (on_edited), this);
	g_object_set (G_OBJECT (renderer), "editable", true, NULL);
	g_object_set_data (G_OBJECT (renderer), "column", (gint *) COLUMN_Y);
	column = gtk_tree_view_column_new_with_attributes (_("y"), renderer, "text", 2, NULL);
	/* set this column to a fixed sizing (of 70 pixels) */
	gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column), GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 70);
	gtk_tree_view_append_column (tree, column);
	/* column for z */
	renderer = gtk_cell_renderer_text_new ();
	g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (on_edited), this);
	g_object_set (G_OBJECT (renderer), "editable", true, NULL);
	g_object_set_data (G_OBJECT (renderer), "column", (gint *) COLUMN_Z);
	column = gtk_tree_view_column_new_with_attributes (_("z"), renderer, "text", 3, NULL);
	/* set this column to a fixed sizing (of 70 pixels) */
	gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column), GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 70);
	gtk_tree_view_append_column (tree, column);
	m_nElt = 0;
	m_AtomSelected = -1;
	m_Atoms = g_array_sized_new (FALSE, FALSE, sizeof (struct AtomStruct), 1);
	CrystalAtomList* Atoms = m_pDoc->GetAtomList ();
	CrystalAtom* pAtom;
	struct AtomStruct s;
	GtkTreeIter iter;
	for (list<CrystalAtom*>::iterator i = Atoms->begin (); i != Atoms->end (); i++) {
		pAtom = *i;
		s.Elt = pAtom->GetZ ();
		s.x = pAtom->x ();
		s.y = pAtom->y ();
		s.z = pAtom->z();
		s.Radius = pAtom->GetRadius ();
		if ((s.CustomColor = pAtom->HasCustomColor ()))
			pAtom->GetColor (&s.Red, &s.Green, &s.Blue, &s.Alpha);
		g_array_append_vals (m_Atoms, &s, 1);
		gtk_list_store_append (AtomList, &iter);
		gtk_list_store_set (AtomList, &iter,
				  0, (s.Elt)? Element::Symbol (s.Elt): _("Unknown"),
				  1, s.x,
				  2, s.y,
				  3, s.z,
				  -1);
	}
	if (!m_Atoms->len)
		gtk_widget_set_sensitive (DeleteAllBtn, false);
	AtomColor = (GtkColorButton *) glade_xml_get_widget (xml, "color");
	CustomColor = (GtkToggleButton *) glade_xml_get_widget (xml, "custom_color");
	gtk_toggle_button_set_active (CustomColor, true);
	g_signal_connect (G_OBJECT (CustomColor), "toggled", G_CALLBACK (on_toggled_color), this);
	ChargeBtn = (GtkSpinButton *) glade_xml_get_widget (xml, "charge");
	g_signal_connect (G_OBJECT (ChargeBtn), "value-changed", G_CALLBACK (on_charge_changed), this);
	RadiusTypeMenu = (GtkComboBox*) glade_xml_get_widget (xml, "radius-type");
	gtk_combo_box_set_active (RadiusTypeMenu, 0);
	g_signal_connect (G_OBJECT (RadiusTypeMenu), "changed", G_CALLBACK (on_radius_type_changed), this);
	RadiusMenu = (GtkComboBox*) glade_xml_get_widget (xml, "radius-menu");
	m_RadiiSignalID = g_signal_connect (G_OBJECT (RadiusMenu), "changed", G_CALLBACK (on_radius_index_changed), this);
	AtomR = (GtkEntry*) glade_xml_get_widget (xml, "atomr");
	g_signal_connect (G_OBJECT (Selection), "changed", GTK_SIGNAL_FUNC (on_select), this);
	m_RadiusType = m_Charge = 0;
	m_Radii = NULL;
	m_Radius.type = GCU_RADIUS_UNKNOWN;
	m_Radius.cn = -1;
	m_Radius.spin = GCU_N_A_SPIN;
	m_Radius.value.value = 0.;
	m_Radius.scale = "custom";
	PopulateRadiiMenu ();
}

gcAtomsDlg::~gcAtomsDlg ()
{
	if (m_Atoms)
		g_array_free (m_Atoms, false);
}

bool gcAtomsDlg::Apply ()
{
	GdkColor color;
	if (m_AtomSelected >= 0) {
		if (gtk_toggle_button_get_active (CustomColor)) {
			g_array_index (m_Atoms, struct AtomStruct, m_AtomSelected).CustomColor = true;
			gtk_color_button_get_color (AtomColor, &color);
			g_array_index (m_Atoms, struct AtomStruct, m_AtomSelected).Red = color.red / 65535.;
			g_array_index (m_Atoms, struct AtomStruct, m_AtomSelected).Green = color.green / 65535.;
			g_array_index (m_Atoms, struct AtomStruct, m_AtomSelected).Blue = color.blue / 65535.;
			g_array_index (m_Atoms, struct AtomStruct, m_AtomSelected).Alpha = gtk_color_button_get_alpha (AtomColor) / 65535.;
		} else
			g_array_index (m_Atoms, struct AtomStruct, m_AtomSelected).CustomColor = false;
		if ((!GetNumber (AtomR, &(m_Radius.value.value), Min, 0)) || (m_Radius.value.value == 0.0)) {
		} else
			g_array_index (m_Atoms, struct AtomStruct, m_AtomSelected).Radius = m_Radius;
	}
	CrystalAtomList* Atoms = m_pDoc->GetAtomList ();
	//First, delete old atoms
	while (!Atoms->empty ()) {
		delete Atoms->front ();
		Atoms->pop_front ();
	}
	//Add new atoms from array
	
	struct AtomStruct* s;
	gcAtom* pAtom;
	for (unsigned i = 0; i  < m_Atoms->len; i++) {
		s = &g_array_index (m_Atoms, struct AtomStruct, i);
		pAtom = new gcAtom (s->Elt, s->x, s->y, s->z);
		pAtom->SetRadius (s->Radius);
		if (s->CustomColor)
			pAtom->SetColor ((float) s->Red, (float) s->Green, (float) s->Blue, (float) s->Alpha);
		Atoms->push_back (pAtom);
	}
	m_pDoc->Update ();
	m_pDoc->SetDirty (true);
	return true;
}

void gcAtomsDlg::AtomAdd ()
{
	GtkTreeIter iter;
	GdkColor color;
	struct AtomStruct s;
	s.Elt = m_nElt;
	s.x = s.y =s.z = 0.0;
	gtk_color_button_get_color (AtomColor, &color);
	s.Red = color.red / 65535.;
	s.Green = color.green / 65535.;
	s.Blue = color.blue / 65535.;
	s.Alpha = gtk_color_button_get_alpha (AtomColor) / 65535.;
	s.CustomColor = gtk_toggle_button_get_active (CustomColor);
	GetNumber (AtomR, &m_Radius.value.value);
	s.Radius = m_Radius;
	g_array_append_vals (m_Atoms, &s, 1);
	gtk_list_store_append (AtomList, &iter);
	gtk_list_store_set (AtomList, &iter,
		      0, (m_nElt)? Element::Symbol (m_nElt): _("Unknown"),
		      1, 0.0,
		      2, 0.0,
		      3, 0.0,
		      -1);
	gtk_widget_set_sensitive (DeleteAllBtn, true);
	gtk_tree_selection_select_iter (Selection, &iter);
}

void gcAtomsDlg::AtomDelete ()
{
	GtkTreeModel* model = GTK_TREE_MODEL (AtomList);

	if(gtk_tree_selection_get_selected (Selection, &model, &m_Iter)) {
		gint i;
		GtkTreePath *path;

		path = gtk_tree_model_get_path (model, &m_Iter);
		i = gtk_tree_path_get_indices (path)[0];
		gtk_list_store_remove (GTK_LIST_STORE (model), &m_Iter);

		g_array_remove_index (m_Atoms, i);

		gtk_tree_path_free (path);
    }
	if (!m_Atoms->len)
		gtk_widget_set_sensitive (DeleteAllBtn, false);
}

void gcAtomsDlg::AtomDeleteAll ()
{
	g_array_free (m_Atoms, false);
	m_Atoms = g_array_sized_new (FALSE, FALSE, sizeof (struct AtomStruct), 1);
	gtk_list_store_clear (AtomList);
	gtk_widget_set_sensitive (DeleteAllBtn, false);
}

void gcAtomsDlg::AtomSelect(GtkTreeSelection *Selection)
{
	//First, store color and radius of selected element if necessary
	if (m_AtomSelected >= 0) {
		if (gtk_toggle_button_get_active (CustomColor)) {
			GdkColor color;
			g_array_index (m_Atoms, struct AtomStruct, m_AtomSelected).CustomColor = true;
			gtk_color_button_get_color (AtomColor, &color);
			g_array_index (m_Atoms, struct AtomStruct, m_AtomSelected).Red = color.red / 65535.;
			g_array_index (m_Atoms, struct AtomStruct, m_AtomSelected).Green = color.green / 65535.;
			g_array_index (m_Atoms, struct AtomStruct, m_AtomSelected).Blue = color.blue / 65535.;
			g_array_index (m_Atoms, struct AtomStruct, m_AtomSelected).Alpha = gtk_color_button_get_alpha (AtomColor) / 65535.;
		} else
			g_array_index (m_Atoms, struct AtomStruct, m_AtomSelected).CustomColor = false;
		if ((!GetNumber (AtomR, &(m_Radius.value.value), Min, 0)) || (m_Radius.value.value == 0.0)) {
		} else
			g_array_index(m_Atoms, struct AtomStruct, m_AtomSelected).Radius = m_Radius;
	}
	GtkTreeModel* model = GTK_TREE_MODEL (AtomList);
	GtkTreePath *path;
	if (gtk_tree_selection_get_selected (Selection, &model, &m_Iter)) {
		gtk_widget_set_sensitive (DeleteBtn, true);
		path = gtk_tree_model_get_path (model, &m_Iter);
		m_AtomSelected = gtk_tree_path_get_indices (path)[0];
		gtk_periodic_set_element (periodic, g_array_index (m_Atoms, struct AtomStruct, m_AtomSelected).Elt);
		if (g_array_index (m_Atoms, struct AtomStruct, m_AtomSelected).CustomColor) {
			gtk_toggle_button_set_active (CustomColor, true);
			GdkColor color;
			color.pixel = 0;
			color.red = (guint) (g_array_index(m_Atoms, struct AtomStruct, m_AtomSelected).Red * 65535.);
			color.green = (guint) (g_array_index(m_Atoms, struct AtomStruct, m_AtomSelected).Green * 65535.);
			color.blue = (guint) (g_array_index(m_Atoms, struct AtomStruct, m_AtomSelected).Blue * 65535.);
			gtk_color_button_set_color (AtomColor, &color);
			gtk_color_button_set_alpha (AtomColor, (guint) (g_array_index(m_Atoms, struct AtomStruct, m_AtomSelected).Alpha * 65535.));
		} else {
			gtk_toggle_button_set_active (CustomColor, false);
		}
		GcuAtomicRadius  r= g_array_index (m_Atoms, struct AtomStruct, m_AtomSelected).Radius;
		gtk_spin_button_set_value (ChargeBtn, r.charge);
		gtk_combo_box_set_active (RadiusTypeMenu, (r.type == GCU_RADIUS_UNKNOWN)? 0: r.type - 1);		
		if (r.scale && !strcmp (r.scale, "custom")) {
			m_Radius = r;
			char buf[20];
			g_snprintf (buf, sizeof (buf), "%g", m_Radius.value.value);
			gtk_entry_set_text (AtomR, buf);
			gtk_combo_box_set_active (RadiusMenu, 0);
		} else for (unsigned i = 1; i < m_RadiiIndex.size (); i++) {
			int j = m_RadiiIndex[i];
			if (r.type != m_Radii[j]->type)
				continue;
			if (r.charge != m_Radii[j]->charge)
				continue;
			if (r.cn != m_Radii[j]->cn)
				continue;
			if (r.spin !=m_Radii[j]->spin)
				continue;
			if ((((!r.scale) || (!m_Radii[j]->scale)) && (r.scale != m_Radii[j]->scale))  ||
				(r.scale && m_Radii[j]->scale && strcmp(r.scale, m_Radii[j]->scale)))
			continue;
			gtk_combo_box_set_active (RadiusMenu, i);
		}
		gtk_tree_path_free (path);
	} else {
		gtk_widget_set_sensitive (DeleteBtn, false);
		if (!m_Atoms->len)
			gtk_widget_set_sensitive (DeleteAllBtn, false);
		m_AtomSelected = -1;
	}
}

void gcAtomsDlg::OnElement (guint Z)
{
	m_nElt = (unsigned short) Z;
	if (m_nElt)
	{
		m_Radii = Element::GetElement (m_nElt)->GetRadii ();
		if ((m_RadiusType == GCU_IONIC) && (m_Charge == 0)) {
			m_RadiusType = GCU_RADIUS_UNKNOWN;
			SetRadiusType(GCU_IONIC - 1);
		}
		PopulateRadiiMenu ();
		gtk_toggle_button_set_active (CustomColor, false);
		double *Colors = Element::GetElement (m_nElt)->GetDefaultColor ();
		GdkColor color;
		color.red = (guint16) (Colors[0] * 65535.);
		color.green = (guint16) (Colors[1] * 65535.);
		color.blue = (guint16) (Colors[2] * 65535.);
		gtk_color_button_set_color (AtomColor, &color);
		gtk_color_button_set_alpha (AtomColor, 65535);
	} else {
		m_Radii = NULL;
		gtk_toggle_button_set_active (CustomColor, true);
	}
	if (m_AtomSelected >= 0) {
		g_array_index (m_Atoms, struct AtomStruct, m_AtomSelected).Elt = Z;
		GtkTreeModel *model = GTK_TREE_MODEL (AtomList);
		gtk_list_store_set (GTK_LIST_STORE (model), &m_Iter, 0, (m_nElt)? Element::Symbol (m_nElt): _("Unknown"), -1);
	}
}

void gcAtomsDlg::OnEdited (GtkCellRendererText *cell, const gchar *path_string, const gchar *new_text)
{
	GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
	GtkTreeIter iter;

	gtk_tree_model_get_iter (GTK_TREE_MODEL (AtomList), &iter, path);

	long j  = (long) g_object_get_data (G_OBJECT (cell), "column");
	double x = atof(new_text);
	gtk_list_store_set (AtomList, &iter, j, x, -1);
	switch (j) {
	case COLUMN_X:
		g_array_index (m_Atoms, struct AtomStruct, m_AtomSelected).x = x;
		break;
	case COLUMN_Y:
		g_array_index (m_Atoms, struct AtomStruct, m_AtomSelected).y = x;
		break;
	case COLUMN_Z:
		g_array_index (m_Atoms, struct AtomStruct, m_AtomSelected).z = x;
		break;
	}
	gtk_tree_path_free (path);
}

void gcAtomsDlg::SetCustomColor (bool custom)
{
	gtk_widget_set_sensitive (GTK_WIDGET(AtomColor), custom);
}

void gcAtomsDlg::SetRadiusType (int type)
{
	if (type)
		type++; //jump GCU_ATOMIC
	if (type == m_RadiusType)
		return;
	int charges[17];
	memset (charges, 0, 17 * sizeof (int));
	m_RadiusType = type;
	if  ((type == GCU_IONIC) && m_Radii) {
		if (m_Charge)
			return;
		const GcuAtomicRadius **radius = m_Radii;
		while (*radius)
		{
			if ((*radius)->type != GCU_IONIC) {
				radius++;
				continue;
			}
			if (((*radius)->charge < -8) || ((*radius)->charge > 8)) {
				radius ++;
				continue;
			}
			charges[(*radius)->charge + 8]++;
			radius++;
		}
		int max = 0;
		m_Charge = 8;
		for (int i = 0; i < 17; i++) {
			if (charges[i] > max) {
				m_Charge = i - 8;
				max = charges[i];
			} else if (charges[i] == max) {
				/*FIXME: This is the most problematic case. The policy here is
				to take the smallest charge and if the two charges have the same absolute
				value, take the negative one. We might have something better in gchemutils.*/
				if ((abs(i - 8) < abs(m_Charge)) || i < 8) {
					m_Charge = i - 8;
					max = charges[i];
				}
			}
		}
	} else
		m_Charge = 0;
	gtk_spin_button_set_value (ChargeBtn, m_Charge);
	PopulateRadiiMenu ();
}

void gcAtomsDlg::SetRadiusIndex(int index)
{
	int i = m_RadiiIndex[index];
	gtk_widget_set_sensitive(GTK_WIDGET(AtomR), i < 0);
	if (i >= 0)
	{
		m_Radius = *(m_Radii[i]);
		char buf[20];
		g_snprintf (buf, sizeof (buf), "%g", m_Radius.value.value);
		gtk_entry_set_text(AtomR, buf);
	}
	else
	{
		m_Radius.scale = "custom";
		m_Radius.spin = GCU_N_A_SPIN;
		m_Radius.charge = m_Charge;
		m_Radius.cn = -1;
		m_Radius.type = (gcu_radius_type)m_RadiusType;
	}
}

void gcAtomsDlg::SetCharge (int charge)
{
	if (m_Charge == charge)
		return;
	int index = -1;
	m_Charge = charge;
	if (charge) {
		m_RadiusType = GCU_IONIC;
		index = 1;
	} else if (m_RadiusType == GCU_IONIC) {
		m_RadiusType = GCU_RADIUS_UNKNOWN;
		index = 0;
	}
	if (index >= 0)
		gtk_combo_box_set_active (RadiusTypeMenu, index);
	PopulateRadiiMenu ();
}

void gcAtomsDlg::PopulateRadiiMenu ()
{
	const GcuAtomicRadius **radius = m_Radii;
	int i = m_RadiiIndex.size () - 2;
	g_signal_handler_block (RadiusMenu, m_RadiiSignalID);
	for (int j = 0; j <= i; j++)
		gtk_combo_box_remove_text (RadiusMenu, 1);
	m_RadiiIndex.clear ();
	string str;
	m_RadiiIndex.push_back (-1);
	i = 0;
	if (radius)
		while (*radius) {
			if (((*radius)->type != m_RadiusType) ||  ((*radius)->charge != m_Charge) || ((*radius)->value.value <= 0.)) {
				radius++;
				i++;
				continue;
			}
			str = ((*radius)->scale)? (*radius)->scale: "";
			if ((*radius)->cn > 0) {
	/*Note for translators: c.n. is for coordination number*/
				str += _(" c.n.=");
				char buf[16];
				g_snprintf(buf, sizeof(buf), " %u", (*radius)->cn);
				str += buf;
			}
			if ((*radius)->spin != GCU_N_A_SPIN)
				str += string(" (") + string(((*radius)->spin == GCU_LOW_SPIN)? _("low spin"): _("high spin")) + string(")");
			if (!str.length())
				str = _("Database");
			gtk_combo_box_append_text (RadiusMenu, str.c_str ());
			m_RadiiIndex.push_back(i++);
			radius++;
		}
	g_signal_handler_unblock (RadiusMenu, m_RadiiSignalID);
	gtk_combo_box_set_active (RadiusMenu, 0);
}
