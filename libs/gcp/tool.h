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

namespace gcp {

class Application;

class Tool
{
public:
	Tool (gcp::Application *App, std::string Id);
	virtual ~Tool ();
	
/*!
@param pView the view instance owning the event.
@param pObject the Object on which the click occured.
@param x the horizontal position of the mouse when the click occured.
@param y the vertical position of the mouse when the click occured.
@param state a bit-mask representing the state of the modifier keys (e.g. Control, Shift and Alt) and the pointer buttons. See GdkModifierType in GDK documentation.

This method is called by the framework when the tool is active and a click occurs. It initialize
some members and the calls the virtual OnClicked() method.
It might be called to simulate a click in some instances (e.g. from a contextual menu handler).

@return true if the mouse drag and button release evens are significative for this tool
in the current context, false otherwise. If true, a mouse move will fire the OnDrag method,
and a button release will result in an OnRelease call. If false, nothing happens for these
events.
*/
	bool OnClicked (View* pView, gcu::Object* pObject, double x, double y, unsigned int state);

	void OnDrag (double x, double y, unsigned int state);
	void OnRelease (double x, double y, unsigned int state);
	bool OnRightButtonClicked (View* pView, gcu::Object* pObject, double x, double y, GtkUIManager *UIManager);
	bool Activate (bool bState);
	std::string& GetName () {return name;}
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
	gcu::Object *m_pObject;
	gcu::Object *m_pObjectGroup;
	View *m_pView;
	WidgetData *m_pData;
	gcu::Dialog *m_OptionDialog;
	GtkWidget *m_pWidget;
	GnomeCanvasGroup *m_pGroup;
	GnomeCanvasItem *m_pItem;
	GnomeCanvasItem *m_pBackground;
	double m_dZoomFactor;
	bool m_bChanged, m_bPressed;
	unsigned int m_nState;
	gcp::Application *m_pApp;
	std::set<std::string> ModifiedObjects;
/*!
if true, the intended operation is allowed. Default value is true, each tool must set
this flag to false if necessary.
*/
	bool m_bAllowed;

private:
	double lastx, lasty;
	std::string name;
};

}	// namespace gcp

#endif // GCHEMPAINT_TOOL_H
