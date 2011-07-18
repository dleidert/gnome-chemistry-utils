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
#include <vector>

#include <gsf/gsf-impl-utils.h>

struct _GcrGrid
{
	GtkLayout base;
	unsigned cols, rows, allocated_rows;
	int col, row;
	int first_visible;
	unsigned nb_visible;
	int header_width, row_height, width, *col_widths, line_offset, scroll_width;
	GtkAdjustment *vadj;
	GtkWidget *scroll;
	std::string *titles;
	GType *types;
	std::vector < std::string * > row_data; // storing as string since this is what is displayed
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
	unsigned i, j;
	GtkAllocation alloc;
	gtk_widget_get_allocation (w, &alloc);
	gtk_style_context_save (ctxt);
	gtk_style_context_add_class (ctxt, GTK_STYLE_CLASS_BUTTON);
	int pos = grid->header_width, y = grid->line_offset, width;
	PangoLayout *l = gtk_widget_create_pango_layout (w, "");
	cairo_save (cr);
	cairo_set_source_rgb (cr, 0.7, 0.7, 0.7);
	cairo_rectangle (cr, 0, 0, grid->width, grid->row_height + 1);
	cairo_fill (cr);
	cairo_restore (cr);
	gtk_render_background (ctxt, cr, 0, 0, grid->header_width + 1, grid->row_height + 1);
	gtk_render_frame (ctxt, cr, 0, 0, grid->header_width + 1, grid->row_height + 1);
	for (i = 0; i < grid->cols; i++) {
		gtk_style_context_set_state (ctxt, (static_cast < int > (i) == grid->col)? GTK_STATE_FLAG_ACTIVE: GTK_STATE_FLAG_NORMAL);
		gtk_render_background (ctxt, cr, pos, 0, grid->col_widths[i] + 1, grid->row_height + 1);
		gtk_render_frame (ctxt, cr, pos, 0, grid->col_widths[i] + 1, grid->row_height + 1);
		pango_layout_set_markup (l, grid->titles[i].c_str (), -1);
		pango_layout_get_pixel_size (l, &width, NULL);
		cairo_move_to (cr, pos + (grid->col_widths[i] - width) / 2, y);
		pango_cairo_show_layout (cr, l);
		pos += grid->col_widths[i];
	}
	gtk_style_context_set_state (ctxt, GTK_STATE_FLAG_NORMAL);
	gtk_render_background (ctxt, cr, pos, 0, grid->scroll_width + 1, grid->row_height + 1);
	gtk_render_frame (ctxt, cr, pos, 0, grid->scroll_width + 1, grid->row_height + 1);
	y = grid->row_height;
	cairo_set_line_width (cr, 1.);
	// draw row headers
	int row = grid->first_visible;
	unsigned max = (grid->nb_visible >= grid->rows - grid->first_visible)? grid->rows - grid->first_visible: grid->nb_visible + 1; 
	for (j = 0; j < max; j++) {
		cairo_save (cr);
		cairo_set_source_rgb (cr, 0.7, 0.7, 0.7);
		cairo_rectangle (cr, 0, y, grid->header_width + 1, grid->row_height + 1);
		cairo_fill (cr);
		cairo_restore (cr);
		gtk_style_context_set_state (ctxt, (row == grid->row)? GTK_STATE_FLAG_ACTIVE: GTK_STATE_FLAG_NORMAL);
		gtk_render_background (ctxt, cr, 0, y, grid->header_width + 1, grid->row_height + 1);
		gtk_render_frame (ctxt, cr, 0, y, grid->header_width + 1, grid->row_height + 1);
		char *buf = g_strdup_printf("%d", ++row);
		pango_layout_set_text (l, buf, -1);
		pango_layout_get_pixel_size (l, &width, NULL);
		cairo_move_to (cr, (grid->header_width - width) / 2, y + grid->line_offset);
		pango_cairo_show_layout (cr, l);
		g_free (buf);
		y += grid->row_height;
	}
	y = grid->row_height;
	cairo_save (cr);
	cairo_rectangle (cr, grid->header_width, y, alloc.width - grid->header_width, alloc.height - y);
	cairo_clip (cr);
	cairo_set_line_width (cr, 1.);
	// draw cells
	row = grid->first_visible;
	for (j = 0; j < max; j++) {
		pos = grid->header_width;
		for (i = 0; i < grid->cols; i++) {
			cairo_save (cr);
			cairo_set_source_rgb (cr, 0.7, 0.7, 0.7);
			cairo_rectangle (cr, pos + .5, y + .5, grid->col_widths[i], grid->row_height);
			cairo_stroke (cr);
			cairo_restore (cr);
			// FIXME: manage booleans, not only strings
			pango_layout_set_text (l, grid->row_data[row][i].c_str(), -1);
			pango_layout_get_pixel_size (l, &width, NULL);
			cairo_move_to (cr, pos + (grid->col_widths[i] - width) / 2, y + grid->line_offset);
			pango_layout_set_markup (l, grid->row_data[row][i].c_str(), -1);
			pango_cairo_show_layout (cr, l);
			pos += grid->col_widths[i];
		}
		row++;
		y += grid->row_height;
	}
	cairo_restore (cr);
	gtk_style_context_restore (ctxt);
	return parent_class->draw (w, cr);
}

