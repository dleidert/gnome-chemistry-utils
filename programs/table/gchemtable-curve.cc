// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * programs/gchemtable-curve.cc 
 *
 * Copyright (C) 2005-2008 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "gchemtable-data.h"
#include "gchemtable-data-allocator.h"
#include <gcu/chemistry.h>
#include <gcu/element.h>
#include <gcu/print-setup-dlg.h>
#include <goffice/data/go-data-simple.h>
#include <goffice/gtk/go-graph-widget.h>
#include <goffice/gtk/goffice-gtk.h>
#include <goffice/graph/gog-axis.h>
#include <goffice/graph/gog-data-allocator.h>
#include <goffice/graph/gog-data-set.h>
#include <goffice/graph/gog-guru.h>
#include <goffice/graph/gog-label.h>
#include <goffice/graph/gog-object.h>
#include <goffice/graph/gog-plot.h>
#include <goffice/graph/gog-series.h>
#include <goffice/graph/gog-style.h>
#include <goffice/graph/gog-styled-object.h>
#include <goffice/math/go-math.h>
#include <goffice/utils/go-locale.h>
#include <goffice/utils/go-line.h>
#include <goffice/utils/go-image.h>
#include <goffice/utils/go-marker.h>
#include <gsf/gsf-input-memory.h>
#include <gsf/gsf-output-memory.h>
#include <glib/gi18n.h>
#include <map>
#include <cstring>

using namespace gcu;
using namespace std;

// FIXME "the following lines should be removed for stable releases"
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

static void on_get_data (GtkClipboard *clipboard, GtkSelectionData *selection_data,  guint info, GogGraph *graph)
{
	guchar *buffer = NULL;
	char *format = NULL;
	GsfOutput *output;
	GsfOutputMemory *omem;
	gsf_off_t osize;
	GOImageFormat fmt = GO_IMAGE_FORMAT_UNKNOWN;
	double w, h;
	gog_graph_get_size (graph, &w, &h);
	output = gsf_output_memory_new ();
	omem   = GSF_OUTPUT_MEMORY (output);
	switch (info) {
	case 0: {
			GsfXMLOut *xout;
			char *old_num_locale, *old_monetary_locale;
		
			old_num_locale = g_strdup (go_setlocale (LC_NUMERIC, NULL));
			go_setlocale (LC_NUMERIC, "C");
			old_monetary_locale = g_strdup (go_setlocale (LC_MONETARY, NULL));
			go_setlocale (LC_MONETARY, "C");
			go_locale_untranslated_booleans ();
		
			xout = gsf_xml_out_new (output);
			gog_object_write_xml_sax (GOG_OBJECT (graph), xout);
			g_object_unref (xout);
		
			/* go_setlocale restores bools to locale translation */
			go_setlocale (LC_MONETARY, old_monetary_locale);
			g_free (old_monetary_locale);
			go_setlocale (LC_NUMERIC, old_num_locale);
			g_free (old_num_locale);
		}
		break;
	case 1:
	case 2:
		fmt = GO_IMAGE_FORMAT_SVG;
		break;
	case 3:
		fmt = GO_IMAGE_FORMAT_PNG;
		break;
	}
	/* FIXME Add a dpi editor. Default dpi to 150 for now */
	bool res = (fmt != GO_IMAGE_FORMAT_UNKNOWN)?
		gog_graph_export_image (graph, fmt, output, 150.0, 150.0):
		true;
	if (res) {
		osize = gsf_output_size (output);
				
		buffer = (guchar*) g_malloc (osize);
		memcpy (buffer, gsf_output_memory_get_bytes (omem), osize);
		gsf_output_close (output);
		g_object_unref (output);
		g_free (format);
		gtk_selection_data_set (selection_data,
					selection_data->target, 8,
					(guchar *) buffer, osize);
		g_free (buffer);
	}
}

void on_clear_data(GtkClipboard *clipboard, GogGraph *graph)
{
	g_object_unref (graph);
}

static GtkTargetEntry const targets[] = {
	{(char *) "application/x-goffice-graph",  0, 0},
	{(char *) "image/svg+xml", 0, 2},
	{(char *) "image/svg", 0, 1},
	{(char *) "image/png", 0, 3}
};

