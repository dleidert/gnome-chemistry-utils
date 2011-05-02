/* 
 * GChemPaint library
 * fontsel.c 
 *
 * Copyright (C) 2006-2010 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "fontsel.h"
#include <gtk/gtkbin.h>
#include <gtk/gtkentry.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkscrolledwindow.h>
#include <gtk/gtktable.h>
#include <gtk/gtktreeview.h>
#include <gtk/gtktreeselection.h>
#include <gtk/gtkcellrenderertext.h>
#include <gsf/gsf-impl-utils.h>
#include <glib/gi18n-lib.h>
#include <cstdlib>
#include <map>
#include <string>
#include <cstring>

using namespace std;

struct _GcpFontSel {
	GtkBin base;
	GtkEntry *m_SizeEntry;
	GtkListStore *FamilyList, *FaceList, *SizeList;
	GtkTreeView *FamilyTree, *FacesTree, *SizeTree;
	guint FamilySignal, FaceSignal, SizeSignal;
	GtkTreeSelection *FamilySel, *FaceSel, *SizeSel;
	GtkLabel *Label;
	map<string, PangoFontFamily*> Families;
	map<string, PangoFontFace*> Faces;
	char *FamilyName;
	PangoStyle Style;
	PangoWeight Weight;
	PangoStretch Stretch;
	PangoVariant Variant;
	int Size;
};

typedef struct {
	GtkBinClass base_class;

	/* signals */
	void (*changed) (GcpFontSel *fs);
} GcpFontSelClass;

static void
gcp_font_sel_size_request (GtkWidget      *widget,
			GtkRequisition *requisition)
{
	GtkWidget *w = GTK_WIDGET (gtk_bin_get_child (GTK_BIN (widget)));
	if (w)
    	gtk_widget_size_request (w, requisition);
	else {
		requisition->width = 0;
		requisition->height = 0;
	}
}

static void
gcp_font_sel_size_allocate (GtkWidget     *widget,
			 GtkAllocation *allocation)
{
	GtkWidget *w = GTK_WIDGET (gtk_bin_get_child (GTK_BIN (widget)));
	if (w)
		gtk_widget_size_allocate (GTK_WIDGET (w), allocation);
}

enum {
	FONT_SEL_PROP_0,
	FONT_SEL_PROP_FAMILY,
	FONT_SEL_PROP_STYLE,
	FONT_SEL_PROP_WEIGHT,
	FONT_SEL_PROP_STRETCH,
	FONT_SEL_PROP_VARIANT,
	FONT_SEL_PROP_SIZE
};

enum {
	CHANGED,
	LAST_SIGNAL
};
static gulong gcp_font_sel_signals [LAST_SIGNAL] = { 0, };

static void gcp_font_sel_set_label (GcpFontSel *fs)
{
	PangoFontDescription *fd = pango_font_description_new ();
	pango_font_description_set_family (fd, fs->FamilyName);
	pango_font_description_set_style (fd, fs->Style);
	pango_font_description_set_weight (fd, fs->Weight);
	pango_font_description_set_variant (fd, fs->Variant);
	pango_font_description_set_stretch (fd, fs->Stretch);
	pango_font_description_set_size (fd, fs->Size);
	char *name = pango_font_description_to_string (fd);
	char *markup = g_markup_printf_escaped ("<span font_desc=\"%s\">%s</span>", name, name);
	gtk_label_set_markup (fs->Label, markup);
	g_free (name);
	g_free (markup);
	pango_font_description_free (fd);
}

static void select_best_font_face (GcpFontSel *fs)
{
	PangoFontDescription *desc;
	int distance, best;
	PangoStyle Style;
	PangoWeight Weight;
	PangoVariant Variant;
	PangoStretch Stretch;
	map <string, PangoFontFace*>::iterator i, iend = fs->Faces.end ();
	char const *name = NULL;
	char *buf;
	GtkTreeIter iter;

	best = 32000; // This should be enough
	for (i = fs->Faces.begin (); i != iend; i++) {
		desc = pango_font_face_describe ((*i).second);
		// Try to select the best available face
		Style = pango_font_description_get_style (desc);
		Weight = pango_font_description_get_weight (desc);
		Variant = pango_font_description_get_variant (desc);
		Stretch = pango_font_description_get_stretch (desc);
		distance = abs (Weight - fs->Weight)
						+ abs ((Style? Style + 2: 0) - (fs->Style? fs->Style + 2: 0)) * 1000
						+ abs (Variant - fs->Variant) * 10 + abs (Stretch - fs->Stretch);
		if (distance < best) {
			best = distance;
			name = (*i).first.c_str ();
		}
		pango_font_description_free (desc);
	}
	// select the found face
	if (!gtk_tree_model_get_iter_first (GTK_TREE_MODEL (fs->FaceList), &iter))
		return;
	do {
		gtk_tree_model_get (GTK_TREE_MODEL (fs->FaceList), &iter, 0, &buf, -1);
		if (!strcmp (name, buf)) {
			GtkTreePath *path = gtk_tree_model_get_path (GTK_TREE_MODEL (fs->FaceList), &iter);
			gtk_tree_view_set_cursor (fs->FacesTree, path, NULL, FALSE);
			gtk_tree_path_free (path);
			g_free (buf);
			break;
		}
		g_free (buf);
	} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (fs->FaceList), &iter));
}

