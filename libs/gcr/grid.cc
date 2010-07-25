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
#include <list>

#include <gsf/gsf-impl-utils.h>

struct _GcrGrid
{
	GtkTable base;
	unsigned cols, rows;
};

typedef struct
{
	GtkTableClass parent_class;
} GcrGridClass;


static void gcr_grid_class_init (G_GNUC_UNUSED GcrGridClass *klass)
{
}

static void
gcr_grid_init (G_GNUC_UNUSED GcrGrid *grid)
{
}

GSF_CLASS (GcrGrid, gcr_grid, gcr_grid_class_init, gcr_grid_init, GTK_TYPE_TABLE)

GtkWidget *gcr_grid_new (G_GNUC_UNUSED char const *col_title,...)
{
	g_return_val_if_fail (col_title && g_utf8_validate (col_title, -1, NULL), NULL);
	GcrGrid *grid = GCR_GRID (g_object_new (GCR_TYPE_GRID, NULL));
	std::list <char const *> titles;
	titles.push_front (col_title);
	va_list args;
	va_start (args, col_title);
	while (1) {
		col_title = va_arg (args, char const *);
		if (!col_title)
			break;
		if (g_utf8_validate (col_title, -1, NULL))
			titles.push_back (col_title);
	}
	va_end (args);
	grid->cols = titles.size ();
	GtkTable *table = GTK_TABLE (grid);
	gtk_table_resize (table, 2, grid->cols);
	for (unsigned i = 0; i < grid->cols; i++) {
		col_title = titles.front ();
		titles.pop_front ();
		GtkWidget *label = gtk_label_new (col_title);
		gtk_table_attach (table, label, i, i + 1, 0, 1, GTK_FILL, static_cast <GtkAttachOptions> (0), 0, 0);
	}
	return reinterpret_cast <GtkWidget *> (grid);
}
