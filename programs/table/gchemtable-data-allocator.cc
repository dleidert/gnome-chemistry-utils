/* 
 * Gnome Chemistry Utils
 * programs/gchemtable-data-allocator.cc
 *
 * Copyright (C) 2007 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "gchemtable-data-allocator.h"
#include "gchemtable-curve.h"
#include "gchemtable-data.h"
#include <goffice/data/go-data-simple.h>
#include <goffice/graph/gog-data-allocator.h>
#include <goffice/graph/gog-data-set.h>
#include <goffice/graph/gog-plot-impl.h>
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
gct_data_allocator_allocate (GogDataAllocator *dalloc, GogPlot *plot)
{
	// Nothing needed
}

typedef struct {
	GtkWidget *box;
	GogDataset *dataset;
	int dim_i;
	GogDataType data_type;
} GraphDimEditor;

static void
on_graph_dim_editor_changed (GtkEntry *box,
			    GraphDimEditor *editor)
{
	if (!GTK_WIDGET_SENSITIVE (box) || editor->dataset == NULL)
		return;

	GOData *data = go_data_scalar_str_new (g_strdup (gtk_entry_get_text (box)), TRUE);

	if (!data) {
		g_message (_("Invalide data"));
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

static gpointer
gct_data_allocator_editor (GogDataAllocator *dalloc,
			    GogDataset *dataset, int dim_i, GogDataType data_type)
{
	GraphDimEditor *editor;

	editor = g_new (GraphDimEditor, 1);
	editor->dataset		= dataset;
	editor->dim_i		= dim_i;
	editor->data_type	= data_type;
					
	if (IS_GOG_SERIES (dataset) && data_type != GOG_DATA_SCALAR) {
		GogPlot *plot = gog_series_get_plot (GOG_SERIES (dataset));
		if (plot->desc.series.dim[dim_i].priority == GOG_SERIES_ERRORS) {
			// FIXME: we might know the errors
			editor->box = gtk_label_new (_("Not supported"));
			g_object_set_data_full (G_OBJECT (editor->box),
				"editor", editor, (GDestroyNotify) graph_dim_editor_free);
			return editor->box;
		}
		editor->box = gtk_combo_box_new_text ();
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
		editor->box = gtk_entry_new ();
		GOData *val = gog_dataset_get_dim (dataset, dim_i);
		if (val != NULL) {
			char *txt = go_data_as_str (val);
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
gct_control_gui_init (GObject *object)
{
}

static GObjectClass *parent_klass;

static void
gct_control_gui_finalize (GObject *object)
{
	GctControlGUI *control = GCT_CONTROL_GUI (object);
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
		GSF_INTERFACE (gct_go_plot_data_allocator_init, GOG_DATA_ALLOCATOR_TYPE));

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
