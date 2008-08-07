/* 
 * GChemPaint GOffice component
 * gofficewin.cc
 *
 * Copyright (C) 2006-2008 Jean Bréfort <jean.brefort@normalesup.org>
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

GOGcpWindow::GOGcpWindow (GOGcpApplication *App, GOGChemUtilsComponent *gogcu):
	gcp::Window (App)
{
	m_gogcu = gogcu;
	gogcu->window = this;
	/* We must first duplicate the document */
	xmlDocPtr xml = NULL; // makes g++ happy
	try {
		gcp::Document *doc = dynamic_cast <gcp::Document *> (gogcu->document);
		if (doc && doc->HasChildren ()) {
			xml = doc->BuildXMLTree ();
			m_Document->Load (xml->children);
			xmlFreeDoc (xml);
			xml = NULL;
		}
		SetTitle (m_Document->GetTitle ());
		gtk_window_present (GetWindow ());
	}
	catch (int i) {
		if (xml)
			xmlFreeDoc (xml);
		xml = NULL;
		throw 1;
	}
}

GOGcpWindow::~GOGcpWindow ()
{
	if (!m_gogcu->document)
		go_component_emit_changed (GO_COMPONENT (m_gogcu));
	m_gogcu->window = NULL;
}

void GOGcpWindow::OnSave ()
{
	delete m_gogcu->document;
	gcp::Document *doc = new gcp::Document (GetApplication (), false);
	m_gogcu->document = doc;
	doc->GetView ()->CreateNewWidget ();
	xmlDocPtr xml = NULL; // makes g++ happy
	try {
		xml = m_Document->BuildXMLTree ();
		doc->Load (xml->children);
		xmlFreeDoc (xml);
		xml = NULL;
		go_component_emit_changed (GO_COMPONENT (m_gogcu));
	}
	catch (int i) {
		if (xml)
			xmlFreeDoc (xml);
		xml = NULL;
		throw 1;
	}
}

char const *GOGcpWindow::GetDefaultTitle ()
{
	return _("Embedded GChemPaint Object");
}
