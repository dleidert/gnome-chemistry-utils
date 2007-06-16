// -*- C++ -*-

/* 
 * GChemPaint library
 * reaction-arrow.cc 
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
#include "reaction-arrow.h"
#include "reaction.h"
#include "reaction-step.h"
#include "document.h"
#include "settings.h"
#include "theme.h"
#include "view.h"
#include "widgetdata.h"
#include <canvas/gcp-canvas-group.h>
#include <canvas/gcp-canvas-line.h>
#include <glib/gi18n-lib.h>
#include <cmath>

namespace gcp {

ReactionArrow::ReactionArrow (Reaction* react, unsigned Type): Arrow (ReactionArrowType)
{
	SetId ("ra1");
	m_Type = Type;
	m_Start = m_End = NULL;
	if (react)
		react->AddChild (this);
}


ReactionArrow::~ReactionArrow ()
{
	if (IsLocked ())
		return;
	if (m_Start)
		m_Start->RemoveArrow (this);
	if (m_End)
		m_End->RemoveArrow (this);
}

xmlNodePtr ReactionArrow::Save (xmlDocPtr xml)
{
	xmlNodePtr parent, node;
	node = xmlNewDocNode (xml, NULL, (xmlChar*) "reaction-arrow", NULL);
	if (!node)
		return NULL;
	if (!Arrow::Save (xml, node)) {
		xmlFreeNode (node);
		return NULL;
	}
	xmlNewProp (node, (xmlChar*) "type", (xmlChar*) ((m_Type == SimpleArrow)? "single": "double"));
	if (m_Type == FullReversibleArrow)
		xmlNewProp (node, (xmlChar*) "heads", (xmlChar*) "full");
	if (m_Start)
		xmlNewProp (node, (xmlChar*) "start",  (xmlChar*) m_Start->GetId ());
	if (m_End)
		xmlNewProp (node, (xmlChar*) "end",  (xmlChar*) m_End->GetId ());
	Reaction* r = (Reaction*) GetReaction();
	if (!r) {
		//save the arrow as an object (this is NOT safe)
		parent = xmlNewDocNode (xml, NULL, (xmlChar*) "object", NULL);
		if (node && parent)
			xmlAddChild (parent, node);
		else {
			xmlFreeNode(node);
			return NULL;
		}
	}
	else
		parent = node;
	SaveChildren (xml, node);
	return parent;
}

bool ReactionArrow::Load (xmlNodePtr node)
{
	char *buf;
	Object *parent, *prop;
	xmlNodePtr child;
	if (Arrow::Load (node)) {
		buf = (char*) xmlGetProp (node, (xmlChar*) "type");
		if (buf) {
			if (!strcmp (buf, "double")) {
				m_Type = ReversibleArrow;
				char *buf0 = (char*) xmlGetProp (node, (xmlChar*) "heads");
				if (buf0) {
					if (!strcmp (buf0, "full"))
						m_Type = FullReversibleArrow;
					xmlFree (buf0);
				}
				m_TypeChanged = true;
			}
			xmlFree (buf);
		}
		/* load children */
		child = GetNodeByName (node, "reaction-prop");
		while (child) {
			prop = CreateObject ("reaction-prop", this);
			if (prop) {
				if (!prop->Load (child))
					delete prop;
			}
			child = GetNextNodeByName (child->next, "reaction-prop");
		}
		parent = GetParent ();
		if (!parent)
			return true;
		buf = (char*) xmlGetProp (node, (xmlChar*) "start");
		if (buf) {
			m_Start = reinterpret_cast<ReactionStep*> (parent->GetDescendant (buf));
			xmlFree (buf);
			if (!m_Start)
				return false;
			m_Start->AddArrow (this);
		}
		buf = (char*) xmlGetProp (node, (xmlChar*) "end");
		if (buf) {
			m_End = reinterpret_cast<ReactionStep*> (parent->GetDescendant (buf));
			xmlFree (buf);
			if (!m_End)
				return false;
			m_End->AddArrow (this);
		}
		return true;
	}
	return false;
}

