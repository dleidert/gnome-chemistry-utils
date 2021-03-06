// -*- C++ -*-

/*
 * GChemPaint library
 * text-object.cc
 *
 * Copyright (C) 2002-2009 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "text-object.h"
#include "text-editor.h"
#include "document.h"
#include "window.h"
#include <gcu/objprops.h>
#include <gcu/xml-utils.h>
#include <gccv/text.h>
#include <cstring>

using namespace gcu;
using namespace std;

namespace gcp {

extern xmlDocPtr pXmlDoc;

TextObject::TextObject (TypeId Type): Object (Type), gccv::TextClient (),
	m_TextItem (NULL)
{
	m_x = 0.;
	m_y = 0.;
	m_ascent = 0;
	m_length = 5;
	m_height = 15;
	m_InsertOffset = -2;
	m_bLoading = false;
	m_RealSave = true;
	m_StartSel = m_EndSel = 0;
	m_Editor = NULL;
}

TextObject::TextObject (double x, double y, TypeId Type): Object (Type),
	gccv::TextClient (),
	m_TextItem (NULL)
{
	m_x = x;
	m_y = y;
	m_ascent = 0;
	m_length = 5;
	m_height = 15;
	m_InsertOffset = -2;
	m_bLoading = false;
	m_RealSave = true;
	m_StartSel = m_EndSel = 0;
	m_Editor = NULL;
}

TextObject::~TextObject ()
{
}

xmlNodePtr TextObject::SaveSelected ()
{
	m_RealSave = false;
	xmlNodePtr node = Save (pXmlDoc);
	m_RealSave = true;
	if (!node)
		return NULL;
	char *buf = g_strdup_printf ("%u", m_StartSel);
	xmlNewProp (node, (xmlChar*) "start-sel", (xmlChar*) buf);
	g_free (buf);
	buf = g_strdup_printf ("%u", m_EndSel);
	xmlNewProp (node, (xmlChar*) "end-sel", (xmlChar*) buf);
	g_free (buf);
	return node;
}

void TextObject::LoadSelected (xmlNodePtr node)
{
	Load (node);
	unsigned start = 0, end = 0;
	char *buf = reinterpret_cast <char*> (xmlGetProp(node, (xmlChar*) "start-sel"));
	if (buf) {
		start = strtoul (buf, NULL, 10);
		xmlFree (buf);
	}
	buf = reinterpret_cast <char*> (xmlGetProp(node, (xmlChar*) "end-sel"));
	if (buf) {
		end = strtoul (buf, NULL, 10);
		xmlFree (buf);
	}
	reinterpret_cast <gccv::Text *> (m_TextItem)->SetSelectionBounds (start, end);
	OnChanged (false);
}

bool TextObject::SaveNode (xmlDocPtr xml, xmlNodePtr node) const
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

void TextObject::Move (double x, double y, G_GNUC_UNUSED double z)
{
	m_x += x;
	m_y += y;
}

string TextObject::GetProperty (unsigned property) const
{
	switch (property) {
	case GCU_PROP_TEXT_TEXT:
		return m_buf;
	default:
		return Object::GetProperty (property);
	}
}

void TextObject::SelectionChanged (unsigned start, unsigned cur)
{
	if (start <= cur) {
		m_StartSel = start;
		m_EndSel = cur;
	} else {
		m_EndSel = start;
		m_StartSel = cur;
	}
	bool activate = m_EndSel > m_StartSel;
	Document* Doc = dynamic_cast<Document*> (GetDocument ());
	Window *Win = static_cast < Window * > (Doc->GetWindow ());
	Win->ActivateActionWidget ("/MainMenu/EditMenu/Erase", activate);
	Win->ActivateActionWidget ("/MainMenu/EditMenu/Copy", activate);
	Win->ActivateActionWidget ("/MainMenu/EditMenu/Cut", activate);
	if (m_Editor)
		m_Editor->SelectionChanged ();
}

void TextObject::TextChanged (G_GNUC_UNUSED unsigned pos)
{
	OnChanged (true);
}

}	//	namespace gcp
