// -*- C++ -*-

/* 
 * Gnome Chemisty Utils
 * crystalviewer/crystalcleavage.cc 
 *
 * Copyright (C) 2002-2003
 *
 * Developed by Jean Bréfort <jean.brefort@ac-dijon.fr>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA  02111-1307, USA.
 */

#include "crystalcleavage.h"
#include <glib.h>

using namespace gcu;

CrystalCleavage::CrystalCleavage()
{
}

CrystalCleavage::~CrystalCleavage()
{
}

CrystalCleavage::CrystalCleavage(CrystalCleavage& ccClivage)
{
	m_nh = ccClivage.m_nh ;
	m_nk = ccClivage.m_nk ;
	m_nl = ccClivage.m_nl ;
	m_nPlanes = ccClivage.m_nPlanes ;
}

CrystalCleavage& CrystalCleavage::operator=(CrystalCleavage& ccClivage)
{
	m_nh = ccClivage.m_nh ;
	m_nk = ccClivage.m_nk ;
	m_nl = ccClivage.m_nl ;
	m_nPlanes = ccClivage.m_nPlanes ;
	return *this ;
}

bool CrystalCleavage::operator==(CrystalCleavage& ccClivage)
{
	return ((m_nh == ccClivage.m_nh) && (m_nk == ccClivage.m_nk) && (m_nl == ccClivage.m_nl));
}

xmlNodePtr CrystalCleavage::Save(xmlDocPtr xml)
{
	xmlNodePtr node;
	gchar buf[256];
	node = xmlNewDocNode(xml, NULL, (xmlChar*)"cleavage", NULL);
	if (!node) return NULL;
	
	snprintf(buf, sizeof(buf), "%d", m_nh);
	xmlSetProp(node, (xmlChar*)"h", (xmlChar*)buf);
	
	snprintf(buf, sizeof(buf), "%d", m_nk);
	xmlSetProp(node, (xmlChar*)"k", (xmlChar*)buf);
	
	snprintf(buf, sizeof(buf), "%d", m_nl);
	xmlSetProp(node, (xmlChar*)"l", (xmlChar*)buf);
	
	snprintf(buf, sizeof(buf), "%d", m_nPlanes);
	xmlSetProp(node, (xmlChar*)"planes", (xmlChar*)buf);
	
	return node;
}

bool CrystalCleavage::Load(xmlNodePtr node)
{
	char *txt;
	txt = (char*)xmlGetProp(node, (xmlChar*)"h");
	if (sscanf(txt, "%d", &m_nh) != 1) return false;
	txt = (char*)xmlGetProp(node, (xmlChar*)"k");
	if (sscanf(txt, "%d", &m_nk) != 1) return false;
	txt = (char*)xmlGetProp(node, (xmlChar*)"l");
	if (sscanf(txt, "%d", &m_nl) != 1) return false;
	txt = (char*)xmlGetProp(node, (xmlChar*)"planes");
	if (sscanf(txt, "%d", &m_nPlanes)!= 1) return false;
	return true;
}
