// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * programs/calc/gchemcalc.cc
 *
 * Copyright (C) 2005-2012 Jean Bréfort <jean.brefort@normalesup.org>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include "config.h"
#include <gcugtk/application.h>
#include <gcugtk/filechooser.h>
#include <gcugtk/message.h>
#include <gcugtk/printable.h>
#include <gcugtk/print-setup-dlg.h>
#include <gcugtk/ui-builder.h>
#include <gcu/atom.h>
#include <gcu/bond.h>
#include <gcu/document.h>
#include <gcu/element.h>
#include <gcu/formula.h>
#include <gcu/molecule.h>
#include <gcu/residue.h>
#include <gcu/value.h>
#include <glib/gi18n.h>
#include <gsf/gsf-input-memory.h>
#include <gsf/gsf-output-memory.h>
#include <gsf/gsf-output-gio.h>
#include <libxml/tree.h>
#include <iostream>
#include <cmath>
#include <cstring>

using namespace gcu;

using namespace std;

class GChemCalc: public gcugtk::Application, public gcugtk::Printable
{
public:
	GChemCalc ();
	~GChemCalc ();
	Formula formula;
	GtkLabel *markup, *raw, *weight, *mono, *monomass;
	GtkWidget *graph_widget;
	GtkWidget *pattern_page;
	GtkWindow *window;
	GogGraph *graph;
	GogChart *chart;
	GogLabel *label;
	GogPlot *plot;
	GogSeries *series;
	GtkListStore *pclist;

	GtkWindow *GetGtkWindow () {return window;}
	void DoPrint (GtkPrintOperation *print, GtkPrintContext *context, int page) const;
	void OnSaveAsImage ();
	bool FileProcess (const gchar* filename, const gchar* mime_type, bool bSave, GtkWindow *window, Document *pDoc = NULL);
	static void OnSize (GChemCalc *calc, GtkAllocation *allocation);

private:
	void ParseNodes (xmlNodePtr node);
	int m_GraphWidth, m_GraphHeight;
GCU_PROP (GtkUIManager *, UIManager)
};

// need a way to create atoms, bonds, and molecules for ambiguous symbols evaluation
static Object* CreateAtom ()
{
	return new Atom ();
}

static Object* CreateBond ()
{
	return new Bond ();
}

static Object* CreateMolecule ()
{
	return new Molecule ();
}

static Object* CreateDocument ()
{
	return new Document ();
}

GChemCalc::GChemCalc ():
	gcugtk::Application ("gchemcalc"),
	formula ("")
{
	AddType ("atom", CreateAtom, AtomType);
	AddType ("pseudo-atom", CreateAtom);
	AddType ("bond", CreateBond, BondType);
	AddType ("molecule", CreateMolecule, MoleculeType);
	AddType ("document", CreateDocument, DocumentType);
	// Load residues
	xmlDocPtr doc;
	char *name;
	doc = xmlParseFile (PKGDATADIR"/residues.xml");
	if (doc) {
		if (!strcmp ((char*) doc->children->name, "residues"))
			ParseNodes (doc->children->children);
		xmlFreeDoc (doc);
	}
	name = g_strconcat (getenv ("HOME"), "/.gchemutils/residues.xml", NULL);
	if (g_file_test (name, G_FILE_TEST_EXISTS) && (doc = xmlParseFile (name))) {
		if (!strcmp ((char*) doc->children->name, "residues"))
			ParseNodes (doc->children->children);
		xmlFreeDoc (doc);
	}
	g_free (name);
}

GChemCalc::~GChemCalc ()
{
	g_object_unref (m_UIManager);
}

void GChemCalc::ParseNodes (xmlNodePtr node)
{
	Residue* r;
	while (node) {
		if (!strcmp ((char*) node->name, "residue")) {
			r = new Residue ();
			r->Load (node, this);
		}
		node = node->next;
	}
}

void GChemCalc::DoPrint (G_GNUC_UNUSED GtkPrintOperation *print, GtkPrintContext *context, G_GNUC_UNUSED int page) const
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
	gog_graph_render_to_cairo (graph, cr, w, h);
	cairo_restore (cr);
}

