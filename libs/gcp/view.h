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

#include <gcu/macros.h>
#include <gccv/client.h>
#include <list>
#include <map>

namespace gccv {
	class Canvas;
	class Text;
}

namespace gcu {
	class Object;
};

namespace gcp {

class Atom;
class Bond;
class Document;
class WidgetData;

#define GCHEMPAINT_ATOM_NAME "application/x-gchempaint"
extern GtkTargetEntry const targets[];

/*!\class View gcp/view.h
The GChempaint document view.
*/
class View: public gccv::Client
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
@param pObject the object to add.

Adds the object to the canvas.
*/
	void AddObject (gcu::Object *pObject);
/*!
@param pObject the object to update.

Updates the object in the canvas.
*/
	void Update (gcu::Object *pObject);
/*!
Creates a new canvas widget for the view.

@return the new widget.
*/
	GtkWidget* CreateNewWidget ();
/*!
@param widget the destroyed widget.

Called by the framework when a widget is destroyed.
*/
	void OnDestroy (GtkWidget* widget);
/*!
@return the current zoom factor.
*/
	double GetZoomFactor ();
/*!
Updates fonts descriptions after a theme change.
*/
	void UpdateFont ();
/*!
@param pObject the object to remove.

Removes the object from the view and destroys the items representing it.
*/
	void Remove (gcu::Object* pObject);
/*!
@return the height of the font used to display atomic symbols.
*/
	double GetFontHeight () {return m_dFontHeight;}
/*!
@return the name of the font used to display atomic symbols.
*/
	gchar* GetFontName () {return m_sFontName;}
/*!
@return the name of the font used to display stoichiometry indices and charges.
*/
	gchar* GetSmallFontName () {return m_sSmallFontName;}
/*!
@return the description of the font used to display atomic symbols.
*/
	PangoFontDescription* GetPangoFontDesc () {return m_PangoFontDesc;}
/*!
@return the description of the font used to display stoichiometry indices and charges.
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
@param w the widget which received the event.
@param event the current event.

Called by the framework when a key has been pressed on the keyboard.
@return true if the key was significant, false otherwise.
*/
	bool OnKeyPress (GtkWidget* w, GdkEventKey* event);
/*!
@param w the widget which received the event.
@param event the current event.

Called by the framework when a key has been released on the keyboard.
@return true if the key was significant, false otherwise.
*/
	bool OnKeyRelease (GtkWidget* w, GdkEventKey* event);
/*!
@param w the widget which received the event.
@param width the ne widget width.
@param height the new widget height.

Called by the framework when the widget size changed.
@return true (always).
*/
	bool OnSize (GtkWidget *w, int width, int height);
/*!
@param x1 the left coordinate of the bounding rectangle of the used canvas area.
@param y1 the top coordinate of the bounding rectangle of the used canvas area.
@param x2 the right coordinate of the bounding rectangle of the used canvas area.
@param y2 the bottom coordinate of the bounding rectangle of the used canvas area.

Ensures the canvas is large enough to display everything and, if x1 or y1 is
negative moves all the items so that they are visible.
*/
	void UpdateSize (double x1, double y1, double x2, double y2);
/*!
@param item a text item or NULL.

Sets the currently edited text item. \a item should be NULL to tell the view
that no text edition is currently taking place.
*/
	void SetTextActive (gccv::Text* item);
/*!
Called by the framework when the active window changes to stop current edition
and inhibit timer events.

@return true if the change is possible, false to abort it.
*/
	bool PrepareUnselect ();
/*!
@param clipboard the clipboard used.
@param selection_data the data to paste.

Called by the framework to effectively paste data in the document.
*/
	void OnReceive (GtkClipboard* clipboard, GtkSelectionData* selection_data);
/*!
Called by the framework to select everything in the document.
*/
	void OnSelectAll ();
/*!
@return true if the view is embedded in another document view.
*/
	bool IsEmbedded () {return m_bEmbedded;}
/*!
@return the number of existing canvases for this view.
*/
	int GetNbWidgets () {return m_Widgets.size ();}
/*!
@param filename the file name to use for the export.
@param type a string representing the image type as used by the GdkPixbuf
library. Other types supported are "svg", "ps", "pdf", and "eps".
@param resolution the image resolution to use for bitmaps.

Exports the current document to an image. The \a resolution parameter is
significative only for bitmap images; if it is not given, of if negative, 1
will be used which will result as a one to one pixel export.
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

	// Signals
	bool OnButtonPressed (gccv::ItemClient *client, unsigned button, double x, double y, unsigned state);
	bool OnButtonReleased (gccv::ItemClient *client, unsigned button, double x, double y, unsigned state);
	bool OnDrag (gccv::ItemClient *client, double x, double y, unsigned state);
	bool OnMotion (gccv::ItemClient *client, double x, double y, unsigned state);

	WidgetData *GetData () {return m_pData;}
	//Implementation
private:
	WidgetData* m_pData;
	Document* m_pDoc;
	GtkWidget* m_pWidget;
	std::list<GtkWidget*> m_Widgets;
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
GCU_RO_PROP (gccv::Text *, ActiveRichText)
/*!\fn GetCHeight()
@return the half height of a carbon atom symbol in the current theme.
*/
GCU_RO_PROP (double, CHeight)
};

/*!
@param clipboard a GtkClipboard.
@param selection_data the data to paste.
@param pView the view of the active document.

A callback to use for receiving data from a clipboard. Calls View::OnReceive().
*/
void on_receive (GtkClipboard *clipboard, GtkSelectionData *selection_data, View * pView);

}	//	namespace gcp

#endif // GCHEMPAINT_VIEW_H
