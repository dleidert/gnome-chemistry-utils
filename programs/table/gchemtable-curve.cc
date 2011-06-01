// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * programs/gchemtable-curve.cc 
 *
 * Copyright (C) 2005-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "gchemtable-curve.h"
#include "gchemtable-data.h"
#include "gchemtable-data-allocator.h"
#include <gcu/chemistry.h>
#include <gcu/element.h>
#include <gcugtk/print-setup-dlg.h>
#include <gsf/gsf-input-memory.h>
#include <gsf/gsf-output-memory.h>
#include <gsf/gsf-output-gio.h>
#include <glib/gi18n.h>
#include <map>
#include <cstring>
#include <sstream>

using namespace gcu;
using namespace std;

map<string, GChemTableCurve*> curves;
GObject *Copied;

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

static void on_get_data (G_GNUC_UNUSED GtkClipboard *clipboard, GtkSelectionData *selection_data,  guint info, GogGraph *graph)
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
		
			GogObject *graph_;
			graph_ = gog_object_dup (GOG_OBJECT (graph),
				NULL, gog_dataset_dup_to_simple);
			xout = gsf_xml_out_new (output);
			gog_object_write_xml_sax (GOG_OBJECT (graph_), xout, NULL);
			g_object_unref (xout);
			g_object_unref (graph_);
		
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
					gtk_selection_data_get_target (selection_data), 8,
					(guchar *) buffer, osize);
		g_free (buffer);
	}
}

void on_clear_data(G_GNUC_UNUSED GtkClipboard *clipboard, GogGraph *graph)
{
	Copied = NULL;
	g_object_unref (graph);
}

static GtkTargetEntry const targets[] = {
	{(char *) "application/x-goffice-graph",  0, 0},
	{(char *) "image/svg+xml", 0, 2},
	{(char *) "image/svg", 0, 1},
	{(char *) "image/png", 0, 3}
};

static void on_copy (G_GNUC_UNUSED GtkWidget *widget, GChemTableCurve *curve)
{
	curve->OnCopy ();
}

static void on_file_save_as_image(G_GNUC_UNUSED GtkWidget* widget, GChemTableCurve *curve)
{
	curve->GetApplication ()->OnSaveAsImage (curve);
}

static void on_print (G_GNUC_UNUSED GtkWidget *widget, GChemTableCurve *curve)
{
	curve->Print (false);
}

static void on_print_preview (G_GNUC_UNUSED GtkWidget *widget, GChemTableCurve *curve)
{
	curve->Print (true);
}

static void on_page_setup (G_GNUC_UNUSED GtkWidget *widget, GChemTableCurve *curve)
{
	curve->OnPageSetup ();
}

void on_properties (G_GNUC_UNUSED GtkWidget *widget, GChemTableCurve *curve)
{
	curve->OnProperties ();
}

static void on_close (G_GNUC_UNUSED GtkWidget *widget, GChemTableCurve *curve)
{
	curve->Destroy ();
}

static void on_help (G_GNUC_UNUSED GtkWidget *widget, GChemTableCurve *curve)
{
	curve->GetApplication ()->OnHelp ();
}

static void on_curve_help (G_GNUC_UNUSED GtkWidget *widget, GChemTableCurve *curve)
{
	curve->GetApplication ()->OnHelp (curve->GetWindowName ());
}

static void on_web (G_GNUC_UNUSED GtkWidget *widget, GChemTableCurve *curve)
{
	curve->GetApplication ()->OnWeb ();
}

static void on_live_assistance (G_GNUC_UNUSED GtkWidget *widget, GChemTableCurve *curve)
{
	curve->GetApplication ()->OnLiveAssistance ();
}

static void on_mail (G_GNUC_UNUSED GtkWidget *widget, GChemTableCurve *curve)
{
	curve->GetApplication ()->OnMail ();
}

static void on_bug (G_GNUC_UNUSED GtkWidget *widget, GChemTableCurve *curve)
{
	curve->GetApplication ()->OnBug ();
}

static void on_about (G_GNUC_UNUSED GtkWidget *widget, GChemTableCurve *curve)
{
	curve->GetApplication ()->OnAbout ();
}