void ReactionArrow::Add (GtkWidget* w)
{
	WidgetData* pData = (WidgetData*) g_object_get_data (G_OBJECT (w), "data");
	Theme *pTheme = pData->m_View->GetDoc ()->GetTheme ();
	GnomeCanvasPoints *points = gnome_canvas_points_new (2);
	GnomeCanvasGroup* group = GNOME_CANVAS_GROUP(gnome_canvas_item_new (pData->Group, gnome_canvas_group_ext_get_type (), NULL));
	GnomeCanvasItem* item;
	switch(m_Type) {
		case SimpleArrow:
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
										"last_arrowhead", true,
										"arrow_shape_a", pTheme->GetArrowHeadA (),
										"arrow_shape_b", pTheme->GetArrowHeadB (),
										"arrow_shape_c", pTheme->GetArrowHeadC (),
										"last_arrowhead_style", (unsigned char) ARROW_HEAD_BOTH,
										NULL);
			g_object_set_data (G_OBJECT (item), "object", this);
			g_object_set_data (G_OBJECT (group), "arrow", item);
			g_signal_connect (G_OBJECT (item), "event", G_CALLBACK (on_event), w);
			break;
		case ReversibleArrow: {
			double dAngle = atan (- m_height / m_width);
			if (m_width < 0)
				dAngle += M_PI;
			points->coords[0] = m_x * pTheme->GetZoomFactor () - pTheme->GetArrowDist ()  / 2 * sin (dAngle);
			points->coords[1] = m_y * pTheme->GetZoomFactor () - pTheme->GetArrowDist ()  / 2 * cos (dAngle);
			points->coords[2] = (m_x + m_width) * pTheme->GetZoomFactor () - pTheme->GetArrowDist ()  / 2 * sin (dAngle);
			points->coords[3] = (m_y + m_height) * pTheme->GetZoomFactor () - pTheme->GetArrowDist ()  / 2 * cos (dAngle);
			item = gnome_canvas_item_new (
								group,
								gnome_canvas_line_ext_get_type (),
								"points", points,
								"fill_color", (pData->IsSelected(this))? SelectColor: Color,
								"width_units", pTheme->GetArrowWidth (),
								"last_arrowhead", true,
								"arrow_shape_a", pTheme->GetArrowHeadA (),
								"arrow_shape_b", pTheme->GetArrowHeadB (),
								"arrow_shape_c", pTheme->GetArrowHeadC (),
								"last_arrowhead_style", (unsigned char) ARROW_HEAD_LEFT,
								NULL);
			g_object_set_data (G_OBJECT (item), "object", this);
			g_object_set_data (G_OBJECT (group), "direct", item);
			g_signal_connect (G_OBJECT (item), "event", G_CALLBACK (on_event), w);
			points->coords[2] = m_x * pTheme->GetZoomFactor () + pTheme->GetArrowDist () / 2 * sin (dAngle);
			points->coords[3] = m_y * pTheme->GetZoomFactor () + pTheme->GetArrowDist ()  / 2 * cos (dAngle);
			points->coords[0] = (m_x + m_width) * pTheme->GetZoomFactor () + pTheme->GetArrowDist ()  / 2 * sin( dAngle);
			points->coords[1] = (m_y + m_height) * pTheme->GetZoomFactor () + pTheme->GetArrowDist ()  / 2 * cos (dAngle);
			item = gnome_canvas_item_new (
								group,
								gnome_canvas_line_ext_get_type (),
								"points", points,
								"fill_color", (pData->IsSelected (this))? SelectColor: Color,
								"width_units", pTheme->GetArrowWidth (),
								"last_arrowhead", true,
								"arrow_shape_a", pTheme->GetArrowHeadA (),
								"arrow_shape_b", pTheme->GetArrowHeadB (),
								"arrow_shape_c", pTheme->GetArrowHeadC (),
								"last_arrowhead_style", (unsigned char) ARROW_HEAD_LEFT,
								NULL);
			g_object_set_data (G_OBJECT (item), "object", this);
			g_object_set_data (G_OBJECT (group), "reverse", item);
			g_signal_connect (G_OBJECT (item), "event", G_CALLBACK (on_event), w);
			break;
		}
		case FullReversibleArrow: {
			double dAngle = atan (- m_height / m_width);
			if (m_width < 0)
				dAngle += M_PI;
			points->coords[0] = m_x * pTheme->GetZoomFactor () - pTheme->GetArrowDist ()  / 2 * sin (dAngle);
			points->coords[1] = m_y * pTheme->GetZoomFactor () - pTheme->GetArrowDist ()  / 2 * cos (dAngle);
			points->coords[2] = (m_x + m_width) * pTheme->GetZoomFactor () - pTheme->GetArrowDist ()  / 2 * sin (dAngle);
			points->coords[3] = (m_y + m_height) * pTheme->GetZoomFactor () - pTheme->GetArrowDist ()  / 2 * cos (dAngle);
			item = gnome_canvas_item_new(
								group,
								gnome_canvas_line_ext_get_type (),
								"points", points,
								"fill_color", (pData->IsSelected (this))? SelectColor: Color,
								"width_units", pTheme->GetArrowWidth (),
								"last_arrowhead", true,
								"arrow_shape_a", pTheme->GetArrowHeadA (),
								"arrow_shape_b", pTheme->GetArrowHeadB (),
								"arrow_shape_c", pTheme->GetArrowHeadC (),
								"last_arrowhead_style", (unsigned char) ARROW_HEAD_BOTH,
								NULL);
			g_object_set_data (G_OBJECT (item), "object", this);
			g_object_set_data (G_OBJECT (group), "direct", item);
			g_signal_connect (G_OBJECT (item), "event", G_CALLBACK (on_event), w);
			points->coords[2] = m_x * pTheme->GetZoomFactor () + pTheme->GetArrowDist () / 2 * sin (dAngle);
			points->coords[3] = m_y * pTheme->GetZoomFactor () + pTheme->GetArrowDist ()  / 2 * cos (dAngle);
			points->coords[0] = (m_x + m_width) * pTheme->GetZoomFactor () + pTheme->GetArrowDist ()  / 2 * sin (dAngle);
			points->coords[1] = (m_y + m_height) * pTheme->GetZoomFactor () + pTheme->GetArrowDist ()  / 2 * cos (dAngle);
			item = gnome_canvas_item_new (
								group,
								gnome_canvas_line_ext_get_type (),
								"points", points,
								"fill_color", (pData->IsSelected (this))? SelectColor: Color,
								"width_units", pTheme->GetArrowWidth (),
								"last_arrowhead", true,
								"arrow_shape_a", pTheme->GetArrowHeadA (),
								"arrow_shape_b", pTheme->GetArrowHeadB (),
								"arrow_shape_c", pTheme->GetArrowHeadC (),
								"last_arrowhead_style", (unsigned char) ARROW_HEAD_BOTH,
								NULL);
			g_object_set_data (G_OBJECT (item), "object", this);
			g_object_set_data (G_OBJECT (group), "reverse", item);
			g_signal_connect (G_OBJECT (item), "event", G_CALLBACK (on_event), w);
			break;
		}
	}
	pData->Items[this] = group;
	gnome_canvas_points_free (points);
}

