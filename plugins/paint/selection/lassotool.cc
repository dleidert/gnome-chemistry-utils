// -*- C++ -*-

/* 
 * GChemPaint selection plugin
 * lassotool.cc
 *
 * Copyright (C) 2010 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "lassotool.h"
#include <gccv/canvas.h>
#include <gccv/group.h>
#include <gccv/item-client.h>
#include <gccv/polygon.h>
#include <gcp/atom.h>
#include <gcp/fragment.h>
#include <gcp/settings.h>
#include <gcp/view.h>
#include <gcp/widgetdata.h>
#include <gcu/bond.h>

gcpLassoTool::gcpLassoTool (gcp::Application *App): gcp::Tool (App, "Lasso")
{
}

gcpLassoTool::~gcpLassoTool ()
{
}

bool gcpLassoTool::OnClicked ()
{
	std::list <gccv::Point> l;
	gccv::Point p;
	gccv::Polygon *poly;
	p.x = m_x0;
	p.y = m_y0;
	l.push_front (p);
	m_Item = poly = new gccv::Polygon (m_pView->GetCanvas (), l);
	poly->SetLineColor (gcp::SelectColor);
	return true;
}

void gcpLassoTool::OnDrag ()
{
	static_cast <gccv::Polygon *> (m_Item)->AddPoint (m_x, m_y);
	// Unselect everything before evaluating current selection
	m_pData->UnselectAll ();
	cairo_t *cr;
	cairo_surface_t *surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 1 , 1);
	cr = cairo_create (surface);
	m_Item->BuildPath (cr);
	std::list <gccv::Item *>::iterator it;
	gccv::Group *group = m_pView->GetCanvas ()->GetRoot ();
	gccv::Item *item = group->GetFirstChild (it);
	double x0, x1, y0, y1;
	gcu::Object *object;
	m_Item->GetBounds (m_x0, m_y0, m_x, m_y);
	while (item) {
		if (item != m_Item) {
			item->GetBounds (x0, y0, x1, y1);
			if ((x0 < m_x) && (y0 < m_y) && (x1 > m_x0) && (y1 > m_y0)) {
				object = dynamic_cast <gcu::Object *> (item->GetClient ());
				if (object && object->GetCoords (&x0, &y0) && !m_pData->IsSelected (object)) {
					x0 *= m_dZoomFactor;
					y0 *= m_dZoomFactor;
					if (cairo_in_fill (cr, x0, y0)) {
						m_pData->SetSelected (object);
						gcp::Atom *atom = static_cast <gcp::Atom *> (object);
						switch (object->GetType ()) {
						case gcu::FragmentType:
								atom = static_cast <gcp::Fragment *> (object)->GetAtom ();
						case gcu::AtomType: {
							// go through the bonds and select them if both ends are selected
							std::map<gcu::Atom*, gcu::Bond*>::iterator i;
							gcu::Bond *bond = atom->GetFirstBond (i);
							while (bond) {
								if (m_pData->IsSelected (bond->GetAtom (atom)))
									m_pData->SetSelected (bond);
								bond = atom->GetNextBond (i);
							}
						}
						default:
							// go through the links and store them for later treatment
							break;
						}
					}
				}
			}
		}
		item = group->GetNextChild (it);
	}
	cairo_destroy (cr);
	cairo_surface_destroy (surface);
}

void gcpLassoTool::OnRelease ()
{
}

void gcpLassoTool::Activate ()
{
}

bool gcpLassoTool::Deactivate ()
{
	return true;
}
