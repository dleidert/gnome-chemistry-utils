// -*- C++ -*-

/*
 * Gnome Chemisty Utils
 * gcr/cleavage.cc
 *
 * Copyright (C) 2002-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include "config.h"
#include "cleavage.h"
#include <glib.h>

namespace gcr
{

Cleavage::Cleavage()
{
}

Cleavage::~Cleavage()
{
}

Cleavage::Cleavage(Cleavage& ccClivage)
{
	m_nh = ccClivage.m_nh ;
	m_nk = ccClivage.m_nk ;
	m_nl = ccClivage.m_nl ;
	m_nPlanes = ccClivage.m_nPlanes ;
}

Cleavage& Cleavage::operator=(Cleavage& ccClivage)
{
	m_nh = ccClivage.m_nh ;
	m_nk = ccClivage.m_nk ;
	m_nl = ccClivage.m_nl ;
	m_nPlanes = ccClivage.m_nPlanes ;
	return *this ;
}

bool Cleavage::operator==(Cleavage& ccClivage)
{
	return ((m_nh == ccClivage.m_nh) && (m_nk == ccClivage.m_nk) && (m_nl == ccClivage.m_nl));
}

xmlNodePtr Cleavage::Save(xmlDocPtr xml) const
{
	xmlNodePtr node;
	char buf[256];
	node = xmlNewDocNode(xml, NULL, (xmlChar*)"cleavage", NULL);
	if (!node) return NULL;

	snprintf(buf, sizeof(buf), "%d", m_nh);
	xmlSetProp(node, (xmlChar*)"h", (xmlChar*)buf);

	snprintf(buf, sizeof(buf), "%d", m_nk);
	xmlSetProp(node, (xmlChar*)"k", (xmlChar*)buf);

	snprintf(buf, sizeof(buf), "%d", m_nl);
	xmlSetProp(node, (xmlChar*)"l", (xmlChar*)buf);

	snprintf(buf, sizeof(buf), "%u", m_nPlanes);
	xmlSetProp(node, (xmlChar*)"planes", (xmlChar*)buf);

	return node;
}

bool Cleavage::Load (xmlNodePtr node)
{
	char *txt;
	txt = (char*) xmlGetProp (node, (xmlChar*) "h");
	if (txt) {
		if (sscanf (txt, "%d", &m_nh) != 1) {
			xmlFree (txt);
			return false;
		}
		xmlFree (txt);
	} else
		return false;
	txt = (char*) xmlGetProp (node, (xmlChar*) "k");
	if (txt) {
		if (sscanf(txt, "%d", &m_nk) != 1) {
			xmlFree (txt);
			return false;
		}
		xmlFree (txt);
	} else
		return false;
	txt = (char*) xmlGetProp (node, (xmlChar*) "l");
	if (txt) {
		if (sscanf(txt, "%d", &m_nl) != 1) {
			xmlFree (txt);
			return false;
		}
		xmlFree (txt);
	} else
		return false;
	txt = (char*) xmlGetProp (node, (xmlChar*) "planes");
	if (txt) {
		if (sscanf(txt, "%u", &m_nPlanes)!= 1) {
			xmlFree (txt);
			return false;
		}
		xmlFree (txt);
	} else
		return false;
	return true;
}

}	//	namespace gcr