static void gcr_grid_get_preferred_height (GtkWidget *w, int *min, int *preferred)
{
	*min = *preferred = reinterpret_cast < GcrGrid * > (w)->row_height * 6 + 1;
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
	delete [] grid->types;
	for (unsigned i = 0 ; i < grid->rows; i++)
		delete [] grid->row_data[i];
	reinterpret_cast < GObjectClass * > (parent_class)->finalize (obj);
}

static void gcr_grid_size_allocate (GtkWidget *w, GtkAllocation *alloc)
{
	GcrGrid *grid = GCR_GRID (w);
	g_object_set (G_OBJECT (grid->scroll), "height-request", alloc->height - grid->row_height, NULL); //default size
	grid->nb_visible = alloc->height / grid->row_height - 1; // -1 to avoid counting the header
	gtk_adjustment_set_page_size (grid->vadj, static_cast < double > (grid->nb_visible) / grid->rows);
	gtk_adjustment_set_upper (grid->vadj, (grid->nb_visible < grid->rows)? grid->rows - grid->nb_visible: .1);
	if (grid->rows < grid->nb_visible + grid->first_visible) {
		grid->first_visible = (grid->nb_visible < grid->rows)? grid->rows - grid->nb_visible: 0;
		gtk_adjustment_set_value (grid->vadj, grid->first_visible);
	}
	parent_class->size_allocate (w, alloc);
}

static gboolean gcr_grid_button_press_event (GtkWidget *widget, GdkEventButton *event)
{
	GcrGrid *grid = GCR_GRID (widget);
	int x = (event->y > grid->row_height)? grid->first_visible + event->y / grid->row_height: - 1, i;
	grid->row = (x < 0 || x >= static_cast < int > (grid->rows))? -1: x;
	if (grid->row < 0) {
		grid->col = -1;
		gtk_widget_queue_draw (widget);
		return true;
	}
	grid->col = -1;
	x = grid->header_width;
	if (event->x >= x)
		for (i = 0; i < static_cast < int > (grid->cols); i++) {
			x += grid->col_widths[i];
			if (event->x < x) {
				grid->col = i;
				break;
			}
		}
	if (grid->col < 0)
		grid->row = -1;
	gtk_widget_queue_draw (widget);
	return true;
}

static void gcr_grid_class_init (GtkWidgetClass *klass)
{
	parent_class = reinterpret_cast < GtkWidgetClass * > (g_type_class_peek_parent (klass));
	klass->draw = gcr_grid_draw;
	klass->size_allocate = gcr_grid_size_allocate;
	klass->button_press_event = gcr_grid_button_press_event;
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
	grid->col = grid->row = -1; // nothing selected
}

GSF_CLASS (GcrGrid, gcr_grid, gcr_grid_class_init, gcr_grid_init, GTK_TYPE_LAYOUT)

// signal callbacks

void gcr_grid_adjustment_changed (GtkAdjustment *adj, GcrGrid *grid)
{
	grid->first_visible = ceil (gtk_adjustment_get_value (adj));
	gtk_widget_queue_draw (GTK_WIDGET (grid));
}

// implementation

