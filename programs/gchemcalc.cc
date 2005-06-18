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
#include <glade/glade.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkentry.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkmessagedialog.h>

using namespace gcu;

class GChemCalc {
public:
	GChemCalc ();
	Formula formula;
	GtkLabel *markup, *raw;
};

GChemCalc::GChemCalc (): formula ("")
{
}

GChemCalc App;

static void cb_entry_active (GtkEntry *entry, gpointer data)
{
	try {
		App.formula.SetFormula (gtk_entry_get_text (entry));
		gtk_label_set_markup (App.markup, App.formula.GetMarkup ());
		gtk_label_set_markup (App.raw, App.formula.GetRawMarkup ());
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
	App.markup = GTK_LABEL (glade_xml_get_widget (xml, "markup"));
	App.raw = GTK_LABEL (glade_xml_get_widget (xml, "raw"));
	GtkWidget *w = glade_xml_get_widget (xml, "entry");
	g_signal_connect (GTK_OBJECT (w), "activate",
		 G_CALLBACK (cb_entry_active),
		 window);
	gtk_main ();
	return 0;
}
