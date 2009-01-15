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
#include <gcp/application.h>
#include <gcp/electron.h>
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
	return false;
}

void gcpCurvedArrowTool::OnDrag ()
{
}

void gcpCurvedArrowTool::OnMotion ()
{
	bool allowed = false;
	if (m_pObject)
		switch (m_pObject->GetType ()) {
		case gcu::AtomType: {
			break;
		}
		case gcu::BondType: {
			allowed = true;
			break;
		}
		default:
			if (m_pObject->GetType () == gcp::ElectronType)
					;
			break;
		}
	gdk_window_set_cursor (gtk_widget_get_parent_window (m_pWidget), allowed? m_pApp->GetCursor (gcp::CursorPencil): m_pApp->GetCursor (gcp::CursorUnallowed));
}

void gcpCurvedArrowTool::OnRelease ()
{
}
