/*
 * GChemPaint GOffice component
 * gogcrystalwin.cc
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
#include "gogcuapp.h"
#include "gogcrystalapp.h"
#include "gogcrystalwin.h"
#include <gcr/application.h>
#include <gcr/document.h>
#include <gcu/application.h>
#include <glib/gi18n-lib.h>

GOGCrystalWindow::GOGCrystalWindow (GOGCrystalApplication *App, GOGChemUtilsComponent *gogcu):
	gcr::Window (App, static_cast <gcr::Document * > (gogcu->document))
{
	m_gogcu = gogcu;
	gogcu->window = this;
	/* We must first duplicate the document */
	xmlDocPtr xml = NULL; // makes g++ happy
	try {
		gcr::Document *doc = dynamic_cast <gcr::Document *> (gogcu->document);
		if (doc) {
			xml = doc->BuildXMLTree ();
			m_Document->ParseXMLTree (xml->children);
			xmlFreeDoc (xml);
			xml = NULL;
		}
		SetTitle (m_Document->GetLabel ());
		gtk_window_present (GetWindow ());
	}
	catch (int i) {
		if (xml)
			xmlFreeDoc (xml);
		xml = NULL;
		throw 1;
	}
}

GOGCrystalWindow::~GOGCrystalWindow ()
{
	if (!m_gogcu->document)
		go_component_emit_changed (GO_COMPONENT (m_gogcu));
	m_gogcu->window = NULL;
}

void GOGCrystalWindow::OnSave ()
{
	delete m_gogcu->document;
	gcr::Document *doc = new gcr::Document (GetApplication ());
	m_gogcu->document = doc;
//	doc->GetView ()->CreateNewWidget ();
	xmlDocPtr xml = NULL; // makes g++ happy
	try {
		xml = m_Document->BuildXMLTree ();
		doc->ParseXMLTree (xml->children);
		xmlFreeDoc (xml);
		xml = NULL;
		m_Document->SetDirty (false);
		go_component_emit_changed (GO_COMPONENT (m_gogcu));
	}
	catch (int i) {
		if (xml)
			xmlFreeDoc (xml);
		xml = NULL;
		throw 1;
	}
}

char const *GOGCrystalWindow::GetDefaultTitle ()
{
	return _("Embedded GCrystal Object");
}
