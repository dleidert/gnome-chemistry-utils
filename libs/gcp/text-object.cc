// -*- C++ -*-

/* 
 * GChemPaint library
 * text-object.cc 
 *
 * Copyright (C) 2002-2007 Jean Bréfort <jean.brefort@normalesup.org>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include "config.h"
#include "text-object.h"
#include "document.h"
#include "window.h"
#include <gcu/xml-utils.h>

namespace gcp {

extern xmlDocPtr pXmlDoc;

TextObject::TextObject (TypeId Type): Object (Type)
{
	m_x = 0.;
	m_y = 0.;
	m_ascent = 0;
	m_length = 5;
	m_height = 15;
	m_InsertOffset = -2;
	m_bLoading = false;
	m_Layout = NULL;
	m_AttrList = NULL;
	m_RealSave = true;
	m_StartSel = m_EndSel = 0;
}

TextObject::TextObject (double x, double y, TypeId Type): Object (Type)
{
	m_x = x;
	m_y = y;
	m_ascent = 0;
	m_length = 5;
	m_height = 15;
	m_InsertOffset = -2;
	m_bLoading = false;
	m_Layout = NULL;
	m_AttrList = NULL;
	m_StartSel = m_EndSel = 0;
}

TextObject::~TextObject ()
{
	if (m_AttrList)
		pango_attr_list_unref (m_AttrList);;
}

xmlNodePtr TextObject::SaveSelected ()
{
	m_RealSave = false;
	xmlNodePtr node = Save (pXmlDoc);
	m_RealSave = true;
	if (!node)
		return NULL;
	gchar* buf = g_strdup_printf ("%u", m_StartSel);
	xmlNewProp (node, (xmlChar*) "start-sel", (xmlChar*) buf);
	g_free (buf);
	buf = g_strdup_printf ("%u", m_EndSel);
	xmlNewProp (node, (xmlChar*) "end-sel", (xmlChar*) buf);
	g_free (buf);
	return node;
}

void TextObject::LoadSelected (xmlNodePtr node)
{
	pango_layout_set_text (m_Layout, "", 0);
	PangoAttrList *l = pango_attr_list_new ();
	pango_layout_set_attributes (m_Layout, l);
	pango_attr_list_unref (l);
	Load(node);
	OnChanged (false);
}

bool TextObject::SaveNode (xmlDocPtr xml, xmlNodePtr node)
{
	SaveId (node);
	return WritePosition (xml, node, NULL, m_x, m_y);
}

bool TextObject::Load (xmlNodePtr node)
{
	char* tmp, *endptr;
	bool result;
	tmp = (char*) xmlGetProp (node, (xmlChar*) "id");
	if (tmp) {
		SetId (tmp);
		xmlFree (tmp);
	}
	if (ReadPosition (node, NULL, &m_x, &m_y))
		return true;
	tmp = (char*) xmlGetProp(node, (xmlChar*) "x");
	if (!tmp)
		return false;
	m_x = strtod (tmp, &endptr);
	result = *endptr;
	xmlFree (tmp);
	if (result)
		return false;
	tmp = (char*) xmlGetProp (node, (xmlChar*) "y");
	if (!tmp)
		return false;
	m_y = strtod (tmp, &endptr);
	result = *endptr;
	xmlFree (tmp);
	if (result)
		return false;
	return true;
}

void TextObject::Move (double x, double y, double z)
{
	m_x += x;
	m_y += y;
}

void TextObject::OnSelChanged (struct GnomeCanvasPangoSelBounds *bounds)
{
	if (bounds->start <= bounds->cur) {
		m_StartSel = bounds->start;
		m_EndSel = bounds->cur;
	} else {
		m_EndSel = bounds->start;
		m_StartSel = bounds->cur;
	}
	bool activate = m_EndSel > m_StartSel;
	Document* pDoc = dynamic_cast<Document*> (GetDocument ());
	pDoc->GetWindow ()->ActivateActionWidget ("/MainMenu/EditMenu/Erase", activate);
	pDoc->GetWindow ()->ActivateActionWidget ("/MainMenu/EditMenu/Copy", activate);
	pDoc->GetWindow ()->ActivateActionWidget ("/MainMenu/EditMenu/Cut", activate);
}

}	//	namespace gcp
