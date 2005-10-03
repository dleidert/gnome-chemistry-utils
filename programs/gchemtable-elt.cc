// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * programs/gchemtable-elt.cc 
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
#include "gchemtable-elt.h"
#include "gchemtable-app.h"
#include <gcu/element.h>
#include <glib/gi18n.h>

#warning "the following lines should be removed for stable releases"
#undef PACKAGE
#define PACKAGE "gchemutils-unstable" 

GChemTableElt::GChemTableElt (GChemTableApp *App, int Z): Dialog (App, DATADIR"/"PACKAGE"/glade/eltpage.glade", "eltdlg")
{
	Element *elt = Element::GetElement (Z);
	char *buf;
	gtk_window_set_title (dialog, elt->GetName ());
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
}

GChemTableElt::~GChemTableElt ()
{
}
