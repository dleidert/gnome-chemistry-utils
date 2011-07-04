// -*- C++ -*-

/*
 * GChemPaint library
 * tool.h
 *
 * Copyright (C) 2001-2011 Jean Bréfort <jean.brefort@normalesup.org>
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

#include <gcu/macros.h>
#include <gtk/gtk.h>
#include <libxml/tree.h>
#include <vector>
#include <set>
#include <string>

namespace gcu {
	class Dialog;
	class Object;
	class UIManager;
}

namespace gccv {
	class Item;
}

/*!\file*/
namespace gcp {

class Application;
class View;
class WidgetData;
class Operation;

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
@param pView the view instance owning the event.
@param pObject the Object on which the click occured.
@param x the horizontal position of the mouse when the event occured.
@param y the vertical position of the mouse when the event occured.
@param state a bit-mask representing the state of the modifier keys (e.g. Control, Shift and Alt) and the pointer buttons. See GdkModifierType in GDK documentation.

This method is called by the framework when the tool is active, the first mouse button
is not pressed and the mouse is moved.
*/
	void OnMotion (View* pView, gcu::Object* pObject, double x, double y, unsigned int state);
/*!
@param pView the view instance owning the event.
@param state a bit-mask representing the state of the modifier keys (e.g. Control, Shift and Alt) and the pointer buttons. See GdkModifierType in GDK documentation.

This method is called by the framework when the tool is active and the mouse is moved
outside the current view.
*/
	void OnLeaveNotify (View* pView, unsigned int state);
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
@param UIManager the gcu::UIManager in use.

This method is called by the framework when the tool is active and the right mouse button
is pressed. It is used to add tool specific menu items to the contextual menu.
It calls OnRightButtonClicked(gcu::UIManager*).

@return true if at least one menu item was added, false otherwise.
*/
	bool OnRightButtonClicked (View* pView, gcu::Object* pObject, double x, double y, gcu::UIManager *UIManager);
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
@param UIManager the gcu::UIManager in use.

Adds menu items to the contextual menu.
Default implementation do not add any menu item and returns false. Derived classes
for which menu items exist must override this method.
@return true if at least one menu item was added, false otherwise.
*/
	virtual bool OnRightButtonClicked (gcu::UIManager *UIManager);
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
@return true on success, false otherwise.
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
Called by the framework for the active tool when a key press event occurs. Default
just returns \a false.

@return true to stop any further propagation of the event, false otherwise.
*/
	virtual bool OnKeyPress (GdkEventKey *event);
/*!
Called by the framework for the active tool when a key release event occurs. Default
just returns \a false.

@return true to stop any further propagation of the event, false otherwise.
*/
	virtual bool OnKeyRelease (GdkEventKey *event);
/*!
Virtual method called by the framework whenthe active view, and hence the active
document has changed, so that the tool can finish its current operation in the
previously active document and update its options box according to
the new active document settings.
@return true to accept the document change, false if something went wrong and the
active document should not change, as in the case of the fragment tool when
the symbols entered can't be interpreted.
*/
	virtual bool NotifyViewChange ();
/*!
Called by the framework to delete the selection. Tools for which it is meaningful
must have an overriden version of this method.
*/
	virtual bool DeleteSelection ();
/*!
Called by the framework to delete the selection. Tools for which it is meaningful
must have an overriden version of this method.
*/
	virtual bool CopySelection (GtkClipboard *clipboard);
/*!
Called by the framework to copy the selection. Tools for which it is meaningful
must have an oveeriden version of this method.
*/
	virtual bool CutSelection (GtkClipboard *clipboard);
/*!
Called by the framework to cut the selection. Tools for which it is meaningful
must have an overriden version of this method.
*/
	virtual bool PasteSelection (GtkClipboard *clipboard);
/*!
Called by the framework to paste data. Tools for which it is meaningful
must have an overriden version of this method.
*/
	virtual void AddSelection (WidgetData* data);
/*!
Called by the framework when clipboard data are available. Tools for which this
is meaningful must have an overriden version of this method.
*/
	virtual bool OnReceive (GtkClipboard *clipboard, GtkSelectionData *data, int type);
/*!
Called by the framework when the user requests to undo the last change. Tools
such as text editing tools for which this
is meaningful must have an overriden version of this method.
*/
	virtual bool OnUndo ();
/*!
Called by the framework when the user requests to redo the last undone change.
Tools such as text editing tools for which this
is meaningful must have an overriden version of this method.
*/
	virtual bool OnRedo ();
/*!
@param node an xml node to push on the tools private undo stack.

Used to store a node after a change while editing a text object by text tools.
*/
	virtual void PushNode (xmlNodePtr node);
/*!
Gets the property page for the tool. Called the first time the tool becomes
active.
@return the new tool property page.
*/
	virtual GtkWidget *GetPropertyPage ();
/*!
Gets the tag used to display the appropriate help topic when the user presses
the help button in the tools box. The framework will prefix the result with
the application name. The text tool in GChemPaint returns "text" which becomes
"gchempaint-text".
@return the help tag for the tool.
*/
	virtual char const *GetHelpTag () {return "";}
/*!
Gets the Application instance owning the tool.
@return the Application instance.
*/
	Application * GetApplication () {return m_pApp;}
/*!
Callback for a settings change event. Only tools which are dependent on some
configuration key need to override this method. Default does nothing.
*/
	virtual void OnConfigChanged () {}

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
Called from OnMotion(gcp::View,double,double,unsigned int) when a motion event
occured. This method must be overriden in
derived classes if motion events are meaningful for the tool.
Default implementation does nothing.
*/
	virtual void OnMotion ();
/*!
Called from OnLeaveNotify(gcp::View,unsigned int) when a leave notify event
occured. This method must be overriden in
derived classes if leave notify events are meaningful for the tool.
Default implementation does nothing.
*/
	virtual void OnLeaveNotify ();
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
/*!
x coordinate for the last mouse click (unless the tool modified it).
*/
	double m_x0;
/*!
y coordinate for the last mouse click (unless the tool modified it).
*/
	double m_y0;
/*!
x coordinate for the last mouse click (unless the tool modified it). It might
be used by tools necessitating two pairs of coordinates.
*/
	double m_x1;
/*!
y coordinate for the last mouse click (unless the tool modified it). It might
be used by tools necessitating two pairs of coordinates.
*/
	double m_y1;
/*!
The current x position of the mouse cursor.
*/
	double m_x;
/*!
The current y position of the mouse cursor.
*/
	double m_y;
/*!
The object on which the last click occured or NULL.
*/
	gcu::Object *m_pObject;
/*!
The group to which m_pObject belongs if any.
*/
	gcu::Object *m_pObjectGroup;
/*!
The active gcp::View.
*/
	View *m_pView;
/*!
The widget data for the current active canvas.
*/
	WidgetData *m_pData;
/*!
The active canvas widget.
*/
	GtkWidget *m_pWidget;
/*!
The item on which the last click occured if any.
*/
	gccv::Item *m_Item;
/*!
The zoom factor when the click occured.
*/
	double m_dZoomFactor;
/*!
Flag that might be used by tools to store whether they changed something
since the last click (and before releasing the button).
*/
	bool m_bChanged;
/*!
The state of modifier keys as a GdkModifierType values combination.
*/
	unsigned int m_nState;
/*!
The application owning the tool.
*/
	gcp::Application *m_pApp;
/*!
A set of modified objects tools might use to track what they did modify.
*/
	std::set<std::string> ModifiedObjects;
/*!
if true, the intended operation is allowed. Default value is true, each tool must set
this flag to false if necessary.
*/
	bool m_bAllowed;

private:
	double lastx, lasty;
	std::string name;
	bool m_bPressed;

/*!\var m_OwnStatus
Whether the tool owns the status bar text.
*/
/*!\fn GetOwnStatus()
@return whether the tool owns the status bar text.
*/
GCU_PROT_PROP (bool, OwnStatus)
};

}	// namespace gcp

#endif // GCHEMPAINT_TOOL_H
