// -*- C++ -*-

/*
 * Gnome Crystal
 * grid.cc
 *
 * Copyright (C) 2010-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include <cstring>

#include <gsf/gsf-impl-utils.h>

#define CURSOR_ON_TIME 800
#define CURSOR_OFF_TIME 400

struct _GcrGrid
{
	GtkLayout base;
	unsigned cols, rows, allocated_rows;
	int col, row;
	int first_visible;
	unsigned nb_visible;
	int header_width, row_height, width, *col_widths, line_offset, scroll_width, *min_widths, cols_min_width;
	int cursor_index, sel_start;
	GtkAdjustment *vadj;
	GtkWidget *scroll;
	std::string *titles;
	GType *types;
	std::vector < std::string * > row_data; // storing as string since this is what is displayed
	bool cursor_visible;
	unsigned long cursor_signal;
	std::string *orig_string;
};

static GtkWidgetClass *parent_class;

enum {
	VALUE_CHANGED,
	ROW_SELECTED,
	LAST_SIGNAL
};

static gulong gcr_grid_signals [LAST_SIGNAL] = { 0, };

typedef struct
{
	GtkLayoutClass parent_class;
	void (*value_changed_event) (GcrGrid *grid, unsigned row, unsigned column, gpointer data);
	void (*row_added_event) (GcrGrid *grid, unsigned row, gpointer data);
	void (*row_selected_event) (GcrGrid *grid, unsigned row, gpointer data);
	void (*row_deleted_event) (GcrGrid *grid, unsigned row, gpointer data);
} GcrGridClass;

// static functions

static bool gcr_grid_validate_change (GcrGrid *grid)
{
	std::string new_string = grid->row_data[grid->row][grid->col];
	if (new_string == *grid->orig_string)
		return true;
	return false;
}

// overriden methods

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
	gtk_render_background (ctxt, cr, pos, 0, grid->scroll_width, grid->row_height + 1);
	gtk_render_frame (ctxt, cr, pos, 0, grid->scroll_width, grid->row_height + 1);
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
	// show a rectangle around current selection
	if (grid->row >= 0) {
		pos = grid->header_width;
		for (i = 0; static_cast < int > (i) < grid->col; i++)
			pos += grid->col_widths[i];
		cairo_save (cr);
		cairo_rectangle (cr, pos + .5, y + (grid->row - grid->first_visible) * grid->row_height + .5, grid->col_widths[grid->col], grid->row_height);
		cairo_set_line_width (cr, 3.);
		cairo_stroke_preserve (cr);
		cairo_restore (cr);
	}
	cairo_set_line_width (cr, 1.);
	// draw cells
	row = grid->first_visible;
	for (j = 0; j < max; j++) {
		pos = grid->header_width;
		for (i = 0; i < grid->cols; i++) {
			cairo_save (cr);
			cairo_rectangle (cr, pos + .5, y + .5, grid->col_widths[i], grid->row_height);
			cairo_set_source_rgb (cr, 0.7, 0.7, 0.7);
			cairo_stroke (cr);
			cairo_restore (cr);
			// FIXME: manage booleans, not only strings
			pango_layout_set_text (l, grid->row_data[row][i].c_str(), -1);
			pango_layout_get_pixel_size (l, &width, NULL);
			cairo_move_to (cr, pos + (grid->col_widths[i] - width) / 2, y + grid->line_offset);
			pango_layout_set_markup (l, grid->row_data[row][i].c_str(), -1);
			pango_cairo_show_layout (cr, l);
			if (grid->cursor_visible && static_cast < int > (j) == grid->row && static_cast < int > (i) == grid->col) {
				PangoRectangle rect;
				pango_layout_get_cursor_pos (l, grid->cursor_index, &rect, NULL);
				cairo_move_to (cr, pos + (grid->col_widths[i] - width) / 2 + rect.x / PANGO_SCALE + .5, y + grid->line_offset + rect.y / PANGO_SCALE);
				cairo_rel_line_to (cr, 0, rect.height / PANGO_SCALE);
				cairo_stroke (cr);
			}
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

static void gcr_grid_unrealize (GtkWidget *w)
{
	GcrGrid *grid = GCR_GRID (w);
	if (grid->cursor_signal > 0)
		g_source_remove (grid->cursor_signal);
	parent_class->unrealize (w);
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
	gtk_layout_move (GTK_LAYOUT (grid), grid->scroll, alloc->width - grid->scroll_width, grid->row_height + 1);
	g_object_set (G_OBJECT (grid->scroll), "height-request", alloc->height - grid->row_height - 1, NULL); //default size
	grid->nb_visible = alloc->height / grid->row_height - 1; // -1 to avoid counting the header
	if (grid->rows) {
		gtk_adjustment_set_page_size (grid->vadj, static_cast < double > (grid->nb_visible) / grid->rows);
		gtk_adjustment_set_upper (grid->vadj, (grid->nb_visible < grid->rows)? grid->rows - grid->nb_visible: .1);
		if (grid->rows < grid->nb_visible + grid->first_visible) {
			grid->first_visible = (grid->nb_visible < grid->rows)? grid->rows - grid->nb_visible: 0;
			gtk_adjustment_set_value (grid->vadj, grid->first_visible);
		}
	} else
		gtk_adjustment_set_page_size (grid->vadj, 1.);
	// adjust column widths
	double ratio = static_cast < double > (alloc->width - grid->header_width - grid->scroll_width) / grid->cols_min_width;
	if (ratio < 0.)
		ratio = 1.;
	double last_pos = 0., pos = 0.;
	for (unsigned i = 0; i < grid->cols; i++) {
		pos += grid->min_widths[i];
		grid->col_widths[i] = pos * ratio - last_pos;
		last_pos += grid->col_widths[i];
	}
	parent_class->size_allocate (w, alloc);
}

static gint on_blink (gpointer data)
{
	GcrGrid *grid = GCR_GRID (data);
	grid->cursor_signal = g_timeout_add (((grid->cursor_visible)? CURSOR_OFF_TIME: CURSOR_ON_TIME), on_blink, data);

	grid->cursor_visible = !grid->cursor_visible;
	gtk_widget_queue_draw (GTK_WIDGET (data));
	/* Remove ourself */
	return false;
}

