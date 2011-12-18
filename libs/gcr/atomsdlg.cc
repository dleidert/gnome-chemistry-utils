// -*- C++ -*-

/*
 * Gnome Crystal
 * atomsdlg.cc
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

namespace gcr {

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
	double EffectiveRadiusRatio;
	bool CustomColor;
	double Blue, Red, Green, Alpha;
};

class AtomsDlgPrivate
{
public:
	static void AddRow (AtomsDlg *pBox);
	static void DeleteRow (AtomsDlg *pBox);
	static void DeleteAll (AtomsDlg *pBox);
	static void SelectAll (AtomsDlg *pBox);
	static void SelectElt (AtomsDlg *pBox);
	static void ValueChanged (AtomsDlg *pBox, unsigned row, unsigned column);
	static void RowSelected (AtomsDlg *pBox, int row);
	static void ElementChanged (AtomsDlg *pBox, unsigned Z);
	static void ColorSet (GtkColorButton *btn, AtomsDlg *pBox);
	static void SetColor (unsigned i, AtomsDlg *pBox);
	static void ColorToggled (GtkToggleButton *btn, AtomsDlg *pBox);
	static void ChargeChanged (GtkSpinButton *btn, AtomsDlg *pBox);
	static void SetCharge (unsigned i, AtomsDlg *pBox);
	static void RadiusTypeChanged (GtkComboBox *menu, AtomsDlg *pBox);
	static void RadiusIndexChanged(GtkComboBox *menu, AtomsDlg *pBox);
	static bool RadiusEdited (AtomsDlg *pBox);
	static void SetRadius (unsigned i, AtomsDlg *pBox);
	static void RadiusScaleChanged (GtkSpinButton *btn, AtomsDlg *pBox);
	static void SetRadiusScale (unsigned i, AtomsDlg *pBox);
};

void AtomsDlgPrivate::AddRow (AtomsDlg *pBox)
{
	Atom *new_atom;
	if (pBox->m_AtomSelected >= 0)
		new_atom = new Atom (*pBox->m_Atoms[pBox->m_AtomSelected]);
	else {
		new_atom = new Atom (pBox->m_nElt, 0., 0., 0.);
		// FIXME: keep the radius value, but the scale should be overriden
		new_atom->SetRadius (pBox->m_Radius);
		new_atom->SetEffectiveRadiusRatio (gtk_spin_button_get_value (pBox->ScaleBtn) / 100.);
		GdkRGBA rgba;
		gtk_color_button_get_rgba (pBox->AtomColor, &rgba);
		new_atom->SetColor (rgba.red, rgba.green, rgba.blue, rgba.alpha);
	}
	unsigned new_row = gcr_grid_append_row (pBox->m_Grid,
	                                        (new_atom->GetZ ())? Element::Symbol (new_atom->GetZ ()): _("Unknown"),
	                                        new_atom->x (), new_atom->y (), new_atom->z ()),
			 max_row = pBox->m_Atoms.capacity ();
	if (new_row >= max_row)
		pBox->m_Atoms.resize (max_row + 10);
	pBox->m_Atoms[new_row] = new_atom;
	pBox->m_pDoc->GetAtomList ()->push_back (new_atom);
	pBox->m_pDoc->Update ();
	pBox->m_pDoc->SetDirty (true);
	gtk_widget_set_sensitive (pBox->DeleteAllBtn, true);
}

void AtomsDlgPrivate::DeleteRow (AtomsDlg *pBox)
{
	pBox->m_pDoc->GetAtomList ()->remove (pBox->m_Atoms[pBox->m_AtomSelected]);
	delete pBox->m_Atoms[pBox->m_AtomSelected];
	pBox->m_Atoms.erase (pBox->m_Atoms.begin () + pBox->m_AtomSelected);
	gcr_grid_delete_row (GCR_GRID (pBox->m_Grid), pBox->m_AtomSelected);
	pBox->m_pDoc->Update ();
	pBox->m_pDoc->SetDirty (true);
	gtk_widget_set_sensitive (pBox->DeleteAllBtn, !pBox->m_pDoc->GetAtomList ()->empty ());
}

void AtomsDlgPrivate::DeleteAll (AtomsDlg *pBox)
{
	gcr_grid_delete_all (pBox->m_Grid);
	for (unsigned i = 0; i < pBox->m_pDoc->GetAtomList ()->size (); i++)
		delete pBox->m_Atoms[i];
	pBox->m_Atoms.clear ();
	pBox->m_pDoc->GetAtomList ()->clear ();
	pBox->m_pDoc->Update ();
	pBox->m_pDoc->SetDirty (true);
	gtk_widget_set_sensitive (pBox->DeleteAllBtn, false);
}

void AtomsDlgPrivate::SelectAll (AtomsDlg *pBox)
{
	gcr_grid_select_all (pBox->m_Grid);
}

void AtomsDlgPrivate::SelectElt (AtomsDlg *pBox)
{
	for (unsigned i = 0; i < pBox->m_pDoc->GetAtomList ()->size (); i++)
		if (pBox->m_Atoms[i]->GetZ () == pBox->m_nElt)
			gcr_grid_add_row_to_selection (pBox->m_Grid, i);
}

void AtomsDlgPrivate::ValueChanged (AtomsDlg *pBox, unsigned row, unsigned column)
{
	double coord = gcr_grid_get_double (pBox->m_Grid, row, column);
	switch (column) {
	case COLUMN_X:
		pBox->m_Atoms[pBox->m_AtomSelected]->x () = coord;
		break;
	case COLUMN_Y:
		pBox->m_Atoms[pBox->m_AtomSelected]->y () = coord;
		break;
	case COLUMN_Z:
		pBox->m_Atoms[pBox->m_AtomSelected]->z () = coord;
		break;
	}
	pBox->m_pDoc->Update ();
	pBox->m_pDoc->SetDirty (true);
}

void AtomsDlgPrivate::RowSelected (AtomsDlg *pBox, int row)
{
	pBox->m_AtomSelected = row;
	gtk_widget_set_sensitive (pBox->DeleteBtn, row >= 0);
	if (row >= 0) {
		pBox->m_nElt = pBox->m_Atoms[row]->GetZ ();
		// update the color
		g_signal_handler_block (pBox->AtomColor, pBox->m_ColorSignalID);
		GdkRGBA rgba, color;
		pBox->m_Atoms[row]->GetColor (rgba);
		gtk_color_button_set_rgba (pBox->AtomColor, &rgba);
		g_signal_handler_unblock (pBox->AtomColor, pBox->m_ColorSignalID);
		if (pBox->m_nElt) {
			double *Colors = Element::GetElement (pBox->m_nElt)->GetDefaultColor ();
			color.red = Colors[0];
			color.green = Colors[1];
			color.blue = Colors[2];
			color.alpha = 1.;
			gtk_toggle_button_set_active (pBox->CustomColor,
			                              static_cast < float > (color.red) != static_cast < float > (rgba.red) ||
				                          static_cast < float > (color.green) != static_cast < float > (rgba.green) ||
				                          static_cast < float > (color.blue) != static_cast < float > (rgba.blue) ||
				                          static_cast < float > (color.alpha) != static_cast < float > (rgba.alpha));
			pBox->m_Radii = Element::GetElement (pBox->m_nElt)->GetRadii ();
		} else {
			gtk_toggle_button_set_active (pBox->CustomColor, true);
			pBox->m_Radii = NULL;
		}
		pBox->m_Radius = pBox->m_Atoms[row]->GetRadius ();
		pBox->m_RadiusType = pBox->m_Radius.type;
		// set the charge
		g_signal_handler_block (pBox->ChargeBtn, pBox->m_ChargeSignalID);
		gtk_spin_button_set_value (pBox->ChargeBtn, (pBox->m_Charge = pBox->m_Radius.charge));
		g_signal_handler_unblock (pBox->ChargeBtn, pBox->m_ChargeSignalID);
		// last, radii
		g_signal_handler_block (pBox->ScaleBtn, pBox->m_ScaleSignalID);
		gtk_spin_button_set_value (pBox->ScaleBtn, pBox->m_Atoms[row]->GetEffectiveRadiusRatio () * 100.);
		g_signal_handler_unblock (pBox->ScaleBtn, pBox->m_ScaleSignalID);
		g_signal_handler_block (pBox->RadiusTypeMenu, pBox->m_RadiusTypeSignalID);
		gtk_combo_box_set_active (GTK_COMBO_BOX (pBox->RadiusTypeMenu), (pBox->m_Radius.type == GCU_RADIUS_UNKNOWN)? 0: pBox->m_Radius.type - 1);
		g_signal_handler_unblock (pBox->RadiusTypeMenu, pBox->m_RadiusTypeSignalID);
		pBox->PopulateRadiiMenu ();
		char *buf;
		buf = gcu_value_get_string (reinterpret_cast < GcuValue * > (&pBox->m_Radius.value));
		gtk_entry_set_text (pBox->AtomR, buf);
		g_free (buf);
	}
}

void AtomsDlgPrivate::ElementChanged (AtomsDlg *pBox, unsigned Z)
{
	GdkRGBA color;
	if ((pBox->m_nElt = Z)) {
		pBox->m_Radii = Element::GetElement (Z)->GetRadii ();
		if ((pBox->m_RadiusType == GCU_IONIC) && (pBox->m_Charge == 0)) {
			pBox->m_RadiusType = GCU_RADIUS_UNKNOWN;
			gtk_combo_box_set_active (GTK_COMBO_BOX (pBox->RadiusTypeMenu), 0);
		} else
			pBox->PopulateRadiiMenu ();
		gtk_toggle_button_set_active (pBox->CustomColor, false);
		double *Colors = Element::GetElement (Z)->GetDefaultColor ();
		color.red = Colors[0];
		color.green = Colors[1];
		color.blue = Colors[2];
		color.alpha = 1.;
		gtk_color_button_set_rgba (pBox->AtomColor, &color);
	} else {
		pBox->m_Radii = NULL;
		gtk_toggle_button_set_active (pBox->CustomColor, true);
		gtk_color_button_get_rgba (pBox->AtomColor, &color);
	}
	if (pBox->m_AtomSelected >= 0) {
		pBox->m_Atoms[pBox->m_AtomSelected]->SetZ (Z);
		gcr_grid_set_string (pBox->m_Grid, pBox->m_AtomSelected, 0, Z > 0? Element::GetElement (Z)->GetSymbol (): _("Unknown"));
		pBox->m_Atoms[pBox->m_AtomSelected]->SetRadius (pBox->m_Radius);
		pBox->m_Atoms[pBox->m_AtomSelected]->SetColor (color.red, color.green, color.blue, color.alpha);
		pBox->m_pDoc->Update ();
		pBox->m_pDoc->SetDirty (true);
	}
}

void AtomsDlgPrivate::SetColor (unsigned i, AtomsDlg *pBox)
{
		pBox->m_Atoms[i]->SetColor (pBox->m_RGBA.red, pBox->m_RGBA.green, pBox->m_RGBA.blue, pBox->m_RGBA.alpha);
}

void AtomsDlgPrivate::ColorSet (GtkColorButton *btn, AtomsDlg *pBox)
{
	if (pBox->m_AtomSelected >= 0) {
		gtk_color_button_get_rgba (btn, &pBox->m_RGBA);
		gcr_grid_for_each_selected (pBox->m_Grid, reinterpret_cast < GridCb > (SetColor), pBox);
		pBox->m_pDoc->Update ();
		pBox->m_pDoc->SetDirty (true);
	}
}

void AtomsDlgPrivate::ColorToggled (GtkToggleButton *btn, AtomsDlg *pBox)
{
	gtk_widget_set_sensitive (GTK_WIDGET(pBox->AtomColor), gtk_toggle_button_get_active (btn));
}

void AtomsDlgPrivate::SetCharge (unsigned i, AtomsDlg *pBox)
{
		pBox->m_Atoms[i]->SetCharge (pBox->m_Charge);
		pBox->m_Atoms[i]->SetRadius (pBox->m_Radius);
}

void AtomsDlgPrivate::ChargeChanged (GtkSpinButton *btn, AtomsDlg *pBox)
{
	int charge = gtk_spin_button_get_value_as_int (btn);
	if (pBox->m_Charge == charge)
		return;
	int index = -1;
	pBox->m_Charge = charge;
	if (charge) {
		pBox->m_RadiusType = GCU_IONIC;
		index = 1;
	} else if (pBox->m_RadiusType == GCU_IONIC) {
		pBox->m_RadiusType = GCU_RADIUS_UNKNOWN;
		index = 0;
	}
	if (index >= 0)
		gtk_combo_box_set_active (GTK_COMBO_BOX (pBox->RadiusTypeMenu), index);
	pBox->PopulateRadiiMenu ();
	if (pBox->m_AtomSelected >= 0) {
		gcr_grid_for_each_selected (pBox->m_Grid, reinterpret_cast < GridCb > (SetCharge), pBox);
		pBox->m_pDoc->Update ();
		pBox->m_pDoc->SetDirty (true);
	}
}

void AtomsDlgPrivate::RadiusTypeChanged (GtkComboBox *menu, AtomsDlg *pBox)
{
	int type = gtk_combo_box_get_active (menu);
	if (type)
		type++; //skip GCU_ATOMIC
	if (type == pBox->m_RadiusType)
		return;
	int charges[17];
	memset (charges, 0, 17 * sizeof (int));
	pBox->m_RadiusType = type;
	if  ((type == GCU_IONIC) && pBox->m_Radii) {
		if (pBox->m_Charge)
			return;
		const GcuAtomicRadius **radius = pBox->m_Radii;
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
		pBox->m_Charge = 8;
		for (int i = 0; i < 17; i++) {
			if (charges[i] > max) {
				pBox->m_Charge = i - 8;
				max = charges[i];
			} else if (charges[i] == max) {
				/*FIXME: This is the most problematic case. The policy here is
				to take the smallest charge and if the two charges have the same absolute
				value, take the negative one. We might have something better in gchemutils.*/
				if ((abs (i - 8) < abs (pBox->m_Charge)) || i < 8) {
					pBox->m_Charge = i - 8;
					max = charges[i];
				}
			}
		}
	} else
		pBox->m_Charge = 0;
	gtk_spin_button_set_value (pBox->ChargeBtn, pBox->m_Charge);
	pBox->PopulateRadiiMenu ();
	if (pBox->m_AtomSelected >= 0) {
		gcr_grid_for_each_selected (pBox->m_Grid, reinterpret_cast < GridCb > (SetRadius), pBox);
		pBox->m_pDoc->Update ();
		pBox->m_pDoc->SetDirty (true);
	}
}