void GChemCalc::OnSaveAsImage ()
{
	list<string> l;
	char const *mime;
	map<string, GdkPixbufFormat*>::iterator i, end = m_SupportedPixbufFormats.end ();
	for (i = m_SupportedPixbufFormats.begin (); i != end; i++)
		l.push_front ((*i).first.c_str ());
	if (go_image_get_format_from_name ("eps") != GO_IMAGE_FORMAT_UNKNOWN) {
		mime = go_image_format_to_mime ("eps");
		if (mime)
			l.push_front (mime);
	}
	l.push_front ("application/postscript");
	l.push_front ("application/pdf");
	l.push_front ("image/svg+xml");
	gcugtk::FileChooser (this, true, l, NULL, _("Save as image"), GetImageSizeWidget ());
}

GChemCalc *App;

bool GChemCalc::FileProcess (const gchar* filename, const gchar* mime_type, bool bSave, G_GNUC_UNUSED GtkWindow *window, G_GNUC_UNUSED Document *pDoc)
{
	if(bSave) {
		GFile *file = g_file_new_for_uri (filename);
		bool err = g_file_query_exists (file, NULL);
		gint result = GTK_RESPONSE_YES;
		if (err) {
			char *unescaped = g_uri_unescape_string (filename, NULL);
			gchar * message = g_strdup_printf (_("File %s\nexists, overwrite?"), unescaped);
			g_free (unescaped);
			gcugtk::Message *box = new gcugtk::Message (static_cast < gcugtk::Application * > (App),
			                                            message, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, App->GetGtkWindow ());
			box->Run ();
			g_free (message);
		}
		if (result == GTK_RESPONSE_YES) {
			g_file_delete (file, NULL, NULL);
			char *fname = go_mime_to_image_format (mime_type);
			GOImageFormat format = go_image_get_format_from_name ((fname)? fname: filename);
			if (format == GO_IMAGE_FORMAT_UNKNOWN)
				return true;
			GError *error = NULL;
			GsfOutput *output = gsf_output_gio_new_for_uri (filename, &error);
			if (error) {
				g_error_free (error);
				return true;
			}
			GogGraph *gr = gog_graph_dup (graph);
			gog_graph_set_size (gr, GetImageWidth (), GetImageHeight ());
			gog_graph_export_image (gr, format, output, -1., -1.);
			g_object_unref (gr);

		}
		g_object_unref (file);
	}
	return false;
}

void GChemCalc::OnSize (GChemCalc *calc, GtkAllocation *allocation)
{
	calc->m_GraphWidth = allocation->width;
	calc->m_GraphHeight = allocation->height;
}

static void on_quit (G_GNUC_UNUSED GtkWidget *widget, G_GNUC_UNUSED void *data)
{
	gtk_widget_destroy (GTK_WIDGET (App->window));
}

static void on_help (G_GNUC_UNUSED GtkWidget *widget, G_GNUC_UNUSED gpointer data)
{
	App->OnHelp ();
}

static void on_web (GtkWidget *widget, G_GNUC_UNUSED gpointer data)
{
	App->OnWeb (gtk_widget_get_screen (widget));
}

static void on_mail (GtkWidget *widget, G_GNUC_UNUSED gpointer data)
{
	App->OnMail (gtk_widget_get_screen (widget));
}

static void on_live_assistance (GtkWidget *widget, G_GNUC_UNUSED gpointer dataw)
{
	App->OnLiveAssistance (gtk_widget_get_screen (widget));
}

static void on_bug (GtkWidget *widget, G_GNUC_UNUSED gpointer data)
{
	App->OnBug (gtk_widget_get_screen (widget));
}