static void
gcp_font_sel_get_property (GObject *obj, guint param_id,
			GValue *value, GParamSpec *pspec)
{
	GcpFontSel *fs = GCP_FONT_SEL (obj);

	switch (param_id) {
	case FONT_SEL_PROP_FAMILY:
		g_value_set_string (value, fs->FamilyName);
		break;
	case FONT_SEL_PROP_STYLE:
		g_value_set_int (value, fs->Style);
		break;
	case FONT_SEL_PROP_WEIGHT:
		g_value_set_int (value, fs->Weight);
		break;
	case FONT_SEL_PROP_STRETCH:
		g_value_set_int (value, fs->Stretch);
		break;
	case FONT_SEL_PROP_VARIANT:
		g_value_set_int (value, fs->Variant);
		break;
	case FONT_SEL_PROP_SIZE:
		g_value_set_int (value, fs->Size);
		break;
	default: G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, param_id, pspec);
		 return;
	}
}

/* These are what we use as the standard font sizes, for the size list.
 */
static const guint16 font_sizes[] = {
  8, 9, 10, 11, 12, 13, 14, 16, 18, 20, 22, 24, 26, 28,
  32, 36, 40, 48, 56, 64, 72
};

static void gcp_font_sel_set_size_full (GcpFontSel *fs, bool update_list)
{
	char *buf = g_strdup_printf ("%.1f", (double) fs->Size / PANGO_SCALE);
	gtk_entry_set_text (fs->m_SizeEntry, buf);
	g_free (buf);
	if (update_list) {
		GtkTreeIter iter;
		bool found = false;
		g_signal_handler_block (fs->SizeSel, fs->SizeSignal);
		
		gtk_tree_model_get_iter_first (GTK_TREE_MODEL (fs->SizeList), &iter);
		for (unsigned i = 0; i < G_N_ELEMENTS (font_sizes) && !found; i++) {
			if (font_sizes[i] * PANGO_SCALE == fs->Size) {
				GtkTreePath *path = gtk_tree_model_get_path (GTK_TREE_MODEL (fs->SizeList), &iter);
				gtk_tree_view_set_cursor (fs->SizeTree, path, NULL, FALSE);
				gtk_tree_path_free (path);
				found = true;
			}

			gtk_tree_model_iter_next (GTK_TREE_MODEL (fs->SizeList), &iter);
		}
		
		if (!found)
			gtk_tree_selection_unselect_all (fs->SizeSel);
		g_signal_handler_unblock (fs->SizeSel, fs->SizeSignal);
	}
	g_signal_emit (G_OBJECT (fs), gcp_font_sel_signals [CHANGED], 0);
	gcp_font_sel_set_label (fs);
}

static void
gcp_font_sel_set_property (GObject *obj, guint param_id,
			GValue const *value, GParamSpec *pspec)
{
	GcpFontSel *fs = GCP_FONT_SEL (obj);

	switch (param_id) {
	case FONT_SEL_PROP_FAMILY:
		if (fs->FamilyName)
			g_free (fs->FamilyName);
		fs->FamilyName = g_strdup (g_value_get_string (value));
		GtkTreeIter iter;
		char *buf;
		gtk_tree_model_get_iter_first (GTK_TREE_MODEL (fs->FamilyList), &iter);
		do {
			gtk_tree_model_get (GTK_TREE_MODEL (fs->FamilyList), &iter, 0, &buf, -1);
			if (!strcmp (fs->FamilyName, buf)) {
				GtkTreePath *path = gtk_tree_model_get_path (GTK_TREE_MODEL (fs->FamilyList), &iter);
				gtk_tree_view_set_cursor (fs->FamilyTree, path, NULL, FALSE);
				gtk_tree_view_scroll_to_cell (fs->FamilyTree, path, NULL, FALSE, 0., 0.);
				gtk_tree_path_free (path);
				g_free (buf);
				break;
			}
			g_free (buf);
		} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (fs->FamilyList), &iter));
	return;
	case FONT_SEL_PROP_STYLE:
		fs->Style = (PangoStyle) g_value_get_int (value);
		break;
	case FONT_SEL_PROP_WEIGHT:
		fs->Weight = (PangoWeight) g_value_get_int (value);		break;
	case FONT_SEL_PROP_STRETCH:
		fs->Stretch = (PangoStretch) g_value_get_int (value);		break;
	case FONT_SEL_PROP_VARIANT:
		fs->Variant = (PangoVariant) g_value_get_int (value);		break;
	case FONT_SEL_PROP_SIZE:
		fs->Size = g_value_get_int (value);
		gcp_font_sel_set_size_full (fs, true);
		return;
	default: G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, param_id, pspec);
		 return;
	}
	select_best_font_face (fs);
}

