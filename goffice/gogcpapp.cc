/* 
 * Gnome Chemistry Utils GOffice component
 * gogcpapp.cc
 *
 * Copyright (C) 2005-2008 Jean Bréfort <jean.brefort@normalesup.org>
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include "config.h"
#include "gchemutils-priv.h"
#include "gogcpapp.h"
#include "gogcpwin.h"
#include <gcp/document.h>
#include <gcp/view.h>
#include <glib/gi18n-lib.h>
#include <cstring>

using namespace gcu;
using namespace std;

GOGcpApplication::GOGcpApplication (): gcp::Application (), GOGcuApplication ()
{
}

GOGcpApplication::~GOGcpApplication ()
{
}

GtkWindow* GOGcpApplication::GetWindow()
{
	return NULL;
}

void GOGcpApplication::ToggleMenu (const string& menuname, bool active)
{
}

gcu::Document *GOGcpApplication::ImportDocument (const string& mime_type, const char* data, int length)
{
	gcp::Document *pDoc = NULL;
	char *old_num_locale, *old_time_locale;
	if (mime_type == "application/x-gchempaint") {
		xmlDocPtr xml;
		if (!(xml = xmlParseMemory(data, length)) ||
			xml->children == NULL ||
			strcmp((char*)xml->children->name, "chemistry"))
			return NULL;
		old_num_locale = g_strdup (setlocale (LC_NUMERIC, NULL));
		setlocale(LC_NUMERIC, "C");
		old_time_locale = g_strdup (setlocale (LC_TIME, NULL));
		setlocale(LC_TIME, "C");
		pDoc = new gcp::Document(this, false);
		pDoc->GetView ()->CreateNewWidget ();
		bool result = pDoc->Load(xml->children);
		setlocale(LC_NUMERIC, old_num_locale);
		g_free(old_num_locale);
		setlocale(LC_TIME, old_time_locale);
		g_free(old_time_locale);
		if (!result)
		{
			delete pDoc;
			return NULL;
		}
	} else {
	}
	return (pDoc);
}

GtkWindow * GOGcpApplication::EditDocument (GOGChemUtilsComponent *gogcu)
{
	GOGcpWindow *win = NULL;
	try {
		win = new GOGcpWindow (this, gogcu);
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

void GOGcpApplication::OnFileNew (char const *Theme)
{
	gchar tmp[32];
	if (m_pActiveDoc && !m_pActiveDoc->GetView ()->PrepareUnselect ())
		return;
	g_snprintf (tmp, sizeof (tmp), _("Untitled %d"), m_NumWindow++);
	new gcp::Window (this, Theme);
}

void GOGcpApplication::OnFileClose ()
{
}