void AtomsDlgPrivate::RadiusIndexChanged (GtkComboBox *menu, AtomsDlg *pBox)
{
//	pBox->SetRadiusIndex (gtk_combo_box_get_active (menu));
	int i = pBox->m_RadiiIndex[gtk_combo_box_get_active (menu)];
	gtk_widget_set_sensitive (GTK_WIDGET (pBox->AtomR), i < 0);
	if (i >= 0) {
		pBox->m_Radius = *(pBox->m_Radii[i]);
		char buf[20];
		g_snprintf (buf, sizeof (buf), "%g", pBox->m_Radius.value.value);
		gtk_entry_set_text (pBox->AtomR, buf);
	} else {
		pBox->m_Radius.scale = "custom";
		pBox->m_Radius.spin = GCU_N_A_SPIN;
		pBox->m_Radius.charge = pBox->m_Charge;
		pBox->m_Radius.cn = -1;
		pBox->m_Radius.type = static_cast < gcu_radius_type > (pBox->m_RadiusType);
	}
	if (pBox->m_AtomSelected >= 0) {
		gcr_grid_for_each_selected (pBox->m_Grid, reinterpret_cast < GridCb > (SetRadius), pBox);
		pBox->m_pDoc->Update ();
		pBox->m_pDoc->SetDirty (true);
	}
}

