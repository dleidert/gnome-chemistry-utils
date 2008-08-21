// -*- C++ -*-

/* 
 * GChemPaint library
 * view.h 
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

#ifndef GCHEMPAINT_VIEW_H
#define GCHEMPAINT_VIEW_H

#include <map>
#include <list>
#include <libgnomecanvas/libgnomecanvas.h>
#include <canvas/gcp-canvas-pango.h>
#include <gcu/macros.h>
#include "atom.h"
#include "bond.h"

namespace gcp {

class Document;
class WidgetData;

#define GCHEMPAINT_ATOM_NAME "application/x-gchempaint"
extern GtkTargetEntry const targets[];

/*!\class
The GChempaint document view.
*/
class View
{
public:
	//Constructor and destructor
/*!
@param pDoc the GchemPaint document for the new view.
@param Embedded whether the document is embedded in another application
or is standalone.

Constructs the document view.
*/
	View (Document *pDoc, bool Embedded);
/*!
The destructor.
*/
	virtual ~View ();

	//Interface	
public:
/*!
@return the canvas widget used for the view.
*/
	GtkWidget* GetWidget () {return m_pWidget;}
/*!
@return the document associated with the view.
*/
	Document* GetDoc () {return m_pDoc;}
/*!
@param item the canvas item at the positionof the event.
@param event the GdkEvent.
@param widget the canvas widget.

Called by the framework when an event occured in the canvas.
@return false if the event should be processed further, true otherwise.
*/
	bool OnEvent (GnomeCanvasItem *item, GdkEvent *event, GtkWidget* widget);
/*!

*/
	void AddObject (gcu::Object const *pObject);
/*!
*/
	void Update (gcu::Object const *pObject);
/*!
*/
	GtkWidget* CreateNewWidget ();
/*!
*/
	void OnDestroy (GtkWidget* widget);
/*!
*/
	GnomeCanvasItem* GetCanvasItem (GtkWidget* widget, gcu::Object* Object);
/*!
*/
	GnomeCanvasItem* GetBackground ();
/*!
*/
	double GetZoomFactor ();
/*!
*/
	void UpdateFont ();
/*!
*/
	void Remove (gcu::Object* pObject);
/*!
*/
	PangoContext* GetPangoContext () {return m_PangoContext;}
/*!
*/
	double GetFontHeight () {return m_dFontHeight;}
/*!
*/
	gchar* GetFontName () {return m_sFontName;}
/*!
*/
	gchar* GetSmallFontName () {return m_sSmallFontName;}
/*!
*/
	PangoFontDescription* GetPangoFontDesc () {return m_PangoFontDesc;}
/*!
*/
	PangoFontDescription* GetPangoSmallFontDesc () {return m_PangoSmallFontDesc;}
/*!
@param w the active document widget.

Removes all objects in the widget.
*/
	void OnDeleteSelection (GtkWidget* w);
/*!
@param w the active document widget.
@param clipboard a GtkClipboard.

Called by the framework to copy the selection.
*/
	void OnCopySelection (GtkWidget* w, GtkClipboard* clipboard);
/*!
@param w the active document widget.
@param clipboard a GtkClipboard.

Called by the framework to paste clipboard contents.
*/
	void OnPasteSelection (GtkWidget* w, GtkClipboard* clipboard);
/*!
@param w the active document widget.
@param clipboard a GtkClipboard.

Called by the framework to copy and delete the selection.
*/
	void OnCutSelection (GtkWidget* w, GtkClipboard* clipboard);
/*!

*/
	bool OnKeyPress (GtkWidget* w, GdkEventKey* event);
/*!

*/
	bool OnKeyRelease (GtkWidget* w, GdkEventKey* event);
/*!

*/
	bool OnSize (GtkWidget *w, int width, int height);
/*!

*/
	void UpdateSize (double x1, double y1, double x2, double y2);
/*!

*/
	void SetGnomeCanvasPangoActive (GnomeCanvasPango* item);
/*!

*/
	bool PrepareUnselect ();
/*!

*/
	void OnReceive (GtkClipboard* clipboard, GtkSelectionData* selection_data);
/*!

*/
	void OnSelectAll ();
/*!

*/
	bool IsEmbedded () {return m_bEmbedded;}
/*!

*/
	int GetNbWidgets () {return m_Widgets.size ();}
/*!

*/
	void ExportImage (std::string const &filename, const char* type, int resolution = -1);
/*!
@return the xml document containig the svg output.
*/
	xmlDocPtr BuildSVG ();
/*!
@param resolution the resolution for the new image.

Builds a new image with the given resolution in ppi. The size is evaluated
using the guessed screen resolution.
@return the new pixbuf.
*/
	GdkPixbuf *BuildPixbuf (int resolution);
/*!
Called by the framework to ensure that the view size is enough large to
contain all objects.
*/
	void EnsureSize ();
/*!
@param zoom the new zoom level.

Sets the zoom level.
*/
	void Zoom (double zoom);
/*!
@param show whether to show the cursor or not.

Shows or hides the cursor in the currently edited text item if any.
*/
	void ShowCursor (bool show);
/*!
Called by the framework to update the default font size after a theme change.
*/
	void UpdateTheme ();
/*!
@param cr the cairo_t to which render.

Renders the document using cairo.
*/
	void Render (cairo_t *cr);

	//Implementation
private:
	WidgetData* m_pData;
	Document* m_pDoc;
	GtkWidget* m_pWidget;
	std::list<GtkWidget*> m_Widgets;
	PangoContext* m_PangoContext;
	PangoFontDescription* m_PangoFontDesc, *m_PangoSmallFontDesc;
	double m_dFontHeight;
	gchar* m_sFontName, *m_sSmallFontName;
	int m_width, m_height;
	double m_lastx, m_lasty;
	bool m_bEmbedded;
	GtkUIManager *m_UIManager;
	bool m_Dragging;
	gcu::Object *m_CurObject;

/*!\fn GetBaseLineOffset()
@return the vertical offset for algnment of an atomic symbol. This value is half the height of the "C" character.
*/
GCU_RO_PROP (double, BaseLineOffset)
/*!\fn GetActiveRichText()
@return the currently edited text item if any, or NULL.
*/
GCU_RO_PROP (GnomeCanvasPango*, ActiveRichText)
};

bool on_event (GnomeCanvasItem *item, GdkEvent *event, GtkWidget* widget);
void on_receive (GtkClipboard *clipboard, GtkSelectionData *selection_data, View * pView);

}	//	namespace gcp

#endif // GCHEMPAINT_VIEW_H
