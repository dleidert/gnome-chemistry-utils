/*
 * Gnome Chemistry Utils
 * cmd-context.h
 *
 * Copyright (C) 2007-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
	friend class Application;
	CmdContext (Application *App);
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

	virtual Response GetResponse (char const *message, int responses) = 0;
	virtual void Message (char const *message, Severity severity, bool modal) = 0;

	GOIOContext *GetNewGOIOContext ();

GCU_PROT_PROP (GOCmdContext *, GOCmdContext)
GCU_PROT_POINTER_PROP (Application, App)
};

}	//	namespace gcu

#endif //	GCU_CMD_CONTEXT_H