static void on_about (G_GNUC_UNUSED GtkWidget *widget, G_GNUC_UNUSED void *data)
{
	const gchar * authors[] = {"Jean Bréfort", NULL};
	const gchar * comments = _("GChemCalc is a simple calculator for chemists");
	/* const gchar * documentors[] = {NULL}; */
	const gchar * copyright = _("Copyright © 2005-2010 Jean Bréfort");
	const gchar * license =
		"This program is free software; you can redistribute it and/or\n"
		"modify it under the terms of the GNU General Public License as\n"
		"published by the Free Software Foundation; either version 3 of the\n"
		"License, or (at your option) any later version.\n\n"
		"This program is distributed in the hope that it will be useful,\n"
		"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
		"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
		"GNU General Public License for more details.\n\n"
		"You should have received a copy of the GNU General Public License\n"
		"along with this program; if not, write to the Free Software\n"
		"Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02111-1307\n"
		"USA";

	/* Note to translators: replace the following string with the appropriate credits for you lang */
	const gchar * translator_credits = _("translator_credits");

	gtk_show_about_dialog (NULL,
	                       "program-name", "GChemCalc",
	                       "authors", authors,
	                       "comments", comments,
	                       "copyright", copyright,
	                       "license", license,
	                       "translator_credits", translator_credits,
	                       "version", VERSION,
	                       "website", "http://gchemutils.nongnu.org",
	                       NULL);
}

static void clear_values (double *values)
{
	delete [] values;
}

static void cb_entry_active (GtkEntry *entry, gpointer)
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
		bool artificial;
		DimensionalValue weight = App->formula.GetMolecularWeight (artificial);
		char *weightstr = (artificial)?
			g_strdup_printf ("(%.0f g.mol<sup>-1</sup>)",weight.GetAsDouble ()):
			g_strdup (weight.GetAsString ());
		gtk_label_set_markup (App->weight, weightstr);
		g_free (weightstr);
		// Composition
		gtk_list_store_clear (App->pclist);
		map<int,int> &raw = App->formula.GetRawFormula ();
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
			pcent = nC * elt->GetWeight ()->GetAsDouble () / weight.GetAsDouble () * 100.;
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
			pcent = nH * elt->GetWeight ()->GetAsDouble () / weight.GetAsDouble () * 100.;
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
			pcent = nC * elt->GetWeight ()->GetAsDouble () / weight.GetAsDouble () * 100.;
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
			gtk_label_set_text (App->monomass, pattern.GetMonoMass ().GetAsString ());
			gtk_widget_show (App->pattern_page);
			nb = pattern.GetValues (&values);
			// correct mean mass (for high molecular weights)
			double t = 0., m = 0;
			for (i = 0; i < nb; i++) {
				pcent = values[i] / nb;
				t += pcent;
				m += i * pcent;
			}
			mass = (int) rint (weight.GetAsDouble () - m / t);
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
			GOData *data = go_data_vector_val_new (x, max, reinterpret_cast <GDestroyNotify> (clear_values));
			gog_series_set_dim (App->series, 0, data, &error);
			data = go_data_vector_val_new (y, max, reinterpret_cast <GDestroyNotify> (clear_values));
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
		gcugtk::Message *box = new gcugtk::Message (static_cast < gcugtk::Application * > (App),
		                                            mess, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, App->GetGtkWindow ());
		box->Show ();
	}
}

static GogGraph *graph = NULL;

static void on_get_data (G_GNUC_UNUSED GtkClipboard *clipboard, GtkSelectionData *selection_data,  guint info, G_GNUC_UNUSED gpointer data)
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
			gog_object_write_xml_sax (GOG_OBJECT (graph), xout, NULL);
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
					gtk_selection_data_get_target (selection_data), 8,
					(guchar *) buffer, osize);
		g_free (buffer);
	}
}

void on_clear_data(G_GNUC_UNUSED GtkClipboard *clipboard, G_GNUC_UNUSED gpointer data)
{
	g_object_unref (graph);
	graph = NULL;
}

static GtkTargetEntry const targets[] = {
	{(char *) "application/x-goffice-graph",  0, 0},
	{(char *) "image/svg+xml", 0, 2},
	{(char *) "image/svg", 0, 1},
	{(char *) "image/png", 0, 3}
};

