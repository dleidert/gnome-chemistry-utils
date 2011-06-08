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
#include <goffice/goffice.h>
#include <list>

#include <gsf/gsf-impl-utils.h>

struct _GcrGrid
{
	GtkGrid base;
	unsigned cols, rows;
	GtkWidget *headers, *contents; /* are these needed */
	GtkAdjustment *hadj, *vadj;
};

typedef struct
{
	GtkGridClass parent_class;
} GcrGridClass;


static void gcr_grid_class_init (G_GNUC_UNUSED GcrGridClass *klass)
{
}

static void
gcr_grid_init (G_GNUC_UNUSED GcrGrid *grid)
{
}

GSF_CLASS (GcrGrid, gcr_grid, gcr_grid_class_init, gcr_grid_init, GTK_TYPE_GRID)

GtkWidget *gcr_grid_new (G_GNUC_UNUSED char const *col_title, GType col_type, ...)
{
	g_return_val_if_fail (col_title && g_utf8_validate (col_title, -1, NULL), NULL);
	GcrGrid *grid = GCR_GRID (g_object_new (GCR_TYPE_GRID, NULL));
	std::list <char const *> titles;
	std::list <GType> types;
	titles.push_front (col_title);
	types.push_front (col_type);
	va_list args;
	va_start (args, col_type);
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
	grid->headers = GTK_WIDGET (g_object_new (GOC_TYPE_CANVAS, NULL));
	GtkBox *box = GTK_BOX (grid);
	gtk_box_pack_start (box, GTK_WIDGET (grid->headers), FALSE, TRUE, 0);
	grid->hadj = GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 1, 1, 0, 1));
	grid->vadj = GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 1, 1, 0, 1));
	GtkWidget *scrolled = gtk_scrolled_window_new (grid->hadj, grid->vadj);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
	gtk_box_pack_start (box, scrolled, FALSE, TRUE, 0);
	
/*	GtkTable *table = GTK_TABLE (grid);
	gtk_table_resize (table, 2, grid->cols);
	for (unsigned i = 0; i < grid->cols; i++) {
		col_title = titles.front ();
		titles.pop_front ();
		GtkWidget *label = gtk_label_new (col_title);
		gtk_table_attach (table, label, i, i + 1, 0, 1, GTK_EXPAND, static_cast <GtkAttachOptions> (0), 0, 0);
	}*/
	return reinterpret_cast <GtkWidget *> (grid);
}