static void on_copy (GtkWidget *widget, GChemTableCurve *curve)
{
	curve->OnCopy ();
}

static void on_print (GtkWidget *widget, GChemTableCurve *curve)
{
	curve->Print (false);
}

static void on_print_preview (GtkWidget *widget, GChemTableCurve *curve)
{
	curve->Print (true);
}

static void on_page_setup (GtkWidget *widget, GChemTableCurve *curve)
{
	curve->OnPageSetup ();
}

void on_properties (GtkWidget *widget, GChemTableCurve *curve)
{
	curve->OnProperties ();
}

static void on_close (GtkWidget *widget, GChemTableCurve *curve)
{
	curve->Destroy ();
}

static void on_help (GtkWidget *widget, GChemTableCurve *curve)
{
	curve->GetApplication ()->OnHelp ();
}

static void on_curve_help (GtkWidget *widget, GChemTableCurve *curve)
{
	curve->Help ();
}

static void on_web (GtkWidget *widget, GChemTableCurve *curve)
{
	curve->GetApplication ()->OnWeb ();
}

static void on_mail (GtkWidget *widget, GChemTableCurve *curve)
{
	curve->GetApplication ()->OnMail ();
}

static void on_bug (GtkWidget *widget, GChemTableCurve *curve)
{
	curve->GetApplication ()->OnBug ();
}

static void on_about (GtkWidget *widget, GChemTableCurve *curve)
{
	curve->GetApplication ()->OnAbout ();
}

static GtkActionEntry entries[] = {
  { "FileMenu", NULL, N_("_File") },
	  { "PageSetup", NULL, N_("Page Set_up..."), NULL,
		  N_("Setup the page settings for your current printer"), G_CALLBACK (on_page_setup) },
	  { "PrintPreview", GTK_STOCK_PRINT_PREVIEW, N_("Print Pre_view"), NULL,
		  N_("Print preview"), G_CALLBACK (on_print_preview) },
	  { "Print", GTK_STOCK_PRINT, N_("_Print..."), "<control>P",
		  N_("Print the current file"), G_CALLBACK (on_print) },
	  { "Properties", GTK_STOCK_PROPERTIES, N_("Prope_rties..."), NULL,
		  N_("Modify the graph properties"), G_CALLBACK (on_properties) },
	  { "Close", GTK_STOCK_CLOSE, N_("_Close"), "<control>W",
		  N_("Close the current file"), G_CALLBACK (on_close) },
	  { "Quit", GTK_STOCK_QUIT, N_("_Quit"), "<control>Q",
		  N_("Quit GChemTable"), G_CALLBACK (gtk_main_quit) },
  { "EditMenu", NULL, N_("_Edit") },
	  { "Copy", GTK_STOCK_COPY, N_("_Copy"), "<control>C",
		  N_("Copy the selection"), G_CALLBACK (on_copy) },
  { "HelpMenu", NULL, N_("_Help") },
	  { "Help", GTK_STOCK_HELP, N_("_Contents"), "F1",
		  N_("View help for the Periodic Table"), G_CALLBACK (on_help) },
	  { "CurveHelp", GTK_STOCK_HELP, N_("_Help"), "<control>F1",
		  N_("View help for the Curve Window"), G_CALLBACK (on_curve_help) },
	  { "Web", NULL, N_("Gnome Chemistry Utils on the _web"), NULL,
		  N_("Browse the Gnome Chemistry Utils's web site"), G_CALLBACK (on_web) },
	  { "Mail", NULL, N_("_Ask a question"), NULL,
		  N_("Ask a question about the Gnome Chemistry Utils"), G_CALLBACK (on_mail) },
	  { "Bug", NULL, N_("Report _Bugs"), NULL,
		  N_("Submit a bug report for the Gnome Chemistry Utils"), G_CALLBACK (on_bug) },
	  { "About", GTK_STOCK_ABOUT, N_("_About"), NULL,
		  N_("About GChemTable"), G_CALLBACK (on_about) }
};

