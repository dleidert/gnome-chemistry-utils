// -*- C++ -*-

/*
 * Gnome Crystal
 * grid.cc
 *
 * Copyright (C) 2010 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "grid.h"
#include <gcugtk/marshalers.h>
#include <goffice/goffice.h>
#include <list>
#include <string>

#include <gsf/gsf-impl-utils.h>

struct _GcrGrid
{
	GtkLayout base;
	unsigned cols, rows;
	int header_height, row_height, width, *col_widths;
	GtkWidget *headers, *contents; /* are these needed */
	GtkAdjustment *hadj, *vadj;
	std::string *titles;
};

static GtkWidgetClass *parent_class;

enum {
  VALUE_CHANGED,
  ROW_ADDED,
  ROW_DELETED,
  LAST_SIGNAL
};

static gulong gcr_grid_signals [LAST_SIGNAL] = { 0, };

typedef struct
{
	GtkLayoutClass parent_class;
	void (*value_changed_event) (GcrGrid *grid, unsigned row, unsigned column, gpointer data);
	void (*row_added_event) (GcrGrid *grid, unsigned row, gpointer data);
	void (*row_deleted_event) (GcrGrid *grid, unsigned row, gpointer data);
} GcrGridClass;

static gboolean gcr_grid_draw (GtkWidget *w, cairo_t* cr)
{
	GcrGrid *grid = reinterpret_cast < GcrGrid * > (w);
	GtkStyleContext *ctxt = gtk_widget_get_style_context (w);
	gtk_style_context_save (ctxt);
	gtk_style_context_add_class (ctxt, GTK_STYLE_CLASS_BUTTON);
	int pos = 0, y = (grid->header_height - grid->row_height) / 2, width;
	PangoLayout *l = gtk_widget_create_pango_layout (w, "");
	for (unsigned i = 0; i < grid->cols; i++) {
		gtk_render_background (ctxt, cr, pos, 0, grid->col_widths[i], grid->header_height);
		gtk_render_frame (ctxt, cr, pos, 0, grid->col_widths[i], grid->header_height);
		pango_layout_set_markup (l, grid->titles[i].c_str (), -1);
		pango_layout_get_pixel_size (l, &width, NULL);
		cairo_move_to (cr, pos + (grid->col_widths[i] - width) / 2, y);
		pango_cairo_show_layout (cr, l);
		pos += grid->col_widths[i];
	}
	gtk_style_context_restore (ctxt);
	return true;
}

static void gcr_grid_get_preferred_height (GtkWidget *w, int *min, int *preferred)
{
	*min = *preferred = reinterpret_cast < GcrGrid * > (w)->row_height * 5 + reinterpret_cast < GcrGrid * > (w)->header_height;
}

static void gcr_grid_get_preferred_width (GtkWidget *w, int *min, int *preferred)
{
	*min = *preferred = reinterpret_cast < GcrGrid * > (w)->width;
}

static void gcr_grid_finalize (GObject *obj)
{
	GcrGrid *grid = reinterpret_cast < GcrGrid * > (obj);
	delete [] grid->col_widths;
	delete [] grid->titles;
	reinterpret_cast < GObjectClass * > (parent_class)->finalize (obj);
}

static void gcr_grid_class_init (GtkWidgetClass *klass)
{
	parent_class = reinterpret_cast < GtkWidgetClass * > (g_type_class_peek_parent (klass));
	klass->draw = gcr_grid_draw;
	reinterpret_cast < GObjectClass * > (klass)->finalize = gcr_grid_finalize;
	klass->get_preferred_height = gcr_grid_get_preferred_height;
	klass->get_preferred_width = gcr_grid_get_preferred_width;
	gcr_grid_signals[VALUE_CHANGED] = g_signal_new ("value-changed",
	                                                G_TYPE_FROM_CLASS(klass),
				                                    G_SIGNAL_RUN_LAST,
				                                    G_STRUCT_OFFSET(GcrGridClass, value_changed_event),
				                                    NULL, NULL,
				                                    gcu__VOID__UINT_UINT,
				                                    G_TYPE_NONE, 2,
				                                    G_TYPE_UINT, G_TYPE_UINT
				                                    );
	gcr_grid_signals[ROW_ADDED] = g_signal_new ("row-added",
	                                            G_TYPE_FROM_CLASS(klass),
				                                G_SIGNAL_RUN_LAST,
				                                G_STRUCT_OFFSET(GcrGridClass, row_added_event),
				                                NULL, NULL,
				                                g_cclosure_marshal_VOID__UINT,
				                                G_TYPE_NONE, 1,
				                                G_TYPE_UINT, G_TYPE_UINT
				                                );
	gcr_grid_signals[ROW_DELETED] = g_signal_new ("row-changed",
	                                              G_TYPE_FROM_CLASS(klass),
				                                  G_SIGNAL_RUN_LAST,
				                                  G_STRUCT_OFFSET(GcrGridClass, row_deleted_event),
				                                  NULL, NULL,
				                                  g_cclosure_marshal_VOID__UINT,
				                                  G_TYPE_NONE, 2,
				                                  G_TYPE_UINT, G_TYPE_UINT
				                                  );
}

