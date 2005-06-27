// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * programs/gchemcalc.cc 
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
#warning "the following lines should be removed for stable releases"
#undef PACKAGE
#define PACKAGE "gchemutils-unstable" 
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
#include <math.h>

using namespace gcu;

class GChemCalc {
public:
	GChemCalc ();
	Formula formula;
	GtkLabel *markup, *raw, *weight;
};

GChemCalc::GChemCalc (): formula ("")
{
}

GChemCalc App;

static void on_quit (GtkWidget *widget, void *data)
{
	gtk_main_quit();
}

static void on_about (GtkWidget *widget, void *data)
{
	char * authors[] = {"Jean Bréfort", NULL};
	char * documentors[] = {NULL};
	char license[] = "This program is free software; you can redistribute it and/or\n" 
		"modify it under the terms of the GNU General Public License as\n"
 		"published by the Free Software Foundation; either version 2 of the\n"
		"License, or (at your option) any later version.\n\n"
		"This program is distributed in the hope that it will be useful,\n"
		"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
		"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
		"GNU General Public License for more details.\n\n"
		"You should have received a copy of the GNU General Public License\n"
		"along with this program; if not, write to the Free Software\n"
		"Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307\n"
		"USA";
/* Note to translators: replace the following string with the appropriate credits for you lang */
	char *translator_credits = _("translator_credits");
	gtk_show_about_dialog (NULL,
					"name", "GChemCalc",
					"authors", authors,
					"comments", _("GChemCalc is a simple calculator for chemists"),
					"copyright", _("(C) 2005 by Jean Bréfort"),
					"license", license,
					"translator_credits", strcmp (translator_credits, "translator_credits") != 0 ? 
											(const char *)translator_credits : NULL,
					"version", VERSION,
					"website", "http://www.nongnu.org/gchemutils",
					NULL);
}

static void cb_entry_active (GtkEntry *entry, gpointer data)
{
	try {
		char *format;
		App.formula.SetFormula (gtk_entry_get_text (entry));
		format = g_strconcat (_("Formula:"), " \t", App.formula.GetMarkup (), NULL);
		gtk_label_set_markup (App.markup, format);
		g_free (format);
		format = g_strconcat (_("Raw formula:"), " \t", App.formula.GetRawMarkup (), NULL);
		gtk_label_set_markup (App.raw, format);
		g_free (format);
		int prec;
		bool artificial;
		double weight = App.formula.GetMolecularWeight (prec, artificial);
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
		gtk_label_set_text (App.weight, weightstr);
		g_free (weightstr);
		g_free (format);
	}
	catch (parse_error &error) {
		int start, length;
		char const *mess = error.what (start, length);
		gtk_entry_select_region (entry, start, start + length);
		GtkWidget *w = gtk_message_dialog_new (GTK_WINDOW (data),
							GTK_DIALOG_DESTROY_WITH_PARENT,
							GTK_MESSAGE_ERROR,
							GTK_BUTTONS_OK,
							mess);
		g_signal_connect_swapped (G_OBJECT (w), "response", G_CALLBACK (gtk_widget_destroy), G_OBJECT (w));
		gtk_widget_show (w);
	}
}

static GtkActionEntry entries[] = {
  { "FileMenu", NULL, N_("_File") },
	  { "Quit", GTK_STOCK_QUIT, N_("_Quit"), "<control>Q",
		  N_("Quit GChemCalc"), G_CALLBACK (on_quit) },
  { "HelpMenu", NULL, N_("_Help") },
	  { "About", NULL, N_("_About"), NULL,
		  N_("About GChem3D"), G_CALLBACK (on_about) }
};

static const char *ui_description =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu action='FileMenu'>"
"      <menuitem action='Quit'/>"
"    </menu>"
"    <menu action='HelpMenu'>"
"      <menuitem action='About'/>"
"    </menu>"
"  </menubar>"
"</ui>";

int main (int argc, char *argv[])
{
	bindtextdomain (GETTEXT_PACKAGE, DATADIR"/locale");
#ifdef ENABLE_NLS
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
#endif
	textdomain (GETTEXT_PACKAGE);
	gtk_init (&argc, &argv);

	GladeXML *xml =  glade_xml_new (DATADIR"/"PACKAGE"/glade/gchemcalc.glade", "gchemcalc", NULL);
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
	GError *error = NULL;
	if (!gtk_ui_manager_add_ui_from_string (ui_manager, ui_description, -1, &error)) {
		g_message ("building menus failed: %s", error->message);
		g_error_free (error);
		exit (EXIT_FAILURE);
	}
	GtkWidget *bar = gtk_ui_manager_get_widget (ui_manager, "/MainMenu");
	gtk_box_pack_start (GTK_BOX (vbox), bar, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (vbox), bar, 0);
	App.markup = GTK_LABEL (glade_xml_get_widget (xml, "markup"));
	App.raw = GTK_LABEL (glade_xml_get_widget (xml, "raw"));
	App.weight = GTK_LABEL (glade_xml_get_widget (xml, "weight"));
	GtkWidget *w = glade_xml_get_widget (xml, "entry");
	g_signal_connect (GTK_OBJECT (w), "activate",
		 G_CALLBACK (cb_entry_active),
		 window);
	gtk_main ();
	return 0;
}
