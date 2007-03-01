// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * programs/gchemcalc.cc 
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include "config.h"
#include <gcu/application.h>
#include <gcu/element.h>
#include <gcu/formula.h>
#include <glib/gi18n.h>
#include <glade/glade.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkentry.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkmessagedialog.h>
#include <gtk/gtkuimanager.h>
#include <gtk/gtkaboutdialog.h>
#include <gtk/gtkstock.h>
#include <gtk/gtkbox.h>
#include <gtk/gtktreeview.h>
#include <gtk/gtkcellrenderertext.h>
#include <gtk/gtkclipboard.h>
#include <goffice/goffice.h>
#include <goffice/app/go-plugin.h>
#include <goffice/app/go-plugin-loader-module.h>
#include <goffice/data/go-data-simple.h>
#include <goffice/gtk/go-graph-widget.h>
#include <goffice/gtk/goffice-gtk.h>
#include <goffice/graph/gog-axis.h>
#include <goffice/graph/gog-data-set.h>
#include <goffice/graph/gog-object.h>
#include <goffice/graph/gog-plot.h>
#include <goffice/graph/gog-series.h>
#include <goffice/graph/gog-style.h>
#include <goffice/graph/gog-styled-object.h>
#include <goffice/utils/go-locale.h>
#include <goffice/utils/go-image.h>
#include <goffice/utils/go-line.h>
#include <goffice/utils/go-marker.h>
#include <gsf/gsf-input-memory.h>
#include <gsf/gsf-output-memory.h>
#include <libgnomevfs/gnome-vfs.h>
#include <math.h>
#include <iostream>

using namespace gcu;

using namespace std;

class GChemCalc: public Application
{
public:
	GChemCalc ();
	Formula formula;
	GtkLabel *markup, *raw, *weight, *mono, *monomass;
	GtkWidget *pattern_page;
	GogChart *chart;
	GogLabel *label;
	GogPlot *plot;
	GogSeries *series;
	GtkListStore *pclist;
};

GChemCalc::GChemCalc (): Application ("gchemcalc"),
formula ("")
{
}

GChemCalc *App;

static void on_quit (GtkWidget *widget, void *data)
{
	gtk_main_quit();
}

static void on_help (GtkWidget *widget, gpointer data)
{
	App->OnHelp ();
}

static void on_web (GtkWidget *widget, gpointer data)
{
	App->OnWeb ();
}

static void on_mail (GtkWidget *widget, gpointer data)
{
	App->OnMail ();
}

static void on_bug (GtkWidget *widget, gpointer data)
{
	App->OnBug ();
}

static void on_about_activate_url (GtkAboutDialog *about, const gchar *url, gpointer data)
{
	GnomeVFSResult error = gnome_vfs_url_show(url);
	if (error != GNOME_VFS_OK) {
		g_print("GnomeVFSResult while trying to launch URL in about dialog: error %u\n", error);
	}
}

static void on_about (GtkWidget *widget, void *data)
{
	const gchar * authors[] = {"Jean Bréfort", NULL};
	const gchar * comments = _("GChemCalc is a simple calculator for chemists");
	/* const gchar * documentors[] = {NULL}; */
	const gchar * copyright = _("Copyright © 2005-2007 Jean Bréfort\n");
	const gchar * license =
		"This program is free software; you can redistribute it and/or\n"
		"modify it under the terms of the GNU General Public License as\n"
		"published by the Free Software Foundation; either version 2 of the\n"
		"License, or (at your option) any later version.\n\n"
		"This program is distributed in the hope that it will be useful,\n"
		"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
		"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
		"GNU General Public License for more details.\n\n"
		"You should have received a copy of the GNU General Public License\n"
		"along with this program; if not, write to the Free Software\n"
		"Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02111-1307\n"
		"USA";
	
	gtk_about_dialog_set_url_hook(on_about_activate_url, NULL, NULL);

	/* Note to translators: replace the following string with the appropriate credits for you lang */
	const gchar * translator_credits = _("translator_credits");

	gtk_show_about_dialog (NULL,
	                       "name", "GChemCalc",
	                       "authors", authors,
	                       "comments", comments,
	                       "copyright", copyright,
	                       "license", license,
	                       "translator_credits", translator_credits,
	                       "version", VERSION,
	                       "website", "http://www.nongnu.org/gchemutils",
	                       NULL);
}

