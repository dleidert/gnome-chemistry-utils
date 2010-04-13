// -*- C++ -*-

/* 
 * GChemPaint library
 * tool.cc
 *
 * Copyright (C) 2001-2010 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "tool.h"
#include "application.h"
#include "document.h"
#include "view.h"
#include "widgetdata.h"
#include <gccv/item.h>

using namespace gcu;
using namespace std;

namespace gcp {

Tool* pActiveTool = NULL;

Tool::Tool (Application *App, string Id)
{
	name = Id;
	m_pApp = App;
	App->SetTool (Id, this);
	m_pObject = NULL;
	m_Item = NULL;
	m_bChanged = m_bPressed = false;
	m_pData = NULL;
	m_OwnStatus = false;
}

Tool::~Tool ()
{
	m_pApp->SetTool (name, NULL);
}

bool Tool::OnClicked (View* pView, Object* pObject, double x, double y, unsigned int state)
{
	m_x0 = m_x1 = lastx = x;
	m_y0 = m_y1 = lasty = y;
	m_nState = state;
	m_bPressed = true;
	m_pObject = pObject;
	if (pObject)
		m_pObjectGroup = pObject->GetGroup ();
	m_pView = pView;
	m_pWidget = m_pView->GetWidget ();
	m_pData = (WidgetData*) g_object_get_data (G_OBJECT(m_pWidget), "data");
	m_dZoomFactor = m_pView->GetZoomFactor ();
	m_bAllowed = true;
	return OnClicked ();
}

void Tool::OnDrag (double x, double y, unsigned int state)
{
	m_x = lastx = x;
	m_y = lasty = y;
	m_nState = state;
	OnDrag ();
}

void Tool::OnMotion (View* pView, Object* pObject, double x, double y, unsigned int state)
{
	m_x = lastx = x;
	m_y = lasty = y;
	m_nState = state;
	m_pObject = pObject;
	if (pObject)
		m_pObjectGroup = pObject->GetGroup ();
	m_pView = pView;
	m_pWidget = m_pView->GetWidget ();
	m_pData = (WidgetData*) g_object_get_data (G_OBJECT(m_pWidget), "data");
	m_dZoomFactor = m_pView->GetZoomFactor ();
	OnMotion ();
}

void Tool::OnLeaveNotify (View* pView, G_GNUC_UNUSED unsigned int state)
{
	m_pView = pView;
	m_pData = pView->GetData ();
	OnLeaveNotify ();
}

void Tool::OnRelease (double x, double y, unsigned int state)
{
	m_x = lastx = x;
	m_y = lasty = y;
	m_nState = state;
	m_bPressed = false;
	OnRelease ();
	m_pView->GetDoc ()->FinishOperation ();
	if (m_Item) {
		delete m_Item;
		m_Item = NULL;
	}
	m_pObject = NULL;
	m_bChanged = false;
	m_pView->EnsureSize ();
}

bool Tool::OnRightButtonClicked (View* pView, Object* pObject, double x, double y, GtkUIManager *UIManager)
{
	m_pObject = pObject;
	m_pView = pView;
	m_pWidget = m_pView->GetWidget ();
	m_pData = (WidgetData*) g_object_get_data (G_OBJECT (m_pWidget), "data");
	m_dZoomFactor = m_pView->GetZoomFactor();
	m_x = x;
	m_y = y;
	bool res = OnRightButtonClicked (UIManager);
	m_pObject = NULL;
	return res;
}

bool Tool::Activate (bool bState)
{
	if (bState) {
		m_pObject = NULL;
		m_pWidget = NULL;
		m_pView = NULL;
		Activate ();
		return true;
	} else {
		if (Deactivate ()) {
			m_pObject = NULL;
			m_pWidget = NULL;
			m_pView = NULL;
			return true;
		}
		return false;
	}
}

bool Tool::OnClicked ()
{
	return false;
}

void Tool::OnDrag ()
{
}

void Tool::OnMotion ()
{
	gdk_window_set_cursor (gtk_widget_get_parent_window (m_pWidget), NULL);
}

void Tool::OnLeaveNotify ()
{
}

void Tool::OnRelease ()
{
}

bool Tool::OnRightButtonClicked (G_GNUC_UNUSED GtkUIManager *UIManager)
{
	return false;
}

void Tool::OnChangeState ()
{
	if (m_bPressed) {
		m_x = lastx;
		m_y = lasty;
		OnDrag ();
	}
}

void Tool::Activate ()
{
}

bool Tool::Deactivate ()
{
	return true;
}

bool Tool::OnKeyPress (G_GNUC_UNUSED GdkEventKey *event)
{
	return false;
}

bool Tool::OnKeyRelease (G_GNUC_UNUSED GdkEventKey *event)
{
	return false;
}

bool Tool::NotifyViewChange ()
{
	return true;
}

bool Tool::DeleteSelection ()
{
	return false;
}

bool Tool::CopySelection (G_GNUC_UNUSED GtkClipboard *clipboard)
{
	return true; // makes copying impossible
}

bool Tool::CutSelection (G_GNUC_UNUSED GtkClipboard *clipboard)
{
	return false;
}

bool Tool::PasteSelection (G_GNUC_UNUSED GtkClipboard *clipboard)
{
	return false;
}

bool Tool::OnReceive (G_GNUC_UNUSED GtkClipboard *clipboard, G_GNUC_UNUSED GtkSelectionData *data, G_GNUC_UNUSED int type)
{
	return false;
}

bool Tool::OnUndo ()
{
	return false;
}

bool Tool::OnRedo ()
{
	return false;
}

void Tool::PushNode (G_GNUC_UNUSED xmlNodePtr node)
{
}

void Tool::AddSelection (G_GNUC_UNUSED WidgetData* data)
{
}

GtkWidget *Tool::GetPropertyPage ()
{
	return NULL;
}

}	//	namespace gcp