void ReactionArrow::Update (GtkWidget* w)
{
	WidgetData* pData = (WidgetData*) g_object_get_data (G_OBJECT (w), "data");
	Theme *pTheme = pData->m_View->GetDoc ()->GetTheme ();
	GnomeCanvasGroup* group = pData->Items[this];
	if (!group) {
		Add (w);
		m_TypeChanged = false;
		return;
	}
	if (m_TypeChanged) {
		gtk_object_destroy (GTK_OBJECT (group));
		Add (w);
		m_TypeChanged = false;
		return;
	}
	GnomeCanvasPoints *points = gnome_canvas_points_new (2);
	switch(m_Type) {
		case SimpleArrow:
			points->coords[0] = m_x * pTheme->GetZoomFactor ();
			points->coords[1] = m_y * pTheme->GetZoomFactor ();
			points->coords[2] = (m_x + m_width) * pTheme->GetZoomFactor ();
			points->coords[3] = (m_y + m_height) * pTheme->GetZoomFactor ();
			g_object_set (G_OBJECT (g_object_get_data (G_OBJECT (group), "arrow")),
								"points", points,
								"width_units", pTheme->GetArrowWidth (),
								"arrow_shape_a", pTheme->GetArrowHeadA (),
								"arrow_shape_b", pTheme->GetArrowHeadB (),
								"arrow_shape_c", pTheme->GetArrowHeadC (),
								NULL);
			break;
		case ReversibleArrow:
			double dAngle = atan (- m_height / m_width);
			if (m_width < 0) dAngle += M_PI;
			points->coords[0] = m_x * pTheme->GetZoomFactor () - pTheme->GetArrowDist () / 2 * sin (dAngle);
			points->coords[1] = m_y * pTheme->GetZoomFactor () - pTheme->GetArrowDist () / 2 * cos (dAngle);
			points->coords[2] = (m_x + m_width) * pTheme->GetZoomFactor () - pTheme->GetArrowDist () / 2 * sin (dAngle);
			points->coords[3] = (m_y + m_height) * pTheme->GetZoomFactor () - pTheme->GetArrowDist () / 2 * cos (dAngle);
			g_object_set (G_OBJECT (g_object_get_data (G_OBJECT (group), "direct")),
								"points", points,
								"width_units", pTheme->GetArrowWidth (),
								"arrow_shape_a", pTheme->GetArrowHeadA (),
								"arrow_shape_b", pTheme->GetArrowHeadB (),
								"arrow_shape_c", pTheme->GetArrowHeadC (),
								NULL);
			points->coords[2] = m_x * pTheme->GetZoomFactor () + pTheme->GetArrowDist () / 2 * sin (dAngle);
			points->coords[3] = m_y * pTheme->GetZoomFactor () + pTheme->GetArrowDist () / 2 * cos (dAngle);
			points->coords[0] = (m_x + m_width) * pTheme->GetZoomFactor () + pTheme->GetArrowDist () / 2 * sin (dAngle);
			points->coords[1] = (m_y + m_height) * pTheme->GetZoomFactor () + pTheme->GetArrowDist () / 2 * cos (dAngle);
			g_object_set (G_OBJECT (g_object_get_data (G_OBJECT (group), "reverse")),
								"points", points,
								"width_units", pTheme->GetArrowWidth (),
								"arrow_shape_a", pTheme->GetArrowHeadA (),
								"arrow_shape_b", pTheme->GetArrowHeadB (),
								"arrow_shape_c", pTheme->GetArrowHeadC (),
								NULL);
			break;
	}
	gnome_canvas_points_free (points);
}

