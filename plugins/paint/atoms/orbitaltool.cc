// -*- C++ -*-

/* 
 * GChemPaint atoms plugin
 * orbitaltool.cc 
 *
 * Copyright (C) 2003-2009 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "orbitaltool.h"
#include <gcp/application.h>
#include <gcp/atom.h>
#include <gcp/document.h>
#include <gcp/theme.h>
#include <gcp/view.h>
#include <gccv/circle.h>

gcpOrbitalTool::gcpOrbitalTool (gcp::Application *App):
	gcp::Tool (App, "Orbital"),
	m_Coef (1.)
{
}

gcpOrbitalTool::~gcpOrbitalTool ()
{
}

bool gcpOrbitalTool::OnClicked ()
{
	if (!m_pObject || (m_pObject->GetType () != gcu::AtomType))
		return false;
	gcp::Atom *atom = static_cast <gcp::Atom *> (m_pObject);
	atom->GetCoords (&m_x0, &m_y0);
	gcp::Document *doc = m_pView->GetDoc ();
	gcp::Theme *theme = doc->GetTheme ();
	m_x0 *= m_dZoomFactor;
	m_y0 *= m_dZoomFactor;
	gccv::Circle *circle = new gccv::Circle (m_pView->GetCanvas (), m_x0, m_y0, theme->GetBondLength () * m_Coef * m_dZoomFactor / 2.);
	circle->SetLineWidth (1.);
	circle->SetLineColor (GO_COLOR_BLACK);
	circle->SetFillColor (m_Coef > 0.? GO_COLOR_GREY (100): GO_COLOR_WHITE);
	m_Item = circle;
	return true;
}

void gcpOrbitalTool::OnDrag ()
{
	if (!m_Item)
		return;
	
}

void gcpOrbitalTool::OnRelease ()
{
}

void gcpOrbitalTool::OnMotion ()
{
	bool allowed = false;
	if (m_pObject)
		switch (m_pObject->GetType ()) {
		case gcu::AtomType:
			allowed = true;
			break;
		default:
			break;
		}
	gdk_window_set_cursor (gtk_widget_get_parent_window (m_pWidget), allowed? m_pApp->GetCursor (gcp::CursorPencil): m_pApp->GetCursor (gcp::CursorUnallowed));
}

GtkWidget *gcpOrbitalTool::GetPropertyPage ()
{
	return NULL;
}
