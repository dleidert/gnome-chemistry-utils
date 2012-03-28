// -*- C++ -*-

/*
 * Gnome Crystal
 * window.cc
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
 * USA
 */

#include "config.h"
#include "window.h"
#include "application.h"
#include "document.h"
#include <gcr/view.h>
#include <gcugtk/print-setup-dlg.h>
#include <gcu/spacegroup.h>
#include <glib/gi18n.h>
#include <cstring>

static const char *ui_description =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu action='FileMenu'>"
"	   <placeholder name='file1'>"
"        <menuitem action='Quit'/>"
"	   </placeholder>"
"    </menu>"
"	 <placeholder name='menu1'>"
"      <menu action='WindowsMenu'>"
"        <menuitem action='NewView'/>"
"        <menuitem action='CloseView'/>"
"      </menu>"
"	 </placeholder>"
"  </menubar>"
"</ui>";

gcWindow::gcWindow (gcApplication *App, gcDocument *Doc): gcr::Window (App, Doc, ui_description)
{
}

gcWindow::~gcWindow ()
{
}

void gcWindow::Destroy ()
{
	if (GetDocument ()->RemoveView (GetView ()))
		gtk_widget_destroy (GTK_WIDGET (GetWindow ()));
}