static void on_copy ()
{
	GtkClipboard* clipboard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
	if (graph)
		g_object_unref (graph);
	graph = (GogGraph*) gog_object_dup (GOG_OBJECT (go_graph_widget_get_graph (GO_GRAPH_WIDGET (App->graph_widget))), NULL, NULL);
	gtk_clipboard_set_with_data (clipboard, targets, 4,
		(GtkClipboardGetFunc) on_get_data, (GtkClipboardClearFunc) on_clear_data,
		NULL);
}

static void on_print ()
{
	App->Print (false);
}

static void on_print_preview ()
{
	App->Print (true);
}

static void on_page_setup ()
{
	new gcugtk::PrintSetupDlg (App, App);
}

static void on_save_as_image ()
{
	App->OnSaveAsImage ();
}

static void on_mode (GtkRadioAction *action, G_GNUC_UNUSED GtkRadioAction *current)
{
	App->formula.SetParseMode (static_cast <FormulaParseMode> (gtk_radio_action_get_current_value (action)));
}

static void on_page (G_GNUC_UNUSED GtkNotebook *book, G_GNUC_UNUSED void *p, int page)
{
	gtk_widget_set_sensitive (gtk_ui_manager_get_widget (App->GetUIManager (), "/MainMenu/FileMenu/SaveAsImage"), page);
	gtk_widget_set_sensitive (gtk_ui_manager_get_widget (App->GetUIManager (), "/MainMenu/FileMenu/PageSetup"), page);
	gtk_widget_set_sensitive (gtk_ui_manager_get_widget (App->GetUIManager (), "/MainMenu/FileMenu/PrintPreview"), page);
	gtk_widget_set_sensitive (gtk_ui_manager_get_widget (App->GetUIManager (), "/MainMenu/FileMenu/Print"), page);
	gtk_widget_set_sensitive (gtk_ui_manager_get_widget (App->GetUIManager (), "/MainMenu/EditMenu/Copy"), page);
}

static GtkActionEntry entries[] = {
  { "FileMenu", NULL, N_("_File"), NULL, NULL, NULL },
	  { "SaveAsImage", GTK_STOCK_SAVE_AS, N_("Save As _Image..."), "<control>I",
		  N_("Save the current file as an image"), G_CALLBACK (on_save_as_image) },
	  { "PageSetup", NULL, N_("Page Set_up..."), NULL,
		  N_("Setup the page settings for your current printer"), G_CALLBACK (on_page_setup) },
	  { "PrintPreview", GTK_STOCK_PRINT_PREVIEW, N_("Print Pre_view"), NULL,
		  N_("Print preview"), G_CALLBACK (on_print_preview) },
	  { "Print", GTK_STOCK_PRINT, N_("_Print..."), "<control>P",
		  N_("Print the current file"), G_CALLBACK (on_print) },
	  { "Quit", GTK_STOCK_QUIT, N_("_Quit"), "<control>Q",
		  N_("Quit GChemCalc"), G_CALLBACK (on_quit) },
  { "EditMenu", NULL, N_("_Edit"), NULL, NULL, NULL },
	  { "Copy", GTK_STOCK_COPY, N_("_Copy"), "<control>C",
		  N_("Copy the selection"), G_CALLBACK (on_copy) },
  { "ModeMenu", NULL, N_("_Mode"), NULL, NULL, NULL },
  { "HelpMenu", NULL, N_("_Help"), NULL, NULL, NULL },
	  { "Help", GTK_STOCK_HELP, N_("_Contents"), "F1",
		  N_("View help for the Chemical Calculator"), G_CALLBACK (on_help) },
	  { "Web", NULL, N_("Gnome Chemistry Utils on the _web"), NULL,
		  N_("Browse the Gnome Chemistry Utils's web site"), G_CALLBACK (on_web) },
	  { "LiveAssistance", NULL, N_("Live assistance"), NULL,
		  N_("Open the Gnome Chemistry Utils IRC channel"), G_CALLBACK (on_live_assistance) },
	  { "Mail", NULL, N_("_Ask a question"), NULL,
		  N_("Ask a question about the Gnome Chemistry Utils"), G_CALLBACK (on_mail) },
	  { "Bug", NULL, N_("Report _Bugs"), NULL,
		  N_("Submit a bug report for the Gnome Chemistry Utils"), G_CALLBACK (on_bug) },
	  { "About", GTK_STOCK_ABOUT, N_("_About"), NULL,
		  N_("About GChemCalc"), G_CALLBACK (on_about) }
};

