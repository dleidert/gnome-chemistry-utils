// -*- C++ -*-

/* 
 * GChemPaint library
 * arrow.cc 
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
#include "widgetdata.h"
#include "view.h"
#include "settings.h"
#include <canvas/gcp-canvas-line.h>
#include <canvas/gprintable.h>
#include "arrow.h"
#include "document.h"

using namespace gcu;

namespace gcp {

Arrow::Arrow (TypeId Type): Object (Type)
{
}

Arrow::~Arrow ()
{
}

bool Arrow::Save (xmlDocPtr xml, xmlNodePtr node)
{
	xmlNodePtr child;
	gchar buf[16];
	if (!node)
		return false;
	SaveId (node);
	child = xmlNewDocNode (xml, NULL, (xmlChar*) "start", NULL);
	if (child)
		xmlAddChild (node, child);
	else
		return false;
	g_snprintf (buf, sizeof (buf), "%g", m_x);
	xmlNewProp (child, (xmlChar*) "x", (xmlChar*) buf);
	g_snprintf (buf, sizeof (buf), "%g", m_y);
	xmlNewProp (child, (xmlChar*) "y", (xmlChar*) buf);
	child = xmlNewDocNode (xml, NULL, (xmlChar*) "end", NULL);
	if (child)
		xmlAddChild(node, child);
	else
		return false;
	g_snprintf (buf, sizeof (buf), "%g", m_x + m_width);
	xmlNewProp (child, (xmlChar*) "x", (xmlChar*) buf);
	g_snprintf (buf, sizeof (buf), "%g", m_y + m_height);
	xmlNewProp (child, (xmlChar*) "y", (xmlChar*) buf);
	return true;
}

bool Arrow::Load (xmlNodePtr node)
{
	char* tmp, *endptr;
	bool result;
	xmlNodePtr child;
	tmp = (char*) xmlGetProp (node, (xmlChar*) "id");
	if (tmp) {
		SetId (tmp);
		xmlFree (tmp);
	}
	child = GetNodeByName (node, "start");
	if (!child)
		return false;
	tmp = (char*) xmlGetProp (child, (xmlChar*) "x");
	if (!tmp)
		return false;
	m_x = strtod (tmp, &endptr);
	result = *endptr;
	xmlFree (tmp);
	if (result)
		return false;
	tmp = (char*) xmlGetProp (child, (xmlChar*) "y");
	if (!tmp)
		return false;
	m_y = strtod (tmp, &endptr);
	result = *endptr;
	xmlFree (tmp);
	if (result)
		return false;
	child = GetNodeByName (node, "end");
	if (!child)
		return false;
	tmp = (char*) xmlGetProp (child, (xmlChar*) "x");
	if (!tmp)
		return false;
	m_width = strtod (tmp, &endptr) - m_x;
	result = *endptr;
	xmlFree (tmp);
	if (result)
		return false;
	tmp = (char*) xmlGetProp (child, (xmlChar*) "y");
	if (!tmp)
		return false;
	m_height = strtod (tmp, &endptr) - m_y;
	result = *endptr;
	xmlFree (tmp);
	if (result)
		return false;
	return true;
}

void Arrow::SetSelected (GtkWidget* w, int state)
{
	WidgetData* pData = (WidgetData*) g_object_get_data (G_OBJECT (w), "data");
	GnomeCanvasGroup* group = pData->Items[this];
	gchar const *color;
	switch (state) {	
	case SelStateUnselected:
		color = Color;
		break;
	case SelStateSelected:
		color = SelectColor;
		break;
	case SelStateUpdating:
		color = AddColor;
		break;
	case SelStateErasing:
		color = DeleteColor;
		break;
	default:
		color = Color;
		break;
	}
	GList* il = group->item_list;
	while (il) {
		g_object_set (G_OBJECT(il->data), "fill_color", color, NULL);
		il = il->next;
	}
}

void Arrow::SetCoords (double xstart, double ystart, double xend, double yend)
{
	m_x = xstart;
	m_y = ystart;
	m_width = xend - xstart;
	m_height = yend - ystart;
}

bool Arrow::GetCoords (double* xstart, double* ystart, double* xend, double* yend)
{
	*xstart = m_x;
	*ystart = m_y;
	*xend = m_x + m_width;
	*yend = m_y + m_height;
	return true;
}

void Arrow::Move (double x, double y, double)
{
	m_x += x;
	m_y += y;
}

void Arrow::Transform2D (Matrix2D& m, double x, double y)
{
	m_x -= x;
	m_y -= y;
	m.Transform (m_x, m_y);
	m_x += x;
	m_y += y;
	m.Transform (m_width, m_height);
}

double Arrow::GetYAlign ()
{
	return m_y + m_height / 2.;
}

}	//	namespace gcp
