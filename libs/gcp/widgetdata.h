// -*- C++ -*-

/* 
 * GChemPaint library
 * widgetdata.h
 *
 * Copyright (C) 2002-2007 Jean Bréfort <jean.brefort@normalesup.org>
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

using namespace gcu;

namespace gcp {

class Application;
class View;

extern guint ClipboardDataType, ClipboardDataType1;
extern xmlChar* ClipboardData;
extern bool cleared;
void on_receive_targets (GtkClipboard *clipboard, GtkSelectionData *selection_data, Application *App);
void on_clear_data (GtkClipboard *clipboard, Application *App);

enum
{
	SelStateUnselected = 0,
	SelStateSelected,
	SelStateUpdating,
	SelStateErasing
};

class WidgetData
{
public:
	View* m_View;
	GtkWidget *Canvas;
	GnomeCanvasGroup *Group;
	GnomeCanvasItem* Background;
	double Zoom;
	std::map<Object*, GnomeCanvasGroup*>Items;
	std::list<Object*>SelectedObjects;
	
	bool IsSelected (Object* obj);
	void SetSelected (Object* obj);
	void Unselect (Object* obj);
	void UnselectAll ();
	void MoveSelectedItems (double dx, double dy);
	void MoveSelection (double dx, double dy);
	void RotateSelection (double dx, double dy, double angle);
	void ClearSelection () {SelectedObjects.clear();}
	void Copy (GtkClipboard* clipboard);
	void GetSelectionBounds (ArtDRect &rect);
	bool HasSelection () {return !(SelectedObjects.empty());}
	void SelectAll ();
	static xmlDocPtr GetXmlDoc (GtkClipboard* clipboard);
	void ShowSelection (bool state);
	void GetObjectBounds (Object* obj, ArtDRect *rect);

private:
	void MoveItems (Object *obj, double dx, double dy);
	void GetObjectBounds (Object* obj, ArtDRect &rect);
};

}	// namespace gcp

#endif //GCHEMPAINT_WIDGET_DATA_H
