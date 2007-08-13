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
#include <goffice/graph/gog-data-allocator.h>
#include <goffice/graph/gog-data-set.h>
#include <goffice/graph/gog-series.h>
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
//	SheetControlGUI *scg = wbcg_cur_scg (WORKBOOK_CONTROL_GUI (dalloc));
//	sv_selection_to_plot (sc_view (SHEET_CONTROL (scg)), plot);
}

typedef struct {
	GtkWidget *box;
	GogDataset *dataset;
	int dim_i;
	GogDataType data_type;
} GraphDimEditor;

static void
on_graph_dim_editor_changed (GtkComboBoxEntry *box,
			    GraphDimEditor *editor)
{
	if (!GTK_WIDGET_SENSITIVE (box) || editor->dataset == NULL)
		return;

	GOData *data = NULL;

	if (!data) {
		/* display "Invalid Data message" */
	} else
		gog_dataset_set_dim (editor->dataset, editor->dim_i, data, NULL);
}

static void
on_graph_dim_editor_update (GtkEntry *box,
			    GraphDimEditor *editor)
{
}

static void
on_graph_dim_entry_unmap (GtkEntry *box,
			    GraphDimEditor *editor)
{
}

static void
on_graph_dim_entry_unrealize (GtkEntry *box,
			    GraphDimEditor *editor)
{
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

static gpointer
gct_data_allocator_editor (GogDataAllocator *dalloc,
			    GogDataset *dataset, int dim_i, GogDataType data_type)
{
	GctControlGUI *acg = GCT_CONTROL_GUI (dalloc);
	GraphDimEditor *editor;
	GOData *val;

	editor = g_new (GraphDimEditor, 1);
	editor->dataset		= dataset;
	editor->dim_i		= dim_i;
	editor->data_type	= data_type;
					
	if (IS_GOG_SERIES (dataset)) {
		editor->box = gtk_combo_box_new_text ();
	} else {
		editor->box = gtk_entry_new ();
		g_signal_connect (G_OBJECT (editor->box),
			"activate",
			G_CALLBACK (on_graph_dim_editor_update), editor);
		g_signal_connect (G_OBJECT (editor->box),
			"unmap",
			G_CALLBACK (on_graph_dim_entry_unmap), editor);
		g_signal_connect (G_OBJECT (editor->box),
			"unrealize",
			G_CALLBACK (on_graph_dim_entry_unrealize), editor);
	}
	g_object_weak_ref (G_OBJECT (editor->dataset),
		(GWeakNotify) on_dim_editor_weakref_notify, editor);

	val = gog_dataset_get_dim (dataset, dim_i);
	if (val != NULL) {
/*		char *txt = go_data_as_str (val);
		gtk_entry_set_text (editor->entry, txt);
		g_free (txt);*/
	}

	g_signal_connect (G_OBJECT (editor->box),
		"changed",
		G_CALLBACK (on_graph_dim_editor_changed), editor);
/*	g_signal_connect (G_OBJECT (editor->entry),
		"activate",
		G_CALLBACK (on_graph_dim_editor_update), editor);
	g_signal_connect (G_OBJECT (editor->entry),
		"unmap",
		G_CALLBACK (on_graph_dim_entry_unmap), editor);
	g_signal_connect (G_OBJECT (editor->entry),
		"unrealize",
		G_CALLBACK (on_graph_dim_entry_unrealize), editor);*/
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
	GctControlGUI *control = GCT_CONTROL_GUI (object);
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
