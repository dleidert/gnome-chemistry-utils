// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * programs/gchemtable-elt.cc
 *
 * Copyright (C) 2005-2011 Jean Bréfort <jean.brefort@normalesup.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the
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
#include "gchemtable-elt.h"
#include "gchemtable-app.h"
#include <gcu/element.h>
#include <glib/gi18n.h>
#include <list>
#include <cstring>

using namespace std;

extern void on_show_curve (GObject *obj, char const* name);
static void on_focus_in (GChemTableElt *dlg)
{
	dlg->OnFocusIn ();
}

GChemTableElt::GChemTableElt (GChemTableApp *App, int Z): gcugtk::Dialog (App, UIDIR"/eltpage.ui", "eltdlg", GETTEXT_PACKAGE)
{
	Element *elt = Element::GetElement (Z);
	m_Z = Z;
	char *buf;
	gtk_window_set_title (dialog, elt->GetName ());
	g_signal_connect_swapped (G_OBJECT (dialog), "focus-in-event", G_CALLBACK (on_focus_in), this);
	GtkWidget *w = GetWidget ("symbol");
	buf = g_strconcat ("<span font_desc=\"64\">", elt->GetSymbol (), "</span>", NULL);
	gtk_label_set_markup (GTK_LABEL (w), buf);
	g_free (buf);
	buf = g_strdup_printf ("%d", Z);
	w = GetWidget ("z");
	gtk_label_set_text (GTK_LABEL (w), buf);
	g_free (buf);
	w = GetWidget ("weight");
	DimensionalValue const *v = elt->GetWeight ();
	gtk_label_set_markup (GTK_LABEL (w), (v)? v->GetAsString (): _("Unknown"));
	w = GetWidget ("elec-conf-lbl");
	/* The <sup> </sup> markup at the end of the chain is there to ensure that
	things will be correcly aligned, add the same to the translated string */
	gtk_label_set_markup (GTK_LABEL (w), _("Electronic configuration:<sup> </sup>"));
	w = GetWidget ("elec-conf");
	gtk_label_set_markup (GTK_LABEL (w), elt->GetElectronicConfiguration ().c_str ());
	//Add composition list
	GtkListStore *pclist = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
	GtkTreeView *tree = GTK_TREE_VIEW (GetWidget ("names"));
	gtk_tree_view_set_model (tree, GTK_TREE_MODEL (pclist));
	g_object_unref (pclist);
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	/* column for element */
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Lang"), renderer, "text", 0, NULL);
	/* set this column to a minimum sizing (of 100 pixels) */
	gtk_tree_view_column_set_sizing(GTK_TREE_VIEW_COLUMN (column), GTK_TREE_VIEW_COLUMN_GROW_ONLY);
	gtk_tree_view_column_set_min_width(GTK_TREE_VIEW_COLUMN (column), 100);
	gtk_tree_view_append_column (tree, column);
	/* column for x */
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Name"), renderer, "text", 1, NULL);
	g_object_set (G_OBJECT (renderer), "xalign", 1.0, NULL);
	/* set this column to a fixed sizing (of 100 pixels) */
	gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column), GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 100);
	gtk_tree_view_append_column (tree, column);
	map<string, string> Names = elt->GetNames ();
	map<string, string>::iterator i, end = Names.end ();
	GtkTreeIter iter;
	for (i = Names.begin (); i != end; i++) {
		gtk_list_store_append (pclist, &iter);
		gtk_list_store_set (pclist, &iter,
				  0, (*i).first.c_str (),
				  1, (*i).second.c_str (),
				  -1);
	}
	// electronic properties page
	w = GetWidget ("pauling-en");
	GcuElectronegativity en;
	en.scale = "Pauling";
	en.Z = elt->GetZ ();
	if (elt->GetElectronegativity (&en)) {
		buf = gcu_value_get_string (&en.value);
		gtk_label_set_text (GTK_LABEL (w), buf);
		g_free (buf);
	} else
		gtk_label_set_text (GTK_LABEL (w), _("n.a."));
	w = GetWidget ("pauling-btn");
	g_object_set_data (G_OBJECT (w), "app", App);
	g_signal_connect (G_OBJECT (w), "clicked", G_CALLBACK (on_show_curve), (void*) "en-Pauling");
	// ionization energies
	int n = 1;
	GcuDimensionalValue const *value;
	GtkWidget *val, *button;
	GtkGrid *grid = GTK_GRID (GetWidget ("ei-grid"));
	while ((value = elt->GetIonizationEnergy (n))) {
		if (n > 1) {
			buf = g_strdup_printf (_("%d:"), n);
			w = gtk_label_new (buf);
			g_object_set (G_OBJECT (w), "xalign", 1., NULL);
 			g_free (buf);
			gtk_grid_attach (grid, w, 0, n - 1, 1, 1);
			val = gtk_label_new ("");
			g_object_set (G_OBJECT (val), "xalign", 0., "hexpand", true, NULL);
			gtk_grid_attach (grid, val, 1, n - 1, 1, 1);
			button = gtk_button_new_with_label (_("Show curve"));
			gtk_grid_attach (grid, button, 2, n - 1, 1, 1);
		} else {
			val = GetWidget ("ei-value");
			button = GetWidget ("ei-btn");
		}
		buf = gcu_dimensional_value_get_string (value);
		gtk_label_set_markup (GTK_LABEL (val), buf);
		g_free (buf);
		buf = g_strdup_printf ("ei-%d", n);
		g_object_set_data (G_OBJECT (button), "app", App);
		g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (on_show_curve), (void*) buf);