static GtkRadioActionEntry radios[] = {
	{ "Guess", NULL, N_("_Guess"), NULL,
		N_("Try to guess what is correct when interpreting ambiguous symbols"),
		GCU_FORMULA_PARSE_GUESS },
	{ "Atom", NULL, N_("_Atom"), NULL,
		N_("Interpreting ambiguous symbols as atoms"),
		GCU_FORMULA_PARSE_ATOM },
	{ "Residue", NULL, N_("_Nickname"), NULL,
		N_("Interpret ambiguous symbols as atoms groups nicknames"),
		GCU_FORMULA_PARSE_RESIDUE },
	{ "Ask", NULL, N_("As_k"), NULL,
		N_("Ask user for the correct interpretation of ambiguous symbols"),
		GCU_FORMULA_PARSE_ASK }
};

static const char *ui_description =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu action='FileMenu'>"
"      <menuitem action='SaveAsImage'/>"
"	   <separator name='file-sep1'/>"
"      <menuitem action='PageSetup'/>"
"      <menuitem action='PrintPreview'/>"
"      <menuitem action='Print'/>"
"	   <separator name='file-sep2'/>"
"      <menuitem action='Quit'/>"
"    </menu>"
"    <menu action='EditMenu'>"
"      <menuitem action='Copy'/>"
"    </menu>"
"    <menu action='ModeMenu'>"
"      <menuitem action='Guess'/>"
"      <menuitem action='Atom'/>"
"      <menuitem action='Residue'/>"
//"      <menuitem action='Ask'/>"
"    </menu>"
"    <menu action='HelpMenu'>"
"      <menuitem action='Help'/>"
"      <menuitem action='Mail'/>"
"      <menuitem action='Web'/>"
"      <menuitem action='LiveAssistance'/>"
"      <menuitem action='Bug'/>"
"      <menuitem action='About'/>"
"    </menu>"
"  </menubar>"
"</ui>";

gboolean cb_print_version (G_GNUC_UNUSED const gchar *option_name, G_GNUC_UNUSED const gchar *value, G_GNUC_UNUSED gpointer data, G_GNUC_UNUSED GError **error)
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
   { NULL, 0, 0, G_OPTION_ARG_NONE, NULL, NULL, NULL }
};