static void
gcp_font_sel_class_init (GcpFontSelClass *klass)
{
	GObjectClass *object_class = (GObjectClass*) klass;
	GtkWidgetClass *widget_class = (GtkWidgetClass *) klass;

	object_class->get_property = gcp_font_sel_get_property;
	object_class->set_property = gcp_font_sel_set_property;

	widget_class->size_request = gcp_font_sel_size_request;
	widget_class->size_allocate = gcp_font_sel_size_allocate;
	g_object_class_install_property (object_class, FONT_SEL_PROP_FAMILY,
		g_param_spec_string ("family", _("Family"),
			_("Font family"),
			"Bitstream Vera Sans", (GParamFlags) G_PARAM_READWRITE));
	g_object_class_install_property (object_class, FONT_SEL_PROP_STYLE,
		g_param_spec_int ("style", _("Style"),
			_("The font style (normal, oblique or italic)"),
			PANGO_STYLE_NORMAL, PANGO_STYLE_ITALIC, PANGO_STYLE_NORMAL,
			(GParamFlags) G_PARAM_READWRITE));
	g_object_class_install_property (object_class, FONT_SEL_PROP_WEIGHT,
		g_param_spec_int ("weight", _("Weight"),
			_("The font weight"),
			PANGO_WEIGHT_ULTRALIGHT, PANGO_WEIGHT_HEAVY, PANGO_WEIGHT_NORMAL,
			(GParamFlags) G_PARAM_READWRITE));
	g_object_class_install_property (object_class, FONT_SEL_PROP_VARIANT,
		g_param_spec_int ("variant", _("Variant"),
			_("The font variant"),
			PANGO_VARIANT_NORMAL, PANGO_VARIANT_SMALL_CAPS, PANGO_VARIANT_NORMAL,
			(GParamFlags) G_PARAM_READWRITE));
	g_object_class_install_property (object_class, FONT_SEL_PROP_STRETCH,
		g_param_spec_int ("stretch", _("Stretch"),
			_("The font stretch (condensed, normal or expanded)"),
			PANGO_STRETCH_ULTRA_CONDENSED, PANGO_STRETCH_ULTRA_EXPANDED, PANGO_STRETCH_NORMAL,
			(GParamFlags) G_PARAM_READWRITE));
	g_object_class_install_property (object_class, FONT_SEL_PROP_SIZE,
		g_param_spec_int ("size", _("Size"),
			_("The font size (in pango units)"),
			0, G_MAXINT, 12 * PANGO_SCALE,
			(GParamFlags) G_PARAM_READWRITE));

	gcp_font_sel_signals [CHANGED] = g_signal_new ("changed",
		G_TYPE_FROM_CLASS (klass),
		G_SIGNAL_RUN_LAST,
		G_STRUCT_OFFSET (GcpFontSelClass, changed),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID,
		G_TYPE_NONE, 0);
}

