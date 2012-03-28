// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * programs/3d/window.cc
 *
 * Copyright (C) 2006-2011 Jean Bréfort <jean.brefort@normalesup.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the
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
#include "application.h"
#include "document.h"
#include "view.h"
#include "window.h"
#include <glib/gi18n.h>

static const char *ui_description =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu action='FileMenu'>"
"	   <separator name='file-sep2'/>"
"	   <placeholder name='file2'>"
"      <menuitem action='Quit'/>"
"	   </placeholder>"
"    </menu>"
"  </menubar>"
"</ui>";

gc3dWindow::gc3dWindow (gc3dApplication *App, gc3dDocument *Doc): gcugtk::Chem3dWindow (App, Doc, ui_description)
{
}

gc3dWindow::~gc3dWindow ()
{
}

void gc3dWindow::SetTitle (char const *title)
{
	gtk_window_set_title (m_Window, title);
}