void AtomsDlgPrivate::SetRadius (unsigned i, AtomsDlg *pBox)
{
		pBox->m_Atoms[i]->SetRadius (pBox->m_Radius);
}

bool AtomsDlgPrivate::RadiusEdited (AtomsDlg *pBox)
{
	if (pBox->m_Radius.type != GCU_RADIUS_UNKNOWN)
		return false; // don't care
	g_signal_handler_block (pBox->AtomR, pBox->m_EntryFocusOutSignalID);
	if (pBox->GetNumber (pBox->AtomR, &(pBox->m_Radius.value.value), gcugtk::Min, 0) && pBox->m_AtomSelected >= 0) {
/*		gcr_grid_for_each_selected (pBox->m_Grid, reinterpret_cast < GridCb > (SetRadius), pBox);
		pBox->m_pDoc->Update ();
		pBox->m_pDoc->SetDirty (true);*/
	}
	g_signal_handler_unblock (pBox->AtomR, pBox->m_EntryFocusOutSignalID);
	return false;
}

void AtomsDlgPrivate::SetRadiusScale (unsigned i, AtomsDlg *pBox)
{
		pBox->m_Atoms[i]->SetEffectiveRadiusRatio (pBox->m_Ratio);
}

void AtomsDlgPrivate::RadiusScaleChanged (GtkSpinButton *btn, AtomsDlg *pBox)
{
	pBox->m_Ratio = gtk_spin_button_get_value (btn) / 100.;
	if (pBox->m_AtomSelected >= 0) {
		gcr_grid_for_each_selected (pBox->m_Grid, reinterpret_cast < GridCb > (SetRadiusScale), pBox);
		pBox->m_pDoc->Update ();
		pBox->m_pDoc->SetDirty (true);
	}
}