static void on_select_family (GtkTreeSelection *selection, GcpFontSel *fs)
{
	GtkTreeModel *model;
	GtkTreeIter iter, selected;
	char const *name;
	if (!gtk_tree_selection_get_selected (selection, &model, &iter))
		return;
	g_free (fs->FamilyName);
	gtk_tree_model_get (model, &iter, 0, &fs->FamilyName, -1);
	PangoFontFamily *family = fs->Families[fs->FamilyName];
	PangoFontFace **faces;
	int i, nb;
	g_signal_handler_block (fs->FaceSel, fs->FaceSignal);
	pango_font_family_list_faces (family, &faces, &nb);
	gtk_list_store_clear (fs->FaceList);
	map<string, PangoFontFace*>::iterator j, jend = fs->Faces.end ();
	for (j = fs->Faces.begin (); j != jend; j++) {
		g_object_unref ((*j).second);
	}
	fs->Faces.clear ();
	PangoFontDescription *desc;
	int distance, best;
	PangoStyle Style;
	PangoWeight Weight;
	PangoVariant Variant;
	PangoStretch Stretch;

	best = 32000; // This should be enough
	for (i = 0; i < nb; i++) {
		name = pango_font_face_get_face_name (faces[i]);
		desc = pango_font_face_describe (faces[i]);
		fs->Faces[name] = (PangoFontFace*) g_object_ref (faces[i]);
		gtk_list_store_append (fs->FaceList, &iter);
		gtk_list_store_set (fs->FaceList, &iter,
				  0, name,
				  -1);
		// Try to select the best available face
		Style = pango_font_description_get_style (desc);
		Weight = pango_font_description_get_weight (desc);
		Variant = pango_font_description_get_variant (desc);
		Stretch = pango_font_description_get_stretch (desc);
		distance = abs (Weight - fs->Weight)
						+ abs ((Style? Style + 2: 0) - (fs->Style? fs->Style + 2: 0)) * 1000
						+ abs (Variant - fs->Variant) * 10 + abs (Stretch - fs->Stretch);
		if (distance < best) {
			best = distance;
			selected = iter;
		}
		// TODO: write this code
		pango_font_description_free (desc);
	}
	g_free (faces);
	g_signal_handler_unblock (fs->FaceSel, fs->FaceSignal);
	GtkTreePath *path = gtk_tree_model_get_path (GTK_TREE_MODEL (fs->FaceList), &selected);
	if (path) {
		gtk_tree_selection_select_path (GTK_TREE_SELECTION (fs->FaceSel), path);
		gtk_tree_path_free (path);
	}
}

static void on_select_face (GtkTreeSelection *selection, GcpFontSel *fs)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	char *name;
	if (!gtk_tree_selection_get_selected (selection, &model, &iter))
		return;
	gtk_tree_model_get (model, &iter, 0, &name, -1);
	PangoFontFace *face = fs->Faces[name];
	g_free (name);
	PangoFontDescription *desc = pango_font_face_describe (face);
	fs->Style = pango_font_description_get_style (desc);
	fs->Weight = pango_font_description_get_weight (desc);
	fs->Variant = pango_font_description_get_variant (desc);
	fs->Stretch = pango_font_description_get_stretch (desc);
	pango_font_description_free (desc);
	g_signal_emit (G_OBJECT (fs), gcp_font_sel_signals [CHANGED], 0);
	gcp_font_sel_set_label (fs);
}

static void on_select_size (GtkTreeSelection *selection, GcpFontSel *fs)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	gtk_tree_selection_get_selected (selection, &model, &iter);
	gtk_tree_model_get (model, &iter, 0, &fs->Size, -1);
	fs->Size *= PANGO_SCALE;
	gcp_font_sel_set_size_full (fs, false);
}

static void on_size_activate (G_GNUC_UNUSED GtkEntry *entry, GcpFontSel *fs)
{
	char const *text = gtk_entry_get_text (fs->m_SizeEntry);
	fs->Size = (int) (MAX (0.1, atof (text) * PANGO_SCALE + 0.5));
	gcp_font_sel_set_size_full (fs, true);
}

static bool on_size_focus_out (GtkEntry *entry, G_GNUC_UNUSED GdkEventFocus *event, GcpFontSel *fs)
{
	on_size_activate (entry, fs);
	return true;
}

