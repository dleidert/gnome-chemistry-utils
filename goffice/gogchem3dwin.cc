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
#include "gchemutils-priv.h"
#include "gogchem3dapp.h"
#include "gogchem3dwin.h"
#include <gcugtk/chem3ddoc.h>
#include <gcugtk/chem3dview.h>
#include <glib/gi18n-lib.h>

static const char *ui_description =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu action='FileMenu'>"
"      <placeholder name='file1'>"
"        <menuitem action='Save'/>"
"      </placeholder>"
"    </menu>"
"  </menubar>"
"</ui>";

GOGChem3dWindow::GOGChem3dWindow (GOGChem3dApplication *App, GOGChemUtilsComponent *gogcu):
	gcugtk::Chem3dWindow (App, new gcugtk::Chem3dDoc (App, NULL), ui_description),
	m_gogcu (gogcu)
{
	gogcu->window = this;
	/* We must first duplicate the document */
	GOComponent *component = GO_COMPONENT (gogcu);
	m_Document->GetView ()->SetRotation (m_gogcu->psi, m_gogcu->theta, m_gogcu->phi);
	m_Document->SetDisplay3D (m_gogcu->mode);
	m_Document->LoadData (component->data, component->mime_type, component->length);
}

GOGChem3dWindow::~GOGChem3dWindow ()
{
	m_gogcu->window = NULL;
}

void GOGChem3dWindow::Save ()
{
	m_gogcu->psi = m_View->GetPsi ();
	m_gogcu->theta = m_View->GetTheta ();
	m_gogcu->phi = m_View->GetPhi ();
	gcu::GLView *view  = static_cast < gcu::Chem3dDoc * > (m_gogcu->document)->GetView ();
	view->SetRotation (m_gogcu->psi, m_gogcu->theta, m_gogcu->phi);
	m_gogcu->mode = m_Document->GetDisplay3D ();
	static_cast < gcu::Chem3dDoc * > (m_gogcu->document)->SetDisplay3D (m_gogcu->mode);
	go_component_emit_changed (GO_COMPONENT (m_gogcu));
}

char const *GOGChem3dWindow::GetDefaultTitle ()
{
	return _("Embedded GChem3d Object");
}
