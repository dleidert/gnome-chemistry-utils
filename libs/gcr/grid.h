/* 
 * Gnome Crystal
 * grid.h 
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

#ifndef GCR_GRID_H
#define GCR_GRID_H

/*!\file
@brief Grid widget.
*/

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GCR_TYPE_GRID	(gcr_grid_get_type ())
#define GCR_GRID(obj)	(G_TYPE_CHECK_INSTANCE_CAST((obj), GCR_TYPE_GRID, GcrGrid))
#define GCR_IS_GRID(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), GCR_TYPE_GRID))

typedef struct _GcrGrid GcrGrid;

GType      gcr_grid_get_type (void);

GtkWidget *gcr_grid_new (char const *col_title,...);

G_END_DECLS

#endif	//	GCR_GRID_H
