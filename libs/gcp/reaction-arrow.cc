// -*- C++ -*-

/*
 * GChemPaint library
 * reaction-arrow.cc
 *
 * Copyright (C) 2004-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "reaction-arrow.h"
#include "reaction.h"
#include "reaction-step.h"
#include "reaction-prop.h"
#include "reaction-prop-dlg.h"
#include "document.h"
#include "settings.h"
#include "theme.h"
#include "view.h"
#include "widgetdata.h"
#include <gccv/arrow.h>
#include <gccv/canvas.h>
#include <gccv/group.h>
#include <gcugtk/ui-manager.h>
#include <gcu/objprops.h>
#include <glib/gi18n-lib.h>
#include <sstream>
#include <cmath>
#include <cstring>

using namespace gcu;
using namespace std;

namespace gcp {

ReactionArrow::ReactionArrow (Reaction* react, unsigned Type): Arrow (ReactionArrowType)
{
	SetId ("ra1");
	m_Type = Type;
	if (react)
		react->AddChild (this);
	m_TypeChanged = false;
}


ReactionArrow::~ReactionArrow ()
{
}

xmlNodePtr ReactionArrow::Save (xmlDocPtr xml) const
{
	xmlNodePtr node;
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
	if (GetStartStep ())
		xmlNewProp (node, (xmlChar*) "start",  (xmlChar*) GetStartStep ()->GetId ());
	if (GetEndStep ())
		xmlNewProp (node, (xmlChar*) "end",  (xmlChar*) GetEndStep ()->GetId ());
	SaveChildren (xml, node);
	return node;
}

bool ReactionArrow::Load (xmlNodePtr node)
{
	char *buf;
	Object *parent, *prop;
	gcu::Document *doc = GetDocument ();
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
			doc->SetTarget (buf, reinterpret_cast <Object **> (GetStartStepPtr ()), GetParent (), this, ActionIgnore);
			xmlFree (buf);
		}
		buf = (char*) xmlGetProp (node, (xmlChar*) "end");
		if (buf) {
			doc->SetTarget (buf, reinterpret_cast <Object **> (GetEndStepPtr ()), GetParent (), this, ActionIgnore);
			xmlFree (buf);
		}
		return true;
	}
	return false;
}

void ReactionArrow::AddItem ()
{
	if (m_Item)
		return;
	Document *doc = static_cast <Document*> (GetDocument ());
	View *view = doc->GetView ();
	Theme *theme = doc->GetTheme ();
	switch(m_Type) {
	case SimpleArrow: {
		gccv::Arrow *arrow = new gccv::Arrow (view->GetCanvas ()->GetRoot (),
											  m_x * theme->GetZoomFactor (),
											  m_y * theme->GetZoomFactor (),
											  (m_x + m_width) * theme->GetZoomFactor (),
											  (m_y + m_height) * theme->GetZoomFactor (),
											  this);
		arrow->SetLineColor ((view->GetData ()->IsSelected (this))? SelectColor: Color);
		arrow->SetLineWidth (theme->GetArrowWidth ());
		arrow->SetA (theme->GetArrowHeadA ());
		arrow->SetB (theme->GetArrowHeadB ());
		arrow->SetC (theme->GetArrowHeadC ());
		m_Item = arrow;
		break;
	}
	case ReversibleArrow: {
		double dAngle = atan2 (m_height, m_width);
		gccv::Group *group = new gccv::Group (view->GetCanvas ()->GetRoot (), this);
		gccv::Arrow *arrow = new gccv::Arrow (group,
											  m_x * theme->GetZoomFactor () - theme->GetArrowDist () / 2. * sin (dAngle),
											  m_y * theme->GetZoomFactor () - theme->GetArrowDist () / 2. * cos (dAngle),
											  (m_x + m_width) * theme->GetZoomFactor () - theme->GetArrowDist () / 2. * sin (dAngle),
											  (m_y + m_height) * theme->GetZoomFactor () - theme->GetArrowDist () / 2. * cos (dAngle),
											  this);
		arrow->SetLineColor ((view->GetData ()->IsSelected (this))? SelectColor: Color);
		arrow->SetLineWidth (theme->GetArrowWidth ());
		arrow->SetA (theme->GetArrowHeadA ());
		arrow->SetB (theme->GetArrowHeadB ());
		arrow->SetC (theme->GetArrowHeadC ());
		arrow->SetEndHead (gccv::ArrowHeadLeft);
		arrow = new gccv::Arrow (group,
								 (m_x + m_width) * theme->GetZoomFactor () + theme->GetArrowDist () / 2. * sin (dAngle),
								 (m_y + m_height) * theme->GetZoomFactor () + theme->GetArrowDist () / 2. * cos (dAngle),
								 m_x * theme->GetZoomFactor () + theme->GetArrowDist () / 2. * sin (dAngle),
								 m_y * theme->GetZoomFactor () + theme->GetArrowDist () / 2. * cos (dAngle),
								 this);
		arrow->SetLineColor ((view->GetData ()->IsSelected (this))? SelectColor: Color);
		arrow->SetLineWidth (theme->GetArrowWidth ());
		arrow->SetA (theme->GetArrowHeadA ());
		arrow->SetB (theme->GetArrowHeadB ());
		arrow->SetC (theme->GetArrowHeadC ());
		arrow->SetEndHead (gccv::ArrowHeadLeft);
		m_Item = group;
		break;
	}
	case FullReversibleArrow: {
		double dAngle = atan2 (m_height, m_width);
		gccv::Group *group = new gccv::Group (view->GetCanvas ()->GetRoot (), this);
		gccv::Arrow *arrow = new gccv::Arrow (group,
											  m_x * theme->GetZoomFactor () - theme->GetArrowDist () / 2. * sin (dAngle),
											  m_y * theme->GetZoomFactor () - theme->GetArrowDist () / 2. * cos (dAngle),
											  (m_x + m_width) * theme->GetZoomFactor () - theme->GetArrowDist () / 2. * sin (dAngle),
											  (m_y + m_height) * theme->GetZoomFactor () - theme->GetArrowDist () / 2. * cos (dAngle),
											  this);
		arrow->SetLineColor ((view->GetData ()->IsSelected (this))? SelectColor: Color);
		arrow->SetLineWidth (theme->GetArrowWidth ());
		arrow->SetA (theme->GetArrowHeadA ());
		arrow->SetB (theme->GetArrowHeadB ());
		arrow->SetC (theme->GetArrowHeadC ());
		arrow = new gccv::Arrow (group,
								 (m_x + m_width) * theme->GetZoomFactor () + theme->GetArrowDist () / 2. * sin (dAngle),
								 (m_y + m_height) * theme->GetZoomFactor () + theme->GetArrowDist () / 2. * cos (dAngle),
								 m_x * theme->GetZoomFactor () + theme->GetArrowDist () / 2. * sin (dAngle),
								 m_y * theme->GetZoomFactor () + theme->GetArrowDist () / 2. * cos (dAngle),
								 this);
		arrow->SetLineColor ((view->GetData ()->IsSelected (this))? SelectColor: Color);
		arrow->SetLineWidth (theme->GetArrowWidth ());
		arrow->SetA (theme->GetArrowHeadA ());
		arrow->SetB (theme->GetArrowHeadB ());
		arrow->SetC (theme->GetArrowHeadC ());
		m_Item = group;
		break;
	}
	}
}

void ReactionArrow::UpdateItem ()
{
	if (m_Item) {
		delete m_Item;
		m_Item = NULL;
	}
	AddItem ();
}

struct CallbackData {
	ReactionArrow *arrow;
	Object *child;
};

static void do_attach_object (struct CallbackData *data)
{
	data->arrow->AddProp (data->child);
}

static void do_free_data (struct CallbackData *data)
{
	delete data;
}

bool ReactionArrow::BuildContextualMenu (gcu::UIManager *UIManager, Object *object, double x, double y)
{
	GtkUIManager *uim = static_cast < gcugtk::UIManager * > (UIManager)->GetUIManager ();
	Document *Doc = dynamic_cast<Document*> (GetDocument ());
	WidgetData* pData = (WidgetData*) g_object_get_data (G_OBJECT (Doc->GetWidget ()), "data");
	// Don't allow more than one child at the moment
	if (pData->SelectedObjects.size () != 1 || HasChildren ())
		return Object::BuildContextualMenu (UIManager, object, x, y);
	Object *obj = *pData->SelectedObjects.begin ();
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
	gtk_ui_manager_add_ui_from_string (uim, "<ui><popup><menu action='Arrow'><menuitem action='attach'/></menu></popup></ui>", -1, NULL);
	gtk_ui_manager_insert_action_group (uim, group, 0);
	g_object_unref (group);
	return true;
}

void ReactionArrow::Move (double x, double y, double z)
{
	// Move the arrow
	Arrow::Move (x, y, z);
	// Move its children
	Object::Move (x, y, z);
}

void ReactionArrow::SetSelected (int state)
{
	// Select the arrow
	Arrow::SetSelected (state);
	// Select its children
//	Object::SetSelected (state);
}

void ReactionArrow::AddProp (Object *object)
{
	Document *Doc = dynamic_cast<Document*> (GetDocument ());
	WidgetData* pData = (WidgetData*) g_object_get_data (G_OBJECT (Doc->GetWidget ()), "data");
	Operation *Op = Doc->GetNewOperation (GCP_MODIFY_OPERATION);
	Op->AddObject (object, 0);
	Object *Group = GetGroup ();
	if (!Group)
		Group = this;
	Op->AddObject (Group, 0);
	ReactionProp *prop = new ReactionProp (this, object);
	// add the child in the object tree
	AddChild (prop);
	// position the child
	PositionChild (prop);
	Op->AddObject (Group, 1);
	Doc->FinishOperation ();
	pData->UnselectAll ();
	EmitSignal (OnChangedSignal);
	new ReactionPropDlg (this, prop);
}

void ReactionArrow::PositionChild (ReactionProp *prop)
{
	// FIXME: this is experimental code
	Document *Doc = dynamic_cast<Document*> (GetDocument ());
	Theme *pTheme = Doc->GetTheme ();
	double xmin, xspan, ymin, yspan,
		length = sqrt (m_width * m_width + m_height * m_height),
		x = m_width / length, y = m_height / length;
	gccv::Rect rect;
	WidgetData* pData = (WidgetData*) g_object_get_data (G_OBJECT (Doc->GetWidget ()), "data");
	pData->GetObjectBounds (prop, &rect);
	if (m_width >=0) {
		if (m_height >=0) {
			xmin = (rect.x0 * x + rect.y0 * y) / pTheme->GetZoomFactor ();
			xspan = (rect.x1 * x + rect.y1 * y) / pTheme->GetZoomFactor () - xmin;
			ymin = (rect.x0 * y - rect.y1 * x) / pTheme->GetZoomFactor ();
			yspan = (rect.x1 * y - rect.y0 * x) / pTheme->GetZoomFactor () - ymin;
		} else {
			xmin = (rect.x0 * x + rect.y1 * y) / pTheme->GetZoomFactor ();
			xspan = (rect.x1 * x + rect.y0 * y) / pTheme->GetZoomFactor () - xmin;
			ymin = (rect.x0 * y - rect.y0 * x) / pTheme->GetZoomFactor ();
			yspan = (rect.x1 * y - rect.y1 * x) / pTheme->GetZoomFactor () - ymin;
		}
	} else {
		if (m_height >=0) {
			xmin = (rect.x1 * x + rect.y0 * y) / pTheme->GetZoomFactor ();
			xspan = (rect.x0 * x + rect.y1 * y) / pTheme->GetZoomFactor () - xmin;
			ymin = (rect.x1 * y - rect.y1 * x) / pTheme->GetZoomFactor ();
			yspan = (rect.x0 * y - rect.y0 * x) / pTheme->GetZoomFactor () - ymin;
		} else {
			xmin = (rect.x1 * x + rect.y1 * y) / pTheme->GetZoomFactor ();
			xspan = (rect.x0 * x + rect.y0 * y) / pTheme->GetZoomFactor () - xmin;
			ymin = (rect.x1 * y - rect.y0 * x) / pTheme->GetZoomFactor ();
			yspan = (rect.x0 * y - rect.y1 * x) / pTheme->GetZoomFactor () - ymin;
		}
	}
	xspan = fabs (xspan);
	yspan = fabs (yspan);
	// xmin and ymin will now be the current center of the object
	xspan += (2* pTheme->GetArrowObjectPadding () + pTheme->GetArrowHeadA ()) / pTheme->GetZoomFactor ();
	// adjust the arrow length if needed
	if (xspan > length) {
		m_width *= xspan / length;
		m_height *= xspan / length;
		length = xspan;
	}
	// now move the child to the right place
	length -= pTheme->GetArrowHeadA () / pTheme->GetZoomFactor ();
	length /= 2.;
	// FIXME: using GetArrowDist is a non-sense, we should have a new variable.
	yspan = yspan / 2. + pTheme->GetArrowDist () / pTheme->GetZoomFactor ();
	// calculate the vector of the needed move
	xmin = m_x + length * x + y * yspan - (rect.x0 + rect.x1) / 2. / pTheme->GetZoomFactor ();
	ymin = m_y + length * y - x * yspan - (rect.y0 + rect.y1) / 2. / pTheme->GetZoomFactor ();
	prop->Move (xmin, ymin);
	Doc->GetView ()->Update (this);
}

bool ReactionArrow::OnSignal (SignalId Signal, G_GNUC_UNUSED Object *Child)
{
	if (Signal == OnChangedSignal) {
		// for now we can have only one child property.
		map<string, Object*>::iterator i;
		ReactionProp *prop = dynamic_cast <ReactionProp *> (GetFirstChild (i));
		if (prop != NULL)
			PositionChild (prop);
	}
	return true;
}

std::string ReactionArrow::GetProperty (unsigned property) const
{
	std::ostringstream res;
	switch (property) {
	case GCU_PROP_REACTION_ARROW_TYPE:
		res << (m_Type == ReversibleArrow)? "double": "simple";
		break;
	default:
		return Arrow::GetProperty (property);
	}
	return res.str ();
}

bool ReactionArrow::SetProperty (unsigned property, char const *value)
{
	switch (property) {
	case GCU_PROP_REACTION_ARROW_TYPE:
		m_Type = (strcmp (value, "double"))? ReversibleArrow: SimpleArrow;
		break;
	default:
		return Arrow::SetProperty (property, value);
	}
	return true;
}

std::string ReactionArrow::Name ()
{
	return _("Reaction arrow");
}

char ReactionArrow::GetSymbolicPosition (double x, double y)
{
	x -= m_x;
	y -= m_y;
	double s = x * m_width + y * m_height;
	if (s < 0.)
		return 't';
	double l = m_width * m_width + m_height * m_height;
	return (s > l)? 'h': 'o';
}

}	//	namespace gcp
