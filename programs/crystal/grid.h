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

#ifndef GCRYSTAL_GRID_H
#define GCRYSTAL_GRID_H

/*!\file
@brief Grid widget.
*/

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GCU_TYPE_GRID	(gcu_grid_get_type ())
#define GCU_GRID(obj)	(G_TYPE_CHECK_INSTANCE_CAST((obj), GCU_TYPE_GRID, GcuGrid))
#define GCU_IS_GRID(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), GCU_TYPE_GRID))

typedef struct _GcuGrid GcuGrid;

GType      gcu_grid_get_type (void);
GtkWidget *gcu_grid_new (void);

G_END_DECLS

#endif	//	GCRYSTAL_GRID_H
