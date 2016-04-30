// -*- C++ -*-

/*
 * Gnome Crystal
 * document.cc
 *
 * Copyright (C) 2000-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "gcrystal.h"
#include <unistd.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "application.h"
#include "document.h"
#include "view.h"
#include "window.h"
//#include "globals.h"
#include <gcugtk/filechooser.h>
#include <gcu/objprops.h>
#include <gcu/xml-utils.h>
#include <libxml/parserInternals.h>
#include <libxml/xmlmemory.h>
#include <clocale>
#include <cmath>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>
#include <gio/gio.h>
#include <glib/gi18n.h>
#include <cstring>

#define PREC 1e-3

using namespace std;

gcDocument::gcDocument (gcApplication *pApp): gcr::Document (pApp)
{
}

gcDocument::~gcDocument ()
{
}

void gcDocument::ParseXMLTree(xmlNode* xml)
{
	char *txt;
	xmlNodePtr node;
	bool bViewLoaded = false;

	Reinit();
	//look for generator node
	unsigned version = 0xffffff , major, minor, micro;
	node = xml->children;
	Reinit ();
	if (m_Author) {
		g_free (m_Author);
		m_Author = NULL;
	}
	if (m_Mail) {
		g_free (m_Mail);
		m_Mail = NULL;
	}
	if (m_Comment) {
		g_free (m_Comment);
		m_Comment = NULL;
	}

	gcu::ReadDate (xml, "creation", &m_CreationDate);
	gcu::ReadDate (xml, "revision", &m_RevisionDate);

	node = GetNodeByName (xml, "title");
	if (node) {
		txt = (char*) xmlNodeGetContent (node);
		if (txt) {
			SetTitle (txt);
			xmlFree (txt);
		}
	}
	node = GetNodeByName (xml, "author");
	if (node) {
		txt = (char*) xmlGetProp (node, (xmlChar*) "name");
		if (txt) {
			m_Author = g_strdup (txt);
			xmlFree (txt);
		}
		txt = (char*) xmlGetProp (node, (xmlChar*) "e-mail");
		if (txt) {
			m_Mail = g_strdup (txt);
			xmlFree (txt);
		}
	}
	node = GetNodeByName (xml, "comment");
	if (node) {
		txt = (char*) xmlNodeGetContent (node);
		if (txt) {
			m_Comment = g_strdup (txt);
			xmlFree (txt);
		}
	}
	while (node)
	{
		if (!strcmp ((const char*)(node->name), "generator")) break;
		node = node->next;
	}
	if (node)
	{
		txt = (char*)xmlNodeGetContent(node);
		if (txt)
		{
			if (sscanf(txt, "Gnome Crystal %d.%d.%d", &major, &minor, &micro) == 3)
				version = micro + minor * 0x100 + major * 0x10000;
			xmlFree(txt);
		}
	}
	if (version >= 0x500)
	{
		gcr::Document::ParseXMLTree(xml);
	}
	else
	{
		node = xml->children;
		while(node) {
			if (!strcmp((char *)node->name, "lattice")) {
				txt = (char*)xmlNodeGetContent(node);
				if (txt) {
					int i = 0;
					while (strcmp (txt, gcr::LatticeName[i]) && (i < 14))
						i++;
					if (i < 14)
						m_lattice = (gcr::Lattice) i;
					xmlFree (txt);
				}
			} else if (!strcmp ((char *) node->name, "cell")) {
				txt = (char*) xmlNodeGetContent (node);
				if (txt) {
					sscanf (txt, "%lg %lg %lg %lg %lg %lg", &m_a, &m_b, &m_c, &m_alpha, &m_beta, &m_gamma);
					xmlFree (txt);
				}
			} else if (!strcmp ((char *) node->name, "size")) {
				txt = (char*) xmlNodeGetContent (node);
				if (txt) {
					sscanf (txt, "%lg %lg %lg %lg %lg %lg", &m_xmin, &m_ymin, &m_zmin, &m_xmax, &m_ymax, &m_zmax);
					xmlFree (txt);
				}
				txt = (char*) xmlGetProp (node, (xmlChar*) "fixed");
				if (txt) {
					if (!strcmp (txt, "true"))
						SetFixedSize (true);
					xmlFree (txt);
				}
			} else if (!strcmp((char *)node->name, "atom")) {
				gcAtom *pAtom = new gcAtom ();
				if (pAtom->LoadOld (node, version))
					AddChild (pAtom);
				else
					delete pAtom;
			} else if (!strcmp ((char *) node->name, "line")) {
				gcLine *pLine = new gcLine ();
				if (pLine->LoadOld (node, version))
					LineDef.push_back ((gcr::Line*) pLine);
				else
					delete pLine;
			} else if (!strcmp((char *)node->name, "cleavage")) {
				gcCleavage *pCleavage = new gcCleavage ();
				if (pCleavage->LoadOld (node))
					Cleavages.push_back ((gcr::Cleavage *) pCleavage);
				else
					delete pCleavage;
			} else if (!strcmp( (char *) node->name, "view")) {
				if (bViewLoaded) {
					gcWindow *pWindow = new gcWindow (dynamic_cast <gcApplication *> (m_App), this);
					gcView *pView = static_cast < gcView * > (pWindow->GetView ());
					pView->LoadOld(node);
				} else {
					m_Views.front ()->Load (node); //the first view is created with the document
					bViewLoaded = true;
				}
			}
			node = node->next;
		}
	}
	Update ();
}

void gcDocument::OnNewDocument ()
{
	Reinit ();
	UpdateAllViews ();
}

gcr::View *gcDocument::CreateNewView()
{
	return new gcView (this);
}

gcr::Atom* gcDocument::CreateNewAtom()
{
	return (gcr::Atom*) new gcAtom();
}

gcr::Line* gcDocument::CreateNewLine()
{
	return (gcr::Line*) new gcLine();
}

gcr::Cleavage* gcDocument::CreateNewCleavage()
{
	return (gcr::Cleavage*) new gcCleavage();
}

const char* gcDocument::GetProgramId () const
{
	return "Gnome Crystal " VERSION;
}
