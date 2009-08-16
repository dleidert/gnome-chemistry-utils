// -*- C++ -*-

/* 
 * GChemPaint arrows plugin
 * curvedarrowtool.cc 
 *
 * Copyright (C) 2004-2009 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "curvedarrowtool.h"
#include <gccv/bezier-arrow.h>
#include <gcp/application.h>
#include <gcp/atom.h>
#include <gcp/document.h>
#include <gcp/electron.h>
#include <gcp/settings.h>
#include <gcp/theme.h>
#include <gcp/view.h>
#include <gtk/gtk.h>

using namespace std;

gcpCurvedArrowTool::gcpCurvedArrowTool (gcp::Application *App, string Id): gcp::Tool (App, Id)
{
	m_Full = Id == "CurvedArrow";
}

gcpCurvedArrowTool::~gcpCurvedArrowTool ()
{
}

bool gcpCurvedArrowTool::OnClicked ()
{
	bool allowed = false;
	double x0 = 0., y0 = 0., x1 = 0., y1 = 0., x2 = 0., y2 = 0., x3 = 0., y3 = 0.;
	gcp::Document *pDoc = m_pView->GetDoc ();
	gcp::Theme *pTheme = pDoc->GetTheme ();
	gccv::ArrowHeads arrow_head = m_Full? gccv::ArrowHeadFull: gccv::ArrowHeadLeft;
	if (m_pObject)
		switch (m_pObject->GetType ()) {
		case gcu::AtomType:
			allowed = reinterpret_cast <gcp::Atom *> (m_pObject)->HasAvailableElectrons (m_Full);
			break;
		case gcu::BondType:
			allowed = true;
			break;
		default:
			if (m_pObject->GetType () == gcp::ElectronType) {
				if (m_Full)
					allowed = static_cast <gcp::Electron *> (m_pObject)->IsPair ();
				else
					allowed = true;
			}
			break;
		}
	if (allowed) {
		gccv::BezierArrow *arrow = new gccv::BezierArrow (m_pView->GetCanvas ());
		arrow->SetControlPoints (x0, y0, x1, y1, x2, y2, x3, y3);
		arrow->SetShowControls (true);
		arrow->SetLineWidth (pTheme->GetArrowWidth ());
		arrow->SetLineColor (gcp::AddColor);
		arrow->SetHead (arrow_head);
		m_Item = arrow;
	}
	return allowed;
}

void gcpCurvedArrowTool::OnDrag ()
{
}

void gcpCurvedArrowTool::OnMotion ()
{
	bool allowed = false;
	if (m_pObject)
		switch (m_pObject->GetType ()) {
		case gcu::AtomType:
			allowed = reinterpret_cast <gcp::Atom *> (m_pObject)->HasAvailableElectrons (m_Full);
			break;
		case gcu::BondType:
			allowed = true;
			break;
		default:
			if (m_pObject->GetType () == gcp::ElectronType) {
				if (m_Full)
					allowed = static_cast <gcp::Electron *> (m_pObject)->IsPair ();
				else
					allowed = true;
			}
			break;
		}
	gdk_window_set_cursor (gtk_widget_get_parent_window (m_pWidget), allowed? m_pApp->GetCursor (gcp::CursorPencil): m_pApp->GetCursor (gcp::CursorUnallowed));
}

void gcpCurvedArrowTool::OnRelease ()
{
}