static const char *ui_description =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu action='FileMenu'>"
"      <menuitem action='PageSetup'/>"
"      <menuitem action='PrintPreview'/>"
"      <menuitem action='Print'/>"
"		<separator/>"
"      <menuitem action='Properties'/>"
"		<separator/>"
"      <menuitem action='Close'/>"
"      <menuitem action='Quit'/>"
"    </menu>"
"    <menu action='EditMenu'>"
"      <menuitem action='Copy'/>"
"    </menu>"
"    <menu action='HelpMenu'>"
"      <menuitem action='Help'/>"
"      <menuitem action='CurveHelp'/>"
"      <placeholder name='mail'/>"
"      <placeholder name='web'/>"
"      <placeholder name='bug'/>"
"      <menuitem action='About'/>"
"    </menu>"
"  </menubar>"
"</ui>";

static const char *ui_mail_description =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu action='HelpMenu'>"
"      <placeholder name='mail'>"
"        <menuitem action='Mail'/>"
"      </placeholder>"
"    </menu>"
"  </menubar>"
"</ui>";

static const char *ui_web_description =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu action='HelpMenu'>"
"      <placeholder name='web'>"
"        <menuitem action='Web'/>"
"      </placeholder>"
"      <placeholder name='bug'>"
"        <menuitem action='Bug'/>"
"      </placeholder>"
"    </menu>"
"  </menubar>"
"</ui>";

