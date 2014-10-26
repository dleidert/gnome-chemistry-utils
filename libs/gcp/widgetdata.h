// -*- C++ -*-

/*
 * GChemPaint library
 * widgetdata.h
 *
 * Copyright (C) 2002-2012 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_WIDGET_DATA_H
#define GCHEMPAINT_WIDGET_DATA_H

#include <gcu/object.h>
#include <gccv/structs.h>
#include <map>
#include <set>

/*!\file*/
namespace gcp {

class Application;
class View;

/*!\var ClipboardDataType
The data type available for the default clipboard.
*/
/*!\var ClipboardDataType
The data type available for the primary clipboard.
*/
extern guint ClipboardDataType, ClipboardDataType1;
/*!
A global variable to store clipboard data as a string representation of an xml
document.
*/
	extern xmlChar* ClipboardData;
/*!
A global variable to store clipboard data as a string, used for text.
*/
extern char* ClipboardTextData;
/*!
@param clipboard a GtkClipboard.
@param selection_data the data to paste.
@param App the Application.

A callback to use for receiving targets from a clipboard.
*/
void on_receive_targets (GtkClipboard *clipboard, GtkSelectionData *selection_data, Application *App);
/*!
@param clipboard a GtkClipboard.
@param obj an object, might be the Application.

A callback to use for clearing the clipboard data.
*/
void on_clear_data (GtkClipboard *clipboard, gcu::Object *obj);

/*!\enum SelectionState
Enumeration of the selection states used in GChemPaint in the gcu::Object::SetSelected method.
*/
enum SelectionState
{
/*!
Unselected object.
*/
	SelStateUnselected = 0,
/*!
Unselected object.
*/
	SelStateSelected,
/*!
Edited or new object.
*/
	SelStateUpdating,
/*!
The object is marked for deletion.
*/
	SelStateErasing
};

/*!\class WidgetData
This class contains all data associated with a widget displaying a document. It
might be deprecated in future versions since it was mostly useful for the Bonobo
version which is not anymore supported.
*/
class WidgetData
{
public:
/*!
The document view.
*/
	View* m_View;
/*!
The canvas widget to which this instance is associated.
*/
	GtkWidget *Canvas;
/*!
The current zoom factor.
*/
	double Zoom;
/*!
Maps the document objects to the canvas items which represent them.
*/
//	std::map<gcu::Object const*, GnomeCanvasGroup*>Items;
/*!
The set of selected objects.
*/
	std::set < gcu::Object * >SelectedObjects;

/*!
@param obj an object.
@return true if the object is selected, false otherwise.
*/
	bool IsSelected (gcu::Object const *obj) const;

/*!
@param obj an object.
@return true if all the object children are selected, false otherwise or if \a obj
has no children.
*/
	bool ChildrenSelected (gcu::Object const *obj) const;


/*!
@param obj an object.
@return the topmost ancestor whose all children are selected, NULL if
none or if the ancestor is the document.
*/
	gcu::Object *GetSelectedAncestor (gcu::Object *obj);

/*!
@param obj the object to select.
@param state the new selection state.

Selects the specified object.
*/
	void SetSelected (gcu::Object *obj, int state = gcp::SelStateSelected);
/*!
@param obj the object to unselect.

Unselects a specified object.
*/
	void Unselect (gcu::Object *obj);
/*!
Unselects everything.
*/
	void UnselectAll ();
/*!
@param dx the x coordinate of the translation vector.
@param dy the y coordinate of the translation vector.

Moves the items representing the selection, but don't move the objects
themselves and don't modify the document. This is used by the selection tool
but might be deprecated in the future.
*/
	void MoveSelectedItems (double dx, double dy);
/*!
@param dx the x coordinate of the translation vector.
@param dy the y coordinate of the translation vector.

Moves the selection. This method creates an Operation instance for the
Undo/Redo framework.
*/
	void MoveSelection (double dx, double dy);
/*!
@param x the x coordinate of the rotation center.
@param y the y coordinate of the rotation center.
@param angle the rotation angle.

Rotates the selection. This method does not create an Operation instance.
*/
	void RotateSelection (double x, double y, double angle);
/*!
Empties the set of selected objects. Called after objects have been deleted.
*/
	void ClearSelection () {SelectedObjects.clear();}
/*!
@param clipboard a GtkClipboard.

Copies the current selection to the clipboard.
*/
	void Copy (GtkClipboard* clipboard);
/*!
@param rect an ArtDRect which will receive the selection bounds.

Gets the selection bounds in canvas coordinates.
*/
	void GetSelectionBounds (gccv::Rect &rect) const;
/*!
@return true if at least one object is selected, false otherwise.
*/
	bool HasSelection () {return !(SelectedObjects.empty());}
/*!
Selects the whole document.
*/
	void SelectAll ();
/*!
@param clipboard a GtkClipboard.
@return the xmlDocPtr associtated with the clipboard.
*/
	static xmlDocPtr GetXmlDoc (GtkClipboard* clipboard);
/*!
@param state whether to show or not the selection.

If \a state is true, the selection is highlighted, otherwise, it is displayed
normally. This is used when printing or exporting an image.
*/
	void ShowSelection (bool state);
/*!
@param obj a gcu::Object.
@param rect a gccv::Rect which will receive the object bounds.

Gets the object bounds in canvas coordinates.
*/
	void GetObjectBounds (gcu::Object const *obj, gccv::Rect *rect) const;
/*!
@param objects a set of gcu::Object.
@param rect a gccv::Rect which will receive the object bounds.

Gets the objects bounds in canvas coordinates.
*/
	void GetObjectsBounds (std::set <gcu::Object const *> const &objects, gccv::Rect *rect) const;
	void GetObjectsBounds (std::set <gcu::Object *> const &objects, gccv::Rect *rect) const;
/*!
@param obj a gcu::Object.
@param rect a gccv::Rect which will receive the object bounds.

Gets the object bounds in canvas coordinates.
*/
	void GetObjectBounds (gcu::Object const* obj, gccv::Rect &rect) const;

/*!
Replace the selected objects by their parents if all parents children are
selected.
*/
	void SimplifySelection ();

private:
	void MoveItems (gcu::Object *obj, double dx, double dy);
	void _GetObjectBounds (gcu::Object const* obj, gccv::Rect &rect) const;
	
};

}	// namespace gcp

#endif //GCHEMPAINT_WIDGET_DATA_H