AtomsDlg::AtomsDlg (Application *App, Document* pDoc): gcugtk::Dialog (App, UIDIR"/atoms.ui", "atoms", GETTEXT_PACKAGE, pDoc)
{
	m_pDoc = pDoc;
	m_Closing = false;
	GtkWidget *frame = GetWidget ("mendeleiev");
	periodic = (GcuPeriodic*) gcu_periodic_new ();
	g_signal_connect_swapped (G_OBJECT (periodic), "element_changed", G_CALLBACK (AtomsDlgPrivate::ElementChanged), this);
	g_object_set (G_OBJECT (periodic), "can_unselect", TRUE, "color-style", GCU_PERIODIC_COLOR_DEFAULT, NULL);
	gtk_container_add (GTK_CONTAINER (frame), (GtkWidget *) periodic);
	GtkWidget *button = GetWidget ("add");
	g_signal_connect_swapped (G_OBJECT (button), "clicked", G_CALLBACK (AtomsDlgPrivate::AddRow), this);
	DeleteBtn = GetWidget ("delete");
	gtk_widget_set_sensitive (DeleteBtn, false);
	g_signal_connect_swapped (G_OBJECT (DeleteBtn), "clicked", G_CALLBACK (AtomsDlgPrivate::DeleteRow), this);
	DeleteAllBtn = GetWidget ("delete_all");
	g_signal_connect_swapped (G_OBJECT (DeleteAllBtn), "clicked", G_CALLBACK (AtomsDlgPrivate::DeleteAll), this);
	g_signal_connect_swapped (GetObject ("select-all"), "clicked", G_CALLBACK (AtomsDlgPrivate::SelectAll), this);
	SelectEltBtn = GetWidget ("select-elt");
	g_signal_connect_swapped (G_OBJECT (SelectEltBtn), "clicked", G_CALLBACK (AtomsDlgPrivate::SelectElt), this);
	m_Grid = GCR_GRID (gcr_grid_new (_("Atom"), G_TYPE_STRING, _("x"), G_TYPE_DOUBLE, _("y"), G_TYPE_DOUBLE, _("z"), G_TYPE_DOUBLE, NULL));
	gcr_grid_set_allow_multiple_selection (m_Grid, true);
	g_object_set (G_OBJECT (m_Grid), "expand", true, NULL);
	gcr_grid_customize_column (m_Grid, COLUMN_ELT, strlen ("Unknown"), false);
	gtk_grid_attach (GTK_GRID (GetWidget ("atoms-grid")), GTK_WIDGET (m_Grid), 3, 1, 1, 5);
	g_signal_connect_swapped (G_OBJECT (m_Grid), "row-selected", G_CALLBACK (AtomsDlgPrivate::RowSelected), this);
	g_signal_connect_swapped (G_OBJECT (m_Grid), "value-changed", G_CALLBACK (AtomsDlgPrivate::ValueChanged), this);
	m_nElt = 0;
	m_AtomSelected = -1;
	gcr::AtomList* Atoms = m_pDoc->GetAtomList ();
	m_Atoms.resize ((Atoms->size () / 10 + 1) * 10);
	list < gcr::Atom * >::iterator i, end = Atoms->end ();
	for (i = Atoms->begin (); i != end; i++)
		m_Atoms[gcr_grid_append_row (m_Grid, ((*i)->GetZ () > 0)? Element::Symbol ((*i)->GetZ ()): _("Unknown"), (*i)->x (), (*i)->y (), (*i)->z ())] = *i;
	if (Atoms->empty ())
		gtk_widget_set_sensitive (DeleteAllBtn, false);
	AtomColor = GTK_COLOR_BUTTON (GetWidget ("color"));
	m_ColorSignalID = g_signal_connect (G_OBJECT (AtomColor), "color-set", G_CALLBACK (AtomsDlgPrivate::ColorSet), this);
	CustomColor = GTK_TOGGLE_BUTTON (GetWidget ("custom_color"));
	gtk_toggle_button_set_active (CustomColor, true);
	g_signal_connect (G_OBJECT (CustomColor), "toggled", G_CALLBACK (AtomsDlgPrivate::ColorToggled), this);
	ChargeBtn = GTK_SPIN_BUTTON (GetWidget ("charge"));
	m_ChargeSignalID = g_signal_connect (G_OBJECT (ChargeBtn), "value-changed", G_CALLBACK (AtomsDlgPrivate::ChargeChanged), this);
	RadiusTypeMenu = GTK_COMBO_BOX_TEXT (GetWidget ("radius-type"));
	gtk_combo_box_set_active (GTK_COMBO_BOX (RadiusTypeMenu), 0);
	m_RadiusTypeSignalID = g_signal_connect (G_OBJECT (RadiusTypeMenu), "changed", G_CALLBACK (AtomsDlgPrivate::RadiusTypeChanged), this);
	RadiusMenu = GTK_COMBO_BOX_TEXT (GetWidget ("radius-menu"));
	m_RadiiSignalID = g_signal_connect (G_OBJECT (RadiusMenu), "changed", G_CALLBACK (AtomsDlgPrivate::RadiusIndexChanged), this);
	AtomR = GTK_ENTRY (GetWidget ("atomr"));
	g_signal_connect_swapped (G_OBJECT (AtomR), "activate", G_CALLBACK (AtomsDlgPrivate::RadiusEdited), this);
	m_EntryFocusOutSignalID = g_signal_connect_swapped (G_OBJECT (AtomR), "focus-out-event", G_CALLBACK (AtomsDlgPrivate::RadiusEdited), this);
	ScaleBtn = GTK_SPIN_BUTTON (GetWidget ("scale-btn"));
	m_ScaleSignalID = g_signal_connect (G_OBJECT (ScaleBtn), "value-changed", G_CALLBACK (AtomsDlgPrivate::RadiusScaleChanged), this);
	m_ScaleSignalID = g_signal_connect (G_OBJECT (ScaleBtn), "activate", G_CALLBACK (AtomsDlgPrivate::RadiusScaleChanged), this);
	m_RadiusType = m_Charge = 0;
	m_Radii = NULL;
	m_Radius.type = GCU_RADIUS_UNKNOWN;
	m_Radius.cn = -1;
	m_Radius.spin = GCU_N_A_SPIN;
	m_Radius.value.value = 0.;
	m_Radius.value.prec = 0;
	m_Radius.value.delta = 0;
	m_Radius.scale = "custom";
	PopulateRadiiMenu ();
	gtk_widget_show_all (GTK_WIDGET (dialog));
}

