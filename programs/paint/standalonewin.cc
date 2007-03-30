// -*- C++ -*-

/* 
 * GChemPaint
 * standalonewin.cc
 *
 * Copyright (C) 2006-2007 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "standaloneapp.h"
#include "standalonewin.h"

static const char *ui_description =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu action='FileMenu'>"
"	   <placeholder name='file1'>"
"        <menuitem action='Quit'/>"
"	   </placeholder>"
"    </menu>"
"  </menubar>"
"</ui>";

gcpStandaloneWindow::gcpStandaloneWindow (gcpStandaloneApp *App, char const *Theme):
	gcp::Window (App, Theme, ui_description)
{
}

gcpStandaloneWindow::~gcpStandaloneWindow ()
{
}