static void
gcr_grid_init (G_GNUC_UNUSED GcrGrid *grid)
{
}

GSF_CLASS (GcrGrid, gcr_grid, gcr_grid_class_init, gcr_grid_init, GTK_TYPE_LAYOUT)

GtkWidget *gcr_grid_new (G_GNUC_UNUSED char const *col_title, GType col_type, ...)
{
	g_return_val_if_fail (col_title && g_utf8_validate (col_title, -1, NULL), NULL);
	GcrGrid *grid = GCR_GRID (g_object_new (GCR_TYPE_GRID, NULL));
	std::list < char const * > titles;
	std::list < GType > types;
	titles.push_front (col_title);
	types.push_front (col_type);
	va_list args;
	va_start (args, col_type);
	int int_size, double_size, col_size, title_size;
	PangoLayout *layout = gtk_widget_create_pango_layout (reinterpret_cast <GtkWidget *> (grid), "000000");
	pango_layout_get_pixel_size (layout, &int_size, &grid->row_height);
	pango_layout_set_text (layout, "0.00000000", -1);
	pango_layout_get_pixel_size (layout, &double_size, NULL);
	grid->row_height += 1; // add room for cell borders
	grid->width = 0; // FIXME: evalutate the real size from columns
	GtkWidget *w = gtk_button_new_with_label ("0");
	gtk_widget_get_preferred_height (w, &grid->header_height, NULL);
	g_object_ref_sink (w);
	while (1) {
		col_title = va_arg (args, char const *);
		if (!col_title)
			break;
		col_type = va_arg (args, GType);
		if (g_utf8_validate (col_title, -1, NULL)) {
			titles.push_back (col_title);
			types.push_back (col_type);
		}
	}
	va_end (args);
	grid->cols = titles.size ();
	grid->col_widths = new int[grid->cols];
	grid->titles = new std::string[grid->cols];
	unsigned i;
	std::list <char const *>::iterator title = titles.begin ();
	std::list <GType>::iterator type = types.begin ();
	for (i = 0; i < grid->cols; i++, title++, type++) {
		switch (*type) {
		case G_TYPE_INT:
			col_size = int_size;
			break;
		case G_TYPE_DOUBLE:
			col_size = double_size;
			break;
		default:
			col_size = 0;
			break;
		}
		// now evaluate the size of the title
		pango_layout_set_markup (layout, *title, -1);
		pango_layout_get_pixel_size (layout, &title_size, NULL);
		if (col_size < title_size)
			col_size = title_size;
		col_size += 6;	// add some padding
		grid->col_widths[i] = col_size;
		grid->width += col_size;
		grid->titles[i] = *title;
	}
/*	grid->cols = titles.size ();
	grid->headers = GTK_WIDGET (g_object_new (GOC_TYPE_CANVAS, NULL));
	GtkBox *box = GTK_BOX (grid);
	gtk_box_pack_start (box, GTK_WIDGET (grid->headers), FALSE, TRUE, 0);
	grid->hadj = GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 1, 1, 0, 1));
	grid->vadj = GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 1, 1, 0, 1));
	GtkWidget *scrolled = gtk_scrolled_window_new (grid->hadj, grid->vadj);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
	gtk_box_pack_start (box, scrolled, FALSE, TRUE, 0);

	GtkTable *table = GTK_TABLE (grid);
	gtk_table_resize (table, 2, grid->cols);
	for (unsigned i = 0; i < grid->cols; i++) {
		col_title = titles.front ();
		titles.pop_front ();
		GtkWidget *label = gtk_label_new (col_title);
		gtk_table_attach (table, label, i, i + 1, 0, 1, GTK_EXPAND, static_cast <GtkAttachOptions> (0), 0, 0);
	}*/
	g_object_unref (layout);
	GdkRGBA rgba = {1.0, 1.0, 1.0, 1.0};
	gtk_widget_override_background_color (reinterpret_cast <GtkWidget *> (grid), GTK_STATE_FLAG_NORMAL, &rgba);
	return reinterpret_cast <GtkWidget *> (grid);
}

int gcr_grid_get_int (GcrGrid *grid, unsigned row, unsigned column)
{
	return 0;
}

double gcr_grid_get_double (GcrGrid *grid, unsigned row, unsigned column)
{
	return go_nan;
}

bool gcr_grid_get_boolean (GcrGrid *grid, unsigned row, unsigned column)
{
	return false;
}

void gcr_grid_set_int (GcrGrid *grid, unsigned row, unsigned column, int value)
{
}

void gcr_grid_set_double (GcrGrid *grid, unsigned row, unsigned column, double value)
{
}

void gcr_grid_set_boolean (GcrGrid *grid, unsigned row, unsigned column, bool value)
{
}

unsigned gcr_grid_append_row (GcrGrid *grid,...)
{
}
