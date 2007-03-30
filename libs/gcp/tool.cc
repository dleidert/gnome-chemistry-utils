// -*- C++ -*-

/* 
 * GChemPaint library
 * tool.cc
 *
 * Copyright (C) 2001-2007 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "document.h"
#include "application.h"

namespace gcp {

Tool* pActiveTool = NULL;

Tool::Tool(Application *App, string Id)
{
	name = Id;
	m_pApp = App;
	App->SetTool (Id, this);
	m_pObject = NULL;
	m_pItem = NULL;
	m_bChanged = m_bPressed = false;
	m_OptionDialog = NULL;
	m_pData = NULL;
}

Tool::~Tool()
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
	m_pGroup = gnome_canvas_root (GNOME_CANVAS (m_pWidget));
	m_pBackground = m_pView->GetBackground ();
	m_dZoomFactor = m_pView->GetZoomFactor ();
	Object* pAtom;
	if (m_pObject && ((pAtom = m_pObject->GetAtomAt (m_x0 / m_dZoomFactor, m_y0 / m_dZoomFactor))))
			m_pObject = pAtom;
	m_bAllowed = true;
	return OnClicked ();
}

void Tool::OnDrag(double x, double y, unsigned int state)
{
	m_x = lastx = x;
	m_y = lasty = y;
	m_nState = state;
	OnDrag();
}

void Tool::OnRelease(double x, double y, unsigned int state)
{
	m_x = lastx = x;
	m_y = lasty = y;
	m_nState = state;
	m_bPressed = false;
	OnRelease();
	if (m_pItem)
	{
		gtk_object_destroy(GTK_OBJECT(GNOME_CANVAS_ITEM(m_pItem)));
		m_pItem = NULL;
	}
	m_pView->GetDoc()->FinishOperation();
	m_pObject = NULL;
	m_bChanged = false;
	g_signal_emit_by_name(m_pWidget, "update_bounds");
}

bool Tool::OnRightButtonClicked (View* pView, Object* pObject, double x, double y, GtkUIManager *UIManager)
{
	m_pObject = pObject;
	m_pView = pView;
	m_pWidget = m_pView->GetWidget();
	m_pData = (WidgetData*)g_object_get_data(G_OBJECT(m_pWidget), "data");
	m_pGroup = gnome_canvas_root(GNOME_CANVAS(m_pWidget));
	m_pBackground = m_pView->GetBackground();
	m_dZoomFactor = m_pView->GetZoomFactor();
	m_x = x;
	m_y = y;
	bool res = OnRightButtonClicked (UIManager);
	m_pObject = NULL;
	return res;
}

bool Tool::Activate(bool bState)
{
	if (bState)
	{
		m_pGroup = NULL;
		m_pBackground = NULL;
		m_pObject = NULL;
		m_pWidget = NULL;
		m_pView = NULL;
		Activate();
		return true;
	}
	else
	{
		if (Deactivate())
		{
			m_pGroup = NULL;
			m_pBackground = NULL;
			m_pObject = NULL;
			m_pWidget = NULL;
			m_pView = NULL;
			return true;
		}
		return false;
	}
}

bool Tool::OnClicked()
{
	return false;
}

void Tool::OnDrag()
{
}

void Tool::OnRelease()
{
}

bool Tool::OnRightButtonClicked(GtkUIManager *UIManager)
{
	return false;
}

void Tool::OnChangeState()
{
	if (m_bPressed)
	{
		m_x = lastx;
		m_y = lasty;
		OnDrag();
	}
}

void Tool::Activate()
{
}

bool Tool::Deactivate()
{
	return true;
}

bool Tool::OnEvent(GdkEvent* event)
{
	return false;
}

bool Tool::NotifyViewChange()
{
	return true;
}

bool Tool::DeleteSelection()
{
	return false;
}

bool Tool::CopySelection(GtkClipboard *clipboard)
{
	return false;
}

bool Tool::CutSelection(GtkClipboard *clipboard)
{
	return false;
}

bool Tool::PasteSelection(GtkClipboard *clipboard)
{
	return false;
}

bool Tool::OnReceive(GtkClipboard *clipboard, GtkSelectionData *data, int type)
{
	return false;
}

bool Tool::OnUndo()
{
	return false;
}

bool Tool::OnRedo()
{
	return false;
}

void Tool::PushNode(xmlNodePtr node)
{
}

void Tool::AddSelection(WidgetData* data)
{
}

GtkWidget *Tool::GetPropertyPage ()
{
	return NULL;
}

}	//	namespace gcp
