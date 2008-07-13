/* 
 * Gnome Chemistry Utils
 * cmd-context.h 
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef GCU_CMD_CONTEXT_H
#define GCU_CMD_CONTEXT_H

#include <goffice/app/go-cmd-context-impl.h>
#include <glib-object.h>

G_BEGIN_DECLS

/*!\file
Declaration of the GcuCmdContext structure, a GObject implementing the
GOCmdContext interface from goffice. This is mandatory to be able to edit
charts.
*/

/*!
@return the GType associated to GcuCmdContext.
*/
#define GCU_CMD_CONTEXT_TYPE		(gcu_cmd_context_get_type ())
GType		gcu_cmd_context_get_type   (void);

/*!
@return a static global GcuCmdContext instance.
*/
GOCmdContext *gcu_get_cmd_context (void);

G_END_DECLS

#endif	/* GCU_CMD_CONTEXT_H */