static gboolean gcr_grid_button_press_event (GtkWidget *widget, GdkEventButton *event)
{
	GcrGrid *grid = GCR_GRID (widget);
	int x = grid->first_visible + event->y / grid->row_height - 1, i, new_row, new_col;
	new_row = (x < 0 || x > static_cast < int > (grid->rows))? -1: x;
	if (new_row < 0)
		new_col = -1;
	else {
		new_col = -1;
		x = grid->header_width;
		if (event->x >= x)
			for (i = 0; i < static_cast < int > (grid->cols); i++) {
				x += grid->col_widths[i];
				if (event->x < x) {
					new_col = i;
					break;
				}
			}
		if (new_col < 0)
			new_row = -1;
	}
	if (grid->col != new_col || grid->row != new_row) {
		if (grid->row >= 0 && !gcr_grid_validate_change (grid))
			return true;
		if (new_row != grid->row)
			g_signal_emit (grid, gcr_grid_signals[ROW_SELECTED], 0, new_row);
		grid->col = new_col;
		grid->row = new_row;
		// evaluate the cursor position if any
		if (grid->col >= 0) {
			switch (grid->types[grid->col]) {
			case G_TYPE_INT:
			case G_TYPE_DOUBLE: {
				x -=  grid->col_widths[grid->col];
				PangoLayout *l = gtk_widget_create_pango_layout (widget, grid->row_data[grid->row][grid->col].c_str());
				int xpos, startx;
				pango_layout_get_pixel_size (l, &xpos, NULL);
				startx = x + (grid->col_widths[grid->col] - xpos) / 2;
				xpos = event->x - startx;
				int index, trailing;
				pango_layout_xy_to_index (l, xpos * PANGO_SCALE, 0, &index, &trailing);
				grid->cursor_index = index + trailing;
				break;
			}
			case G_TYPE_BOOLEAN:
				grid->cursor_index = -1;
				// nothing to do, just wait and see if the mouse button is released inside the cell
				break;
			default:
				grid->cursor_index = -1;
				g_critical ("Unsupported type.");
				break;
			}
			*grid->orig_string = grid->row_data[grid->row][grid->col];
			gtk_widget_grab_focus (widget);
		} else
			grid->cursor_index = -1;
	}
	if (grid->cursor_index >= 0 && grid->cursor_signal == 0) {
		grid->cursor_signal = g_timeout_add (CURSOR_ON_TIME, on_blink, grid);
		grid->cursor_visible = true;
	} else if (grid->cursor_index < 0 && grid->cursor_signal != 0) {
		g_source_remove (grid->cursor_signal);
		grid->cursor_signal = 0;
	}
	gtk_widget_queue_draw (widget);
	return true;
}

