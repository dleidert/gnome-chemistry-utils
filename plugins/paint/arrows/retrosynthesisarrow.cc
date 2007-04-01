// -*- C++ -*-

/* 
 * GChemPaint arrows plugin
 * retrosynthesisarrow.cc
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
#include "retrosynthesisarrow.h"
#include "retrosynthesis.h"
#include "retrosynthesisstep.h"
#include <gcp/document.h>
#include <gcp/settings.h>
#include <gcp/theme.h>
#include <gcp/view.h>
#include <gcp/widgetdata.h>
#include <canvas/gcp-canvas-bpath.h>
#include <canvas/gcp-canvas-group.h>
#include <math.h>

TypeId RetrosynthesisArrowType = NoType;

gcpRetrosynthesisArrow::gcpRetrosynthesisArrow (gcpRetrosynthesis *rs): gcp::Arrow (RetrosynthesisArrowType)
{
	SetId ("rsa1");
	if (rs)
		rs->AddChild( this);
	m_Start = m_End = NULL;
}

gcpRetrosynthesisArrow::~gcpRetrosynthesisArrow ()
{
	if (IsLocked ())
		return;
	if (m_Start && m_End) {
		m_Start->RemoveArrow (this, m_End);
		m_End->RemoveArrow (this, m_Start);
	}
}

xmlNodePtr gcpRetrosynthesisArrow::Save (xmlDocPtr xml)
{
	xmlNodePtr parent, node;
	node = xmlNewDocNode (xml, NULL, (xmlChar*) "retrosynthesis-arrow", NULL);
	if (!node) return NULL;
	if (!gcp::Arrow::Save (xml, node)) {
		xmlFreeNode (node);
		return NULL;
	}
	if (m_Start)
		xmlNewProp (node, (xmlChar*) "start",  (xmlChar*) m_Start->GetId ());
	if (m_End)
		xmlNewProp (node, (xmlChar*) "end",  (xmlChar*) m_End->GetId ());
	gcpRetrosynthesis* r = (gcpRetrosynthesis*) GetParentOfType (RetrosynthesisType);
	if (!r) {
		//save the arrow as an object
		parent = xmlNewDocNode (xml, NULL, (xmlChar*)"object", NULL);
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

bool gcpRetrosynthesisArrow::Load (xmlNodePtr node)
{
	char *buf;
	Object *parent;
	if (gcp::Arrow::Load (node)) {
		parent = GetParent ();
		if (!parent)
			return true;
		buf = (char*) xmlGetProp (node, (xmlChar*) "start");
		if (buf) {
			m_Start = reinterpret_cast<gcpRetrosynthesisStep*> (parent->GetDescendant (buf));
			xmlFree (buf);
			if (!m_Start)
				return false;
		}
		buf = (char*) xmlGetProp (node, (xmlChar*) "end");
		if (buf) {
			m_End = reinterpret_cast<gcpRetrosynthesisStep*> (parent->GetDescendant (buf));
			xmlFree (buf);
			if (!m_End)
				return false;
			m_End->AddArrow (this, m_Start, false);
		}
		if (m_Start)
			m_Start->AddArrow (this, m_End, true);
		return true;
	}
	return false;
}

void gcpRetrosynthesisArrow::Add (GtkWidget* w)
{
	gcp::WidgetData* pData = (gcp::WidgetData*) g_object_get_data (G_OBJECT (w), "data");
	gcp::Theme *pTheme = pData->m_View->GetDoc ()->GetTheme ();
	double x0, y0, x1, y1, dx, dy, dAngle;
	x0 = m_x * pTheme->GetZoomFactor ();
	y0 = m_y * pTheme->GetZoomFactor ();
	x1 = (m_x + m_width) * pTheme->GetZoomFactor ();
	y1 = (m_y + m_height) * pTheme->GetZoomFactor ();
	if (m_width == 0.) {
		if (m_height == 0.)
			return;
		dAngle = (m_height < 0.) ? M_PI / 2 : 1.5 * M_PI;
	} else {
		dAngle = atan(- m_height / m_width);
		if (m_width < 0)
			dAngle += M_PI;
	}
	GnomeCanvasGroup* group = GNOME_CANVAS_GROUP (gnome_canvas_item_new (pData->Group, gnome_canvas_group_ext_get_type(), NULL));
	GnomeCanvasItem* item;
	dx = pTheme->GetArrowDist () / 2 * sin (dAngle);
	dy = pTheme->GetArrowDist () / 2 * cos (dAngle);
	GnomeCanvasPathDef* path = gnome_canvas_path_def_new ();
	gnome_canvas_path_def_moveto (path, x0 - dx, y0 - dy);
	gnome_canvas_path_def_lineto (path, x1 - dx - dy, y1 - dy + dx);
	gnome_canvas_path_def_moveto (path, x0 + dx, y0 + dy);
	gnome_canvas_path_def_lineto (path, x1 + dx - dy, y1 + dy + dx);
	dx += pTheme->GetArrowHeadC () * sin (dAngle);
	dy += pTheme->GetArrowHeadC () * cos (dAngle);
	gnome_canvas_path_def_moveto (path, x1 - dx - dy, y1 - dy + dx);
	gnome_canvas_path_def_lineto (path, x1, y1);
	gnome_canvas_path_def_lineto (path, x1 + dx - dy, y1 + dy + dx);
	item = gnome_canvas_item_new (
							group,
							gnome_canvas_bpath_ext_get_type (),
							"bpath", path,
							"outline_color", (pData->IsSelected (this))? gcp::SelectColor: gcp::Color,
							"width_units", pTheme->GetArrowWidth (),
							"cap-style", GDK_CAP_BUTT,
							"join-style", GDK_JOIN_MITER,
							NULL);
	g_object_set_data (G_OBJECT (item), "object", this);
	g_object_set_data( G_OBJECT (group), "arrow", item);
	g_signal_connect(G_OBJECT (item), "event", G_CALLBACK (gcp::on_event), w);
	pData->Items[this] = group;
}

void gcpRetrosynthesisArrow::Update (GtkWidget* w)
{
	gcp::WidgetData* pData = (gcp::WidgetData*) g_object_get_data (G_OBJECT (w), "data");
	gcp::Theme *pTheme = pData->m_View->GetDoc ()->GetTheme ();
	GnomeCanvasGroup* group = pData->Items[this];
	double x0, y0, x1, y1, dx, dy, dAngle;
	x0 = m_x * pTheme->GetZoomFactor ();
	y0 = m_y * pTheme->GetZoomFactor ();
	x1 = (m_x + m_width) * pTheme->GetZoomFactor ();
	y1 = (m_y + m_height) * pTheme->GetZoomFactor ();
	if (m_width == 0.) {
		if (m_height == 0.)
			return;
		dAngle = (m_height < 0.) ? M_PI / 2 : 1.5 * M_PI;
	} else {
		dAngle = atan (- m_height / m_width);
		if (m_width < 0)
			dAngle += M_PI;
	}
	dx = pTheme->GetArrowDist () / 2 * sin (dAngle);
	dy = pTheme->GetArrowDist () / 2 * cos (dAngle);
	GnomeCanvasPathDef* path = gnome_canvas_path_def_new ();
	gnome_canvas_path_def_moveto (path, x0 - dx, y0 - dy);
	gnome_canvas_path_def_lineto (path, x1 - dx - dy, y1 - dy + dx);
	gnome_canvas_path_def_moveto (path, x0 + dx, y0 + dy);
	gnome_canvas_path_def_lineto (path, x1 + dx - dy, y1 + dy + dx);
	dx += pTheme->GetArrowHeadC () * sin (dAngle);
	dy += pTheme->GetArrowHeadC () * cos (dAngle);
	gnome_canvas_path_def_moveto (path, x1 - dx - dy, y1 - dy + dx);
	gnome_canvas_path_def_lineto (path, x1, y1);
	gnome_canvas_path_def_lineto (path, x1 + dx - dy, y1 + dy + dx);
	g_object_set (G_OBJECT (g_object_get_data (G_OBJECT (group), "arrow")),
						"bpath", path,
						NULL);
}

void gcpRetrosynthesisArrow::SetSelected (GtkWidget* w, int state)
{
	gcp::WidgetData* pData = (gcp::WidgetData*) g_object_get_data (G_OBJECT (w), "data");
	GnomeCanvasGroup* group = pData->Items[this];
	gchar* color;
	switch (state) {	
	case gcp::SelStateUnselected:
		color = gcp::Color;
		break;
	case gcp::SelStateSelected:
		color = gcp::SelectColor;
		break;
	case gcp::SelStateUpdating:
		color = gcp::AddColor;
		break;
	case gcp::SelStateErasing:
		color = gcp::DeleteColor;
		break;
	default:
		color = gcp::Color;
		break;
	}
	g_object_set (G_OBJECT (g_object_get_data (G_OBJECT (group), "arrow")),
						"outline_color", color,
						NULL);
}
