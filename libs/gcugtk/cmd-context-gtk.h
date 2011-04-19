/* 
 * Gnome Chemistry Utils
 * gcugtk/cmd-context-gtk.h 
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef GCU_GTK_CMD_CONTEXT_GTK_H
#define GCU_GTK_CMD_CONTEXT_GTK_H

#include <gcu/cmd-context.h>

namespace gcugtk {

class CmdContextGtk: public gcu::CmdContext {
public:
	CmdContextGtk (Application *App);
	virtual ~CmdContextGtk ();

	Response GetResponse (char const *message, int responses);
	void Message (char const *message, Severity severity, bool modal);
};

}	//	namespace gcu

#endif	/* GCU_GTK_CMD_CONTEXT_GTK_H */
