/*
 * Gnome Chemistry Utils GOffice component
 * gogchem3dapp.cc
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
#include <gcugtk/chem3ddoc.h>
#include <gcu/glview.h>
#include <glib/gi18n-lib.h>

GOGChem3dApplication::GOGChem3dApplication ():
	gcugtk::Application (_("GChem3D Viewer"), DATADIR, "gchem3d"),
	GOGcuApplication ()
{
	m_Docs.insert (NULL);	// avoids a call to gtk_main_quit() when all the documents are deleted
}

GOGChem3dApplication::~GOGChem3dApplication ()
{
}

void GOGChem3dApplication::ImportDocument (GOGChemUtilsComponent *gogcu)
{
	gcugtk::Chem3dDoc *doc = new gcugtk::Chem3dDoc (this, NULL);
	GOComponent *component = GO_COMPONENT (gogcu);
	gcu::ContentType type = doc->LoadData (component->data, component->mime_type, component->length);
	if (type != gcu::ContentType3D) {
		// FIXME: load in the appropriate application, may be asking in case of a 2d view 
	}
	gogcu->document = doc;
	component->resizable = true;
}

GtkWindow *GOGChem3dApplication::EditDocument (GOGChemUtilsComponent *gogcu)
{
	return NULL; // FIXME
}

bool GOGChem3dApplication::GetData (GOGChemUtilsComponent *gogcu, gpointer *data, int *length, void (**clearfunc) (gpointer), gpointer *user_data)
{
	return false; // FIXME
}

void GOGChem3dApplication::Render (GOGChemUtilsComponent *gogcu, cairo_t *cr, double width, double height)
{
	gcu::Chem3dDoc *doc = static_cast <gcu::Chem3dDoc *> (gogcu->document);
	doc->GetView ()->RenderToCairo (cr, width, height, false);
}

void GOGChem3dApplication::UpdateBounds (GOGChemUtilsComponent *gogcu)
{
}
