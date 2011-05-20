/* 
 * Gnome Chemistry Utils GOffice component
 * gogcrystal.cc
 *
 * Copyright (C) 2010 Jean Bréfort <jean.brefort@normalesup.org>
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

GOGCrystalApplication::GOGCrystalApplication (): gcu::Application ("gcrystal"), GOGcuApplication ()
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

gcu::Document *GOGCrystalApplication::ImportDocument (const string& mime_type, const char* data, int length)
{
	gcr::Document *doc = NULL;
	if (mime_type == "application/x-gcrystal") {
		xmlDocPtr xml;
		if (!(xml = xmlParseMemory(data, length)) ||
			xml->children == NULL ||
			strcmp (reinterpret_cast <char const *> (xml->children->name), "crystal")) {
			xmlFreeDoc (xml);
			return NULL;
		}
		doc = new gcr::Document (this);
		gtk_widget_show_all (doc->GetView ()->GetWidget ());
		doc->ParseXMLTree (xml->children);
		xmlFreeDoc (xml);
	} else {
	}
	return (doc);
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
//	gcp::Document *doc = static_cast <gcp::Document *> (gogcu->document);
	bool result = false;
/*	xmlDocPtr xml = NULL;
	char *old_num_locale, *old_time_locale;

	if (!doc || !doc->HasChildren ()) {
		*data = NULL;
		*length = 0;
		* clearfunc = NULL;
		return true;
	}
	
	old_num_locale = g_strdup (setlocale (LC_NUMERIC, NULL));
	setlocale(LC_NUMERIC, "C");
	old_time_locale = g_strdup (setlocale (LC_TIME, NULL));
	setlocale (LC_TIME, "C");

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
		result = false;
	}
	setlocale (LC_NUMERIC, old_num_locale);
	g_free (old_num_locale);
	setlocale (LC_TIME, old_time_locale);
	g_free (old_time_locale);*/

	return result;
}

void GOGCrystalApplication::Render (GOGChemUtilsComponent *gogcu, cairo_t *cr, double width, double height)
{
	gcr::Document *doc = static_cast <gcr::Document *> (gogcu->document);
	doc->GetView ()->RenderToCairo (cr, width, height, false);
}

void GOGCrystalApplication::UpdateBounds (GOGChemUtilsComponent *gogcu)
{
/*	gcp::Document *doc = static_cast <gcp::Document *> (gogcu->document);
	gcp::Theme *pTheme = doc->GetTheme ();
	GtkWidget *w = doc->GetWidget ();
	gccv::Rect rect;
	gcp::WidgetData *pData = (gcp::WidgetData*) g_object_get_data (G_OBJECT (w), "data");
	pData->GetObjectBounds (doc, &rect);
	double y = doc->GetYAlign ();
	y += doc->GetView ()->GetBaseLineOffset ();
	y *= pTheme->GetZoomFactor ();
	if (rect.x0 || rect.y0)
		doc->Move (- rect.x0 / pTheme->GetZoomFactor (), - rect.y0 / pTheme->GetZoomFactor ());
	doc->GetView ()->Update (doc);
	if (y < rect.y0)
		y = rect.y1;
	// assuming 96 dpi, setting dimensions as inches.
	gogcu->parent.ascent = (y - rect.y0) / 96;
	gogcu->parent.descent = (rect.y1 - y) / 96;
	gogcu->parent.width = (rect.x1 - rect.x0) / 96;
	gogcu->parent.height = gogcu->parent.ascent + gogcu->parent.descent;*/
}

void GOGCrystalApplication::OnFileNew (char const *Theme)
{
/*	gchar tmp[32];
	if (m_pActiveDoc && !m_pActiveDoc->GetView ()->PrepareUnselect ())
		return;
	g_snprintf (tmp, sizeof (tmp), _("Untitled %d"), m_NumWindow++);
	new gcp::Window (this, Theme);*/
}

void GOGCrystalApplication::OnFileClose ()
{
}
