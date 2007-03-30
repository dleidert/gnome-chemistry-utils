// -*- C++ -*-

/* 
 * GChemPaint library
 * tool.h 
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

#ifndef GCHEMPAINT_TOOL_H
#define GCHEMPAINT_TOOL_H

#include <vector>
#include <string>
#include "widgetdata.h"
#include "view.h"
#include "operation.h"
#include <gcu/dialog.h>

using namespace std;
using namespace gcu;

class gcp::Application;

namespace gcp {

class Tool
{
public:
	Tool (gcp::Application *App, string Id);
	virtual ~Tool ();
	
	bool OnClicked (View* pView, Object* pObject, double x, double y, unsigned int state);
	void OnDrag (double x, double y, unsigned int state);
	void OnRelease (double x, double y, unsigned int state);
	bool OnRightButtonClicked (View* pView, Object* pObject, double x, double y, GtkUIManager *UIManager);
	bool Activate (bool bState);
	string& GetName () {return name;}
	virtual bool OnClicked ();
	virtual void OnDrag ();
	virtual void OnRelease ();
	virtual bool OnRightButtonClicked (GtkUIManager *UIManager);
	virtual void Activate ();
	virtual bool Deactivate ();
	virtual void OnChangeState ();
	void OnKeyPressed (unsigned int code) {m_nState |= code; OnChangeState ();}
	void OnKeyReleased (unsigned int code) {if (m_nState & code) m_nState -= code; OnChangeState ();}
	virtual bool OnEvent (GdkEvent* event);
	virtual bool NotifyViewChange ();
	virtual bool DeleteSelection ();
	virtual bool CopySelection (GtkClipboard *clipboard);
	virtual bool CutSelection (GtkClipboard *clipboard);
	virtual bool PasteSelection (GtkClipboard *clipboard);
	virtual void AddSelection (WidgetData* data);
	virtual bool OnReceive (GtkClipboard *clipboard, GtkSelectionData *data, int type);
	virtual bool OnUndo ();
	virtual bool OnRedo ();
	virtual void PushNode (xmlNodePtr node);
	virtual GtkWidget *GetPropertyPage ();
	virtual char const *GetHelpTag () {return "";}
	
protected:
	gdouble m_x0, m_y0, m_x1, m_y1, m_x, m_y;
	Object *m_pObject;
	Object *m_pObjectGroup;
	View *m_pView;
	WidgetData *m_pData;
	Dialog *m_OptionDialog;
	GtkWidget *m_pWidget;
	GnomeCanvasGroup *m_pGroup;
	GnomeCanvasItem *m_pItem;
	GnomeCanvasItem *m_pBackground;
	double m_dZoomFactor;
	bool m_bChanged, m_bPressed;
	unsigned int m_nState;
	gcp::Application *m_pApp;
	set<string> ModifiedObjects;
/*!
if true, the intended operation is allowed. Default value is true, each tool must set
this flag to false if necessary.
*/
	bool m_bAllowed;

private:
	double lastx, lasty;
	string name;
};

}	// namespace gcp

#endif // GCHEMPAINT_TOOL_H
