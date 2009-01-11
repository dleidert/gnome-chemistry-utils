/* 
 * Gnome Chemistry Utils
 * programs/gchemtable-data-allocator.h 
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02111-1307
 * USA
 */

#ifndef GCHEMTABLE_DATA_ALLOCATOR_H
#define GCHEMTABLE_DATA_ALLOCATOR_H

#include <glib-object.h>

class GChemTableCurve;

G_BEGIN_DECLS

typedef struct _GctControlGUI GctControlGUI;
typedef GObjectClass GctControlGUIClass;

#define GCT_CONTROL_GUI_TYPE	(gct_control_gui_get_type ())
#define GCT_CONTROL_GUI(obj)	(G_TYPE_CHECK_INSTANCE_CAST ((obj), GCT_CONTROL_GUI_TYPE, GctControlGUI))
#define IS_GCT_CONTROL_GUI(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), GCT_CONTROL_GUI_TYPE))

GType gct_control_gui_get_type ();
void gct_control_gui_set_owner (GctControlGUI *gui, GChemTableCurve *curve);
GChemTableCurve *gct_control_gui_get_owner (GctControlGUI *gui);

G_END_DECLS

#endif	/*	GCHEMTABLE_DATA_ALLOCATOR_H	*/
