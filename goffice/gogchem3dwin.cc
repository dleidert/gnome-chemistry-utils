/*
 * GChemPaint GOffice component
 * gogchem3dwin.h
 *
 * Copyright (C) 2011 Jean Bréfort <jean.brefort@normalesup.org>
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

#include "config.h"
#include "gogchem3dapp.h"
#include "gogchem3dwin.h"
#include <gcugtk/chem3ddoc.h>
#include <gcugtk/chem3dview.h>
#include <glib/gi18n-lib.h>

GOGChem3dWindow::GOGChem3dWindow (GOGChem3dApplication *App, GOGChemUtilsComponent *gogcu):
	gcugtk::Chem3dWindow (App, new gcugtk::Chem3dDoc (App, GetView ())),
	m_gogcu (gogcu)
{
}

GOGChem3dWindow::~GOGChem3dWindow ()
{
}

void GOGChem3dWindow::OnSave ()
{
}

char const *GOGChem3dWindow::GetDefaultTitle ()
{
	return _("Embedded GChem3d Object");
}
