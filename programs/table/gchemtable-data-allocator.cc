/* 
 * Gnome Chemistry Utils
 * programs/gchemtable-data-allocator.cc
 *
 * Copyright (C) 2007-2008 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "gchemtable-data-allocator.h"
#include "gchemtable-curve.h"
#include "gchemtable-data.h"
#include <gtk/gtkcombobox.h>
#include <gtk/gtkentry.h>
#include <gsf/gsf-impl-utils.h>
#include <glib/gi18n-lib.h>

struct _GctControlGUI
{
	GObject base;
	GChemTableCurve *pCurve;
};
	
//
// GogDataAllocator interface implementation for GChemTableCurve
//

static void
gct_data_allocator_allocate (G_GNUC_UNUSED GogDataAllocator *dalloc, G_GNUC_UNUSED GogPlot *plot)
{
	// Nothing needed
}

static void
gct_data_editor_set_format (G_GNUC_UNUSED GogDataEditor *editor, G_GNUC_UNUSED GOFormat const *fmt)
{
}

static void
gct_data_editor_set_value_double (G_GNUC_UNUSED GogDataEditor *editor, G_GNUC_UNUSED double val,
				      G_GNUC_UNUSED GODateConventions const *date_conv)
{
}

typedef GtkComboBox GctComboBox;
typedef GtkComboBoxClass GctComboBoxClass;

static void
gct_data_editor_iface_init (GogDataEditorClass *iface)
{
	iface->set_format = gct_data_editor_set_format;
	iface->set_value_double = gct_data_editor_set_value_double;
}

GSF_CLASS_FULL (GctComboBox, gct_combo_box,
		NULL, NULL, NULL, NULL,
		NULL, GTK_TYPE_COMBO_BOX, 0,
		GSF_INTERFACE (gct_data_editor_iface_init, GOG_TYPE_DATA_EDITOR))

GogDataEditor *gct_combo_box_new ()
{
	GogDataEditor *editor = GOG_DATA_EDITOR (g_object_new (gct_combo_box_get_type (), NULL));
	// code from gtk_combo_box_new_text
	// Copyright (C) 2002, 2003  Kristian Rietveld <kris@gtk.org>
	GtkCellRenderer *cell;
	GtkListStore *store;

	store = gtk_list_store_new (1, G_TYPE_STRING);
	gtk_combo_box_set_model (GTK_COMBO_BOX (editor), GTK_TREE_MODEL (store));
	g_object_unref (store);

	cell = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (editor), cell, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (editor), cell,
		                          "text", 0,
		                          NULL);

	return editor;
}

static void
gct_entry_set_value_double (GogDataEditor *editor, double val,
				      G_GNUC_UNUSED GODateConventions const *date_conv)
{
	GtkEntry *entry = GTK_ENTRY (editor);
	char *buf = g_strdup_printf ("%g", val);
	gtk_entry_set_text (entry, buf);
	g_free (buf);
}

typedef GtkEntry GctEntry;
typedef GtkEntryClass GctEntryClass;

static void
gct_entry_iface_init (GogDataEditorClass *iface)
{
	iface->set_format = gct_data_editor_set_format;
	iface->set_value_double = gct_entry_set_value_double;
}

GSF_CLASS_FULL (GctEntry, gct_entry,
		NULL, NULL, NULL, NULL,
		NULL, GTK_TYPE_ENTRY, 0,
		GSF_INTERFACE (gct_entry_iface_init, GOG_TYPE_DATA_EDITOR))

GogDataEditor *gct_entry_new ()
{
	return GOG_DATA_EDITOR (g_object_new (gct_entry_get_type (), NULL));
}

typedef GtkLabel GctLabel;
typedef GtkLabelClass GctLabelClass;

GSF_CLASS_FULL (GctLabel, gct_label,
		NULL, NULL, NULL, NULL,
		NULL, GTK_TYPE_LABEL, 0,
		GSF_INTERFACE (gct_data_editor_iface_init, GOG_TYPE_DATA_EDITOR))

GogDataEditor *gct_label_new (char const *label)
{
	return GOG_DATA_EDITOR (g_object_new (gct_label_get_type (), "label", label, NULL));
}

typedef struct {
	GogDataEditor *box;
	GogDataset *dataset;
	int dim_i;
	GogDataType data_type;
} GraphDimEditor;

static void
on_graph_dim_editor_changed (GtkEntry *box,
			    GraphDimEditor *editor)
{
	bool sensitive = false;
	g_object_get (G_OBJECT (box), "sensitive", &sensitive, NULL);
	if (!sensitive || editor->dataset == NULL)
		return;

	GOData *data = go_data_scalar_str_new (g_strdup (gtk_entry_get_text (box)), TRUE);

	if (!data) {
		g_message (_("Invalid data"));
	} else
		gog_dataset_set_dim (editor->dataset, editor->dim_i, data, NULL);
}

static void
on_dim_editor_weakref_notify (GraphDimEditor *editor, GogDataset *dataset)
{
	g_return_if_fail (editor->dataset == dataset);
	editor->dataset = NULL;
}

static void
graph_dim_editor_free (GraphDimEditor *editor)
{
	if (editor->dataset)
		g_object_weak_unref (G_OBJECT (editor->dataset),
			(GWeakNotify) on_dim_editor_weakref_notify, editor);
	g_free (editor);
}

static void on_vector_data_changed (GtkComboBox *box, GraphDimEditor *editor)
{
	char *name = gtk_combo_box_get_active_text (box);
	GOData *data = gct_data_vector_get_from_name (name);
	gog_dataset_set_dim (editor->dataset, editor->dim_i, data, NULL);
	g_free (name);
}

static GogDataEditor *
gct_data_allocator_editor (G_GNUC_UNUSED GogDataAllocator *dalloc,
			    GogDataset *dataset, int dim_i, GogDataType data_type)
{
	GraphDimEditor *editor;

	editor = g_new (GraphDimEditor, 1);
	editor->dataset		= dataset;
	editor->dim_i		= dim_i;
	editor->data_type	= data_type;
					
	if (GOG_IS_SERIES (dataset) && data_type != GOG_DATA_SCALAR) {
		GogPlot *plot = gog_series_get_plot (GOG_SERIES (dataset));
		if (plot->desc.series.dim[dim_i].priority == GOG_SERIES_ERRORS) {
			// FIXME: we might know the errors
			editor->box = gct_label_new (_("Not supported"));
			g_object_set_data_full (G_OBJECT (editor->box),
				"editor", editor, (GDestroyNotify) graph_dim_editor_free);
			return editor->box;
		}
		editor->box = GOG_DATA_EDITOR (gct_combo_box_new ());
		GOData *data = gog_dataset_get_dim (dataset, dim_i), *cur;
		int i = 1, sel = 0;
		GtkComboBox *box = GTK_COMBO_BOX (editor->box);
		gtk_combo_box_append_text (box, _("None"));
		if (data_type == GOG_DATA_VECTOR) {
			void *closure = NULL;
			char const *entry = gct_data_vector_get_first (&cur, &closure);
			while (entry)  {
				gtk_combo_box_append_text (box, entry);
				if (cur == data)
					sel = i;
				i++;
				g_object_unref (cur);
				entry = gct_data_vector_get_next (&cur, &closure);
			};
		}
		gtk_combo_box_set_active (box, sel);
		g_signal_connect (G_OBJECT (editor->box), "changed",
						  G_CALLBACK (on_vector_data_changed), editor);
		// FIXME: what about matrices?
	} else {
		editor->box = GOG_DATA_EDITOR (gct_entry_new ());
		GOData *val = gog_dataset_get_dim (dataset, dim_i);
		if (val != NULL) {
			char *txt = go_data_serialize (val, NULL);
			gtk_entry_set_text (GTK_ENTRY (editor->box), txt);
			g_free (txt);
		}

		g_signal_connect (G_OBJECT (editor->box),
			"changed",
			G_CALLBACK (on_graph_dim_editor_changed), editor);
	}
	g_object_weak_ref (G_OBJECT (editor->dataset),
		(GWeakNotify) on_dim_editor_weakref_notify, editor);

	g_object_set_data_full (G_OBJECT (editor->box),
		"editor", editor, (GDestroyNotify) graph_dim_editor_free);

	return editor->box;
}

static void
gct_go_plot_data_allocator_init (GogDataAllocatorClass *iface)
{
	iface->allocate   = gct_data_allocator_allocate;
	iface->editor	  = gct_data_allocator_editor;
}

static void
gct_control_gui_init (G_GNUC_UNUSED GObject *object)
{
}

static GObjectClass *parent_klass;

static void
gct_control_gui_finalize (GObject *object)
{
//	GctControlGUI *control = GCT_CONTROL_GUI (object);
	(parent_klass->finalize) (object);
}

static void
gct_control_gui_class_init (GObjectClass *klass)
{
	parent_klass = static_cast<GObjectClass*> (g_type_class_peek_parent (klass));
	klass->finalize = gct_control_gui_finalize;
}

GSF_CLASS_FULL (GctControlGUI, gct_control_gui,
		NULL, NULL, gct_control_gui_class_init, NULL,
		gct_control_gui_init, G_TYPE_OBJECT, 0,
		GSF_INTERFACE (gct_go_plot_data_allocator_init, GOG_TYPE_DATA_ALLOCATOR));

void
gct_control_gui_set_owner (GctControlGUI *gui, GChemTableCurve *curve)
{
	gui->pCurve = curve;
}

GChemTableCurve *
gct_control_gui_get_owner (GctControlGUI *gui)
{
	return gui->pCurve;
}