int main (int argc, char *argv[])
{
	GOptionContext *context;
	GError *error = NULL;
	textdomain (GETTEXT_PACKAGE);
	gtk_init (&argc, &argv);
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

	if (argc > 1) {
		cout << _("For usage see: gchemcalc [-?|--help]") << endl;
		return -1;
	}

	App = new GChemCalc ();

	/* Initialize plugins manager */
	go_plugins_init (NULL, NULL, NULL, NULL, TRUE, GO_TYPE_PLUGIN_LOADER_MODULE);

	gcugtk::UIBuilder *builder = new gcugtk::UIBuilder (UIDIR"/gchemcalc.ui", GETTEXT_PACKAGE);
	App->window = GTK_WINDOW (builder->GetRefdWidget ("gchemcalc"));
	gtk_window_set_icon_name (App->window, App->GetIconName ().c_str ());
	g_signal_connect (G_OBJECT (App->window), "destroy",
		 G_CALLBACK (gtk_main_quit),
		 NULL);

	GtkWidget *grid = builder->GetWidget ("calc-grid");
	GtkUIManager *ui_manager = gtk_ui_manager_new ();
	App->SetUIManager (ui_manager);
	GtkActionGroup *action_group = gtk_action_group_new ("MenuActions");
	gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);
	gtk_action_group_add_actions (action_group, entries, G_N_ELEMENTS (entries), NULL);
	gtk_action_group_add_radio_actions (action_group, radios, G_N_ELEMENTS (radios), 0, G_CALLBACK (on_mode), NULL);
	gtk_ui_manager_insert_action_group (ui_manager, action_group, 0);
	GtkAccelGroup *accel_group = gtk_ui_manager_get_accel_group (ui_manager);
	gtk_window_add_accel_group (GTK_WINDOW (App->window), accel_group);
	if (!gtk_ui_manager_add_ui_from_string (ui_manager, ui_description, -1, &error)) {
		g_message ("building menus failed: %s", error->message);
		g_error_free (error);
		exit (EXIT_FAILURE);
	}
	GtkWidget *bar = gtk_ui_manager_get_widget (ui_manager, "/MainMenu");
	gtk_grid_attach (GTK_GRID (grid), bar, 0, 0, 2, 1);
	App->markup = GTK_LABEL (builder->GetWidget ("markup"));
	App->raw = GTK_LABEL (builder->GetWidget ("raw"));
	App->weight = GTK_LABEL (builder->GetWidget ("weight"));

	//Add composition list
	App->pclist = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
	GtkTreeView *tree = GTK_TREE_VIEW (builder->GetWidget ("composition"));
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
	App->mono = GTK_LABEL (builder->GetWidget ("mono"));
	App->monomass = GTK_LABEL (builder->GetWidget ("monomass"));
	App->pattern_page = builder->GetWidget ("pattern");
	App->graph_widget = go_graph_widget_new (NULL);
	g_signal_connect_swapped (App->graph_widget, "size-allocate", G_CALLBACK (GChemCalc::OnSize), App);
	gtk_widget_show (App->graph_widget);
	gtk_grid_attach (GTK_GRID (App->pattern_page), App->graph_widget, 0, 1, 2, 1);
	g_object_set (G_OBJECT (App->graph_widget), "hexpand", true, "vexpand", true, NULL);
	App->graph = go_graph_widget_get_graph (GO_GRAPH_WIDGET (App->graph_widget));
	App->chart = go_graph_widget_get_chart (GO_GRAPH_WIDGET (App->graph_widget));
	App->plot = (GogPlot *) gog_plot_new_by_name ("GogXYPlot");
	gog_object_add_by_name (GOG_OBJECT (App->chart), "Plot", GOG_OBJECT (App->plot));
	// Create a series for the plot and populate it with some simple data
	App->series = gog_plot_new_series (App->plot);
	gog_object_add_by_name (GOG_OBJECT (App->series), "Vertical drop lines", NULL);
	GOStyle *style = go_styled_object_get_style (GO_STYLED_OBJECT (App->series));
	go_marker_set_shape (style->marker.mark, GO_MARKER_NONE);
	style->marker.auto_shape = false;
	style->line.dash_type = GO_LINE_NONE;
	style->line.auto_dash = false;
	GogObject *obj = gog_object_get_child_by_role (GOG_OBJECT (App->chart),
			gog_object_find_role_by_name (GOG_OBJECT (App->chart), "Y-Axis"));
	GOData *data = go_data_scalar_val_new (100.);
	gog_dataset_set_dim (GOG_DATASET (obj), GOG_AXIS_ELEM_MAX, data, &error);

	gtk_widget_hide (App->pattern_page);
	GtkWidget *w = builder->GetWidget ("entry");
	g_signal_connect (G_OBJECT (w), "activate",
		 G_CALLBACK (cb_entry_active),
		 App->window);
	gcu_element_load_databases ("isotopes", NULL);
	Element::LoadBODR ();
	if (argc == 1){
		gtk_entry_set_text (GTK_ENTRY (w), argv[0]);
		cb_entry_active (GTK_ENTRY (w), App->window);
	}

	w = builder->GetWidget ("calc-book");
	g_signal_connect (w, "switch-page", G_CALLBACK (on_page), NULL);
	on_page (NULL, NULL, 0); // force menus deactivation

	delete builder;

	gtk_main ();
	if (graph)
		g_object_unref (graph);
	delete App;
	return 0;
}