static gboolean gcr_grid_key_press_event (GtkWidget *widget, GdkEventKey *event)
{
	GcrGrid *grid = GCR_GRID (widget);
	if (grid->row < 0)
		return false;
	int new_row = grid->row, new_col = grid->col, new_index = grid->cursor_index;
	char new_char = 0;
	switch (event->keyval) {
	case GDK_KEY_Shift_L:
	case GDK_KEY_Shift_Lock:
	case GDK_KEY_Shift_R:
		return true;
	case GDK_KEY_Tab:
	case GDK_KEY_ISO_Left_Tab:
		if (!gcr_grid_validate_change (grid))
			return true;
		if (event->state & GDK_CONTROL_MASK)
			return false; // give up the grab
		if (event->state & GDK_SHIFT_MASK) {
			if (new_col > 0)
				new_col--;
			else if (new_row > 0) {
				new_row--;
				new_col = grid->cols - 1;
			}
		} else if (grid->col < static_cast < int > (grid->cols) - 1)
			new_col++;
		else if (grid->row < static_cast < int > (grid->rows) - 1) {
			new_row++;
			new_col = 0;
		} else
			return true;
		// FIXME: select the whole cell
		new_index = 0;
		break;
	case GDK_KEY_Left:
		if (grid->cursor_index > 0)
			new_index = g_utf8_prev_char (grid->row_data[new_row][new_col].c_str () + grid->cursor_index) - grid->row_data[new_row][new_col].c_str ();
		else {
			if (new_col > 0)
				new_col--;
			else if (new_row > 0) {
				new_row--;
				new_col = grid->cols - 1;
			}
			new_index = grid->row_data[new_row][new_col].length ();
		}
		break;
	case GDK_KEY_Right:
		if (grid->cursor_index < static_cast < int > (grid->row_data[new_row][new_col].length ()))
			new_index = g_utf8_next_char (grid->row_data[new_row][new_col].c_str () + grid->cursor_index) - grid->row_data[new_row][new_col].c_str ();
		else {
			if (new_col < static_cast < int > (grid->cols) - 1)
				new_col++;
			else if (grid->row < static_cast < int > (grid->rows) - 1) {
				new_row++;
				new_col = 0;
			} else
				return true;
			new_index = 0;
		}
		break;
	case GDK_KEY_KP_Subtract:
	case GDK_KEY_minus:
		break;
	case GDK_KEY_0:
	case GDK_KEY_1:
	case GDK_KEY_2:
	case GDK_KEY_3:
	case GDK_KEY_4:
	case GDK_KEY_5:
	case GDK_KEY_6:
	case GDK_KEY_7:
	case GDK_KEY_8:
	case GDK_KEY_9:
		new_char = '0' + event->keyval - GDK_KEY_0;
		break;
	case GDK_KEY_KP_0:
	case GDK_KEY_KP_1:
	case GDK_KEY_KP_2:
	case GDK_KEY_KP_3:
	case GDK_KEY_KP_4:
	case GDK_KEY_KP_5:
	case GDK_KEY_KP_6:
	case GDK_KEY_KP_7:
	case GDK_KEY_KP_8:
	case GDK_KEY_KP_9:
		new_char = '0' + event->keyval - GDK_KEY_KP_0;
		break;
	case GDK_KEY_period:
		break;
	case GDK_KEY_comma:
		break;
	case GDK_KEY_KP_Decimal:
		break;
	case GDK_KEY_space:
		break;
	default:
		return true;
	}
	if (new_char > 0) {
		// insert the new char
		if (grid->cursor_index == 0 && !strncmp (grid->row_data[new_row][new_col].c_str (), "−", strlen ("−")))
		    return true;	// do not insert a figure before the minus sign
		grid->row_data[new_row][new_col].insert (new_index, 1, new_char);
		new_index++;
	}
	if (new_row != grid->row || new_col != grid->col) {
		if (grid->row >= 0 && !gcr_grid_validate_change (grid))
			return true;
		if (new_row != grid->row)
			g_signal_emit (grid, gcr_grid_signals[ROW_SELECTED], 0, new_row);
		grid->row = new_row;
		grid->col = new_col;
		*grid->orig_string = grid->row_data[new_row][new_col];
		// ensure that the selection is visible
	}
	grid->cursor_index = new_index;
	gtk_widget_queue_draw (widget);
	return true;
}