GChemTableCurve::GChemTableCurve (GChemTableApp *App, char const *name):
	Dialog (App, GLADEDIR"/curve.glade", "curvedlg"),
	Printable (),
	m_Guru (NULL)
{
	m_GraphBox = glade_xml_get_widget (xml, "vbox1");
	GtkUIManager *ui_manager = gtk_ui_manager_new ();
	GtkActionGroup *action_group = gtk_action_group_new ("MenuActions");
	gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);
	gtk_action_group_add_actions (action_group, entries, G_N_ELEMENTS (entries), this);
	gtk_ui_manager_insert_action_group (ui_manager, action_group, 0);
	GtkAccelGroup *accel_group = gtk_ui_manager_get_accel_group (ui_manager);
	gtk_window_add_accel_group (GTK_WINDOW (dialog), accel_group);
	GError *error = NULL;
	if (!gtk_ui_manager_add_ui_from_string (ui_manager, ui_description, -1, &error)) {
		g_message ("building menus failed: %s", error->message);
		g_error_free (error);
		exit (EXIT_FAILURE);
	}
	if (m_App->HasWebBrowser () && !gtk_ui_manager_add_ui_from_string (ui_manager, ui_web_description, -1, &error)) {
		g_message ("building menus failed: %s", error->message);
		g_error_free (error);
	}
	if (m_App->HasMailAgent () && !gtk_ui_manager_add_ui_from_string (ui_manager, ui_mail_description, -1, &error)) {
		g_message ("building menus failed: %s", error->message);
		g_error_free (error);
	}
	GtkWidget *bar = gtk_ui_manager_get_widget (ui_manager, "/MainMenu");
	gtk_box_pack_start (GTK_BOX (m_GraphBox), bar, FALSE, FALSE, 0);
	m_GraphWidget = go_graph_widget_new (NULL);
	gtk_widget_set_size_request (m_GraphWidget, 400, 250);
	gtk_widget_show (m_GraphWidget);
	gtk_box_pack_end (GTK_BOX (m_GraphBox), m_GraphWidget, TRUE, TRUE, 0);
	m_Graph = go_graph_widget_get_graph (GO_GRAPH_WIDGET (m_GraphWidget));
	GogChart *chart = go_graph_widget_get_chart (GO_GRAPH_WIDGET (m_GraphWidget));
	if (!name) {
		OnProperties ();
		return;
	}
	GogPlot *plot = (GogPlot *) gog_plot_new_by_name ("GogXYPlot");
	gog_object_add_by_name (GOG_OBJECT (chart), "Plot", GOG_OBJECT (plot));
	m_Name = name;
	// Create a series for the plot and populate it with some simple data
	GogSeries *series = gog_plot_new_series (plot);
	double *yvals = g_new0 (double, MAX_ELT);
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
	} else if (!strcmp (name, "covalent")) {
		Element *elt;
		GcuAtomicRadius r;
		r.type = GCU_COVALENT;
		r.charge = 0;
		r.scale = NULL;
		r.cn = -1;
		r.spin = GCU_N_A_SPIN;
		for (i = 1; i <= MAX_ELT; i++) {
			r.Z = i;
			elt = Element::GetElement (i);
			yvals[i - 1] = (elt && elt->GetRadius (&r))? r.value.value: go_nan;
		}
		obj = gog_object_get_child_by_role (GOG_OBJECT (chart),
				gog_object_find_role_by_name (GOG_OBJECT (chart), "Y-Axis"));
		data = go_data_scalar_str_new (_("Covalent radii"), FALSE);
		label = (GogObject*) g_object_new (GOG_LABEL_TYPE, NULL);
		gog_dataset_set_dim (GOG_DATASET (label), 0, data, &error);
		gog_object_add_by_name (obj, "Label", label);
		gtk_window_set_title (dialog, _("Covalent radii"));
	} else if (!strcmp (name, "vdw")) {
		Element *elt;
		GcuAtomicRadius r;
		r.type = GCU_VAN_DER_WAALS;
		r.charge = 0;
		r.scale = NULL;
		r.cn = -1;
		r.spin = GCU_N_A_SPIN;
		for (i = 1; i <= MAX_ELT; i++) {
			r.Z = i;
			elt = Element::GetElement (i);
			yvals[i - 1] = (elt && elt->GetRadius (&r))? r.value.value: go_nan;
		}
		obj = gog_object_get_child_by_role (GOG_OBJECT (chart),
				gog_object_find_role_by_name (GOG_OBJECT (chart), "Y-Axis"));
		data = go_data_scalar_str_new (_("Van der Waals radii"), FALSE);
		label = (GogObject*) g_object_new (GOG_LABEL_TYPE, NULL);
		gog_dataset_set_dim (GOG_DATASET (label), 0, data, &error);
		gog_object_add_by_name (obj, "Label", label);
		gtk_window_set_title (dialog, _("Van der Waals radii"));
	} else if (!strcmp (name, "metallic")) {
		Element *elt;
		GcuAtomicRadius r;
		r.type = GCU_METALLIC;
		r.charge = 0;
		r.scale = NULL;
		r.cn = -1;
		r.spin = GCU_N_A_SPIN;
		for (i = 1; i <= MAX_ELT; i++) {
			r.Z = i;
			elt = Element::GetElement (i);
			yvals[i - 1] = (elt && elt->GetRadius (&r))? r.value.value: go_nan;
		}
		obj = gog_object_get_child_by_role (GOG_OBJECT (chart),
				gog_object_find_role_by_name (GOG_OBJECT (chart), "Y-Axis"));
		data = go_data_scalar_str_new (_("Metallic radii"), FALSE);
		label = (GogObject*) g_object_new (GOG_LABEL_TYPE, NULL);
		gog_dataset_set_dim (GOG_DATASET (label), 0, data, &error);
		gog_object_add_by_name (obj, "Label", label);
		gtk_window_set_title (dialog, _("Metallic radii"));
	} else if (!strcmp (name, "mp")) {
		Element *elt;
		Value const *prop;
		for (i = 1; i <= MAX_ELT; i++) {
			elt = Element::GetElement (i);
			prop = elt->GetProperty ("meltingpoint");
			yvals[i - 1] = (prop)? prop->GetAsDouble (): go_nan;
		}
		obj = gog_object_get_child_by_role (GOG_OBJECT (chart),
				gog_object_find_role_by_name (GOG_OBJECT (chart), "Y-Axis"));
		data = go_data_scalar_str_new (_("Melting point"), FALSE);
		label = (GogObject*) g_object_new (GOG_LABEL_TYPE, NULL);
		gog_dataset_set_dim (GOG_DATASET (label), 0, data, &error);
		gog_object_add_by_name (obj, "Label", label);
		gtk_window_set_title (dialog, _("Melting point"));
	} else if (!strcmp (name, "bp")) {
		Element *elt;
		Value const *prop;
		for (i = 1; i <= MAX_ELT; i++) {
			elt = Element::GetElement (i);
			prop = elt->GetProperty ("boilingpoint");
			yvals[i - 1] = (prop)? prop->GetAsDouble (): go_nan;
		}
		obj = gog_object_get_child_by_role (GOG_OBJECT (chart),
				gog_object_find_role_by_name (GOG_OBJECT (chart), "Y-Axis"));
		data = go_data_scalar_str_new (_("Boiling point"), FALSE);
		label = (GogObject*) g_object_new (GOG_LABEL_TYPE, NULL);
		gog_dataset_set_dim (GOG_DATASET (label), 0, data, &error);
		gog_object_add_by_name (obj, "Label", label);
		gtk_window_set_title (dialog, _("Boiling point"));
	} else {
		gtk_widget_destroy (GTK_WIDGET (dialog));
		return;
	}
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
	gog_series_set_dim (series, 0, gct_data_vector_get_from_name (_("Atomic number")), &error);
	obj = gog_object_get_child_by_role (GOG_OBJECT (chart),
			gog_object_find_role_by_name (GOG_OBJECT (chart), "X-Axis"));
	data = go_data_scalar_str_new ("Z", FALSE);
	label = (GogObject*) g_object_new (GOG_LABEL_TYPE, NULL);
	gog_dataset_set_dim (GOG_DATASET (label), 0, data, &error);
	gog_object_add_by_name (obj, "Label", label);
}