GtkWidget *gcr_grid_new (G_GNUC_UNUSED char const *col_title, GType col_type, ...)
{
	g_return_val_if_fail (col_title && g_utf8_validate (col_title, -1, NULL), NULL);
	GcrGrid *grid = GCR_GRID (g_object_new (GCR_TYPE_GRID, NULL));
	gtk_widget_add_events (GTK_WIDGET (grid),
						   GDK_POINTER_MOTION_MASK |
						   GDK_BUTTON_MOTION_MASK |
						   GDK_BUTTON_PRESS_MASK |
						   GDK_BUTTON_RELEASE_MASK |
    					   GDK_LEAVE_NOTIFY_MASK
						   );
	std::list < char const * > titles;
	std::list < GType > types;
	titles.push_front (col_title);
	types.push_front (col_type);
	va_list args;
	va_start (args, col_type);
	int int_size, double_size, col_size, title_size, height;
	PangoLayout *layout = gtk_widget_create_pango_layout (reinterpret_cast <GtkWidget *> (grid), "000000");
	pango_layout_get_pixel_size (layout, &int_size, &height);
	pango_layout_set_text (layout, "0.00000000", -1);
	pango_layout_get_pixel_size (layout, &double_size, NULL);
	grid->width = 0; // FIXME: evalutate the real size from columns
	GtkWidget *w = gtk_button_new_with_label ("00");
	gtk_widget_get_preferred_height (w, &grid->row_height, NULL);
	grid->line_offset = (grid->row_height - height) / 2;
	gtk_widget_get_preferred_width (w, &grid->header_width, NULL);
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
	grid->types = new GType[grid->cols];
	unsigned i;
	std::list <char const *>::iterator title = titles.begin ();
	std::list <GType>::iterator type = types.begin ();
	grid->width = grid->header_width;
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
		grid->types[i] = *type;
	}
	g_object_unref (layout);
	GdkRGBA rgba = {1.0, 1.0, 1.0, 1.0};
	gtk_widget_override_background_color (reinterpret_cast <GtkWidget *> (grid), GTK_STATE_FLAG_NORMAL, &rgba);
	// add a vertical scrollbar
	grid->vadj = gtk_adjustment_new (0, 0, 1, 1, 10, 0);
	grid->scroll = gtk_scrollbar_new (GTK_ORIENTATION_VERTICAL, grid->vadj);
	g_object_set (G_OBJECT (grid->scroll), "height-request", grid->row_height * 5 + 1, NULL); //default size
	gtk_layout_put (GTK_LAYOUT (grid), grid->scroll, grid->width + 1, grid->row_height + 1);
	gtk_widget_get_preferred_width (grid->scroll, &grid->scroll_width, NULL);
	grid->width += grid->scroll_width + 1;
	// add signals
	g_signal_connect (grid->vadj, "value-changed", G_CALLBACK (gcr_grid_adjustment_changed), grid);
	return reinterpret_cast <GtkWidget *> (grid);
}

int gcr_grid_get_int (GcrGrid *grid, unsigned row, unsigned column)
{
	g_return_val_if_fail (GCR_IS_GRID (grid) && row < grid->rows && column < grid->cols, 0);
	return 0;
}

double gcr_grid_get_double (GcrGrid *grid, unsigned row, unsigned column)
{
	g_return_val_if_fail (GCR_IS_GRID (grid) && row < grid->rows && column < grid->cols, go_nan);
	return go_nan;
}

bool gcr_grid_get_boolean (GcrGrid *grid, unsigned row, unsigned column)
{
	g_return_val_if_fail (GCR_IS_GRID (grid) && row < grid->rows && column < grid->cols, false);
	return false;
}

void gcr_grid_set_int (GcrGrid *grid, unsigned row, unsigned column, int value)
{
	g_return_if_fail (GCR_IS_GRID (grid) && row < grid->rows && column < grid->cols);
}

void gcr_grid_set_double (GcrGrid *grid, unsigned row, unsigned column, double value)
{
	g_return_if_fail (GCR_IS_GRID (grid) && row < grid->rows && column < grid->cols);
}

void gcr_grid_set_boolean (GcrGrid *grid, unsigned row, unsigned column, bool value)
{
	g_return_if_fail (GCR_IS_GRID (grid) && row < grid->rows && column < grid->cols);
}

unsigned gcr_grid_append_row (GcrGrid *grid,...)
{
	g_return_val_if_fail (GCR_IS_GRID (grid) && grid->cols > 0, 0);
	unsigned row = grid->rows++, col;
	char *buf;
	if (grid->row_data.capacity () < grid->rows)
		grid->row_data.resize (grid->row_data.capacity () + 5);
	grid->row_data[row] = new std::string[grid->cols];
	va_list args;
	va_start (args, grid);
	for (col = 0; col < grid->cols; col++) {
		switch (grid->types[col]) {
		case G_TYPE_INT:
			buf = g_strdup_printf ("%d", va_arg (args, int));
			grid->row_data[row][col] = buf;
			g_free (buf);
			break;
		case G_TYPE_DOUBLE:
			buf = g_strdup_printf ("%f", va_arg (args, double));
			grid->row_data[row][col] = buf;
			g_free (buf);
			break;
		case G_TYPE_BOOLEAN:
			grid->row_data[row][col] = (va_arg (args, double))? "t": "f";
			break;
		default:
			// do nothing, unsupported type
			break;
		}
	}
	va_end (args);
	return row;
}
