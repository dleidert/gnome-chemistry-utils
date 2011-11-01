/*
 * Gnome Chemistry Utils GOffice component
 * gogcrystal.cc
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
 * USA
 */

#include "config.h"
#include "gchemutils-priv.h"
#include "gogcrystalapp.h"
#include "gogcrystalwin.h"
#include <gcr/document.h>
#include <gcr/view.h>
#include <glib/gi18n-lib.h>
#include <cstring>

using namespace gcu;
using namespace std;

GOGCrystalApplication::GOGCrystalApplication (): gcr::Application (), GOGcuApplication ()
{
}

GOGCrystalApplication::~GOGCrystalApplication ()
{
}

GtkWindow* GOGCrystalApplication::GetWindow()
{
	return NULL;
}

void GOGCrystalApplication::ToggleMenu (G_GNUC_UNUSED const string& menuname, G_GNUC_UNUSED bool active)
{
}

void GOGCrystalApplication::ImportDocument (GOGChemUtilsComponent *gogcu)
{
	gcr::Document *doc = NULL;
	GOComponent *component = GO_COMPONENT (gogcu);
	if (!strcmp (component->mime_type, "application/x-gcrystal")) {
		xmlDocPtr xml;
		if (!(xml = xmlParseMemory (component->data, component->length)) ||
			xml->children == NULL ||
			strcmp (reinterpret_cast <char const *> (xml->children->name), "crystal")) {
			xmlFreeDoc (xml);
			return;
		}
		doc = new gcr::Document (this);
		gtk_widget_show_all (doc->GetView ()->GetWidget ());
		doc->ParseXMLTree (xml->children);
		xmlFreeDoc (xml);
	} else {
	}
	gogcu->document = doc;
}

GtkWindow * GOGCrystalApplication::EditDocument (GOGChemUtilsComponent *gogcu)
{
	GOGCrystalWindow *win = NULL;
	try {
		win = new GOGCrystalWindow (this, gogcu);
		if (!win)
			return NULL;
		win->Show ();	// ensure the window is visible
		return win->GetWindow ();
	}
	catch (int i) {
		if (win)
			delete win;
		return NULL;
	}
}

bool GOGCrystalApplication::GetData (GOGChemUtilsComponent *gogcu, gpointer *data, int *length, void (**clearfunc) (gpointer), G_GNUC_UNUSED gpointer *user_data)
{
	gcr::Document *doc = static_cast <gcr::Document *> (gogcu->document);
	bool result = false;
	xmlDocPtr xml = NULL;

	if (!doc || doc->GetEmpty ()) {
		*data = NULL;
		*length = 0;
		*clearfunc = NULL;
		return true;
	}

	try {
		xml = doc->BuildXMLTree ();
		xmlChar *mem;
		int size;
		xmlDocDumpMemory (xml, &mem, &size);
		xmlFreeDoc (xml);
		*data = mem;
		*length = size;
		*clearfunc = xmlFree;
		result = true;
	}
	catch (int) {
		if (xml)
			xmlFreeDoc (xml);
		xml = NULL;
		*data = NULL;
		*length = 0;
		*clearfunc = NULL;
		result = false;
	}

	return result;
}

void GOGCrystalApplication::Render (GOGChemUtilsComponent *gogcu, cairo_t *cr, double width, double height)
{
	gcr::Document *doc = static_cast <gcr::Document *> (gogcu->document);
	doc->GetView ()->RenderToCairo (cr, width, height, false);
}

void GOGCrystalApplication::UpdateBounds (GOGChemUtilsComponent *)
{
}

gcr::Document *GOGCrystalApplication::OnFileNew ()
{
	return new gcr::Document (this);
}

void GOGCrystalApplication::OnFileClose ()
{
}