AtomsDlg::~AtomsDlg ()
{
}

void AtomsDlg::PopulateRadiiMenu ()
{
	const GcuAtomicRadius **radius = m_Radii;
	int i = m_RadiiIndex.size () - 2, j = 1, selected = 0;
	g_signal_handler_block (RadiusMenu, m_RadiiSignalID);
	for (int j = 0; j <= i; j++)
		gtk_combo_box_text_remove (RadiusMenu, 1);
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
			gtk_combo_box_text_append_text (RadiusMenu, str.c_str ());
			if (m_Radius.cn == (*radius)->cn && m_Radius.spin == (*radius)->spin
			    && ((!m_Radius.scale && !(*radius)->scale) ||
				(m_Radius.scale && (*radius)->scale && !strcmp (m_Radius.scale, (*radius)->scale))))
				selected = j;
			m_RadiiIndex.push_back(i++);
			j++;
			radius++;
		}
	gtk_combo_box_set_active (GTK_COMBO_BOX (RadiusMenu), selected);
	gtk_widget_set_sensitive (SelectEltBtn, m_nElt > 0);
	g_signal_handler_unblock (RadiusMenu, m_RadiiSignalID);
	m_Radius.charge = m_Charge;
}

void AtomsDlg::Closed ()
{
	// check if the cleavages list is coherent
	m_Closing = true;
	m_pDoc->CheckAtoms ();
}

void AtomsDlg::ReloadData ()
{
	if (m_Closing)
		return;
	gcr_grid_delete_all (GCR_GRID (m_Grid));
	m_Atoms.clear ();
	gcr::AtomList* Atoms = m_pDoc->GetAtomList ();
	for (list < Atom * >::iterator i = Atoms->begin (); i != Atoms->end (); i++)
		m_Atoms[gcr_grid_append_row (GCR_GRID (m_Grid), ((*i)->GetZ ())? Element::Symbol ((*i)->GetZ ()): _("Unknown"),
	                                        (*i)->x (), (*i)->y (), (*i)->z ())] = *i;
	if (!m_Atoms.size ())
		gtk_widget_set_sensitive (DeleteAllBtn, false);
}

}	//	namespace gcr
