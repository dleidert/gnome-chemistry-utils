// -*- C++ -*-

/* 
 * GChemPaint library
 * mesomery-arrow.cc 
 *
 * Copyright (C) 2004-2007 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "mesomery-arrow.h"
#include "mesomery.h"
#include "mesomer.h"
#include "document.h"
#include "settings.h"
#include "theme.h"
#include "view.h"
#include "widgetdata.h"
#include <canvas/gcp-canvas-line.h>
#include <canvas/gcp-canvas-group.h>

namespace gcp {

MesomeryArrow::MesomeryArrow (Mesomery* mesomery): Arrow (MesomeryArrowType)
{
	if (mesomery)
		mesomery->AddChild( this);
	m_Start = m_End = NULL;
}

MesomeryArrow::~MesomeryArrow ()
{
	if (IsLocked ())
		return;
	if (m_Start && m_End) {
		m_Start->RemoveArrow (this, m_End);
		m_End->RemoveArrow (this, m_Start);
	}
}

xmlNodePtr MesomeryArrow::Save (xmlDocPtr xml)
{
	xmlNodePtr parent, node;
	node = xmlNewDocNode (xml, NULL, (xmlChar*) "mesomery-arrow", NULL);
	if (!node)
		return NULL;
	if (!Arrow::Save (xml, node)) {
		xmlFreeNode (node);
		return NULL;
	}
	if (m_Start)
		xmlNewProp (node, (xmlChar*) "start",  (xmlChar*) m_Start->GetId ());
	if (m_End)
		xmlNewProp (node, (xmlChar*) "end",  (xmlChar*) m_End->GetId ());
	Mesomery* m = (Mesomery*) GetParentOfType (MesomeryType);
	if (!m)
	{
		//save the arrow as an object
		parent = xmlNewDocNode (xml, NULL, (xmlChar*) "object", NULL);
		if (node && parent)
			xmlAddChild (parent, node);
		else {
			xmlFreeNode (node);
			return NULL;
			}
	}
	else parent = node;
	return parent;
}

bool MesomeryArrow::Load (xmlNodePtr node)
{
	char *buf;
	Object *parent;
	if (Arrow::Load (node)) {
		parent = GetParent ();
		if (!parent)
			return true;
		buf = (char*) xmlGetProp (node, (xmlChar*) "start");
		if (buf) {
			m_Start = reinterpret_cast<Mesomer*> (parent->GetDescendant (buf));
			xmlFree (buf);
			if (!m_Start)
				return false;
		}
		buf = (char*) xmlGetProp (node, (xmlChar*) "end");
		if (buf) {
			m_End = reinterpret_cast<Mesomer*> (parent->GetDescendant (buf));
			xmlFree (buf);
			if (!m_End)
				return false;
			m_End->AddArrow (this, m_Start);
		}
		if (m_Start)
			m_Start->AddArrow (this, m_End);
		return true;
	}
	return false;
}

void MesomeryArrow::Add (GtkWidget* w)
{
	WidgetData* pData = (WidgetData*) g_object_get_data (G_OBJECT (w), "data");
	Theme *pTheme = pData->m_View->GetDoc ()->GetTheme ();
	GnomeCanvasPoints *points = gnome_canvas_points_new (2);
	GnomeCanvasGroup* group = GNOME_CANVAS_GROUP(gnome_canvas_item_new (pData->Group, gnome_canvas_group_ext_get_type (), NULL));
	GnomeCanvasItem* item;
	points->coords[0] = m_x * pTheme->GetZoomFactor ();
	points->coords[1] = m_y * pTheme->GetZoomFactor ();
	points->coords[2] = (m_x + m_width) * pTheme->GetZoomFactor ();
	points->coords[3] = (m_y + m_height) * pTheme->GetZoomFactor ();
	item = gnome_canvas_item_new (
								group,
								gnome_canvas_line_ext_get_type (),
								"points", points,
								"fill_color", (pData->IsSelected (this))? SelectColor: Color,
								"width_units", pTheme->GetArrowWidth (),
								"first_arrowhead", true,
								"last_arrowhead", true,
								"arrow_shape_a", pTheme->GetArrowHeadA (),
								"arrow_shape_b", pTheme->GetArrowHeadB (),
								"arrow_shape_c", pTheme->GetArrowHeadC (),
								"first_arrowhead_style", (unsigned char) ARROW_HEAD_BOTH,
								"last_arrowhead_style", (unsigned char) ARROW_HEAD_BOTH,
								NULL);
	g_object_set_data (G_OBJECT(item), "object", this);
	g_object_set_data (G_OBJECT(group), "arrow", item);
	g_signal_connect(G_OBJECT (item), "event", G_CALLBACK (on_event), w);
	pData->Items[this] = group;
	gnome_canvas_points_free (points);
}

void MesomeryArrow::Update (GtkWidget* w)
{
	WidgetData* pData = (WidgetData*) g_object_get_data (G_OBJECT (w), "data");
	Theme *pTheme = pData->m_View->GetDoc ()->GetTheme ();
	GnomeCanvasGroup* group = pData->Items[this];
	GnomeCanvasPoints *points = gnome_canvas_points_new( 2);
	points->coords[0] = m_x * pTheme->GetZoomFactor ();
	points->coords[1] = m_y * pTheme->GetZoomFactor ();
	points->coords[2] = (m_x + m_width) * pTheme->GetZoomFactor ();
	points->coords[3] = (m_y + m_height) * pTheme->GetZoomFactor ();
	g_object_set (G_OBJECT(g_object_get_data (G_OBJECT (group), "arrow")),
						"points", points,
						"width_units", pTheme->GetArrowWidth (),
						"arrow_shape_a", pTheme->GetArrowHeadA (),
						"arrow_shape_b", pTheme->GetArrowHeadB (),
						"arrow_shape_c", pTheme->GetArrowHeadC (),
						NULL);
	gnome_canvas_points_free (points);
}

void MesomeryArrow::Reverse ()
{
	Mesomer *mesomer = m_Start;
	m_Start= m_End;
	m_End = mesomer;
	m_x = m_x + m_width;
	m_y = m_y + m_height;
	m_width = - m_width;
	m_height = - m_height;
}

}	//	namespace gcp