GChemTableCurve::~GChemTableCurve ()
{
	if (m_Guru)
		gtk_widget_destroy (m_Guru);
	curves.erase (m_Name);
}

void GChemTableCurve::DoPrint (GtkPrintOperation *print, GtkPrintContext *context) const
{
	cairo_t *cr;
	gdouble width, height;

	cr = gtk_print_context_get_cairo_context (context);
	width = gtk_print_context_get_width (context);
	height = gtk_print_context_get_height (context);

	int w, h; // size in points
	w = m_GraphWidget->allocation.width;
	h = m_GraphWidget->allocation.height;
	switch (GetScaleType ()) {
	case GCU_PRINT_SCALE_NONE:
		break;
	case GCU_PRINT_SCALE_FIXED:
		w *= Printable::GetScale ();
		h *= Printable::GetScale ();
		break;
	case GCU_PRINT_SCALE_AUTO:
		if (GetHorizFit ())
			w = width;
		if (GetVertFit ())
			h = height;
		break;
	}
	double x = 0., y = 0.;
	if (GetHorizCentered ())
		x = (width - w) / 2.;
	if (GetVertCentered ())
		y = (height - h) / 2.;
	cairo_save (cr);
	cairo_translate (cr, x, y);
	gog_graph_render_to_cairo (m_Graph, cr, w, h);
	cairo_restore (cr);
}

void GChemTableCurve::OnPageSetup ()
{
	new PrintSetupDlg (m_App, this);
}

void GChemTableCurve::OnCopy ()
{
	GtkClipboard* clipboard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
	g_object_ref (m_Graph);
	gtk_clipboard_set_with_data (clipboard, targets, 4,
		(GtkClipboardGetFunc) on_get_data, (GtkClipboardClearFunc) on_clear_data, m_Graph);
}

static void
graph_user_config_free_data (gpointer data,
					  GClosure *closure)
{
	g_object_unref (data);
	closure->data = NULL;
}

static void
on_update_graph (GogGraph *graph, gpointer data)
{
	g_return_if_fail (IS_GOG_GRAPH (graph));
	GctControlGUI *tcg = GCT_CONTROL_GUI (data);
	GChemTableCurve *curve = gct_control_gui_get_owner (tcg);
	curve->SetGraph (graph);
}

void GChemTableCurve::OnProperties ()
{
	GctControlGUI *tcg = GCT_CONTROL_GUI (g_object_new (GCT_CONTROL_GUI_TYPE, NULL));
	gct_control_gui_set_owner (tcg, this);
	GClosure *closure = g_cclosure_new (G_CALLBACK (on_update_graph), tcg,
					(GClosureNotify) graph_user_config_free_data);
	m_Guru = gog_guru (m_Graph, GOG_DATA_ALLOCATOR (tcg), NULL, closure);
	gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (m_Guru));
	gtk_widget_show (m_Guru);
	g_closure_sink (closure);
}

void GChemTableCurve::SetGraph (GogGraph *graph)
{
	gtk_widget_destroy (m_GraphWidget);
	m_GraphWidget = go_graph_widget_new (graph);
	gtk_widget_show (m_GraphWidget);
	gtk_box_pack_end (GTK_BOX (m_GraphBox), m_GraphWidget, TRUE, TRUE, 0);
	m_Graph = go_graph_widget_get_graph (GO_GRAPH_WIDGET (m_GraphWidget));
}