void ReactionArrow::RemoveStep (ReactionStep *Step)
{
	if (Step == m_Start)
		m_Start = NULL;
	else if (Step == m_End)
		m_End = NULL;
}

struct CallbackData {
	ReactionArrow *arrow;
	Object *child;
};

static void do_attach_object ()
{
}

static void do_free_data (struct CallbackData *data)
{
	delete data;
}

bool ReactionArrow::BuildContextualMenu (GtkUIManager *UIManager, Object *object, double x, double y)
{
	gcp::Document *Doc = dynamic_cast<gcp::Document*> (GetDocument ());
	gcp::WidgetData* pData = (gcp::WidgetData*) g_object_get_data (G_OBJECT (Doc->GetWidget ()), "data");
	if (pData->SelectedObjects.size () != 1)
		return Object::BuildContextualMenu (UIManager, object, x, y);
	Object *obj = pData->SelectedObjects.front ();
	TypeId Id = obj->GetType ();
	if ((Id != MoleculeType && Id != TextType) || obj->GetGroup ())
		return Object::BuildContextualMenu (UIManager, object, x, y);
	GtkActionGroup *group = gtk_action_group_new ("reaction-arrow");
	GtkAction *action = gtk_action_new ("Arrow", _("Arrow"), NULL, NULL);
	gtk_action_group_add_action (group, action);
	g_object_unref (action);
	struct CallbackData *data = new struct CallbackData ();
	data->arrow = this;
	data->child = obj;
	action = gtk_action_new ("attach", _("Attach selection to arrow..."), NULL, NULL);
	g_object_set_data_full (G_OBJECT (action), "data", data, (GDestroyNotify) do_free_data);
	g_signal_connect_swapped (action, "activate", G_CALLBACK (do_attach_object), data);
	gtk_action_group_add_action (group, action);
	g_object_unref (action);
	gtk_ui_manager_add_ui_from_string (UIManager, "<ui><popup><menu action='Arrow'><menuitem action='attach'/></menu></popup></ui>", -1, NULL);
	gtk_ui_manager_insert_action_group (UIManager, group, 0);
	g_object_unref (group);
	return true;
}

}	//	namespace gcp