static void gcr_grid_class_init (GtkWidgetClass *klass)
{
	parent_class = reinterpret_cast < GtkWidgetClass * > (g_type_class_peek_parent (klass));
	klass->draw = gcr_grid_draw;
	klass->size_allocate = gcr_grid_size_allocate;
	klass->button_press_event = gcr_grid_button_press_event;
	klass->key_press_event = gcr_grid_key_press_event;
	reinterpret_cast < GObjectClass * > (klass)->finalize = gcr_grid_finalize;
	klass->get_preferred_height = gcr_grid_get_preferred_height;
	klass->get_preferred_width = gcr_grid_get_preferred_width;
	klass->unrealize = gcr_grid_unrealize;
	gcr_grid_signals[VALUE_CHANGED] = g_signal_new ("value-changed",
	                                                G_TYPE_FROM_CLASS(klass),
				                                    G_SIGNAL_RUN_LAST,
				                                    G_STRUCT_OFFSET(GcrGridClass, value_changed_event),
				                                    NULL, NULL,
				                                    gcu__VOID__UINT_UINT,
				                                    G_TYPE_NONE, 2,
				                                    G_TYPE_UINT, G_TYPE_UINT
				                                    );
	gcr_grid_signals[ROW_SELECTED] = g_signal_new ("row-selected",
	                                            G_TYPE_FROM_CLASS(klass),
				                                G_SIGNAL_RUN_LAST,
				                                G_STRUCT_OFFSET(GcrGridClass, row_selected_event),
				                                NULL, NULL,
				                                g_cclosure_marshal_VOID__INT,
				                                G_TYPE_NONE, 1,
				                                G_TYPE_INT
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

GtkWidget *gcr_grid_new (G_GNUC_UNUSED char const *col_title, GType col_type, ...)
{
	g_return_val_if_fail (col_title && g_utf8_validate (col_title, -1, NULL), NULL);
	GcrGrid *grid = GCR_GRID (g_object_new (GCR_TYPE_GRID, NULL));
	gtk_widget_add_events (GTK_WIDGET (grid),
						   GDK_POINTER_MOTION_MASK |
						   GDK_BUTTON_MOTION_MASK |
						   GDK_BUTTON_PRESS_MASK |
						   GDK_BUTTON_RELEASE_MASK |
	                       GDK_KEY_PRESS_MASK |
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
	grid->min_widths = new int[grid->cols];
	grid->titles = new std::string[grid->cols];
	grid->types = new GType[grid->cols];
	unsigned i;
	std::list <char const *>::iterator title = titles.begin ();
	std::list <GType>::iterator type = types.begin ();
	grid->width = grid->header_width;
	grid->cols_min_width = 0;
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
		grid->min_widths[i] = col_size;
		grid->cols_min_width += col_size;
		grid->titles[i] = *title;
		grid->types[i] = *type;
	}
	grid->width += grid->cols_min_width;
	g_object_unref (layout);
	GdkRGBA rgba = {1.0, 1.0, 1.0, 1.0};
	gtk_widget_override_background_color (reinterpret_cast <GtkWidget *> (grid), GTK_STATE_FLAG_NORMAL, &rgba);
	// add a vertical scrollbar
	grid->vadj = gtk_adjustment_new (0, 0, 1, 1, 10, 0);
	grid->scroll = gtk_scrollbar_new (GTK_ORIENTATION_VERTICAL, grid->vadj);
	g_object_set (G_OBJECT (grid->scroll), "height-request", grid->row_height * 5, NULL); //default size
	gtk_layout_put (GTK_LAYOUT (grid), grid->scroll, grid->width + 1, grid->row_height + 1);
	gtk_widget_get_preferred_width (grid->scroll, &grid->scroll_width, NULL);
	grid->width += grid->scroll_width + 1;
	gtk_widget_set_can_focus(reinterpret_cast <GtkWidget *> (grid), true);
	// add signals
	g_signal_connect (grid->vadj, "value-changed", G_CALLBACK (gcr_grid_adjustment_changed), grid);
	grid->orig_string = new std::string ();
	return reinterpret_cast <GtkWidget *> (grid);
}

int gcr_grid_get_int (GcrGrid *grid, unsigned row, unsigned column)
{
	g_return_val_if_fail (GCR_IS_GRID (grid) && row < grid->rows && column < grid->cols, 0);
	return atoi (grid->row_data[row][column].c_str ());
}

double gcr_grid_get_double (GcrGrid *grid, unsigned row, unsigned column)
{
	g_return_val_if_fail (GCR_IS_GRID (grid) && row < grid->rows && column < grid->cols, go_nan);
	return atof (grid->row_data[row][column].c_str ());
}

bool gcr_grid_get_boolean (GcrGrid *grid, unsigned row, unsigned column)
{
	g_return_val_if_fail (GCR_IS_GRID (grid) && row < grid->rows && column < grid->cols, false);
	return grid->row_data[row][column] == "t";
}

void gcr_grid_set_int (GcrGrid *grid, unsigned row, unsigned column, int value)
{
	g_return_if_fail (GCR_IS_GRID (grid) && row < grid->rows && column < grid->cols);
	char *buf;
	if (value >= 0)
		buf = g_strdup_printf ("%d", value);
	else
		buf = g_strdup_printf ("−%d", -value);
	grid->row_data[row][column] = buf;
	g_free (buf);
	gtk_widget_queue_draw (GTK_WIDGET (grid));
}

void gcr_grid_set_double (GcrGrid *grid, unsigned row, unsigned column, double value)
{
	g_return_if_fail (GCR_IS_GRID (grid) && row < grid->rows && column < grid->cols);
	char *buf = g_strdup_printf ("%g", value);
	grid->row_data[row][column] = buf;
	g_free (buf);
	gtk_widget_queue_draw (GTK_WIDGET (grid));
}

void gcr_grid_set_boolean (GcrGrid *grid, unsigned row, unsigned column, bool value)
{
	g_return_if_fail (GCR_IS_GRID (grid) && row < grid->rows && column < grid->cols);
	grid->row_data[row][column] = value? "t": "f";
	gtk_widget_queue_draw (GTK_WIDGET (grid));
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
		case G_TYPE_INT: {
			int value = va_arg (args, int);
			if (value >= 0)
				buf = g_strdup_printf ("%d", value);
			else
				buf = g_strdup_printf ("−%d", -value);
			grid->row_data[row][col] = buf;
			g_free (buf);
			break;
		}
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
	gtk_adjustment_set_page_size (grid->vadj, static_cast < double > (grid->nb_visible) / grid->rows);
	gtk_adjustment_set_upper (grid->vadj, (grid->nb_visible < grid->rows)? grid->rows - grid->nb_visible: .1);
	if (grid->rows < grid->nb_visible + grid->first_visible) {
		grid->first_visible = (grid->nb_visible < grid->rows)? grid->rows - grid->nb_visible: 0;
		gtk_adjustment_set_value (grid->vadj, grid->first_visible);
	}
	gtk_widget_queue_draw (GTK_WIDGET (grid));
	return row;
}

void gcr_grid_delete_row (GcrGrid *grid, unsigned row)
{
	g_return_if_fail (GCR_IS_GRID (grid) && grid->rows > row);
	delete [] grid->row_data[row];
	for (row++ ; row < grid->rows; row++)
		grid->row_data[row - 1] = grid->row_data[row];
	grid->rows--;
	if (grid->row == static_cast < int > (grid->rows)) {
		grid->row = -1;
		g_signal_emit (grid, gcr_grid_signals[ROW_SELECTED], 0, -1);
	}
	gtk_widget_queue_draw (GTK_WIDGET (grid));
}

void gcr_grid_delete_all (GcrGrid *grid)
{
	g_return_if_fail (GCR_IS_GRID (grid));
	for (unsigned i = 0; i < grid->rows; i++)
		delete [] grid->row_data[i];
	grid->rows = 0;
	if (grid->row >= 0) {;
		grid->row = -1;
		g_signal_emit (grid, gcr_grid_signals[ROW_SELECTED], 0, -1);
	}
	gtk_widget_queue_draw (GTK_WIDGET (grid));
}