// FIXME: clean this on exit
		n++;
	}
	gtk_widget_show_all (GTK_WIDGET (grid));
	if (n == 1) {
		w = GetWidget ("ei-lbl");
		gtk_label_set_text (GTK_LABEL (w), _("n.a."));
		w = GetWidget ("ei-btn");
		gtk_widget_hide (w);
	}
	// electronic affinities
	n = 1;
	grid = GTK_GRID (GetWidget ("ae-grid"));
	while ((value = elt->GetElectronAffinity (n))) {
		if (n > 1) {
			buf = g_strdup_printf (_("%d:"), n);
			w = gtk_label_new (buf);
			g_object_set (G_OBJECT (w), "xalign", 1., NULL);
 			g_free (buf);
			gtk_grid_attach (grid, w, 0, n - 1, 1, 1);
			val = gtk_label_new ("");
			g_object_set (G_OBJECT (val), "xalign", 0., "hexpand", true, NULL);
			gtk_grid_attach (grid, val, 1, n - 1, 1, 1);
			button = NULL; // not enough values to draw a curve.
		} else {
			val = GetWidget ("ae-value");
			button = GetWidget ("ae-btn");
			g_object_set_data (G_OBJECT (button), "app", App);
			g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (on_show_curve), (void*) "ae");
		}
		buf = gcu_dimensional_value_get_string (value);
		gtk_label_set_markup (GTK_LABEL (val), buf);
		g_free (buf);
		n++;
	}
	gtk_widget_show_all (GTK_WIDGET (grid));
	if (n == 1) {
		w = GetWidget ("ae-lbl");
		gtk_label_set_text (GTK_LABEL (w), _("n.a."));
		w = GetWidget ("ae-btn");
		gtk_widget_hide (w);
	}
	// Radii page
	// First covalent radius
	GcuAtomicRadius r;
	r.Z = Z;
	r.type = GCU_COVALENT;
	r.charge = 0;
	r.scale = NULL;
	r.cn = -1;
	r.spin = GCU_N_A_SPIN;
	button = GetWidget ("covalent-btn");
	if (elt->GetRadius (&r)) {
		buf = gcu_dimensional_value_get_string (&r.value);
		w = GetWidget ("covalent-radius");
		gtk_label_set_text (GTK_LABEL (w), buf);
		g_free (buf);
		g_object_set_data (G_OBJECT (button), "app", App);
		g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (on_show_curve), (void*) "covalent");
	} else {
		w = GetWidget ("covalent-radius");
		gtk_label_set_text (GTK_LABEL (w), _("n.a."));
		gtk_widget_hide (button);
	}
	r.type = GCU_VAN_DER_WAALS;
	button = GetWidget ("vdw-btn");
	if (elt->GetRadius (&r)) {
		buf = gcu_dimensional_value_get_string (&r.value);
		w = GetWidget ("vdw-radius");
		gtk_label_set_text (GTK_LABEL (w), buf);
		g_free (buf);
		g_object_set_data (G_OBJECT (button), "app", App);
		g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (on_show_curve), (void*) "vdw");
	} else {
		w = GetWidget ("vdw-radius");
		gtk_label_set_text (GTK_LABEL (w), _("n.a."));
		gtk_widget_hide (button);
	}
	r.type = GCU_METALLIC;
	button = GetWidget ("metallic-btn");
	if (elt->GetRadius (&r)) {
		buf = gcu_dimensional_value_get_string (&r.value);
		w = GetWidget ("metallic-radius");
		gtk_label_set_text (GTK_LABEL (w), buf);
		g_free (buf);
		g_object_set_data (G_OBJECT (button), "app", App);
		g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (on_show_curve), (void*) "metallic");
	} else {
		w = GetWidget ("metallic-radius");
		gtk_label_set_text (GTK_LABEL (w), _("n.a."));
		gtk_widget_hide (button);
	}
	GcuAtomicRadius const **radii = elt->GetRadii ();
	list <GcuAtomicRadius const*> radii_list;
	list <GcuAtomicRadius const*>::iterator j, jend;
	int maxspin = 0;
	if (radii) while (*radii) {
		if (((*radii)->type == GCU_IONIC) && !strcmp ((*radii)->scale, "Shannon")) {
			j = radii_list.begin ();
			jend = radii_list.end ();
			while ((j != jend) && (((*j)->charge < (*radii)->charge) ||
				(((*j)->charge == (*radii)->charge) && ((*j)->cn < (*radii)->cn)) ||
				(((*j)->cn == (*radii)->cn) && ((*j)->spin < (*radii)->spin))))
				j++;
			radii_list.insert (j, *radii);
			if ((*radii)->spin > maxspin)
				maxspin = (*radii)->spin;
		}
		radii++;
	}
	if (radii_list.size () == 0) {
		w = gtk_label_new (_("n.a."));
		gtk_widget_show (w);
		gtk_grid_attach (GTK_GRID (GetWidget ("radii-grid")),
								w, 2, 3, 1, 1);
		gtk_widget_hide (GetWidget ("radii-scrolled"));
	} else {
		pclist = gtk_list_store_new (4, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
		tree = GTK_TREE_VIEW (GetWidget ("radii-list"));
		gtk_tree_view_set_model (tree, GTK_TREE_MODEL (pclist));
		g_object_unref (pclist);
		/* column for element */
		renderer = gtk_cell_renderer_text_new ();
		column = gtk_tree_view_column_new_with_attributes (_("Ion"), renderer, "markup", 0, NULL);
		/* set this column to a minimum sizing (of 80 pixels) */
		gtk_tree_view_column_set_spacing (column, 5);
		gtk_tree_view_append_column (tree, column);
		/* column for x */
		renderer = gtk_cell_renderer_text_new ();
		/* C.N. stands for coordination number */
		column = gtk_tree_view_column_new_with_attributes (_("C.N."), renderer, "text", 1, NULL);
		g_object_set (G_OBJECT (renderer), "xalign", 1.0, NULL);
		/* set this column to a fixed sizing (of 50 pixels) */
		gtk_tree_view_column_set_spacing (column, 5);
		gtk_tree_view_append_column (tree, column);
		column = gtk_tree_view_column_new_with_attributes (_("Spin"), renderer, "text", 2, NULL);
		if (maxspin == 0)
			g_object_set (G_OBJECT (column), "visible", false, NULL);
		g_object_set (G_OBJECT (renderer), "xalign", 1.0, NULL);
		/* set this column to a fixed sizing (of 50 pixels) */
		gtk_tree_view_column_set_spacing (column, 5);
		gtk_tree_view_append_column (tree, column);
		column = gtk_tree_view_column_new_with_attributes (_("Value"), renderer, "text", 3, NULL);
		g_object_set (G_OBJECT (renderer), "xalign", 1.0, NULL);
		/* set this column to a fixed sizing (of 50 pixels) */
		gtk_tree_view_column_set_spacing (column, 5);
		gtk_tree_view_append_column (tree, column);
		jend = radii_list.end ();
		char *ion, *cn;
		char const *spin;
		for (j = radii_list.begin (); j != jend; j++) {
			if ((*j)->charge > 1)
				ion = g_strdup_printf ("%s<sup>%d+</sup><sub> </sub>",elt->GetSymbol (),(*j)->charge);
			else if ((*j)->charge < -1)
				ion = g_strdup_printf ("%s<sup>%d\xE2\x88\x92</sup><sub> </sub>",elt->GetSymbol (),-(*j)->charge);
			else if ((*j)->charge == 1)
				ion = g_strdup_printf ("%s<sup>+</sup><sub> </sub>",elt->GetSymbol ());
			else
				ion = g_strdup_printf ("%s<sup>\xE2\x88\x92</sup><sub> </sub>",elt->GetSymbol ());
			cn = g_strdup_printf ("%d", (*j)->cn);
			switch ((*j)->spin) {
			case GCU_LOW_SPIN:
				spin = _("Low");
				break;
			case GCU_HIGH_SPIN:
				spin = _("High");
				break;
			default:
				spin = "";
				break;
			}
			buf = gcu_dimensional_value_get_string (&(*j)->value);
			gtk_list_store_append (pclist, &iter);
			gtk_list_store_set (pclist, &iter,
					  0, ion,
					  1, cn,
					  2, spin,
					  3, buf,
					  -1);
			g_free (ion);
			g_free (cn);
			g_free (buf);
		}
	}
	Value const *prop = elt->GetProperty ("meltingpoint");
	button = GetWidget ("melting-btn");
	if (prop) {
		w = GetWidget ("melting");
		gtk_label_set_text (GTK_LABEL (w), prop->GetAsString ());
		g_object_set_data (G_OBJECT (button), "app", App);
		g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (on_show_curve), (void*) "mp");
	} else
		gtk_widget_hide (w);
	prop = elt->GetProperty ("boilingpoint");
	button = GetWidget ("boiling-btn");
	if (prop) {
		w = GetWidget ("boiling");
		gtk_label_set_text (GTK_LABEL (w), prop->GetAsString ());
		g_object_set_data (G_OBJECT (button), "app", App);
		g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (on_show_curve), (void*) "bp");
	} else
		gtk_widget_hide (button);
}

GChemTableElt::~GChemTableElt ()
{
	reinterpret_cast<GChemTableApp*> (m_App)->ClearPage (m_Z);
}

void GChemTableElt::OnFocusIn ()
{
	reinterpret_cast<GChemTableApp*> (m_App)->SetCurZ (m_Z);
}
