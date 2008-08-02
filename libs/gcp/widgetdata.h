// -*- C++ -*-

/* 
 * GChemPaint library
 * widgetdata.h
 *
 * Copyright (C) 2002-2008 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_WIDGET_DATA_H
#define GCHEMPAINT_WIDGET_DATA_H

#include <libgnomecanvas/gnome-canvas.h>
#include <map>
#include <list>
#include <gcu/object.h>

/*!\file*/
namespace gcp {

class Application;
class View;

extern guint ClipboardDataType, ClipboardDataType1;
extern xmlChar* ClipboardData;
extern bool cleared;
void on_receive_targets (GtkClipboard *clipboard, GtkSelectionData *selection_data, Application *App);
void on_clear_data (GtkClipboard *clipboard, Application *App);

/*!\enum
Enumeration of the selection states used in GChemPaint in the gcu::Object::SetSelected method.
*/
enum
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
The root group containing all other items, except the background.
*/
	GnomeCanvasGroup *Group;
/*!
The white rectangle used as background (to be deprecated).
*/
	GnomeCanvasItem* Background;
/*!
The current zoom factor.
*/
	double Zoom;
/*!
Maps the document objects to the canvas items which represent them.
*/
	std::map<gcu::Object const*, GnomeCanvasGroup*>Items;
/*!
The list of selected objects.
*/
	std::list<gcu::Object*>SelectedObjects;
	
/*!
@param obj an object.
@return true if the object is selected, false otherwise.
*/
	bool IsSelected (gcu::Object const *obj) const;
/*!
@param obj the object to select.

Selects the specified object.
*/
	void SetSelected (gcu::Object *obj);
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
@param x the x coordinate of the translation vector.
@param y the y coordinate of the translation vector.

Moves the items representing the selection, but don't move the objects
themselves and don't modify the document. This is used by the selection tool
but might be deprecated in the future.
*/
	void MoveSelectedItems (double dx, double dy);
/*!
@param x the x coordinate of the translation vector.
@param y the y coordinate of the translation vector.

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
	void RotateSelection (double dx, double dy, double angle);
/*!
Empties the list of selected objects. Called after objects have been deleted.
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
	void GetSelectionBounds (ArtDRect &rect) const;
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
@param rect an ArtDRect which will receive the object bounds.

Gets the object bounds in canvas coordinates.
*/
	void GetObjectBounds (gcu::Object const *obj, ArtDRect *rect) const;

private:
	void MoveItems (gcu::Object *obj, double dx, double dy);
	void GetObjectBounds (gcu::Object const* obj, ArtDRect &rect) const;
};

}	// namespace gcp

#endif //GCHEMPAINT_WIDGET_DATA_H
