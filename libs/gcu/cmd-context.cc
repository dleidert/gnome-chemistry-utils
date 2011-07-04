/*
 * Gnome Chemistry Utils
 * cmd-context.cc
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

#include "config.h"
#include "cmd-context.h"
#include <gsf/gsf-impl-utils.h>
#include <stdio.h>

namespace gcu {

#define GCU_TYPE_CMD_CONTEXT		(gcu_cmd_context_get_type ())
GType		gcu_cmd_context_get_type   (void);

CmdContext::CmdContext (Application *App)
{
	m_App = App;
	m_GOCmdContext = NULL;
}

CmdContext::~CmdContext ()
{
	if (m_GOCmdContext)
		g_object_unref (G_OBJECT (m_GOCmdContext));
}

GOIOContext *CmdContext::GetNewGOIOContext ()
{
	return go_io_context_new (m_GOCmdContext);
}

}	//	namespace gcu