static void
gcp_font_sel_init (GcpFontSel *fs)
{
	int i, nb;
	PangoFontFamily **families;
	GtkWidget *sc, *w = gtk_table_new (3, 4, FALSE);
	g_object_set (G_OBJECT (w), "border-width", 6, NULL);
	fs->Families = map<string, PangoFontFamily*>();
	fs->Faces = map<string, PangoFontFace*>();
	GtkTable *table = GTK_TABLE (w);
	gtk_table_set_col_spacings (table, 12);
	gtk_container_add (GTK_CONTAINER (fs), GTK_WIDGET (w));
	w = gtk_label_new ("");
	fs->Label = GTK_LABEL (w);
	gtk_table_attach (table, w, 0, 3, 3, 4,
			(GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
			(GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 6);
	// Initialize faces list
	fs->FaceList = gtk_list_store_new (1, G_TYPE_STRING);
	gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (fs->FaceList), 0, GTK_SORT_ASCENDING);
	fs->FacesTree = GTK_TREE_VIEW (gtk_tree_view_new_with_model (GTK_TREE_MODEL (fs->FaceList)));
	gtk_tree_view_set_headers_visible (fs->FacesTree, false);
	sc = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sc), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (sc), GTK_WIDGET (fs->FacesTree));
	gtk_table_attach (table, sc, 1, 2, 1, 3,
			(GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
			(GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new ();
	GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes (NULL, renderer, "text", 0, NULL);
	gtk_tree_view_append_column (fs->FacesTree, column);
	GtkTreeSelection *selection = gtk_tree_view_get_selection (fs->FacesTree);
	fs->FaceSel = selection;
	fs->FaceSignal = g_signal_connect (fs->FaceSel, "changed", G_CALLBACK (on_select_face), fs);
	// Initialize sizes list
	fs->SizeList = gtk_list_store_new (1, G_TYPE_INT);
	w = gtk_tree_view_new_with_model (GTK_TREE_MODEL (fs->SizeList));
	fs->SizeTree =  GTK_TREE_VIEW (w);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (w), false);
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (NULL, renderer, "text", 0, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (w), column);
	GtkTreeIter iter;
	for (i = 0; i < (int) G_N_ELEMENTS (font_sizes); i++) {
		gtk_list_store_append (fs->SizeList, &iter);
		gtk_list_store_set (fs->SizeList, &iter,
				  0, font_sizes[i],
				  -1);
	}
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (w));
	fs->SizeSel = selection;
	fs->SizeSignal = g_signal_connect (fs->SizeSel, "changed", G_CALLBACK (on_select_size), fs);
	sc = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sc), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (sc), w);
	gtk_table_attach (table, sc, 2, 3, 2, 3,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
	w = gtk_entry_new ();
	fs->m_SizeEntry = GTK_ENTRY (w);
	g_signal_connect (w, "activate", G_CALLBACK (on_size_activate), fs);
	g_signal_connect_after (w, "focus_out_event", G_CALLBACK (on_size_focus_out), fs);
	gcp_font_sel_set_size_full (fs, true);
	gtk_table_attach (table, w, 2, 3, 1, 2,
			(GtkAttachOptions) (0),
			(GtkAttachOptions) 0, 0, 0);
	PangoContext *pc = gtk_widget_get_pango_context (w);
	PangoLayout *pl = pango_layout_new (pc);
	pango_layout_set_text (pl, "0000000", -1);
	PangoRectangle rect;
	pango_layout_get_extents (pl, NULL, &rect);
	g_object_unref (G_OBJECT (pl));
	gtk_widget_set_size_request (sc, -1, rect.height / PANGO_SCALE * 12);
	gtk_widget_set_size_request (w, rect.width / PANGO_SCALE, -1);
	pango_context_list_families (pc, &families, &nb);
	fs->FamilyList = gtk_list_store_new (1, G_TYPE_STRING);
	gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (fs->FamilyList), 0, GTK_SORT_ASCENDING);
	fs->FamilyTree =  GTK_TREE_VIEW (gtk_tree_view_new_with_model (GTK_TREE_MODEL (fs->FamilyList)));
	gtk_tree_view_set_headers_visible (fs->FamilyTree, false);
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (NULL, renderer, "text", 0, NULL);
	gtk_tree_view_append_column (fs->FamilyTree, column);
	string name;
	for (i = 0; i < nb; i++) {
		PangoFontFace **faces;
		int *sizes, n;
		pango_font_family_list_faces (families[i], &faces, &n);
		if (n <= 0) {
			g_free (faces);
			continue;
		}
		pango_font_face_list_sizes (faces[0], &sizes, &n);
		if (n > 0) {	// Do not use bitmap fonts
			g_free (faces);
			g_free (sizes);
			continue;
		}
		name = pango_font_family_get_name (families[i]);
		fs->Families[name] = (PangoFontFamily*) g_object_ref (families[i]);
		gtk_list_store_append (fs->FamilyList, &iter);
		gtk_list_store_set (fs->FamilyList, &iter,
				  0, name.c_str (),
				  -1);
		g_free (faces);
		g_free (sizes);
	}
	fs->FamilySel = gtk_tree_view_get_selection (fs->FamilyTree);
	gtk_tree_selection_set_mode (fs->FamilySel, GTK_SELECTION_BROWSE);
	fs->FamilySignal = g_signal_connect (G_OBJECT (fs->FamilySel), "changed", G_CALLBACK (on_select_family), fs);
	sc = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sc), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (sc), GTK_WIDGET (fs->FamilyTree));
	gtk_table_attach (table, sc, 0, 1, 1, 3,
			(GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
			(GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
	g_free (families);
}

GSF_CLASS (GcpFontSel, gcp_font_sel,
	   gcp_font_sel_class_init, gcp_font_sel_init,
	   GTK_TYPE_BIN)
