/* 
 * Gnome Chemistry Utils
 * cmd-context.h 
 *
 * Copyright (C) 2007-2010 Jean Bréfort <jean.brefort@normalesup.org>
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

#include "macros.h"
#include <goffice/goffice.h>

namespace gcu {

class Application;

class CmdContext {
public:
	CmdContext ();
	virtual ~CmdContext ();

	typedef enum {
		ResponseDefault,
		ResponseOK = 1 << 0,
		ResponseCancel = 1 << 1,
		ResponseYes = 1 << 2,
		ResponseNo = 1 << 3,
		ResponseClose = 1 << 4
	} Response;

	typedef enum {
		SeverityMessage,
		SeverityWarning,
		SeverityError
	} Severity;

	virtual Response GetResponse (Application *App, char const *message, int responses) = 0;
	virtual void Message (Application *App, char const *message, Severity severity, bool modal) = 0;

GCU_PROT_PROP (GOCmdContext *, GOCmdContext)
};

}	//	namespace gcu

G_BEGIN_DECLS

/*!\file
Declaration of the GcuCmdContext structure, a GObject implementing the
GOCmdContext interface from goffice. This is mandatory to be able to edit
charts.
*/

/*!
@return the GType associated to GcuCmdContext.
*/
#define GCU_TYPE_CMD_CONTEXT		(gcu_cmd_context_get_type ())
GType		gcu_cmd_context_get_type   (void);

/*!
@return a static global GcuCmdContext instance.
*/
GOCmdContext *gcu_get_cmd_context (void);

G_END_DECLS

#endif	/* GCU_CMD_CONTEXT_H */
