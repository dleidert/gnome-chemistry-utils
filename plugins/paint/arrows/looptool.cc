// -*- C++ -*-

/*
 * GChemPaint arrows plugin
 * loopwtool.cc
 *
 * Copyright (C) 2014 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "looptool.h"
#include <gccv/canvas.h>
#include <gccv/arc.h>
#include <gccv/group.h>
#include <gccv/item-client.h>
#include <gcp/application.h>
#include <gcp/document.h>
#include <gcp/mechanism-step.h>
#include <gcp/settings.h>
#include <gcp/theme.h>
#include <gcp/view.h>
#include <gcp/widgetdata.h>

gcpLoopTool::gcpLoopTool (gcp::Application* App):
	gcp::Tool (App, "TolmanLoop"),
	clockwise (true)
{
}

gcpLoopTool::~gcpLoopTool ()
{
}

bool gcpLoopTool::OnClicked ()
{
	if (m_pObject == NULL)
		return false;
	if (m_pObject->GetType () == gcu::TextType && m_pObject->GetGroup () == NULL);
	else if (m_pObject->GetGroup ()->GetType () == gcp::MechanismStepType)
		m_pObject = m_pObject->GetGroup ();
	else {
		gcu::Object *mol = m_pObject->GetMolecule ();
		if (!mol || mol->GetGroup () != NULL)
			return false;
		m_pObject = mol;
	}
	m_pData->SetSelected (m_pObject);
	gccv::Rect rect;
	double x, y, radius, start, end;
	gcp::Document *doc = m_pView->GetDoc ();
	gcp::Theme *theme = doc->GetTheme ();
	m_pData->GetObjectBounds (m_pObject, rect);
	// use twice the object diagonal as radius
	radius = sqrt ((rect.x1 - rect.x0) * (rect.x1 - rect.x0) + (rect.y1 - rect.y0) * (rect.y1 - rect.y0));
	// put the initial arrow below the first object
	x = (rect.x0 + rect.x1) / 2.;
	y = (rect.y0 + rect.y1) / 2. + radius;
	start = atan2 (radius, (rect.x1 - rect.x0) / 2 + theme->GetArrowPadding () / theme->GetZoomFactor ());
	end = 3. * M_PI / 2. - start;
	start -= M_PI / 2.;
	gccv::Arc *arc = new gccv::Arc (m_pView->GetCanvas (), x, y, radius, start, end);
	arc->SetLineColor (gcp::AddColor);
	arc->SetLineWidth (theme->GetArrowWidth ());
	arc->SetA (theme->GetArrowHeadA ());
	arc->SetB (theme->GetArrowHeadB ());
	arc->SetC (theme->GetArrowHeadC ());
	arc->SetHead (gccv::ArrowHeadFull);
	steps[m_pObject] = rect;
	m_Item = arc;
	return true;
}

void gcpLoopTool::OnDrag ()
{
	gccv::Item *item = m_pView->GetCanvas ()->GetItemAt (m_x, m_y);
	if (item == NULL)
		return;
	gcu::Object *obj = dynamic_cast <gcu::Object *> (item->GetClient ());
	if (obj->GetType () == gcu::TextType && obj->GetGroup () == NULL);
	else if (obj->GetGroup ()->GetType () == gcp::MechanismStepType)
		obj = obj->GetGroup ();
	else {
		gcu::Object *mol = obj->GetMolecule ();
		if (!mol || mol->GetGroup () != NULL)
			return;
		obj = mol;
	}
	if (steps.find (obj) != steps.end())
		return;
	m_pData->SetSelected (obj);
	delete m_Item; //deleting the old item, we need to rebuild it
	// find objects center
	gccv::Rect rect;
	m_pData->GetObjectBounds (obj, rect);
	steps[obj] = rect;
	std::map < gcu::Object *, gccv::Rect >::iterator i, iend = steps.end();
	double x = 0., y = 0., minx = G_MAXDOUBLE, miny = G_MAXDOUBLE, curx, cury, r, minr;
	size_t n = steps.size ();
	for (i = steps.begin (); i != iend; i++) {
		rect = (*i).second;
		x += curx = (rect.x0 + rect.x1) / 2.;
		y += cury = (rect.x0 + rect.y1) / 2.;
		r = sqrt ((rect.x1 - rect.x0) * (rect.x1 - rect.x0) + (rect.y1 - rect.y0) * (rect.y1 - rect.y0));
		if (minr < r)
			minr = r;
		if (minx > curx)
			minx = curx;
		if (miny > cury)
			miny = cury;
	}
	x /= n;
	y /= n;
	r = MAX (x - minx, y - miny);
	if (r < minr)
		r = minr;
	// class the objects  according to their positions around the center
	std::map < double, gcu::Object *> ordered;
	double a;
	for (i = steps.begin (); i != iend; i++) {
		rect = (*i).second;
		curx = (rect.x0 + rect.x1) / 2.;
		cury = (rect.x0 + rect.y1) / 2.;
		a = atan2 (cury - y, curx - x);
		while (ordered .find(a) !=  ordered.end ())
			a += 1e-10; // arbitrary value, just to avoid duplicates
		ordered[a] = (*i).first;
	}
	std::map < double, gcu::Object *>::iterator j, jend = ordered.end ();
	// now add the arrows
	gccv::Group *group = new gccv::Group (m_pView->GetCanvas ());
	j = ordered.begin ();
	obj = (*j).second;
	a = (*j).first;
	for (j++; j != jend; j++) {
		
		obj = (*j).second;
		a = (*j).first;
	}
	obj = (*ordered.begin ()).second;

	m_Item = group;
}

void gcpLoopTool::OnRelease ()
{
	m_pData->UnselectAll ();
	if (m_Item)
		delete m_Item;
	m_Item = NULL;
}

void gcpLoopTool::OnMotion ()
{
	m_pData->UnselectAll ();
	bool allowed = false;
	if (m_pObject) {
		if (m_pObject->GetType () == gcu::TextType && m_pObject->GetGroup () == NULL)
		    allowed = true;
		else if (m_pObject->GetGroup ()->GetType () == gcp::MechanismStepType) {
			m_pObject = m_pObject->GetGroup ();
			allowed = true;
		} else {
			gcu::Object *mol = m_pObject->GetMolecule ();
			if (mol || mol->GetGroup () == NULL) {
				allowed = true;
				m_pObject = mol;
			}
		}
	}
	if (allowed)
		m_pData->SetSelected (m_pObject);
	gdk_window_set_cursor (gtk_widget_get_parent_window (m_pWidget), allowed? NULL: m_pApp->GetCursor (gcp::CursorUnallowed));
}
