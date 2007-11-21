// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * programs/gchemtable-elt.cc 
 *
 * Copyright (C) 2005-2007 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "gchemtable-elt.h"
#include "gchemtable-app.h"
#include <gcu/element.h>
#include <glib/gi18n.h>
#include <list>

// FIXME "the following lines should be removed for stable releases"
#undef PACKAGE
#define PACKAGE "gchemutils-unstable" 

using namespace std;

extern void on_show_curve (GObject *obj, char const* name);
static void on_focus_in (GChemTableElt *dlg)
{
	dlg->OnFocusIn ();
}

GChemTableElt::GChemTableElt (GChemTableApp *App, int Z): Dialog (App, GLADEDIR"/eltpage.glade", "eltdlg")
{
	Element *elt = Element::GetElement (Z);
	m_Z = Z;
	char *buf;
	gtk_window_set_title (dialog, elt->GetName ());
	g_signal_connect_swapped (G_OBJECT (dialog), "focus-in-event", G_CALLBACK (on_focus_in), this);
	GtkWidget *w = glade_xml_get_widget (xml, "symbol");
	buf = g_strconcat ("<span font_desc=\"64\">", elt->GetSymbol (), "</span>", NULL);
	gtk_label_set_markup (GTK_LABEL (w), buf);
	g_free (buf);
	buf = g_strdup_printf ("%d", Z);
	w = glade_xml_get_widget (xml, "z");
	gtk_label_set_text (GTK_LABEL (w), buf);
	g_free (buf);
	int prec;
	double weight = elt->GetWeight (prec);
	char *format = (prec > 0)? g_strdup_printf ("%%0.%df",prec): g_strdup ("(%.0f)");
	buf = g_strdup_printf (format, weight);
	w = glade_xml_get_widget (xml, "weight");
	gtk_label_set_text (GTK_LABEL (w), buf);
	g_free (format);
	g_free (buf);
	w = glade_xml_get_widget (xml, "elec-conf-lbl");
	/* The <sup> </sup> markup at the end of the chain is there to ensure that
	things will be correcly aligned, add the same to the translated string */
	gtk_label_set_markup (GTK_LABEL (w), _("Electronic configuration:<sup> </sup>"));
	w = glade_xml_get_widget (xml, "elec-conf");
	gtk_label_set_markup (GTK_LABEL (w), elt->GetElectronicConfiguration ().c_str ());
	//Add composition list
	GtkListStore *pclist = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
	GtkTreeView *tree = GTK_TREE_VIEW (glade_xml_get_widget (xml, "names"));
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
	w = glade_xml_get_widget (xml, "pauling-en");
	GcuElectronegativity en;
	en.scale = "Pauling";
	en.Z = elt->GetZ ();
	if (elt->GetElectronegativity (&en)) {
		buf = gcu_value_get_string (&en.value);
		gtk_label_set_text (GTK_LABEL (w), buf);
		g_free (buf);
	} else
		gtk_label_set_text (GTK_LABEL (w), _("n.a."));
	w = glade_xml_get_widget (xml, "pauling-btn");
	g_object_set_data (G_OBJECT (w), "app", App);
	g_signal_connect (G_OBJECT (w), "clicked", G_CALLBACK (on_show_curve), (void*) "en/Pauling");
	// ionization energies
	int n = 1;
	GcuDimensionalValue const *value;
	GtkWidget *val, *button;
	GtkTable *table = GTK_TABLE (glade_xml_get_widget (xml, "ei-table"));
	while ((value = elt->GetIonizationEnergy (n))) {
		if (n > 1) {
			gtk_table_resize (table, 4, n);
			buf = g_strdup_printf (_("%d:"), n);
			w = gtk_label_new (buf);
			gtk_misc_set_padding (GTK_MISC (w), 8, 0);
 			g_free (buf);
			gtk_table_attach (table, w, 0, 1, n - 1, n,
				(GtkAttachOptions) (GTK_SHRINK | GTK_FILL),
				(GtkAttachOptions) 0 , 0, 0);
			val = gtk_label_new ("");
			gtk_misc_set_alignment (GTK_MISC (val), 0., 0.);
			gtk_table_attach (table, val, 1, 2, n - 1, n,
				(GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
				(GtkAttachOptions) 0 , 0, 0);
			button = gtk_button_new_with_label (_("Show curve"));
			gtk_table_attach (table, button, 2, 3, n - 1, n,
				(GtkAttachOptions) GTK_FILL,
				(GtkAttachOptions) 0 , 0, 0);
		} else {
			val = glade_xml_get_widget (xml, "ei-value");
			button = glade_xml_get_widget (xml, "ei-btn");
		}
		buf = gcu_dimensional_value_get_string (value);
		gtk_label_set_markup (GTK_LABEL (val), buf);
		g_free (buf);
		buf = g_strdup_printf ("ei/%d", n);
		g_object_set_data (G_OBJECT (button), "app", App);
		g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (on_show_curve), (void*) buf);
// FIXME: clean this on exit
		n++;
	}
	gtk_widget_show_all (GTK_WIDGET (table));
	if (n == 1) {
		w = glade_xml_get_widget (xml, "ei-lbl");
		gtk_label_set_text (GTK_LABEL (w), _("n.a."));
		w = glade_xml_get_widget (xml, "ei-btn");
		gtk_widget_hide (w);
	}
	// electronic affinities
	n = 1;
	table = GTK_TABLE (glade_xml_get_widget (xml, "ae-table"));
	while ((value = elt->GetElectronAffinity (n))) {
		if (n > 1) {
			gtk_table_resize (table, 4, n);
			buf = g_strdup_printf (_("%d:"), n);
			w = gtk_label_new (buf);
			gtk_misc_set_alignment (GTK_MISC (w), 0., 0.);
			g_free (buf);
			gtk_table_attach (table, w, 0, 1, n - 1, n,
				(GtkAttachOptions) (GTK_SHRINK | GTK_FILL),
				(GtkAttachOptions) 0 , 0, 0);
			val = gtk_label_new ("");
			gtk_table_attach (table, val, 1, 2, n - 1, n,
				(GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
				(GtkAttachOptions) 0 , 0, 0);
			button = NULL; // not enough values to draw a curve.
		} else {
			val = glade_xml_get_widget (xml, "ae-value");
			button = glade_xml_get_widget (xml, "ae-btn");
			g_object_set_data (G_OBJECT (button), "app", App);
			g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (on_show_curve), (void*) "ae");
		}
		buf = gcu_dimensional_value_get_string (value);
		gtk_label_set_markup (GTK_LABEL (val), buf);
		g_free (buf);
		n++;
	}
	gtk_widget_show_all (GTK_WIDGET (table));
	if (n == 1) {
		w = glade_xml_get_widget (xml, "ae-lbl");
		gtk_label_set_text (GTK_LABEL (w), _("n.a."));
		w = glade_xml_get_widget (xml, "ae-btn");
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
	button = glade_xml_get_widget (xml, "covalent-btn");
	if (elt->GetRadius (&r)) {
		buf = gcu_dimensional_value_get_string (&r.value);
		w = glade_xml_get_widget (xml, "covalent-radius");
		gtk_label_set_text (GTK_LABEL (w), buf);
		g_free (buf);
		g_object_set_data (G_OBJECT (button), "app", App);
		g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (on_show_curve), (void*) "covalent");
	} else {
		w = glade_xml_get_widget (xml, "covalent-radius");
		gtk_label_set_text (GTK_LABEL (w), _("n.a."));
		gtk_widget_hide (button);
	}
	r.type = GCU_VAN_DER_WAALS;
	button = glade_xml_get_widget (xml, "vdw-btn");
	if (elt->GetRadius (&r)) {
		buf = gcu_dimensional_value_get_string (&r.value);
		w = glade_xml_get_widget (xml, "vdw-radius");
		gtk_label_set_text (GTK_LABEL (w), buf);
		g_free (buf);
		g_object_set_data (G_OBJECT (button), "app", App);
		g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (on_show_curve), (void*) "vdw");
	} else {
		w = glade_xml_get_widget (xml, "vdw-radius");
		gtk_label_set_text (GTK_LABEL (w), _("n.a."));
		gtk_widget_hide (button);
	}
	r.type = GCU_METALLIC;
	button = glade_xml_get_widget (xml, "metallic-btn");
	if (elt->GetRadius (&r)) {
		buf = gcu_dimensional_value_get_string (&r.value);
		w = glade_xml_get_widget (xml, "metallic-radius");
		gtk_label_set_text (GTK_LABEL (w), buf);
		g_free (buf);
		g_object_set_data (G_OBJECT (button), "app", App);
		g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (on_show_curve), (void*) "metallic");
	} else {
		w = glade_xml_get_widget (xml, "metallic-radius");
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
		gtk_box_pack_start (GTK_BOX (glade_xml_get_widget (xml, "ionic-radii")),
								w, FALSE, FALSE, 0);
		gtk_widget_hide (glade_xml_get_widget (xml, "radii-scrolled"));
	} else {
		pclist = gtk_list_store_new (4, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
		tree = GTK_TREE_VIEW (glade_xml_get_widget (xml, "radii-list"));
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
	button = glade_xml_get_widget (xml, "melting-btn");
	if (prop) {
		w = glade_xml_get_widget (xml, "melting");
		gtk_label_set_text (GTK_LABEL (w), prop->GetAsString ());
		g_object_set_data (G_OBJECT (button), "app", App);
		g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (on_show_curve), (void*) "mp");
	} else
		gtk_widget_hide (w);
	prop = elt->GetProperty ("boilingpoint");
	button = glade_xml_get_widget (xml, "boiling-btn");
	if (prop) {
		w = glade_xml_get_widget (xml, "boiling");
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