static GtkActionEntry entries[] = {
  { "FileMenu", NULL, N_("_File"), NULL, NULL, NULL },
	  { "SaveAsImage", GTK_STOCK_SAVE_AS, N_("Save As _Image..."), "<control>I",
		  N_("Save the current file as an image"), G_CALLBACK (on_file_save_as_image) },
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
  { "EditMenu", NULL, N_("_Edit"), NULL, NULL, NULL },
	  { "Copy", GTK_STOCK_COPY, N_("_Copy"), "<control>C",
		  N_("Copy the selection"), G_CALLBACK (on_copy) },
  { "HelpMenu", NULL, N_("_Help"), NULL, NULL, NULL },
	  { "Help", GTK_STOCK_HELP, N_("_Contents"), "F1",
		  N_("View help for the Periodic Table"), G_CALLBACK (on_help) },
	  { "CurveHelp", GTK_STOCK_HELP, N_("_Help"), "<control>F1",
		  N_("View help for the Curve Window"), G_CALLBACK (on_curve_help) },
	  { "Web", NULL, N_("Gnome Chemistry Utils on the _web"), NULL,
		  N_("Browse the Gnome Chemistry Utils's web site"), G_CALLBACK (on_web) },
	  { "LiveAssistance", NULL, N_("Live assistance"), NULL,
		  N_("Open the Gnome Chemistry Utils IRC channel"), G_CALLBACK (on_live_assistance) },
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
"      <menuitem action='SaveAsImage'/>"
"		<separator/>"
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
"      <menuitem action='Mail'/>"
"      <menuitem action='Web'/>"
"      <menuitem action='LiveAssistance'/>"
"      <menuitem action='Bug'/>"
"      <menuitem action='About'/>"
"    </menu>"
"  </menubar>"
"</ui>";

GChemTableCurve::GChemTableCurve (GChemTableApp *App, char const *name):
	gcugtk::Dialog (App, UIDIR"/curve.ui", "curvedlg", GETTEXT_PACKAGE),
	gcugtk::Printable (),
	m_Guru (NULL)
{
	m_GraphBox = GetWidget ("curve-grid");
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
	GtkWidget *bar = gtk_ui_manager_get_widget (ui_manager, "/MainMenu");
	gtk_container_add (GTK_CONTAINER (m_GraphBox), bar);
	g_object_unref (ui_manager);
	m_GraphWidget = go_graph_widget_new (NULL);
	g_signal_connect_swapped (m_GraphWidget, "size-allocate", G_CALLBACK (GChemTableCurve::OnSize), this);
	g_signal_connect_swapped (m_GraphWidget, "motion-notify-event", G_CALLBACK (GChemTableCurve::OnMotion), this);
	gtk_widget_set_size_request (m_GraphWidget, 400, 250);
	g_object_set (G_OBJECT (m_GraphWidget), "expand", true, NULL);
	gtk_widget_show (m_GraphWidget);
	gtk_container_add (GTK_CONTAINER (m_GraphBox), m_GraphWidget);
	m_Graph = go_graph_widget_get_graph (GO_GRAPH_WIDGET (m_GraphWidget));
	GogChart *chart = go_graph_widget_get_chart (GO_GRAPH_WIDGET (m_GraphWidget));
	if (!name) {
		static unsigned id = 0;
		char *buf = g_strdup_printf ("Custom%u\n", id++);
		SetRealName (buf, App);
		g_free (buf);
		OnProperties ();
		return;
	}
	SetRealName (name, App);
	GogPlot *plot = (GogPlot *) gog_plot_new_by_name ("GogXYPlot");
	gog_object_add_by_name (GOG_OBJECT (chart), "Plot", GOG_OBJECT (plot));
	m_Name = name;
	// Create a series for the plot and populate it with some simple data
	GogSeries *series = gog_plot_new_series (plot);
	GogObject *obj, *label;
	GOData *data, *ydata = NULL;
	int i;
	// FIXME: find a better way to do the following things !
	if (!strcmp (name, "en-Pauling")) {
		ydata = gct_data_vector_get_from_name (_("Pauling electronegativity"));
		obj = gog_object_get_child_by_role (GOG_OBJECT (chart),
				gog_object_find_role_by_name (GOG_OBJECT (chart), "Y-Axis"));
		data = go_data_scalar_str_new (_("Pauling electronegativity"), FALSE);
		label = (GogObject*) g_object_new (GOG_TYPE_LABEL, NULL);
		gog_dataset_set_dim (GOG_DATASET (label), 0, data, &error);
		gog_object_add_by_name (obj, "Label", label);
		gtk_window_set_title (dialog, _("Pauling electronegativity"));
	} else if (!strcmp (name, "ae")) {
		ydata = gct_data_vector_get_from_name (_("Electronic affinity"));
		obj = gog_object_get_child_by_role (GOG_OBJECT (chart),
				gog_object_find_role_by_name (GOG_OBJECT (chart), "Y-Axis"));
		data = go_data_scalar_str_new (_("Electron affinity (kJ/mol)"), FALSE);
		label = (GogObject*) g_object_new (GOG_TYPE_LABEL, NULL);
		gog_dataset_set_dim (GOG_DATASET (label), 0, data, &error);
		gog_object_add_by_name (obj, "Label", label);
		gtk_window_set_title (dialog, _("Electron affinity"));
	} else if (!strncmp (name, "ei-", 3)) {
		unsigned rank = strtol (name + 3, NULL, 10);
		char *rk, *buf;		
		switch (rank) {
		case 1:
			ydata = gct_data_vector_get_from_name (_("First ionization energy"));
			rk = g_strdup (_("1st. "));
			break;
		case 2:
			ydata = gct_data_vector_get_from_name (_("Second ionization energy"));
			rk = g_strdup (_("2nd. "));
			break;
		case 3:
			ydata = gct_data_vector_get_from_name (_("Third ionization energy"));
			rk = g_strdup (_("3rd. "));
			break;
		default:
			rk = g_strdup_printf (_("%dth. "), rank);
			buf = g_strconcat (rk, "ionization energy", NULL);
//			ydata = gct_vector_get_ionization_energies (buf); // FIXME: not implemented
			g_free (buf);
			break;
		}
		buf = g_strconcat (rk, _("ionization energy (MJ/mol)"), NULL);
		obj = gog_object_get_child_by_role (GOG_OBJECT (chart),
				gog_object_find_role_by_name (GOG_OBJECT (chart), "Y-Axis"));
		data = go_data_scalar_str_new (buf, TRUE);
		label = (GogObject*) g_object_new (GOG_TYPE_LABEL, NULL);
		gog_dataset_set_dim (GOG_DATASET (label), 0, data, &error);
		gog_object_add_by_name (obj, "Label", label);
		buf = g_strconcat (rk, _("ionization energy"), NULL);
		gtk_window_set_title (dialog, buf);
		g_free (buf);
		g_free (rk);
	} else if (!strcmp (name, "covalent")) {
		ydata = gct_data_vector_get_from_name (_("Covalent radius"));
		obj = gog_object_get_child_by_role (GOG_OBJECT (chart),
				gog_object_find_role_by_name (GOG_OBJECT (chart), "Y-Axis"));
		data = go_data_scalar_str_new (_("Covalent radii"), FALSE);
		label = (GogObject*) g_object_new (GOG_TYPE_LABEL, NULL);
		gog_dataset_set_dim (GOG_DATASET (label), 0, data, &error);
		gog_object_add_by_name (obj, "Label", label);
		gtk_window_set_title (dialog, _("Covalent radii"));
	} else if (!strcmp (name, "vdw")) {
		ydata = gct_data_vector_get_from_name (_("Van der Waals radius"));
		obj = gog_object_get_child_by_role (GOG_OBJECT (chart),
				gog_object_find_role_by_name (GOG_OBJECT (chart), "Y-Axis"));
		data = go_data_scalar_str_new (_("Van der Waals radii"), FALSE);
		label = (GogObject*) g_object_new (GOG_TYPE_LABEL, NULL);
		gog_dataset_set_dim (GOG_DATASET (label), 0, data, &error);
		gog_object_add_by_name (obj, "Label", label);
		gtk_window_set_title (dialog, _("Van der Waals radii"));
	} else if (!strcmp (name, "metallic")) {
		ydata = gct_data_vector_get_from_name (_("Metallic radius"));
		obj = gog_object_get_child_by_role (GOG_OBJECT (chart),
				gog_object_find_role_by_name (GOG_OBJECT (chart), "Y-Axis"));
		data = go_data_scalar_str_new (_("Metallic radii"), FALSE);
		label = (GogObject*) g_object_new (GOG_TYPE_LABEL, NULL);
		gog_dataset_set_dim (GOG_DATASET (label), 0, data, &error);
		gog_object_add_by_name (obj, "Label", label);
		gtk_window_set_title (dialog, _("Metallic radii"));
	} else if (!strcmp (name, "mp")) {
		ydata = gct_data_vector_get_from_name (_("Fusion temperature"));
		obj = gog_object_get_child_by_role (GOG_OBJECT (chart),
				gog_object_find_role_by_name (GOG_OBJECT (chart), "Y-Axis"));
		data = go_data_scalar_str_new (_("Melting point"), FALSE);
		label = (GogObject*) g_object_new (GOG_TYPE_LABEL, NULL);
		gog_dataset_set_dim (GOG_DATASET (label), 0, data, &error);
		gog_object_add_by_name (obj, "Label", label);
		gtk_window_set_title (dialog, _("Melting point"));
	} else if (!strcmp (name, "bp")) {
		ydata = gct_data_vector_get_from_name (_("Ebullition temperature"));
		obj = gog_object_get_child_by_role (GOG_OBJECT (chart),
				gog_object_find_role_by_name (GOG_OBJECT (chart), "Y-Axis"));
		data = go_data_scalar_str_new (_("Boiling point"), FALSE);
		label = (GogObject*) g_object_new (GOG_TYPE_LABEL, NULL);
		gog_dataset_set_dim (GOG_DATASET (label), 0, data, &error);
		gog_object_add_by_name (obj, "Label", label);
		gtk_window_set_title (dialog, _("Boiling point"));
	} else {
		gtk_widget_destroy (GTK_WIDGET (dialog));
		return;
	}
	i = MAX_ELT - 1;
	if (ydata) {
		while (!go_finite (go_data_vector_get_value (GO_DATA_VECTOR (ydata), i)))
			i--;
		i++;
		gog_series_set_dim (series, 1, ydata, &error);
	}
	obj = gog_object_get_child_by_role (GOG_OBJECT (chart),
			gog_object_find_role_by_name (GOG_OBJECT (chart), "X-Axis"));
	data = go_data_scalar_val_new ((double) i);
	gog_dataset_set_dim (GOG_DATASET (obj), GOG_AXIS_ELEM_MAX, data, &error);
	gog_series_set_dim (series, 0, gct_data_vector_get_from_name (_("Atomic number")), &error);
	obj = gog_object_get_child_by_role (GOG_OBJECT (chart),
			gog_object_find_role_by_name (GOG_OBJECT (chart), "X-Axis"));
	data = go_data_scalar_str_new ("Z", FALSE);
	label = (GogObject*) g_object_new (GOG_TYPE_LABEL, NULL);
	gog_dataset_set_dim (GOG_DATASET (label), 0, data, &error);
	gog_object_add_by_name (obj, "Label", label);
}

GChemTableCurve::~GChemTableCurve ()
{
	if (m_Guru)
		gtk_widget_destroy (m_Guru);
	curves.erase (m_Name);
}

void GChemTableCurve::DoPrint (G_GNUC_UNUSED GtkPrintOperation *print, GtkPrintContext *context, G_GNUC_UNUSED int page) const
{
	cairo_t *cr;
	gdouble width, height;

	cr = gtk_print_context_get_cairo_context (context);
	width = gtk_print_context_get_width (context);
	height = gtk_print_context_get_height (context);

	int w, h; // size in points
	w = m_GraphWidth;
	h = m_GraphHeight;
	switch (GetScaleType ()) {
	case gcugtk::GCU_PRINT_SCALE_NONE:
		break;
	case gcugtk::GCU_PRINT_SCALE_FIXED:
		w *= Printable::GetScale ();
		h *= Printable::GetScale ();
		break;
	case gcugtk::GCU_PRINT_SCALE_AUTO:
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
	new gcugtk::PrintSetupDlg (GetApplication (), this);
}

void GChemTableCurve::OnCopy ()
{
	GtkClipboard* clipboard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
	Copied = G_OBJECT (g_object_ref (m_Graph));
	gtk_clipboard_set_with_data (clipboard, targets, 4,
		(GtkClipboardGetFunc) on_get_data, (GtkClipboardClearFunc) on_clear_data, m_Graph);
}

static void
graph_user_config_free_data (gpointer data,
					  GClosure *closure)
{
	GChemTableCurve *curve = gct_control_gui_get_owner (GCT_CONTROL_GUI (data));
	curve->ClearGuru ();
	g_object_unref (data);
	closure->data = NULL;
}

static void
on_update_graph (GogGraph *graph, gpointer data)
{
	g_return_if_fail (GOG_IS_GRAPH (graph));
	GctControlGUI *tcg = GCT_CONTROL_GUI (data);
	GChemTableCurve *curve = gct_control_gui_get_owner (tcg);
	curve->SetGraph (graph);
}

static void
on_guru_help (Application *app)
{
	app->OnHelp ("customize-curve");
}

void GChemTableCurve::OnProperties ()
{
	GctControlGUI *tcg = GCT_CONTROL_GUI (g_object_new (GCT_TYPE_CONTROL_GUI, NULL));
	gct_control_gui_set_owner (tcg, this);
	GClosure *closure = g_cclosure_new (G_CALLBACK (on_update_graph), tcg,
					(GClosureNotify) graph_user_config_free_data);
	m_Guru = gog_guru (m_Graph, GOG_DATA_ALLOCATOR (tcg), NULL, closure);
	gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (m_Guru));
	g_signal_connect_swapped (G_OBJECT (gog_guru_get_help_button (m_Guru)), "clicked", G_CALLBACK (on_guru_help), m_App);
	gtk_widget_show (m_Guru);
	g_closure_sink (closure);
}

void GChemTableCurve::SetGraph (GogGraph *graph)
{
	gtk_widget_destroy (m_GraphWidget);
	m_GraphWidget = go_graph_widget_new (graph);
	gtk_widget_show (m_GraphWidget);
	gtk_container_add (GTK_CONTAINER (m_GraphBox), m_GraphWidget);
	m_Graph = go_graph_widget_get_graph (GO_GRAPH_WIDGET (m_GraphWidget));
}

void GChemTableCurve::SaveAsImage (string const &filename, char const *mime_type, unsigned width, unsigned height) const
{
	char *fname = go_mime_to_image_format (mime_type);
	GOImageFormat format = go_image_get_format_from_name ((fname)? fname: filename.c_str ());
	if (format == GO_IMAGE_FORMAT_UNKNOWN)
		return;
	GError *error = NULL;
	GsfOutput *output = gsf_output_gio_new_for_uri (filename.c_str (), &error);
	if (error) {
		g_error_free (error);
		return;
	}
	GogGraph *graph = gog_graph_dup (m_Graph);
	gog_graph_set_size (graph, width, height);
	gog_graph_export_image (graph, format, output, -1., -1.);
	g_object_unref (graph);
}

void GChemTableCurve::OnSize (GChemTableCurve *curve, GtkAllocation *allocation)
{
	curve->m_GraphWidth = allocation->width;
	curve->m_GraphHeight = allocation->height;
}

bool GChemTableCurve::OnMotion (GChemTableCurve *curve, GdkEventMotion *event)
{
	GogRenderer *renderer = go_graph_widget_get_renderer (GO_GRAPH_WIDGET (curve->m_GraphWidget));
	GogView *view;
	GogObject *obj;
	GogSeries *series;
	int index;
	ostringstream buf;
	g_object_get (G_OBJECT (renderer), "view", &view, NULL);
	gog_view_get_view_at_point (view, event->x, event->y, &obj, NULL);
	if (!obj)
		goto tooltip;
	obj = gog_object_get_parent_typed (obj, GOG_TYPE_PLOT);
	if (!obj)
		goto tooltip;
	view = gog_view_find_child_view (view, obj);
	index = gog_plot_view_get_data_at_point (GOG_PLOT_VIEW (view), event->x, event->y, &series);
	if (index >=0) {
		buf << Element::GetElement (index + 1)->GetName ();
		gtk_widget_set_tooltip_text (curve->m_GraphWidget, buf.str ().c_str ());
		return true;
	}

tooltip:
	gtk_widget_set_tooltip_text (curve->m_GraphWidget, NULL);
	return true;
}
