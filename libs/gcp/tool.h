// -*- C++ -*-

/* 
 * GChemPaint library
 * tool.h 
 *
 * Copyright (C) 2001-2008 Jean Bréfort <jean.brefort@normalesup.org>
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

/*!\file*/
namespace gcp {

class Application;

/*!\class Tool
Base clas for GChemPaint tools.	
*/
class Tool
{
public:
/*!
@param App the GChemPaint application.
@param Id the name of the tool.

Constructs a new tool.
*/
	Tool (gcp::Application *App, std::string Id);
/*!
The destructor.
*/
	virtual ~Tool ();
	
/*!
@param pView the view instance owning the event.
@param pObject the Object on which the click occured.
@param x the horizontal position of the mouse when the click occured.
@param y the vertical position of the mouse when the click occured.
@param state a bit-mask representing the state of the modifier keys (e.g. Control, Shift and Alt) and the pointer buttons. See GdkModifierType in GDK documentation.

This method is called by the framework when the tool is active and a click occurs. It initialize
some members and then calls the virtual OnClicked() method.
It might be called to simulate a click in some instances (e.g. from a contextual menu handler).

@return true if the mouse drag and button release evens are significative for this tool
in the current context, false otherwise. If true, a mouse move will fire the OnDrag method,
and a button release will result in an OnRelease call. If false, nothing happens for these
events.
*/
	bool OnClicked (View* pView, gcu::Object* pObject, double x, double y, unsigned int state);

/*!
@param x the horizontal position of the mouse when the event occured.
@param y the vertical position of the mouse when the event occured.
@param state a bit-mask representing the state of the modifier keys (e.g. Control, Shift and Alt) and the pointer buttons. See GdkModifierType in GDK documentation.

This method is called by the framework when the tool is active, the first mouse button
is pressed and the mouse is moved.
*/
	void OnDrag (double x, double y, unsigned int state);
/*!
@param x the horizontal position of the mouse when the event occured.
@param y the vertical position of the mouse when the event occured.
@param state a bit-mask representing the state of the modifier keys (e.g. Control, Shift and Alt) and the pointer buttons. See GdkModifierType in GDK documentation.

This method is called by the framework when the tool is active and the first mouse button
is released.
*/
	void OnRelease (double x, double y, unsigned int state);
/*!
@param pView the view where the event occured.
@param pObject the object on which the event occured.
@param x the horizontal position of the mouse when the event occured.
@param y the vertical position of the mouse when the event occured.
@param UIManager the GtkUIManager in use.

This method is called by the framework when the tool is active and the right mouse button
is pressed. It is used to add tool specific menu items to the contextual menu.
It calls OnRightButtonClicked(GtkUIManager*).

@return true if at least one menu item was added, false otherwise.
*/
	bool OnRightButtonClicked (View* pView, gcu::Object* pObject, double x, double y, GtkUIManager *UIManager);
/*!
@param bState whether to activate or deactivate the tool.

When \a bState is true, the tool is activated, otherwise it is deactivated.
Activate() or Deactivate() is called for this instance.
@return true on success, and false otherwise. Activation always succeeds. 
*/
	bool Activate (bool bState);
/*!
@return the tool name.
*/
	std::string& GetName () {return name;}
/*!
@param UIManager the GtkUIManager in use.

Adds menu items to the contextual menu.
Default implementation do not add any menu item and returns false. Derived classes
for which menu items exist must override this method.
@return true if at least one menu item was added, false otherwise.
*/
	virtual bool OnRightButtonClicked (GtkUIManager *UIManager);
/*!
Virtual method called when the tool is activated.
This method should be overriden for all tools which need some initialization
when activated. Default does nothing.
*/
	virtual void Activate ();
/*!
Virtual method called when the tool is deactivated.
This method should be overriden for all tools which need some cleaning
when deactivated. Default does nothing.
return true on success, false otherwise.
*/
	virtual bool Deactivate ();
/*!
@param code the state of the mofifier keys as given inthe state field or
some GdkEvent derived structures.

Called by the framework when a modifier key has been pressed, updates
m_nState, and calls Tool::OnChangeState ().
*/
	void OnKeyPressed (unsigned int code) {m_nState |= code; OnChangeState ();}
/*!
@param code the state of the mofifier keys as given inthe state field or
some GdkEvent derived structures.

Called by the framework when a modifier key has been released, updates
m_nState, and calls Tool::OnChangeState ().
*/
	void OnKeyReleased (unsigned int code) {if (m_nState & code) m_nState -= code; OnChangeState ();}
/*!
Called by the framework for the active tool when an event occurs. Default
just returns \a false.

@return true to stop any further propagation of the event, false otherwise.
*/
	virtual bool OnEvent (GdkEvent* event);
/*!
*/
	virtual bool NotifyViewChange ();
/*!
*/
	virtual bool DeleteSelection ();
/*!
*/
	virtual bool CopySelection (GtkClipboard *clipboard);
/*!
*/
	virtual bool CutSelection (GtkClipboard *clipboard);
/*!
*/
	virtual bool PasteSelection (GtkClipboard *clipboard);
/*!
*/
	virtual void AddSelection (WidgetData* data);
/*!
*/
	virtual bool OnReceive (GtkClipboard *clipboard, GtkSelectionData *data, int type);
/*!
*/
	virtual bool OnUndo ();
/*!
*/
	virtual bool OnRedo ();
/*!
*/
	virtual void PushNode (xmlNodePtr node);
/*!
*/
	virtual GtkWidget *GetPropertyPage ();
/*!
*/
	virtual char const *GetHelpTag () {return "";}

protected:
/*!
Called from OnClicked(View*,gcu::Object*,double,doubl,unsigned int) when a
click occured. This method must be overriden in
derived classes, and return true if the drag and release events are meaningful
for the tool in the current context. Default implementation does nothing and
returns false.

@return true if drag and release events are needed, false otherwise.
*/
	virtual bool OnClicked ();
/*!
Called from OnDrag(double,double,unsigned int) when a drag event occured
occured. This method must be overriden in
derived classes if drag events are meaningful for the tool.
Default implementation does nothing.
*/
	virtual void OnDrag ();
/*!
Called from OnRelease(double,double,unsigned int) when a button release
event occured. This method must be overriden in
derived classes if button release events are meaningful for the tool.
Default implementation does nothing.
*/
	virtual void OnRelease ();
/*!
Called when a modifier key has been pressed or released, and fires a drag
event so that the tool can update things if necessary.
*/
	virtual void OnChangeState ();

protected:
	gdouble m_x0, m_y0, m_x1, m_y1, m_x, m_y;
	gcu::Object *m_pObject;
	gcu::Object *m_pObjectGroup;
	View *m_pView;
	WidgetData *m_pData;
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