static void cb_entry_active (GtkEntry *entry, gpointer data)
{
	GError *error;
	try {
		char *format;
		App->formula.SetFormula (gtk_entry_get_text (entry));
		format = g_strconcat (_("Formula:"), " \t", App->formula.GetMarkup (), NULL);
		gtk_label_set_markup (App->markup, format);
		g_free (format);
		format = g_strconcat (_("Raw formula:"), " \t", App->formula.GetRawMarkup (), NULL);
		gtk_label_set_markup (App->raw, format);
		g_free (format);
		int prec;
		bool artificial;
		double weight = App->formula.GetMolecularWeight (prec, artificial);
		if (prec > 0) {
			format = g_strdup_printf ("%%0.%df",prec);
		} else {
			if (prec < 0) {
				// round the value to replace not significant figures by 0s.
				double offs = pow10 (prec);
				weight = rint (weight * offs) / offs;
			}
			format = artificial? g_strdup ("(%.0f)"): g_strdup ("%.0f");
		}
		char *weightstr = g_strdup_printf (format, weight);
		gtk_label_set_text (App->weight, weightstr);
		g_free (weightstr);
		g_free (format);
		// Composition
		gtk_list_store_clear (App->pclist);
		map<int,int> &raw = App->formula.GetRawFormula ();
		map<int,int>::iterator ri, riend = raw.end ();
		double pcent;
		map<string, int> elts;
		int nC = 0, nH = 0;
		map<int, int>::iterator j, jend = raw.end();
		for (j = raw.begin (); j != jend; j++) {
			switch ((*j).first) {
			case 1:
				nH = (*j).second;
				break;
			case 6:
				nC = (*j).second;
				break;
			default:
				elts[Element::Symbol((*j).first)] = (*j).second;
				break;
			}
		}
		GtkTreeIter iter;
		Element *elt;
		if (nC > 0) {
			elt = Element::GetElement (6);
			pcent = nC * elt->GetWeight (prec) / weight * 100.;
			weightstr = g_strdup_printf ((artificial)? "(%.0f)": "%.2f", pcent);
			gtk_list_store_append (App->pclist, &iter);
			gtk_list_store_set (App->pclist, &iter,
					  0, "C",
					  1, weightstr,
					  -1);
			g_free (weightstr);
		}
		if (nH > 0) {
			elt = Element::GetElement (1);
			pcent = nH * elt->GetWeight (prec) / weight * 100.;
			weightstr = g_strdup_printf ((artificial)? "(%.0f)": "%.2f", pcent);
			gtk_list_store_append (App->pclist, &iter);
			gtk_list_store_set (App->pclist, &iter,
					  0, "H",
					  1, weightstr,
					  -1);
			g_free (weightstr);
		}
		map<string, int>::iterator k, kend = elts.end ();
		for (k = elts.begin (); k != kend; k++) {
			nC = (*k).second;
			elt = Element::GetElement ((*k).first.c_str ());
			pcent = nC * elt->GetWeight (prec) / weight * 100.;
			weightstr = g_strdup_printf ((artificial)? "(%.0f)": "%.2f", pcent);
			gtk_list_store_append (App->pclist, &iter);
			gtk_list_store_set (App->pclist, &iter,
					  0, (*k).first.c_str (),
					  1, weightstr,
					  -1);
			g_free (weightstr);
		}
		// Isotopic pattern
		IsotopicPattern pattern;
		App->formula.CalculateIsotopicPattern (pattern);
		double *values, *x, *y;
		int n, mass, nb, min, max, i;
		mass = pattern.GetMinMass ();
		if (mass == 0) {
			// invalid pattern, do not display anything
			gtk_widget_hide (App->pattern_page);
			return;
		} else {
			weightstr = g_strdup_printf ("%g", pattern.GetMonoMass ());
			gtk_label_set_text (App->monomass, weightstr);
			g_free (weightstr);
			gtk_widget_show (App->pattern_page);
			nb = pattern.GetValues (&values);
			// correct mean mass (for high molecular weights)
			double t = 0., m = 0;
			for (i = 0; i < nb; i++) {
				pcent = values[i] / nb;
				t += pcent;
				m += i * pcent;
			}
			mass = (int) rint (weight - m / t);
			// do not display values < 0.1
			min = 0;
			while (values[min] < 0.1)
				min++;
			max = nb - 1;
			while (values[max] < 0.1)
				max--;
			max = max - min + 1;
			x = g_new (double, max);
			y  = g_new (double, max);
			for (i = 0, n = min; i < max; i++, n++) {
				x[i] = mass + n;
				y[i] = values[n];
			}
			GOData *data = go_data_vector_val_new (x, max, g_free);
			gog_series_set_dim (App->series, 0, data, &error);
			data = go_data_vector_val_new (y, max, g_free);
			gog_series_set_dim (App->series, 1, data, &error);
			g_free (values);
			// set axis bounds
			if (max - min < 30) {
				n = (30 - max + min) / 2;
				max += n;
				min -= n;
				if (mass + min < 0) {
					max -= mass + min;
					min = - mass;
				}
			}
			nb = (mass + min) / 10 * 10;
			n = (mass + min + max + 10) / 10 * 10;
			GogObject *obj = gog_object_get_child_by_role (GOG_OBJECT (App->chart),
					gog_object_find_role_by_name (GOG_OBJECT (App->chart), "X-Axis"));
			data = go_data_scalar_val_new (nb);
			gog_dataset_set_dim (GOG_DATASET (obj), GOG_AXIS_ELEM_MIN, data, &error);
			data = go_data_scalar_val_new (n);
			gog_dataset_set_dim (GOG_DATASET (obj), GOG_AXIS_ELEM_MAX, data, &error);
		}
	}
	catch (parse_error &error) {
		int start, length;
		char const *mess = error.what (start, length);
		gtk_editable_select_region (GTK_EDITABLE (entry), start, start + length);
		GtkWidget *w = gtk_message_dialog_new (GTK_WINDOW (data),
							GTK_DIALOG_DESTROY_WITH_PARENT,
							GTK_MESSAGE_ERROR,
							GTK_BUTTONS_OK,
							mess);
		g_signal_connect_swapped (G_OBJECT (w), "response", G_CALLBACK (gtk_widget_destroy), G_OBJECT (w));
		gtk_widget_show (w);
	}
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

static void on_copy (GOGraphWidget *widget)
{
	GtkClipboard* clipboard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
	gtk_clipboard_set_with_data (clipboard, targets, 4,
		(GtkClipboardGetFunc) on_get_data, (GtkClipboardClearFunc) on_clear_data,
		gog_object_dup (GOG_OBJECT (go_graph_widget_get_graph (widget)), NULL, NULL));
}

static GtkActionEntry entries[] = {
  { "FileMenu", NULL, N_("_File") },
	  { "Quit", GTK_STOCK_QUIT, N_("_Quit"), "<control>Q",
		  N_("Quit GChemCalc"), G_CALLBACK (on_quit) },
  { "HelpMenu", NULL, N_("_Help") },
	  { "Help", GTK_STOCK_HELP, N_("_Contents"), "F1",
		  N_("View help for the Chemical Calculator"), G_CALLBACK (on_help) },
	  { "Web", NULL, N_("Gnome Chemistry Utils on the _web"), NULL,
		  N_("Browse the Gnome Chemistry Utils's web site"), G_CALLBACK (on_web) },
	  { "Mail", NULL, N_("_Ask a question"), NULL,
		  N_("Ask a question about the Gnome Chemistry Utils"), G_CALLBACK (on_mail) },
	  { "Bug", NULL, N_("Report _Bugs"), NULL,
		  N_("Submit a bug report for the Gnome Chemistry Utils"), G_CALLBACK (on_bug) },
	  { "About", GTK_STOCK_ABOUT, N_("_About"), NULL,
		  N_("About GChemCalc"), G_CALLBACK (on_about) }
};

static const char *ui_description =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu action='FileMenu'>"
"      <menuitem action='Quit'/>"
"    </menu>"
"    <menu action='HelpMenu'>"
"      <menuitem action='Help'/>"
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

gboolean cb_print_version (const gchar *option_name, const gchar *value, gpointer data, GError **error)
{
	char *version = g_strconcat (_("GChemCalc Calculator version: "), VERSION, NULL);
	puts (version);
	g_free (version);
	exit (0);
	return TRUE;
}

static GOptionEntry options[] = 
{
  { "version", 'v', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK, (void*) cb_print_version, "prints GChemCalc version", NULL },
   { NULL }
};

int main (int argc, char *argv[])
{
	GOptionContext *context;
	GError *error = NULL;
	textdomain (GETTEXT_PACKAGE);
	gtk_init (&argc, &argv);
	gnome_vfs_init ();
	if (argc > 1 && argv[1][0] == '-') {
		context = g_option_context_new (_(" [formula]"));
		g_option_context_add_main_entries (context, options, GETTEXT_PACKAGE);
		g_option_context_add_group (context, gtk_get_option_group (TRUE));
		g_option_context_set_help_enabled (context, TRUE);
		g_option_context_parse (context, &argc, &argv, &error);
		if (error) {
			puts (error->message);
			g_error_free (error);
			return -1;
		}
	} else {
		argc --;
		argv ++;
	}

#warning "the following line should be edited for stable releases"
	if (argc > 1) {
		cout << _("For usage see: gchemcalc [-?|--help]") << endl;
		return -1;
	}
	
	/* Initialize libgoffice */
	libgoffice_init ();
	/* Initialize plugins manager */
	go_plugins_init (NULL, NULL, NULL, NULL, TRUE, GO_PLUGIN_LOADER_MODULE_TYPE);

	App = new GChemCalc ();

	GladeXML *xml =  glade_xml_new (GLADEDIR"/gchemcalc.glade", "gchemcalc", NULL);
	GtkWidget *window = glade_xml_get_widget (xml, "gchemcalc");
	g_signal_connect (GTK_OBJECT (window), "destroy",
		 G_CALLBACK (gtk_main_quit),
		 NULL);
	
	GtkWidget *vbox = glade_xml_get_widget (xml, "vbox1");
	GtkUIManager *ui_manager = gtk_ui_manager_new ();
	GtkActionGroup *action_group = gtk_action_group_new ("MenuActions");
	gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);
	gtk_action_group_add_actions (action_group, entries, G_N_ELEMENTS (entries), NULL);
	gtk_ui_manager_insert_action_group (ui_manager, action_group, 0);
	GtkAccelGroup *accel_group = gtk_ui_manager_get_accel_group (ui_manager);
	gtk_window_add_accel_group (GTK_WINDOW (window), accel_group);
	if (!gtk_ui_manager_add_ui_from_string (ui_manager, ui_description, -1, &error)) {
		g_message ("building menus failed: %s", error->message);
		g_error_free (error);
		exit (EXIT_FAILURE);
	}
	if (App->HasWebBrowser () && !gtk_ui_manager_add_ui_from_string (ui_manager, ui_web_description, -1, &error)) {
		g_message ("building menus failed: %s", error->message);
		g_error_free (error);
	}
	if (App->HasMailAgent () && !gtk_ui_manager_add_ui_from_string (ui_manager, ui_mail_description, -1, &error)) {
		g_message ("building menus failed: %s", error->message);
		g_error_free (error);
	}
	GtkWidget *bar = gtk_ui_manager_get_widget (ui_manager, "/MainMenu");
	gtk_box_pack_start (GTK_BOX (vbox), bar, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (vbox), bar, 0);
	App->markup = GTK_LABEL (glade_xml_get_widget (xml, "markup"));
	App->raw = GTK_LABEL (glade_xml_get_widget (xml, "raw"));
	App->weight = GTK_LABEL (glade_xml_get_widget (xml, "weight"));

	//Add composition list
	App->pclist = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
	GtkTreeView *tree = GTK_TREE_VIEW (glade_xml_get_widget (xml, "composition"));
	gtk_tree_view_set_model (tree, GTK_TREE_MODEL (App->pclist));
	g_object_unref (App->pclist);
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	/* column for element */
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Element"), renderer, "text", 0, NULL);
	/* set this column to a minimum sizing (of 100 pixels) */
	gtk_tree_view_column_set_sizing(GTK_TREE_VIEW_COLUMN (column), GTK_TREE_VIEW_COLUMN_GROW_ONLY);
	gtk_tree_view_column_set_min_width(GTK_TREE_VIEW_COLUMN (column), 100);
	gtk_tree_view_append_column (tree, column);
	/* column for x */
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Mass %"), renderer, "text", 1, NULL);
	gtk_tree_view_column_set_alignment (column, 1.0);
	g_object_set (G_OBJECT (renderer), "xalign", 1.0, NULL);
	/* set this column to a fixed sizing (of 150 pixels) */
	gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column), GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 150);
	gtk_tree_view_append_column (tree, column);
	
	// Add isotopic pattern chart
	App->mono = GTK_LABEL (glade_xml_get_widget (xml, "mono"));
	App->monomass = GTK_LABEL (glade_xml_get_widget (xml, "monomass"));
	App->pattern_page = glade_xml_get_widget (xml, "pattern");
