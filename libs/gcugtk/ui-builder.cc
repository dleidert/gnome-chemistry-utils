// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gcugtk/ui-builder.cc
 *
 * Copyright (C) 2009-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "ui-builder.h"
#include <glib/gi18n-lib.h>
#include <string>

namespace gcugtk
{

UIBuilder::UIBuilder (char const *filename, char const *domain) throw (std::runtime_error)
{
	m_Builder = go_gtk_builder_new (filename, domain, NULL);
	if (!m_Builder) {
		char *buf = g_strdup_printf (_("Could not load %s."), filename);
		std::string mess = buf;
		g_free (buf);
		throw std::runtime_error (mess);
	}
}

UIBuilder::~UIBuilder ()
{
	if (m_Builder)
		g_object_unref (m_Builder);
}

GtkWidget *UIBuilder::GetWidget (char const *wname)
{
	GObject *obj = gtk_builder_get_object (m_Builder, wname);
	return (obj)? GTK_WIDGET (obj): NULL;
}

GtkWidget *UIBuilder::GetRefdWidget (char const *wname)
{
	// this code is copied from a goffice patch, replace it when goffice has it
	GObject *obj = gtk_builder_get_object (m_Builder, wname);
	if (obj) {
		g_object_ref (obj);
		return GTK_WIDGET (obj);
	} else
		return NULL;
}

GtkComboBox *UIBuilder::GetComboBox (char const *cbname)
{
	GtkComboBox *box;
	GList *cells;
	box = GTK_COMBO_BOX (gtk_builder_get_object (m_Builder, cbname));
	/* search for the model and create one if none exists */
	g_return_val_if_fail (box != NULL, NULL);
	if (gtk_combo_box_get_model (box) == NULL) {
		GtkListStore *store = gtk_list_store_new (1, G_TYPE_STRING);
		gtk_combo_box_set_model (box, GTK_TREE_MODEL (store));
		g_object_unref (store);
	}
	cells = gtk_cell_layout_get_cells (GTK_CELL_LAYOUT (box));
	if (g_list_length (cells) == 0) {
		/* populate the cell layout */
		GtkCellRenderer *cell = gtk_cell_renderer_text_new ();
		gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (box), cell, TRUE);
		gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (box), cell,
						"text", 0, NULL);
	}
	g_list_free (cells);
	return box;
//	return go_gtk_builder_combo_box_init_text (m_Builder, cbname);
}

}   //  namespace gcu
