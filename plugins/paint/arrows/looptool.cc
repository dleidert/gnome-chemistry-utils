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
#include <gcp/application.h>
#include <gcp/mechanism-step.h>
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
//	gcp::Document *doc = m_pView->GetDoc ();
//	gcp::Theme *theme = doc->GetTheme ();
	return false;
}

void gcpLoopTool::OnDrag ()
{
	gccv::Item *item = m_pView->GetCanvas ()->GetItemAt (m_x, m_y);
	if (item == NULL)
		return;
}

void gcpLoopTool::OnRelease ()
{
	m_pData->UnselectAll ();
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