#ifdef GO_GRAPH_WIDGET_OLD_API
	GtkWidget *pw = go_graph_widget_new ();
#else
	GtkWidget *pw = go_graph_widget_new (NULL);
#endif
	gtk_widget_show (pw);
	gtk_box_pack_end (GTK_BOX (App->pattern_page), pw, TRUE, TRUE, 0);
	App->chart = go_graph_widget_get_chart (GO_GRAPH_WIDGET (pw));
	App->plot = (GogPlot *) gog_plot_new_by_name ("GogXYPlot");
	gog_object_add_by_name (GOG_OBJECT (App->chart), "Plot", GOG_OBJECT (App->plot));
	// Create a series for the plot and populate it with some simple data
	App->series = gog_plot_new_series (App->plot);
	gog_object_add_by_name (GOG_OBJECT (App->series), "Vertical drop lines", NULL);
	GogStyle *style = gog_styled_object_get_style (GOG_STYLED_OBJECT (App->series));
	go_marker_set_shape (style->marker.mark, GO_MARKER_NONE);
	style->marker.auto_shape = false;
	style->line.dash_type = GO_LINE_NONE;
	style->line.auto_dash = false;
	GogObject *obj = gog_object_get_child_by_role (GOG_OBJECT (App->chart),
			gog_object_find_role_by_name (GOG_OBJECT (App->chart), "Y-Axis"));
	GOData *data = go_data_scalar_val_new (100.);
	gog_dataset_set_dim (GOG_DATASET (obj), GOG_AXIS_ELEM_MAX, data, &error);

	gtk_widget_hide (App->pattern_page);
	GtkWidget *w = glade_xml_get_widget (xml, "entry");
	g_signal_connect (GTK_OBJECT (w), "activate",
		 G_CALLBACK (cb_entry_active),
		 window);
	gcu_element_load_databases ("isotopes", NULL);
	if (argc == 1){
		gtk_entry_set_text (GTK_ENTRY (w), argv[0]);
		cb_entry_active (GTK_ENTRY (w), window);
	}

	w = glade_xml_get_widget (xml, "copy");
	g_signal_connect_swapped (w, "clicked", G_CALLBACK (on_copy), pw);

	gtk_main ();
	delete App;
	return 0;
}
