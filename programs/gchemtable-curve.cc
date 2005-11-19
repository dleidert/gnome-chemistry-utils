// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * programs/gchemtable-curve.cc 
 *
 * Copyright (C) 2005
 *
 * Developed by Jean Bréfort <jean.brefort@normalesup.org>
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
#include "gchemtable-curve.h"
#include "gchemtable-app.h"
#include <gcu/chemistry.h>
#include <gcu/element.h>
#include <goffice/data/go-data-simple.h>
#include <goffice/gtk/go-graph-widget.h>
#include <goffice/graph/gog-axis.h>
#include <goffice/graph/gog-data-set.h>
#include <goffice/graph/gog-label.h>
#include <goffice/graph/gog-object.h>
#include <goffice/graph/gog-plot.h>
#include <goffice/graph/gog-series.h>
#include <goffice/graph/gog-style.h>
#include <goffice/graph/gog-styled-object.h>
#include <goffice/utils/go-line.h>
#include <goffice/utils/go-marker.h>
#include <goffice/utils/go-math.h>
#include <glib/gi18n.h>
#include <map>

using namespace gcu;

#warning "the following lines should be removed for stable releases"
#undef PACKAGE
#define PACKAGE "gchemutils-unstable" 

map<string, GChemTableCurve*> curves;

void on_show_curve (GObject *obj, char const* name)
{
	GChemTableApp *App = reinterpret_cast<GChemTableApp*> (g_object_get_data (obj, "app"));
	if (App == NULL)
		return;
	GChemTableCurve *curve = curves[name];
	if (curve)
		gtk_window_present (curve->GetWindow ());
	else
		curves[name] = new GChemTableCurve (App, name);
}

GChemTableCurve::GChemTableCurve (GChemTableApp *App, char const *name):
	Dialog (App, DATADIR"/"PACKAGE"/glade/curve.glade", "curvedlg")
{
	m_Name = name;
	GtkWidget *w = glade_xml_get_widget (xml, "vbox1");
#ifdef GO_GRAPH_WIDGET_OLD_API
	GtkWidget *pw = go_graph_widget_new ();
#else
	GtkWidget *pw = go_graph_widget_new (NULL);
#endif
	gtk_widget_set_size_request (pw, 400, 250);
	gtk_widget_show (pw);
	gtk_box_pack_end (GTK_BOX (w), pw, TRUE, TRUE, 0);
	GogChart *chart = go_graph_widget_get_chart (GO_GRAPH_WIDGET (pw));
	GogPlot *plot = (GogPlot *) gog_plot_new_by_name ("GogXYPlot");
	gog_object_add_by_name (GOG_OBJECT (chart), "Plot", GOG_OBJECT (plot));
	// Create a series for the plot and populate it with some simple data
	GogSeries *series = gog_plot_new_series (plot);
	double *yvals = g_new0 (double, MAX_ELT);
	GError *error;
	GogObject *obj, *label;
	GOData *data;
	int i;
	// FIXME: find a better way to do the following things !
	if (!strcmp (name, "en/Pauling")) {
		GcuElectronegativity en;
		en.scale = "Pauling";
		for (i = 1; i <= MAX_ELT; i++) {
			en.Z = i;
			yvals[i - 1] = (gcu_element_get_electronegativity (&en))?
				 				en.value.value: go_nan;
		}
		obj = gog_object_get_child_by_role (GOG_OBJECT (chart),
				gog_object_find_role_by_name (GOG_OBJECT (chart), "Y-Axis"));
		data = go_data_scalar_str_new (_("Pauling electronegativity"), FALSE);
		label = (GogObject*) g_object_new (GOG_LABEL_TYPE, NULL);
		gog_dataset_set_dim (GOG_DATASET (label), 0, data, &error);
		gog_object_add_by_name (obj, "Label", label);
		gtk_window_set_title (dialog, _("Pauling electronegativity"));
	} else if (!strcmp (name, "ae")) {
		GcuDimensionalValue const *val;
		Element *elt;
		// assuming all data are in kJ/mol
		for (i = 1; i <= MAX_ELT; i++) {
			elt = Element::GetElement (i);
			val = (elt)? elt->GetElectronAffinity (): NULL;
			yvals[i - 1] = (val)? val->value: go_nan;
		}
		obj = gog_object_get_child_by_role (GOG_OBJECT (chart),
				gog_object_find_role_by_name (GOG_OBJECT (chart), "Y-Axis"));
		data = go_data_scalar_str_new (_("Electron affinity (kJ/mol)"), FALSE);
		label = (GogObject*) g_object_new (GOG_LABEL_TYPE, NULL);
		gog_dataset_set_dim (GOG_DATASET (label), 0, data, &error);
		gog_object_add_by_name (obj, "Label", label);
		gtk_window_set_title (dialog, _("Electron affinity"));
	} else if (!strncmp (name, "ei/", 3)) {
		unsigned rank = strtol (name + 3, NULL, 10);
		GcuDimensionalValue const *val;
		Element *elt;
		// assuming all data are in MJ/mol
		for (i = 1; i <= MAX_ELT; i++) {
			elt = Element::GetElement (i);
			val = (elt)? elt->GetIonizationEnergy (rank): NULL;
			yvals[i - 1] = (val)? val->value: go_nan;
		}
		char *rk, *buf;
		switch (rank) {
		case 1:
			rk = g_strdup (_("1st. "));
			break;
		case 2:
			rk = g_strdup (_("2nd. "));
			break;
		case 3:
			rk = g_strdup (_("3rd. "));
			break;
		default:
			rk = g_strdup_printf (_("%dth. "), rank);
			break;
		}
		buf = g_strconcat (rk, _("ionization energy (MJ/mol)"), NULL);
		obj = gog_object_get_child_by_role (GOG_OBJECT (chart),
				gog_object_find_role_by_name (GOG_OBJECT (chart), "Y-Axis"));
		data = go_data_scalar_str_new (buf, TRUE);
		label = (GogObject*) g_object_new (GOG_LABEL_TYPE, NULL);
		gog_dataset_set_dim (GOG_DATASET (label), 0, data, &error);
		gog_object_add_by_name (obj, "Label", label);
		buf = g_strconcat (rk, _("ionization energy"), NULL);
		gtk_window_set_title (dialog, buf);
		g_free (buf);
		g_free (rk);
	} else
		gtk_widget_destroy (GTK_WIDGET (dialog));
	i = MAX_ELT - 1;
	while (!go_finite (yvals[i]))
		i--;
	i++;
	obj = gog_object_get_child_by_role (GOG_OBJECT (chart),
			gog_object_find_role_by_name (GOG_OBJECT (chart), "X-Axis"));
	data = go_data_scalar_val_new ((double) i);
	gog_dataset_set_dim (GOG_DATASET (obj), GOG_AXIS_ELEM_MAX, data, &error);
	data = go_data_vector_val_new (yvals, MAX_ELT, g_free);
	gog_series_set_dim (series, 1, data, &error);
	obj = gog_object_get_child_by_role (GOG_OBJECT (chart),
			gog_object_find_role_by_name (GOG_OBJECT (chart), "X-Axis"));
	data = go_data_scalar_str_new ("Z", FALSE);
	label = (GogObject*) g_object_new (GOG_LABEL_TYPE, NULL);
	gog_dataset_set_dim (GOG_DATASET (label), 0, data, &error);
	gog_object_add_by_name (obj, "Label", label);
}

GChemTableCurve::~GChemTableCurve ()
{
	curves.erase (m_Name);
}
