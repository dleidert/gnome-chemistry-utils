// -*- C++ -*-

/*
 * GChemPaint arrows plugin
 * retrosynthesisarrow.cc
 *
 * Copyright (C) 2004-2010 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "retrosynthesisarrow.h"
#include "retrosynthesis.h"
#include "retrosynthesisstep.h"
#include <gcp/document.h>
#include <gcp/settings.h>
#include <gcp/theme.h>
#include <gcp/view.h>
#include <gcp/widgetdata.h>
#include <gccv/canvas.h>
#include <gccv/group.h>
#include <gccv/line.h>
#include <gccv/poly-line.h>
#include <glib/gi18n.h>
#include <cmath>

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

xmlNodePtr gcpRetrosynthesisArrow::Save (xmlDocPtr xml) const
{
	xmlNodePtr node;
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
	return node;
}

bool gcpRetrosynthesisArrow::Load (xmlNodePtr node)
{
	char *buf;
	Object *parent;
	gcu::Document *doc = GetDocument ();
	if (gcp::Arrow::Load (node)) {
		parent = GetParent ();
		if (!parent)
			return true;
		buf = (char*) xmlGetProp (node, (xmlChar*) "start");
		if (buf) {
			doc->SetTarget (buf, reinterpret_cast <Object **> (&m_Start), GetParent (), this, ActionIgnore);
			xmlFree (buf);
		}
		buf = (char*) xmlGetProp (node, (xmlChar*) "end");
		if (buf) {
			doc->SetTarget (buf, reinterpret_cast <Object **> (&m_End), GetParent (), this, ActionIgnore);
			xmlFree (buf);
		}
		if (m_Start)
			m_Start->AddArrow (this, m_End, true);
		doc->ObjectLoaded (this);
		return true;
	}
	return false;
}

void gcpRetrosynthesisArrow::OnLoaded ()
{
	if (m_Start)
		m_Start->AddArrow (this, m_End, false);
	if (m_End)
		m_End->AddArrow (this, m_Start, true);
}

void gcpRetrosynthesisArrow::AddItem ()
{
	if (m_Item)
		return;
	gcp::Document *doc = static_cast <gcp::Document*> (GetDocument ());
	gcp::View *view = doc->GetView ();
	gcp::Theme *theme = doc->GetTheme ();
	double x0, y0, x1, y1, dx, dy, dAngle;
	x0 = m_x * theme->GetZoomFactor ();
	y0 = m_y * theme->GetZoomFactor ();
	x1 = (m_x + m_width) * theme->GetZoomFactor ();
	y1 = (m_y + m_height) * theme->GetZoomFactor ();
	if (m_width == 0.) {
		if (m_height == 0.)
			return;
		dAngle = (m_height < 0.) ? M_PI / 2 : 1.5 * M_PI;
	} else {
		dAngle = atan(- m_height / m_width);
		if (m_width < 0)
			dAngle += M_PI;
	}
	dx = theme->GetArrowDist () / 2 * sin (dAngle);
	dy = theme->GetArrowDist () / 2 * cos (dAngle);
	GOColor color = (view->GetData ()->IsSelected (this))? gcp::SelectColor: gcp::Color;
	gccv::Group *group = new gccv::Group (view->GetCanvas ()->GetRoot (), this);
	gccv::Line *line = new gccv::Line (group, x0 - dx, y0 - dy,
									  x1- dx - dy, y1 - dy + dx,
									   this);
	line->SetLineColor (color);
	line->SetLineWidth (theme->GetArrowWidth ());
	line = new gccv::Line (group, x0 + dx, y0 + dy,
						   x1 + dx - dy, y1 + dy + dx,
						   this);
	line->SetLineColor (color);
	line->SetLineWidth (theme->GetArrowWidth ());
	dx += theme->GetArrowHeadC () * sin (dAngle);
	dy += theme->GetArrowHeadC () * cos (dAngle);
	list <gccv::Point> points;
	gccv::Point point;
	point.x = x1 - dx - dy;
	point.y = y1 - dy + dx;
	points.push_back (point);
	point.x = x1;
	point.y = y1;
	points.push_back (point);
	point.x = x1 + dx - dy;
	point.y = y1 + dy + dx;
	points.push_back (point);
	gccv::PolyLine *pl = new gccv::PolyLine (group, points, this);
	pl->SetLineColor (color);
	pl->SetLineWidth (theme->GetArrowWidth ());
	m_Item = group;
}

void gcpRetrosynthesisArrow::SetSelected (int state)
{
	if (!m_Item)
		return;
	GOColor color;
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
	gccv::Group *group = static_cast <gccv::Group *> (m_Item);
	std::list<gccv::Item *>::iterator it;
	gccv::LineItem *item = static_cast <gccv::LineItem *> (group->GetFirstChild (it));
	while (item) {
		item->SetLineColor (color);
		item = static_cast <gccv::LineItem *> (group->GetNextChild (it));
	}
}

std::string gcpRetrosynthesisArrow::Name ()
{
	return _("Retrosynthesis arrow");
}
