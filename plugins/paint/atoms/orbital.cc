// -*- C++ -*-

/* 
 * GChemPaint atoms plugin
 * orbital.cc 
 *
 * Copyright (C) 2009-2010 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "orbital.h"
#include <gcp/atom.h>
#include <gcp/document.h>
#include <gcp/settings.h>
#include <gcp/theme.h>
#include <gcp/view.h>
#include <gcp/widgetdata.h>
#include <gccv/canvas.h>
#include <gccv/circle.h>
#include <gccv/group.h>
#include <gccv/leaf.h>
#include <glib/gi18n.h>
#include <cstring>

gcu::TypeId OrbitalType;

gcpOrbital::gcpOrbital (gcp::Atom *parent, gcpOrbitalType type):
	gcu::Object (OrbitalType),
	gccv::ItemClient (),
	m_Atom (parent),
	m_Type (type),
	m_Coef (1.),
	m_Rotation (0.)
{
	SetId ("orb1");
	if (parent)
		parent->AddChild (this);
}

gcpOrbital::~gcpOrbital ()
{
}

void gcpOrbital::AddItem ()
{
	if (!m_Atom || m_Item)
		return;
	gcp::Document *doc = static_cast <gcp::Document *> (GetDocument ());
	gcp::Theme *theme = doc->GetTheme ();
	gcp::View *view = doc->GetView ();
	double zoom = theme->GetZoomFactor ();
	gccv::Group *group = static_cast <gccv::Group *> (m_Atom->GetItem ());
	switch (m_Type) {
	case GCP_ORBITAL_TYPE_S: {
		gccv::Circle *circle = new gccv::Circle (group, 0., 0., theme->GetBondLength () * fabs (m_Coef) * zoom / 2., this);
		circle->SetLineWidth (1.);
		circle->SetLineColor ((view->GetData ()->IsSelected (this))? gcp::SelectColor: gcp::Color);
		circle->SetFillColor (m_Coef > 0.? GO_COLOR_GREY (100): GO_COLOR_WHITE);
		m_Item = circle;
		group->MoveToBack (m_Item);
		break;
	}
	case GCP_ORBITAL_TYPE_P: {
		gccv::Group *new_group = new gccv::Group (group, this);
		gccv::Leaf *leaf = new gccv::Leaf (new_group, 0., 0., theme->GetBondLength () * m_Coef * zoom, this);
		leaf->SetRotation (m_Rotation / 180. * M_PI);
		leaf->SetWidthFactor (GCP_ORBITAL_P_WIDTH);
		leaf->SetLineWidth (1.);
		leaf->SetLineColor ((view->GetData ()->IsSelected (this))? gcp::SelectColor: gcp::Color);
		leaf->SetFillColor (GO_COLOR_GREY (100));
		leaf = new gccv::Leaf (new_group, 0., 0., theme->GetBondLength () * m_Coef * zoom, this);
		leaf->SetWidthFactor (GCP_ORBITAL_P_WIDTH);
		leaf->SetRotation (m_Rotation / 180. * M_PI + M_PI);
		leaf->SetLineWidth (1.);
		leaf->SetLineColor ((view->GetData ()->IsSelected (this))? gcp::SelectColor: gcp::Color);
		leaf->SetFillColor (GO_COLOR_WHITE);
		m_Item = new_group;
		group->MoveToBack (m_Item);
		break;
	}
	case GCP_ORBITAL_TYPE_DXY: {
		gccv::Group *new_group = new gccv::Group (group, this);
		gccv::Leaf *leaf = new gccv::Leaf (new_group, 0., 0., theme->GetBondLength () * m_Coef * zoom, this);
		leaf->SetRotation ((m_Rotation / 180. + .25) * M_PI);
		leaf->SetWidthFactor (GCP_ORBITAL_D_WIDTH);
		leaf->SetLineWidth (1.);
		leaf->SetLineColor ((view->GetData ()->IsSelected (this))? gcp::SelectColor: gcp::Color);
		leaf->SetFillColor (m_Coef > 0.? GO_COLOR_GREY (100): GO_COLOR_WHITE);
		leaf = new gccv::Leaf (new_group, 0., 0., theme->GetBondLength () * m_Coef * zoom, this);
		leaf->SetWidthFactor (GCP_ORBITAL_D_WIDTH);
		leaf->SetRotation ((m_Rotation / 180. + 1.25) * M_PI);
		leaf->SetLineWidth (1.);
		leaf->SetLineColor ((view->GetData ()->IsSelected (this))? gcp::SelectColor: gcp::Color);
		leaf->SetFillColor (m_Coef > 0.? GO_COLOR_GREY (100): GO_COLOR_WHITE);
		leaf = new gccv::Leaf (new_group, 0., 0., theme->GetBondLength () * m_Coef * zoom, this);
		leaf->SetWidthFactor (GCP_ORBITAL_D_WIDTH);
		leaf->SetRotation ((m_Rotation / 180. + .75) * M_PI);
		leaf->SetLineWidth (1.);
		leaf->SetLineColor ((view->GetData ()->IsSelected (this))? gcp::SelectColor: gcp::Color);
		leaf->SetFillColor (m_Coef > 0.? GO_COLOR_WHITE: GO_COLOR_GREY (100));
		leaf = new gccv::Leaf (new_group, 0., 0., theme->GetBondLength () * m_Coef * zoom, this);
		leaf->SetWidthFactor (GCP_ORBITAL_D_WIDTH);
		leaf->SetRotation ((m_Rotation / 180. + 1.75) * M_PI);
		leaf->SetLineWidth (1.);
		leaf->SetLineColor ((view->GetData ()->IsSelected (this))? gcp::SelectColor: gcp::Color);
		leaf->SetFillColor (m_Coef > 0.? GO_COLOR_WHITE: GO_COLOR_GREY (100));
		m_Item = new_group;
		group->MoveToBack (m_Item);
		break;
	}
	case GCP_ORBITAL_TYPE_DZ2: {
		gccv::Group *new_group = new gccv::Group (group, this);
		gccv::Leaf *leaf = new gccv::Leaf (new_group, 0., 0., theme->GetBondLength () * m_Coef * zoom, this);
		leaf->SetRotation (m_Rotation / 180. * M_PI);
		leaf->SetWidthFactor (GCP_ORBITAL_D_WIDTH);
		leaf->SetLineWidth (1.);
		leaf->SetLineColor ((view->GetData ()->IsSelected (this))? gcp::SelectColor: gcp::Color);
		leaf->SetFillColor (m_Coef > 0.? GO_COLOR_GREY (100): GO_COLOR_WHITE);
		leaf = new gccv::Leaf (new_group, 0., 0., theme->GetBondLength () * m_Coef * zoom, this);
		leaf->SetWidthFactor (GCP_ORBITAL_D_WIDTH);
		leaf->SetRotation ((m_Rotation / 180. + 1.) * M_PI);
		leaf->SetLineWidth (1.);
		leaf->SetLineColor ((view->GetData ()->IsSelected (this))? gcp::SelectColor: gcp::Color);
		leaf->SetFillColor (m_Coef > 0.? GO_COLOR_GREY (100): GO_COLOR_WHITE);
		leaf = new gccv::Leaf (new_group, 0., 0., theme->GetBondLength () * m_Coef * GCP_ORBITAL_DZ2_FACTOR * zoom, this);
		leaf->SetWidthFactor (GCP_ORBITAL_D_WIDTH);
		leaf->SetRotation ((m_Rotation / 180. + .5) * M_PI);
		leaf->SetLineWidth (1.);
		leaf->SetLineColor ((view->GetData ()->IsSelected (this))? gcp::SelectColor: gcp::Color);
		leaf->SetFillColor (m_Coef > 0.? GO_COLOR_WHITE: GO_COLOR_GREY (100));
		leaf = new gccv::Leaf (new_group, 0., 0., theme->GetBondLength () * m_Coef * GCP_ORBITAL_DZ2_FACTOR * zoom, this);
		leaf->SetWidthFactor (GCP_ORBITAL_D_WIDTH);
		leaf->SetRotation ((m_Rotation / 180. + 1.5) * M_PI);
		leaf->SetLineWidth (1.);
		leaf->SetLineColor ((view->GetData ()->IsSelected (this))? gcp::SelectColor: gcp::Color);
		leaf->SetFillColor (m_Coef > 0.? GO_COLOR_WHITE: GO_COLOR_GREY (100));
		m_Item = new_group;
		group->MoveToBack (m_Item);
		break;
	}
	}
}

xmlNodePtr gcpOrbital::Save (xmlDocPtr xml) const
{
	xmlNodePtr node = xmlNewDocNode (xml, NULL, reinterpret_cast <xmlChar const *> ("orbital"), NULL);
	char *buf;
	switch (m_Type) {
	case GCP_ORBITAL_TYPE_S:
		xmlNewProp (node, reinterpret_cast <xmlChar const *> ("type"), reinterpret_cast <xmlChar const *> ("s"));
		break;
	case GCP_ORBITAL_TYPE_P:
		xmlNewProp (node, reinterpret_cast <xmlChar const *> ("type"), reinterpret_cast <xmlChar const *> ("p"));
		break;
	case GCP_ORBITAL_TYPE_DXY:
		xmlNewProp (node, reinterpret_cast <xmlChar const *> ("type"), reinterpret_cast <xmlChar const *> ("dxy"));
		break;
	case GCP_ORBITAL_TYPE_DZ2:
		xmlNewProp (node, reinterpret_cast <xmlChar const *> ("type"), reinterpret_cast <xmlChar const *> ("dz2"));
		break;
	}
	buf = g_strdup_printf("%g", m_Coef);
	xmlNewProp (node, reinterpret_cast <xmlChar const *> ("coef"), reinterpret_cast <xmlChar *> (buf));
	g_free (buf);
	if (m_Rotation != 0. && m_Type != GCP_ORBITAL_TYPE_S) {
		buf = g_strdup_printf("%g", m_Rotation);
		xmlNewProp (node, reinterpret_cast <xmlChar const *> ("rotation"), reinterpret_cast <xmlChar *> (buf));
		g_free (buf);
	}
	return node;
}

bool gcpOrbital::Load (xmlNodePtr node)
{
	m_Atom = dynamic_cast <gcp::Atom *> (GetParent ());
	char *buf = reinterpret_cast <char *> (xmlGetProp (node, reinterpret_cast <xmlChar const *> ("type")));
	if (buf) {
		if (!strcmp (buf, "s"))
			m_Type = GCP_ORBITAL_TYPE_S;
		else if (!strcmp (buf, "p"))
			m_Type = GCP_ORBITAL_TYPE_P;
		else if (!strcmp (buf, "dxy"))
			m_Type = GCP_ORBITAL_TYPE_DXY;
		else if (!strcmp (buf, "dz2"))
			m_Type = GCP_ORBITAL_TYPE_DZ2;
		xmlFree (buf);
	}
	buf = reinterpret_cast <char *> (xmlGetProp (node, reinterpret_cast <xmlChar const *> ("coef")));
	if (buf) {
		m_Coef = g_strtod (buf, NULL);
		xmlFree (buf);
	}
	buf = reinterpret_cast <char *> (xmlGetProp (node, reinterpret_cast <xmlChar const *> ("rotation")));
	if (buf) {
		m_Rotation = g_strtod (buf, NULL);
		xmlFree (buf);
	}
	return true;
}

void gcpOrbital::SetSelected (int state)
{
	GOColor color;
	
	switch (state) {	
	case gcp::SelStateUnselected:
		color = GO_COLOR_BLACK;
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
		color = GO_COLOR_BLACK;
		break;
	}
	if (m_Type == GCP_ORBITAL_TYPE_S)
		static_cast <gccv::LineItem *> (m_Item)->SetLineColor (color);
	else {
		gccv::Group *group = static_cast <gccv::Group *> (m_Item);
		std::list<gccv::Item *>::iterator it;
		for (gccv::Item *item = group->GetFirstChild (it); item; item = group->GetNextChild (it))
			static_cast <gccv::LineItem *> (item)->SetLineColor (color);
	}
}

std::string gcpOrbital::Name ()
{
	return _("Orbital");
}
