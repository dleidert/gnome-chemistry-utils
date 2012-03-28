// -*- C++ -*-

/*
 * GChemPaint residues plugin
 * pseudo-atom.cc
 *
 * Copyright (C) 2007-2008 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "pseudo-atom.h"
#include <gcp/document.h>
#include <gcp/settings.h>
#include <gcp/theme.h>
#include <gcp/view.h>
#include <gcp/widgetdata.h>
#include <gcu/xml-utils.h>
#include <gccv/canvas.h>
#include <gccv/circle.h>
#include <cstring>

using namespace gcu;
using namespace gccv;

TypeId PseudoAtomType = NoType;

gcpPseudoAtom::gcpPseudoAtom (): gcp::Atom ()
{
	gcu::Atom::SetZ (0);
	Lock (true);
}

gcpPseudoAtom::gcpPseudoAtom (double x, double y): gcp::Atom ()
{
	gcu::Atom::SetZ (0);
	SetCoords (x, y);
	Lock (true);
}

gcpPseudoAtom::~gcpPseudoAtom ()
{
}

void gcpPseudoAtom::Update ()
{
}

void gcpPseudoAtom::AddItem ()
{
	if (m_Item != NULL)
		return;
	gcp::Document *doc = static_cast <gcp::Document *> (GetDocument ());
	gcp::View *view = doc->GetView ();
	gcp::WidgetData* pData = view->GetData ();
	double x, y, r;
	GetCoords (&x, &y);
	gcp::Theme *pTheme = static_cast <gcp::Document *> (GetDocument ())->GetTheme ();
	x *= pTheme->GetZoomFactor ();
	y *= pTheme->GetZoomFactor ();
	r = pTheme->GetFontSize () / PANGO_SCALE / 2;
	Circle *circle = new Circle (pData->m_View->GetCanvas ()->GetRoot (), x, y, r, this);
	circle->SetFillColor ((pData->IsSelected (this))? gcp::SelectColor: gcp::Color);
	circle->SetLineColor (0);
	m_Item = circle;
/*	gcp::WidgetData* pData = reinterpret_cast<gcp::WidgetData*> (g_object_get_data (G_OBJECT (w), "data"));
	if (pData->Items[this] != NULL)
		return;
	gcp::Theme *pTheme = pData->m_View->GetDoc ()->GetTheme ();
	GnomeCanvasGroup* group = GNOME_CANVAS_GROUP (gnome_canvas_item_new (pData->Group, gnome_canvas_group_ext_get_type(), NULL));
	double x, y, r;
	GetCoords (&x, &y);
	x *= pTheme->GetZoomFactor ();
	y *= pTheme->GetZoomFactor ();
	r = pTheme->GetFontSize () / PANGO_SCALE / 2;
	GnomeCanvasItem *item = gnome_canvas_item_new (
						group,
						gnome_canvas_ellipse_ext_get_type (),
						"x1", x - r,
						"y1", y  - r,
						"x2", x + r,
						"y2", y  + r,
						"fill_color", (pData->IsSelected (this))? gcp::SelectColor: RGBA_BLACK,
						NULL);
	g_object_set_data (G_OBJECT (group), "ellipse", item);
	g_signal_connect (G_OBJECT (item), "event", G_CALLBACK (gcp::on_event), w);
	g_object_set_data (G_OBJECT (item), "object", (void *) (this));
	pData->Items[this] = group;*/
}

void gcpPseudoAtom::UpdateItem ()
{
	if (!m_Item) {
		AddItem ();
		return;
	}
	gcp::WidgetData* pData = static_cast <gcp::Document *> (GetDocument ())->GetView ()->GetData ();
	gcp::Theme *pTheme = pData->m_View->GetDoc ()->GetTheme ();
	double x, y, r;
	GetCoords (&x, &y);
	x *= pTheme->GetZoomFactor ();
	y *= pTheme->GetZoomFactor ();
	r = pTheme->GetFontSize () / PANGO_SCALE / 2;
	Circle *circle = static_cast <Circle *> (m_Item);
	circle->SetPosition (x, y);
	circle->SetRadius (r);
}

xmlNodePtr gcpPseudoAtom::Save (xmlDocPtr xml) const
{
	xmlNodePtr parent;
	parent = xmlNewDocNode (xml, NULL, (xmlChar*) "pseudo-atom", NULL);
	if (!parent)
		return NULL;
	SaveId (parent);

	if (!WritePosition (xml, parent, NULL, m_x, m_y, m_z)) {
		xmlFreeNode (parent);
		return NULL;
	}

	return parent;
}

bool gcpPseudoAtom::Load (xmlNodePtr node)
{
	char* tmp;
	tmp = (char*) xmlGetProp (node, (xmlChar*) "id");
	if (tmp) {
		SetId (tmp);
		xmlFree (tmp);
		if (strcmp (GetId(), "a1"))
			return false;
	}
	if (!ReadPosition (node, NULL, &m_x, &m_y, &m_z))
		return false;
	GetDocument ()->ObjectLoaded (this);
	return true;
}

bool gcpPseudoAtom::LoadNode (xmlNodePtr)
{
	return true;
}

void gcpPseudoAtom::SetSelected (int state)
{
	if (!m_Item)
		return;
	static_cast <FillItem *> (m_Item)->SetFillColor ((state == gcp::SelStateSelected)? gcp::SelectColor: gcp::Color);
/*	gcp::WidgetData* pData = reinterpret_cast<gcp::WidgetData*> (g_object_get_data (G_OBJECT (w), "data"));
	GnomeCanvasGroup* group = pData->Items[this];
	g_object_set (G_OBJECT(g_object_get_data(G_OBJECT(group), "ellipse")),
				"fill_color", ((state == gcp::SelStateSelected)? gcp::SelectColor: RGBA_BLACK), NULL);*/
}
