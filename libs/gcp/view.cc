// -*- C++ -*-

/* 
 * GChemPaint library
 * view.cc
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
#include "widgetdata.h"
#include "view.h"
#include "settings.h"
#include "document.h"
#include "application.h"
#include "tool.h"
#include "tools.h"
#include "text.h"
#include "theme.h"
#include "window.h"
#include <canvas/gprintable.h>
#include <canvas/gcp-canvas-group.h>
#include <canvas/gcp-canvas-rect-ellipse.h>
#include <libgnomevfs/gnome-vfs-file-info.h>
#include <libgnomevfs/gnome-vfs-ops.h>
#include <unistd.h>
#include <pango/pango-context.h>
#include <gdk/gdkkeysyms.h>
#include <glib/gi18n-lib.h>
#include <clocale>
#include <cmath>
#include <fstream>
#include <map>
#include <string>
#include <cstring>
#include <iostream>

/*
Derivation of a new widget from gnome_canvas with an event for updating canvas size
*/

typedef struct _GnomeCanvasGCP           GnomeCanvasGCP;
typedef struct _GnomeCanvasGCPClass      GnomeCanvasGCPClass;

#define GNOME_TYPE_CANVAS_GCP            (gnome_canvas_gcp_get_type ())
#define GNOME_CANVAS_GCP(obj)            (GTK_CHECK_CAST ((obj), GNOME_TYPE_CANVAS_GCP, GnomeCanvasGCP))
#define GNOME_CANVAS_CLASS_GCP(klass)    (GTK_CHECK_CLASS_CAST ((klass), GNOME_TYPE_CANVAS_GCP, GnomeCanvasGCPClass))
#define GNOME_IS_CANVAS_GCP(obj)         (GTK_CHECK_TYPE ((obj), GNOME_TYPE_CANVAS_GCP))
#define GNOME_IS_CANVAS_GCP_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GNOME_TYPE_CANVAS_GCP))
#define GNOME_CANVAS_GCP_GET_CLASS(obj)  (GTK_CHECK_GET_CLASS ((obj), GNOME_TYPE_CANVAS_GCP, GnomeCanvasGCPClass))

enum {
  UPDATE_BOUNDS,
  LAST_SIGNAL
};

struct _GnomeCanvasGCP {
	GnomeCanvas canvas;
};

struct _GnomeCanvasGCPClass {
	GnomeCanvasClass parent_class;

	void (* update_bounds) (GnomeCanvasGCP *canvas);
};

GType gnome_canvas_gcp_get_type (void) G_GNUC_CONST;
static void gnome_canvas_gcp_class_init (GnomeCanvasGCPClass *Class);
static void gnome_canvas_gcp_init (GnomeCanvasGCP *canvas);
static void gnome_canvas_gcp_update_bounds (GnomeCanvasGCP *canvas);
static GtkWidget *gnome_canvas_gcp_new (void);

static guint gnome_canvas_gcp_signals[LAST_SIGNAL] = { 0 };

GType
gnome_canvas_gcp_get_type (void)
{
	static GType canvas_gcp_type;

	if (!canvas_gcp_type) {
		static const GTypeInfo object_info = {
			sizeof (GnomeCanvasGCPClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) gnome_canvas_gcp_class_init,
			(GClassFinalizeFunc) NULL,
			NULL,			/* class_data */
			sizeof (GnomeCanvasGCP),
			0,			/* n_preallocs */
			(GInstanceInitFunc) gnome_canvas_gcp_init,
			NULL			/* value_table */
		};

		canvas_gcp_type = g_type_register_static (GNOME_TYPE_CANVAS, "GnomeCanvasGCP",
						      &object_info, (GTypeFlags) 0);
	}

	return canvas_gcp_type;
}

GtkWidget *gnome_canvas_gcp_new (void)
{
	return GTK_WIDGET (g_object_new (GNOME_TYPE_CANVAS_GCP, "aa", TRUE, NULL));
}

void gnome_canvas_gcp_class_init (GnomeCanvasGCPClass *Class)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (Class);
	gnome_canvas_gcp_signals[UPDATE_BOUNDS] =
	g_signal_new ("update_bounds",
				  G_TYPE_FROM_CLASS (gobject_class),
				  G_SIGNAL_RUN_LAST,
				  G_STRUCT_OFFSET (GnomeCanvasGCPClass, update_bounds),
				  NULL, NULL,
				  g_cclosure_marshal_VOID__VOID,
				  G_TYPE_NONE, 0
				  );
	Class->update_bounds = gnome_canvas_gcp_update_bounds;
}

void gnome_canvas_gcp_init (GnomeCanvasGCP *canvas)
{
}

void gnome_canvas_gcp_update_bounds (GnomeCanvasGCP *canvas)
{
	GnomeCanvas* w = GNOME_CANVAS (canvas);
	while (w->idle_id)
		gtk_main_iteration();
	gnome_canvas_update_now (w);
	gcp::WidgetData* pData = (gcp::WidgetData*) g_object_get_data (G_OBJECT (canvas), "data");
	double x1, y1, x2, y2;
	gnome_canvas_item_get_bounds (GNOME_CANVAS_ITEM (pData->Group), &x1, &y1, &x2, &y2);
	gcp::View *pView = (gcp::View*) g_object_get_data (G_OBJECT (canvas), "view");
	pView->UpdateSize (x1, y1, x2, y2);
}

using namespace gcu;
using namespace std;

namespace gcp {

bool on_event (GnomeCanvasItem *item, GdkEvent *event, GtkWidget* widget)
{
	View* pView = (View*) g_object_get_data(G_OBJECT(widget), "view");
	return pView->OnEvent(item, event, widget);
}

static bool on_destroy (GtkWidget *widget, View * pView)
{
	pView->OnDestroy (widget);
	return true;
}

static bool on_size (GtkWidget *widget, GtkAllocation *alloc, View * pView)
{
	pView->OnSize (widget, alloc->width, alloc->height);
	return true;
}

void on_receive (GtkClipboard *clipboard, GtkSelectionData *selection_data, View * pView)
{
	pView->OnReceive (clipboard, selection_data);
}

View::View (Document *pDoc, bool Embedded)
{
	m_pDoc = pDoc;
	Theme *pTheme = pDoc->GetTheme ();
	m_PangoFontDesc = pango_font_description_new ();
	pango_font_description_set_family (m_PangoFontDesc, pTheme->GetFontFamily ());
	pango_font_description_set_style (m_PangoFontDesc, pTheme->GetFontStyle ());
	pango_font_description_set_weight (m_PangoFontDesc, pTheme->GetFontWeight ());
	pango_font_description_set_variant (m_PangoFontDesc, pTheme->GetFontVariant ());
	pango_font_description_set_stretch (m_PangoFontDesc, pTheme->GetFontStretch ());
	pango_font_description_set_size (m_PangoFontDesc, pTheme->GetFontSize ());
	m_sFontName = pango_font_description_to_string (m_PangoFontDesc);
	m_PangoSmallFontDesc = pango_font_description_new ();
	pango_font_description_set_family (m_PangoSmallFontDesc, pTheme->GetFontFamily ());
	pango_font_description_set_style (m_PangoSmallFontDesc, pTheme->GetFontStyle ());
	pango_font_description_set_weight (m_PangoSmallFontDesc, pTheme->GetFontWeight ());
	pango_font_description_set_variant (m_PangoSmallFontDesc, pTheme->GetFontVariant ());
	pango_font_description_set_stretch (m_PangoSmallFontDesc, pTheme->GetFontStretch ());
	pango_font_description_set_size (m_PangoSmallFontDesc, pTheme->GetFontSize () * 2 / 3);
	m_sSmallFontName = pango_font_description_to_string (m_PangoSmallFontDesc);
	m_width = 400;
	m_height = 300;
	m_ActiveRichText = NULL;
	m_bEmbedded = Embedded;
	m_UIManager = gtk_ui_manager_new ();
	m_Dragging = false;
	m_pWidget = NULL;
	m_PangoContext = NULL;
	m_CurObject = NULL;
}

View::~View ()
{
	if (m_PangoContext)
		g_object_unref (G_OBJECT (m_PangoContext));
	if (m_sFontName)
		g_free (m_sFontName);
	if (m_sSmallFontName)
		g_free (m_sSmallFontName);
	pango_font_description_free (m_PangoFontDesc);
	pango_font_description_free (m_PangoSmallFontDesc);
	g_object_unref (m_UIManager);
}

bool View::OnEvent (GnomeCanvasItem *item, GdkEvent *event, GtkWidget* widget)
{
	Application *App = m_pDoc->GetApplication ();
	Theme *pTheme = m_pDoc->GetTheme ();
	Tool* pActiveTool = App? App->GetActiveTool (): NULL;
	if ((!m_pDoc->GetEditable ()) || (!pActiveTool))
		return true;
	m_CurObject = (item) ? (Object*) g_object_get_data (G_OBJECT (item), "object") : NULL;
	if (item == (GnomeCanvasItem*) m_ActiveRichText) {
		GnomeCanvasItemClass* klass = GNOME_CANVAS_ITEM_CLASS (((GTypeInstance*) item)->g_class);
		return klass->event (item, event);
	} else if (pActiveTool->OnEvent (event))
		return true;
	m_pData = (WidgetData*) g_object_get_data (G_OBJECT (widget), "data");
	m_pWidget = widget;
	double x, y;
	x = event->button.x;
	y = event->button.y;
	gnome_canvas_item_w2i (GNOME_CANVAS_ITEM (m_pData->Group), &x, &y);
	if (event->type == GDK_BUTTON_PRESS) {
		if (item == m_pData->Background) {
			item = NULL;
			std::map<Object const*, GnomeCanvasGroup*>::iterator i = m_pData->Items.begin (),
						end = m_pData->Items.end ();
			Bond* pBond;
			while (i != end) {
				if ((*i).first->GetType () == gcu::BondType) {
					pBond = (Bond*) (*i).first;
					if (pBond->GetDist(x / pTheme->GetZoomFactor (), y / pTheme->GetZoomFactor ()) < (pTheme->GetPadding () + pTheme->GetBondWidth () / 2) / pTheme->GetZoomFactor ()) {
						item = GNOME_CANVAS_ITEM ((*i).second);
						m_CurObject = pBond;
						break;
					}
				}
				i++;
			}
		}
	}
	Object *pAtom;
	if (m_CurObject && ((pAtom = m_CurObject->GetAtomAt (x / pTheme->GetZoomFactor (), y / pTheme->GetZoomFactor ()))))
			m_CurObject = pAtom;
	switch (event->type) {
	case GDK_BUTTON_PRESS:
		switch (event->button.button) {
			case 1: {
				if (m_Dragging) break;
				bool result = pActiveTool->OnClicked(this, m_CurObject, x, y, event->button.state);
				if (item && (item == (GnomeCanvasItem*)m_ActiveRichText)) {
					GnomeCanvasItemClass* klass = GNOME_CANVAS_ITEM_CLASS (((GTypeInstance*) item)->g_class);
					return klass->event (item, event);
				}
				m_Dragging = result;
				return true;
			}
			case 2: {
				m_lastx = x;
				m_lasty = y;
				GtkClipboard *clipboard = gtk_clipboard_get (GDK_SELECTION_PRIMARY);
				OnPasteSelection (m_pWidget, clipboard);
				return true;
			}
			case 3: {
				bool result;
				g_object_unref (m_UIManager);
			 	m_UIManager = gtk_ui_manager_new ();
				result = pActiveTool->OnRightButtonClicked (this, m_CurObject, event->button.x, event->button.y, m_UIManager);
				if (m_CurObject)
					result |= m_CurObject->BuildContextualMenu (m_UIManager, m_CurObject, x / GetZoomFactor (), y / GetZoomFactor ());
				if (result) {
					GtkWidget *w = gtk_ui_manager_get_widget (m_UIManager, "/popup");
					gtk_menu_popup (GTK_MENU (w), NULL, NULL, NULL, NULL, 3,  gtk_get_current_event_time ());
					return true;
				}
			}
		}
		break;
	case GDK_MOTION_NOTIFY:
		if (!m_Dragging)
			break;
		pActiveTool->OnDrag (x, y, event->button.state);
		return true;
	case GDK_BUTTON_RELEASE:
	switch (event->button.button)
		{
		case 1:
			if (!m_Dragging)
				break;
			m_Dragging = false;
			pActiveTool->OnRelease (x, y, event->button.state);
			m_pDoc->GetApplication ()->ClearStatus ();
			return true;
		}
		break;
	default:
		break;
	}
	return false;
}

void View::AddObject (Object const *pObject)
{
	std::list<GtkWidget*>::iterator i, end = m_Widgets.end ();
	for (i = m_Widgets.begin (); i != end; i++)
		pObject->Add (*i);
}

GtkWidget* View::CreateNewWidget ()
{
	gtk_widget_push_colormap (gdk_rgb_get_colormap ());
	m_pWidget = gnome_canvas_gcp_new();
	gtk_widget_pop_colormap ();
	GtkWidget* pWidget = (m_Widgets.size() > 0) ? m_Widgets.front () : NULL;
	if (m_pWidget) {
		g_object_set_data (G_OBJECT (m_pWidget), "view", this);
		g_object_set_data (G_OBJECT (m_pWidget), "doc", m_pDoc);
		m_pData = new WidgetData ();
		m_pData->Canvas = m_pWidget;
		g_object_set_data (G_OBJECT (m_pWidget), "data", m_pData);
		m_pData->m_View = this;
		gnome_canvas_set_pixels_per_unit (GNOME_CANVAS (m_pWidget), 1);
		gnome_canvas_set_scroll_region (GNOME_CANVAS (m_pWidget), 0, 0, m_width, m_height);
		m_pData->Zoom = 1.0;
		m_pData->Background = gnome_canvas_item_new (
									gnome_canvas_root (GNOME_CANVAS (m_pWidget)),
									gnome_canvas_rect_ext_get_type (),
									"x1", 0.0,
									"y1", 0.0,
									"x2", (double) m_width,
									"y2", (double) m_height,
									"fill_color", "white",
									NULL);
		m_pData->Group = GNOME_CANVAS_GROUP (gnome_canvas_item_new (
									gnome_canvas_root (GNOME_CANVAS (m_pWidget)),
									gnome_canvas_group_ext_get_type (),
									NULL));
		if (m_pDoc->GetEditable ())
			g_signal_connect (G_OBJECT (m_pData->Background), "event", G_CALLBACK (on_event), m_pWidget);
		g_signal_connect (G_OBJECT (m_pWidget), "destroy", G_CALLBACK (on_destroy), this);
		g_signal_connect (G_OBJECT (m_pWidget), "size_allocate", G_CALLBACK (on_size), this);
		g_signal_connect (G_OBJECT (m_pWidget), "realize", G_CALLBACK (gnome_canvas_gcp_update_bounds), this);
		gtk_widget_show (m_pWidget);
		m_Widgets.push_back (m_pWidget);
		if (pWidget) {
			WidgetData* pData = (WidgetData*) g_object_get_data (G_OBJECT (pWidget), "data");
			std::map<Object const*, GnomeCanvasGroup*>::iterator i, iend = pData->Items.end ();
			for (i = pData->Items.begin (); i != iend; i++)
				if ((*i).first->GetType () != gcu::BondType)
				 (*i).first->Add (m_pWidget);
			for (i = pData->Items.begin (); i != iend; i++)
				if ((*i).first->GetType () == gcu::BondType)
				 (*i).first->Add (m_pWidget);
		} else {
			m_PangoContext = gtk_widget_create_pango_context (m_pWidget);
			g_object_ref (G_OBJECT (m_PangoContext));
			UpdateFont ();
		}
	}
	return m_pWidget;
}

void View::OnDestroy (GtkWidget* widget)
{
	if (m_bEmbedded) {
		delete (WidgetData*) g_object_get_data (G_OBJECT (widget), "data");
		m_Widgets.remove (widget);
	} else
		delete m_pDoc;
}

GnomeCanvasItem* View::GetCanvasItem (GtkWidget* widget, Object* Object)
{
	WidgetData* pData = reinterpret_cast<WidgetData*> (g_object_get_data (G_OBJECT (widget), "data"));
	if ((!pData) || (pData->m_View != this))
		return NULL;
	GnomeCanvasItem *result = reinterpret_cast<GnomeCanvasItem*> (pData->Items[Object]);
	if (!result)
		pData->Items.erase (Object);
	return result;
}

void View::Update (Object const *pObject)
{
	std::list<GtkWidget*>::iterator i;
	for (i = m_Widgets.begin (); i != m_Widgets.end (); i++)
		pObject->Update(*i);
}

GnomeCanvasItem* View::GetBackground ()
{
	return m_pData->Background;
}

double View::GetZoomFactor ()
{
	return m_pDoc->GetTheme ()->GetZoomFactor ();
}

void View::UpdateFont ()
{
	pango_context_set_font_description (m_PangoContext, m_PangoFontDesc);
	PangoLayout* pl = pango_layout_new (m_PangoContext);
	PangoRectangle rect;
	pango_layout_set_text (pl, "lj", 2);
	pango_layout_get_extents (pl, &rect, NULL);
	m_dFontHeight = rect.height / PANGO_SCALE;
	g_object_unref (G_OBJECT (pl));
	pl = pango_layout_new (m_PangoContext);
	pango_layout_set_text (pl, "C", 1);
	pango_layout_get_extents (pl, &rect, NULL);
	m_BaseLineOffset =  (double (rect.height / PANGO_SCALE) / 2.0) / m_pDoc->GetTheme ()->GetZoomFactor ();
	g_object_unref (G_OBJECT (pl));
}
	
void View::Remove (Object* pObject)
{
	std::list<GtkWidget*>::iterator i, end = m_Widgets.end ();
	for (i = m_Widgets.begin (); i != end; i++) {
		WidgetData* pData = (WidgetData*) g_object_get_data (G_OBJECT (*i), "data");
		Object* pObj = pObject->GetMolecule ();
		if (pObj)
			pData->SelectedObjects.remove (pObj);
		else
			pData->SelectedObjects.remove (pObject);
		if (pData->Items[pObject])
			gtk_object_destroy (GTK_OBJECT (pData->Items[pObject]));
		pData->Items.erase (pObject);
	}
}

void View::OnDeleteSelection (GtkWidget* w)
{
	m_pWidget = w;
	Application *pApp = m_pDoc->GetApplication ();
	Tool *pActiveTool = pApp->GetActiveTool ();
	Object *parent;
	if (!pActiveTool->DeleteSelection ()) {
		m_pData = (WidgetData*) g_object_get_data (G_OBJECT (w), "data");
		WidgetData *pData;
		std::list<GtkWidget*>::iterator j, jend = m_Widgets.end ();
		for (j = m_Widgets.begin (); j != jend; j++) {
			if (*j == m_pWidget)
				continue;
			pData = (WidgetData*) g_object_get_data (G_OBJECT (*j), "data");
			pData->UnselectAll ();
		}
		Object *pObject, *Group;
		set<string> ModifiedObjects;
		bool modify = false;
		list<Object *>::iterator i, iend = m_pData->SelectedObjects.end ();
		// first search if we are deleting top objects or not
		for (i = m_pData->SelectedObjects.begin (); i != iend; i++)
			if ((*i)->GetGroup ()) {
				modify = true;
				break;
			}
		Operation *Op = m_pDoc->GetNewOperation (((modify)?
								GCP_MODIFY_OPERATION: GCP_DELETE_OPERATION));
		while (!m_pData->SelectedObjects.empty ()) {
			pObject = m_pData->SelectedObjects.front ();
			Group = pObject->GetGroup ();
			if (Group && 
				(ModifiedObjects.find (Group->GetId ()) == ModifiedObjects.end ())) {
				Op->AddObject (Group);
				ModifiedObjects.insert (Group->GetId ());
			} else
				Op->AddObject (pObject);
			pObject->Lock ();
			parent = pObject->GetParent ();
			m_pDoc->Remove (pObject);
			if (parent)
				parent->EmitSignal (OnChangedSignal);
		}
		m_pData->ClearSelection ();
		set<string>::iterator k, kend = ModifiedObjects.end();
		for (k = ModifiedObjects.begin(); k != kend; k++) {
			pObject = m_pDoc->GetDescendant ((*k).c_str ());
			if (pObject)
				Op->AddObject (pObject, 1);
		}
	}
	m_pDoc->FinishOperation ();
	Window *Win = m_pDoc->GetWindow ();
	if (Win) {
		Win->ActivateActionWidget ("/MainMenu/EditMenu/Copy", false);
		Win->ActivateActionWidget ("/MainMenu/EditMenu/Cut", false);
		Win->ActivateActionWidget ("/MainMenu/EditMenu/Erase", false);
	}
}

GtkTargetEntry const targets[] = {
	{(char *) GCHEMPAINT_ATOM_NAME,  0, GCP_CLIPBOARD_NATIVE},
	{(char *) "image/svg",  0, GCP_CLIPBOARD_SVG},
	{(char *) "image/svg+xml",  0, GCP_CLIPBOARD_SVG_XML},
	{(char *) "image/x-eps",  0, GCP_CLIPBOARD_EPS},
	{(char *) "image/png",  0, GCP_CLIPBOARD_PNG},
	{(char *) "image/jpeg",  0, GCP_CLIPBOARD_JPEG},
	{(char *) "image/bmp",  0, GCP_CLIPBOARD_BMP},
	{(char *) "UTF8_STRING", 0, GCP_CLIPBOARD_UTF8_STRING},
	{(char *) "STRING", 0, GCP_CLIPBOARD_STRING}
};

void View::OnCopySelection (GtkWidget* w, GtkClipboard* clipboard)
{
	if (m_pDoc->GetAllowClipboard ()) {
		Application * pApp = m_pDoc->GetApplication ();
		Tool* pActiveTool = pApp->GetActiveTool ();
		m_pWidget = w;
		m_pData = (WidgetData*) g_object_get_data (G_OBJECT (w), "data");
		if (!pActiveTool->CopySelection (clipboard))
			m_pData->Copy (clipboard);
	}
}

void View::OnReceive (GtkClipboard* clipboard, GtkSelectionData* selection_data)
{
	if ((selection_data->length <= 0) || !selection_data->data)
		return;
	Application *pApp = m_pDoc->GetApplication ();
	Tool *pActiveTool = pApp->GetActiveTool ();
	guint *DataType = (clipboard == gtk_clipboard_get (GDK_SELECTION_CLIPBOARD))? &ClipboardDataType: &ClipboardDataType1;
	g_return_if_fail ((selection_data->target == gdk_atom_intern (targets[*DataType].target, FALSE)));
	if (pActiveTool->OnReceive (clipboard, selection_data, *DataType))
		return;
	else if (pActiveTool) {
		pApp->ActivateTool ("Select", true);
		pActiveTool = pApp->GetActiveTool ();
	}
	if (!pActiveTool || (pActiveTool != pApp->GetTool ("Select")))
		return;
	xmlDocPtr xml;
	m_pData->UnselectAll ();
	switch (*DataType) {
	case GCP_CLIPBOARD_NATIVE:
		xml = xmlParseMemory ((const char*) selection_data->data, selection_data->length);
		m_pDoc->AddData (xml->children->children);
		xmlFreeDoc (xml);
		break;
	case GCP_CLIPBOARD_UTF8_STRING: {
			Text* text = new Text ();
			text->SetText ((char const*) selection_data->data);
			text->OnChanged (true);
			m_pDoc->AddObject (text);
			m_pData->SetSelected (text);
		}
		break;
	case GCP_CLIPBOARD_STRING: {
			Text* text = new Text ();
			if (!g_utf8_validate ((const char*) selection_data->data, selection_data->length, NULL)) {
				gsize r, w;
				gchar* newstr = g_locale_to_utf8 ((const char*) selection_data->data, selection_data->length, &r, &w, NULL);
				text->SetText (newstr);
				g_free (newstr);
			} else
				text->SetText ((char const*) selection_data->data);
			text->OnChanged (true);
			m_pDoc->AddObject (text);
			m_pData->SetSelected (text);
		}
		break;
	}
	ArtDRect rect;
	double dx, dy;
	while (gtk_events_pending ())
		gtk_main_iteration ();
	m_pDoc->AbortOperation ();
	m_pData->GetSelectionBounds (rect);
	if (clipboard == gtk_clipboard_get (GDK_SELECTION_CLIPBOARD)) {
		//center the pasted data at the center of the visible area
		if (m_bEmbedded) {
			dx = m_pWidget->allocation.width / 2. - (rect.x0 + rect.x1) / 2.;
			dy = m_pWidget->allocation.height / 2. - (rect.y0 + rect.y1) / 2.;
		} else {
			GtkAdjustment *horiz, *vert;
			GtkWidget* parent = gtk_widget_get_parent (m_pWidget);
			horiz = gtk_viewport_get_hadjustment (GTK_VIEWPORT (parent));
			vert = gtk_viewport_get_vadjustment (GTK_VIEWPORT (parent));
			dx = horiz->value + horiz->page_size / 2.  - (rect.x0 + rect.x1) / 2.;
			dy = vert->value + vert->page_size / 2.  - (rect.y0 + rect.y1) / 2.;
		}
	} else {
		//center the pasted data at the mouse click position
		dx = m_lastx - (rect.x0 + rect.x1) / 2.;
		dy = m_lasty - (rect.y0 + rect.y1) / 2.;
	}
	m_pData->MoveSelection (dx, dy);
	Tool *pTool = pApp->GetTool ("Select");
	if (pTool)
		pTool->AddSelection (m_pData);
	m_pDoc->PopOperation ();
	Operation* pOp = m_pDoc->GetNewOperation (GCP_ADD_OPERATION);
	std::list<Object*>::iterator i, end = m_pData->SelectedObjects.end();
	for (i = m_pData->SelectedObjects.begin(); i != end; i++) 
		pOp->AddObject (*i);
	m_pDoc->FinishOperation ();
	gnome_canvas_gcp_update_bounds (GNOME_CANVAS_GCP(m_pData->Canvas));
}

void View::OnPasteSelection (GtkWidget* w, GtkClipboard* clipboard)
{
	Application *pApp = m_pDoc->GetApplication ();
	Tool *pActiveTool = pApp->GetActiveTool ();
	if (pActiveTool->PasteSelection (clipboard))
		return;
	m_pWidget = w;
	m_pData = (WidgetData*) g_object_get_data (G_OBJECT (w), "data");
	guint *DataType = (clipboard == gtk_clipboard_get (GDK_SELECTION_CLIPBOARD))? &ClipboardDataType: &ClipboardDataType1;
	GdkAtom targets_atom  = gdk_atom_intern (targets[*DataType].target, FALSE);
	gtk_clipboard_request_contents (clipboard, targets_atom,  (GtkClipboardReceivedFunc) on_receive, this);
}

void View::OnCutSelection (GtkWidget* w, GtkClipboard* clipboard)
{
	Application *pApp = m_pDoc->GetApplication ();
	Tool *pActiveTool = pApp->GetActiveTool ();
	if (!pActiveTool->CutSelection(clipboard)) {
		OnCopySelection (w, clipboard);
		OnDeleteSelection (w);
	}
	Window *Win = m_pDoc->GetWindow ();
	if (Win) {
		Win->ActivateActionWidget ("/MainMenu/EditMenu/Copy", false);
		Win->ActivateActionWidget ("/MainMenu/EditMenu/Cut", false);
		Win->ActivateActionWidget ("/MainMenu/EditMenu/Erase", false);
	}
}

static void do_set_symbol (GtkAction *action, Object *obj)
{
	Document* pDoc = static_cast<Document*> (obj->GetDocument ());
	Application *App = static_cast<Application*> (pDoc->GetApplication ());
	Tools *tools = static_cast<Tools*> (App->GetDialog ("tools"));
	int Z = Element::Z (gtk_action_get_name(action));
	tools->SetElement (Z);
	if (obj->GetType () == AtomType) {
		Atom *atom = static_cast<Atom*> (obj);
		if (atom->GetZ () == Z || atom->GetZ () == 0)
			return;
		Object *group = obj->GetGroup ();
		Operation *op = pDoc->GetNewOperation (GCP_MODIFY_OPERATION);
		op->AddObject (group);
		atom->SetZ (Z);
		pDoc->GetView ()->Update (obj);
		op->AddObject (group, 1);
		pDoc->FinishOperation ();
	}
}

bool View::OnKeyPress (GtkWidget* w, GdkEventKey* event)
{
	Application *pApp = m_pDoc->GetApplication ();
	Tool *pActiveTool = pApp->GetActiveTool ();
	if (pActiveTool->OnEvent ((GdkEvent*) event))
		return true;
	switch (event->keyval) {
	case GDK_Delete:
	case GDK_Clear:
	case GDK_BackSpace:
		OnDeleteSelection(w);
		return true;
	case GDK_Shift_L:
	case GDK_Shift_R:
		if (pActiveTool)
			pActiveTool->OnKeyPressed (GDK_SHIFT_MASK);
		return true;
	case GDK_Control_L:
	case GDK_Control_R:
		if (pActiveTool)
			pActiveTool->OnKeyPressed (GDK_CONTROL_MASK);
		return true;
	case GDK_Alt_L:
	case GDK_Alt_R:
		if (pActiveTool)
			pActiveTool->OnKeyPressed (GDK_MOD1_MASK);//FIXME: might be not portable
		return true;
	case GDK_ISO_Level3_Shift:
		if (pActiveTool)
			pActiveTool->OnKeyPressed (GDK_MOD5_MASK);//FIXME: might be not portable
		return true;
	case GDK_Caps_Lock:
		if (pActiveTool) {
			if (event->state & GDK_LOCK_MASK)
				pActiveTool->OnKeyReleased (GDK_LOCK_MASK);
			else
				pActiveTool->OnKeyPressed (GDK_LOCK_MASK);
		}
		return true;
	default: {
			if ((event->state & (GDK_CONTROL_MASK | GDK_MOD1_MASK)) != 0 || event->keyval > 127)
				break;
			// Now try to get the atom at the cursor
			Atom *atom = dynamic_cast<Atom*> (m_CurObject);
			unsigned min_bonds = (atom)? atom->GetTotalBondsNumber (): 0;
			int Z = 0;
			switch (event->keyval) {
			case GDK_a:
				Z = 13;
				break;
			case GDK_b:
				Z = 5;
				break;
			case GDK_c:
				Z = 6;
				break;
			case GDK_d:
				Z = 11;
				break;
			case GDK_e:
				Z = 34;
				break;
			case GDK_f:
				Z = 9;
				break;
			case GDK_g:
				Z = 32;
				break;
			case GDK_h:
				Z = 1;
				break;
			case GDK_i:
				Z = 53;
				break;
			case GDK_j:
				Z = 22;
				break;
			case GDK_k:
				Z = 19;
				break;
			case GDK_l:
				Z = 3;
				break;
			case GDK_m:
				Z = 12;
				break;
			case GDK_n:
				Z = 7;
				break;
			case GDK_o:
				Z = 8;
				break;
			case GDK_p:
				Z = 15;
				break;
			case GDK_q:
				Z = 14;
				break;
			case GDK_r:
				Z = 35;
				break;
			case GDK_s:
				Z = 16;
				break;
			case GDK_t:
				Z = 78;
				break;
			case GDK_u:
				Z = 29;
				break;
			case GDK_v:
				Z = 23;
				break;
			case GDK_w:
				Z = 74;
				break;
			case GDK_x:
				Z = 17;
				break;
			case GDK_y:
				Z = 39;
				break;
			case GDK_z:
				Z = 40;
				break;
			}
			if (Z) {
				if (!atom) {
					Tools *tools = static_cast<Tools*> (pApp->GetDialog ("tools"));
					tools->SetElement (Z);
				} else if (atom->GetZ () != Z && Element::GetElement (Z)->GetMaxBonds () >= min_bonds) {
					Object *group = atom->GetGroup ();
					Operation *op = m_pDoc->GetNewOperation (GCP_MODIFY_OPERATION);
					op->AddObject (group);
					atom->SetZ (Z);
					Update (atom);
					op->AddObject (group, 1);
					m_pDoc->FinishOperation ();
				}
				return true;
			}
			// build a contextual menu of all known symbols starting with the pressed key.
			map<string, Element*> entries;
			string symbol;
			unsigned key = gdk_keyval_to_upper (event->keyval);
			for (int i = 1; i <= 128; i++) {
				Element *elt = Element::GetElement (i);
				if (!elt || elt->GetMaxBonds () < min_bonds)
					continue;
				symbol = elt->GetSymbol ();
				if ((unsigned char) symbol[0] == key) {
					entries[symbol] = elt;
				}
			}
			if (entries.empty ())
				break;
			map<string, Element*>::iterator i, end = entries.end ();
			g_object_unref (m_UIManager);
			m_UIManager = gtk_ui_manager_new ();
			GtkActionGroup *group = gtk_action_group_new ("element");;
			GtkAction *action;
			string ui;
			for (i = entries.begin (); i != end; i++) {
				symbol = (*i).first;
				symbol.insert (((symbol.length () > 1)? 1: 0), "_");
				action = GTK_ACTION (gtk_action_new ((*i).second->GetSymbol (), symbol.c_str (), (*i).second->GetName (), NULL));
				g_signal_connect (action, "activate", G_CALLBACK (do_set_symbol), (atom)? static_cast<Object*> (atom): static_cast<Object*> (m_pDoc));
				gtk_action_group_add_action (group, action);
				g_object_unref (action);
				ui = string ("<ui><popup><menuitem action='") + (*i).second->GetSymbol () + "'/></popup></ui>";
				gtk_ui_manager_add_ui_from_string (m_UIManager, ui.c_str (), -1, NULL);
			}
			gtk_ui_manager_insert_action_group (m_UIManager, group, 0);
			g_object_unref (group);
			GtkWidget *w = gtk_ui_manager_get_widget (m_UIManager, "/popup");
			gtk_menu_popup (GTK_MENU (w), NULL, NULL, NULL, NULL, 3,  gtk_get_current_event_time ());
			break;
		}
	}
	return false;
}

bool View::OnKeyRelease (GtkWidget* w, GdkEventKey* event)
{
	Application *pApp = m_pDoc->GetApplication ();
	Tool *pActiveTool = pApp->GetActiveTool ();
	switch(event->keyval) {
	case GDK_Shift_L:
	case GDK_Shift_R:
		if (pActiveTool)
			pActiveTool->OnKeyReleased (GDK_SHIFT_MASK);
		return true;
	case GDK_Control_L:
	case GDK_Control_R:
		if (pActiveTool)
			pActiveTool->OnKeyReleased (GDK_CONTROL_MASK);
		return true;
	case GDK_Alt_L:
	case GDK_Alt_R:
		if (pActiveTool)
			pActiveTool->OnKeyReleased (GDK_MOD1_MASK);//FIXME: might be not portable
		return true;
	case 0:
		// Seems that when releasing the AltGr key, the keyval is 0 !!!
		if (!(event->state & GDK_MOD5_MASK))
			break;
	case GDK_ISO_Level3_Shift:
		if (pActiveTool)
			pActiveTool->OnKeyReleased (GDK_MOD5_MASK);//FIXME: might be not portable
		return true;
	default:
		break;
	}
	return false;
}
	
bool View::OnSize (GtkWidget *w, int width, int height)
{
	WidgetData* pData = (WidgetData*) g_object_get_data (G_OBJECT (w), "data");
	gnome_canvas_set_scroll_region (GNOME_CANVAS (w), 0, 0, (double) width / pData->Zoom, (double) height / pData->Zoom);
	if (pData->Background)
		g_object_set (G_OBJECT (pData->Background), "x2", (double) width / pData->Zoom, "y2", (double) height / pData->Zoom, NULL);
	return true;
}

void View::UpdateSize (double x1, double y1, double x2, double y2)
{
	if (x1 < 0.0) x2 -= x1;
	if (y1 < 0.0) y2 -= y1;
	list<GtkWidget*>::iterator i, end = m_Widgets.end ();
	if ((x2 != m_width) || (y2 != m_height))
		for (i = m_Widgets.begin(); i != end; i++) {
			WidgetData* pData = (WidgetData*) g_object_get_data (G_OBJECT (*i), "data");
			gtk_widget_set_size_request (*i, (int) ceil (x2 * pData->Zoom), (int) ceil (y2 * pData->Zoom));
		}
	if ((x1 < 0.0) || (y1 < 0.0))
	{
		x1 = - x1;
		y1 = - y1;
		Theme* pTheme = m_pDoc->GetTheme ();
		m_pDoc->Move(x1 / pTheme->GetZoomFactor (), y1 / pTheme->GetZoomFactor ());
		Update (m_pDoc);
	}
}

void View::SetGnomeCanvasPangoActive (GnomeCanvasPango* item)
{
	m_ActiveRichText = item;
	m_Dragging = false;
}

bool View::PrepareUnselect ()
{
	Application *pApp = m_pDoc->GetApplication ();
	Tool *pActiveTool = pApp->GetActiveTool ();
	return (pActiveTool)? pActiveTool->NotifyViewChange (): true;
}

void View::OnSelectAll ()
{
	Application *pApp = m_pDoc->GetApplication();
	Tool *pActiveTool = pApp->GetTool ("Select");
	if (pActiveTool)
		pApp->ActivateTool ("Select", true);
	m_pData->SelectAll ();
	if (pActiveTool)
		pActiveTool->AddSelection (m_pData);
}

static gboolean do_save_image (const gchar *buf, gsize count, GError **error, gpointer data)
{
	GnomeVFSHandle *handle = (GnomeVFSHandle*) data;
	GnomeVFSFileSize written = 0;
	GnomeVFSResult res;
	while (count) {
		res = gnome_vfs_write (handle, buf, count, &written);
		if (res != GNOME_VFS_OK) {
			g_set_error (error, g_quark_from_static_string ("gchempaint"), res, gnome_vfs_result_to_string (res));
			return false;
		}
		count -= written;
	}
	return true;
}

void View::ExportImage (string const &filename, const char* type, int resolution)
{
	ArtDRect rect;
	m_pData->GetObjectBounds (m_pDoc, &rect);
	m_pData->ShowSelection (false);
	int w = (int) (ceil (rect.x1) - floor (rect.x0)), h = (int) (ceil (rect.y1) - floor (rect.y0));
	if (!strcmp (type, "eps")) {
/*		GnomePrintConfig* config = gnome_print_config_default();
		GnomePrintContext *pc;
		GnomePrintJob *gpj = gnome_print_job_new (config);
		pc = gnome_print_job_get_context (gpj);
		gnome_print_beginpage(pc, (const guchar*)"");
		gdouble width, height;
		gnome_print_config_get_page_size(config, &width, &height);
		Print(pc, width, height);
		gnome_print_showpage(pc);
		g_object_unref(pc);
		gnome_print_job_close(gpj);
		char *tmpname = g_strdup ("/tmp/2epsXXXXXX");
		int f = g_mkstemp (tmpname);
		close (f);
		double hp, mt, ml;
		GnomePrintUnit const *unit;
		GnomePrintUnit const *inches = gnome_print_unit_get_by_abbreviation ((const guchar *) "in");
		gnome_print_config_get_length (config, (const guchar*) "Settings.Output.Media.PhysicalSize.Height", &hp, &unit);
		gnome_print_convert_distance (&hp, unit, inches);
		hp *= 72;*/
/*		gnome_print_config_get_length (config, (const guchar*) "Settings.Document.Page.Margins.Left", &ml, &unit);
		gnome_print_convert_distance (&ml, unit, inches);
		ml *= 72;
		gnome_print_config_get_length (config, (const guchar*) "Settings.Document.Page.Margins.Top", &mt, &unit);
		gnome_print_convert_distance (&mt, unit, inches);
		mt *= 72;	*/
/*		ml = mt = 30.; // see View::Print !
		gnome_print_config_set_boolean (config, (const guchar*) "Settings.Output.Job.PrintToFile", true);
		gnome_print_config_set (config, (const guchar*) GNOME_PRINT_KEY_OUTPUT_FILENAME, (const guchar*) tmpname);
		gnome_print_job_print (gpj);
		g_object_unref (gpj);
		gnome_print_config_unref(config);
		char buf[256];
		ifstream *fin = new ifstream (tmpname);
		ostringstream fout (filename.c_str ());
		fout << "%!PS-Adobe-3.0 EPSF-3.0" << endl;
		fout << "%%BoundingBox: " << (int) (rect.x0 * .75 + ml) << " " << (int) (hp - mt - rect.y1 * .75)  <<  " "  << (int) (rect.x1 * .75 + ml) << " " << (int) (hp - mt - rect.y0 * .75) << endl;
		fout << "%%HiResBoundingBox: " << rect.x0 * .75 + ml << " " << hp - mt - rect.y1 * .75 <<  " "  << rect.x1 * .75 + ml << " " << hp - mt - rect.y0 * .75 << endl;
		fin->getline (buf, 256);
		while (!fin->eof ()) {
			fin->getline (buf, 256);
			if (strlen (buf) >= 255)
				exit (-1); *//* This is violent but should not occur */
/*			if (!strncmp (buf, "%%", 2)) {
				if (!strncmp (buf + 2, "Orientation", strlen ("Orientation")))
					continue;
				else if (!strncmp (buf + 2, "Pages", strlen ("Pages")))
					continue;
				else if (!strncmp (buf + 2, "BoundingBox", strlen ("BoundingBox")))
					continue;
				else if (!strncmp (buf + 2, "BeginDefaults", strlen ("BeginDefaults")))
					continue;
				else if (!strncmp (buf + 2, "EndDefaults", strlen ("EndDefaults")))
					continue;
				else if (!strncmp (buf + 2, "BeginSetup", strlen ("BeginSetup")))
					continue;
				else if (!strncmp (buf + 2, "EndSetup", strlen ("EndSetup")))
					continue;
				else if (!strncmp (buf + 2, "PageMedia", strlen ("PageMedia")))
					continue;
				else if (!strncmp (buf + 2, "EndProlog", strlen ("EndProlog")))
					continue;
				else if (!strncmp (buf + 2, "BeginResource", strlen ("BeginResource")))
					continue;
				else if (!strncmp (buf + 2, "EndResource", strlen ("EndResource")))
					continue;
				else if (!strncmp (buf + 2, "Page: ", strlen ("Page: ")))
					continue;
				else if (!strncmp (buf + 2, "%%", 2))
					continue;
				else if (!strncmp (buf + 2, "PageResources", strlen ("PageResources")))
					continue;
				else if (!strncmp (buf + 2, "EndComments", strlen ("EndComments"))) {
					 fout << buf << endl;
					if (!filename.compare (filename.length () - 5, 5, ".epsi")) {
						gnome_canvas_update_now (GNOME_CANVAS (m_pWidget));
						GdkPixbuf *pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, w, h);
						GnomeCanvasBuf cbuf;
						int i, j, mask, lines, bytes;
						cbuf.buf = gdk_pixbuf_get_pixels (pixbuf);
						cbuf.rect.x0 = (int) floor (rect.x0);
						cbuf.rect.x1 = (int) ceil (rect.x1);
						cbuf.rect.y0 = (int) floor (rect.y0);
						cbuf.rect.y1 = (int) ceil (rect.y1);
						cbuf.buf_rowstride = gdk_pixbuf_get_rowstride (pixbuf);
						cbuf.bg_color = 0xffffff;
						cbuf.is_buf = 1;
						(* GNOME_CANVAS_ITEM_GET_CLASS (m_pData->Group)->render) (GNOME_CANVAS_ITEM (m_pData->Group), &cbuf);
	*/					/* use 8 bits depth, no more than 250 bytes per line*/
	/*					lines = (w + 249) / 250 * h;
						fout << "%%BeginPreview: " << w << " " << h << " " << 8 << " " << lines << endl;
						fout << hex;
						for (j = 0; j < h; j++) {
							bytes = 0;
							for (i = 0; i < 3 * w; i+= 3) {
								if (bytes == 0)
									fout << "%";
								mask = 0xff - (cbuf.buf[i] + cbuf.buf[i + 1] + cbuf.buf[i + 2]) / 3;
								fout << (mask & 0xf);
								mask = mask >> 4;
								fout << mask;
								bytes++;
								if (bytes == 250) {
									fout << endl;
									bytes = 0;
								}
							}
							if (bytes)
								fout << endl;
							cbuf.buf += cbuf.buf_rowstride;
						}*/
						/* use 1 bit depth, no more than 128 bytes per line (max width 1024) */
/*						lines = (w + 1023) / 1024 * h;
						fout << "%%BeginPreview: " << w << " " << h << " " << 1 << " " << lines << endl;
						fout << hex;
						for (j = 0; j < h; j++) {
							mask = 1; b = 0;
							bytes = 0;
							for (i = 0; i < 3 * w; i+= 3) {
								if (bytes == 0)
									fout << "%";
								if ((cbuf.buf[i] + cbuf.buf[i + 1] + cbuf.buf[i + 2]) / 3 < 0x80)
									b |= mask;
								mask <<= 1;
								if (mask == 16) {
									fout << (int) b;
									mask = 1;
									b = 0;
									bytes++;
									if (bytes == 128) {
										fout << endl;
										bytes = 0;
									}
								}
							}
							if (mask != 1) {
								fout << (int) b;
								bytes++;
							}
							if (bytes)
								fout << endl;
							cbuf.buf += cbuf.buf_rowstride;
						}*/
/*						fout << dec;
						fout << "%%EndPreview" << endl;
						g_object_unref (pixbuf);
					}
				} else if (!strncmp (buf + 2, "BeginProlog", strlen ("BeginProlog"))) {
					 fout << buf << endl;
					 fout << "save" << endl;
					 fout << "countdictstack" << endl;
					 fout << "mark" << endl;
					 fout << "newpath" << endl;
					 fout << "/showpage {} def" << endl;
					 fout << "/setpagedevice {pop} def" << endl;
					 fout << "%%EndProlog" << endl;
					 fout << "%%Page 1 1" << endl;
				} else if (!strncmp (buf + 2, "Trailer", strlen ("Trailer"))) {
					 fout << buf << endl;
					 fout << "cleartomark" << endl;
					 fout << "countdictstack" << endl;
					 fout << "exch sub { end } repeat" << endl;
					 fout << "restore" << endl;
				} else 
					 fout << buf << endl;
			} else
			 fout << buf << endl;
		}
		fin->close ();
		delete fin;
		GnomeVFSHandle *handle = NULL;
		GnomeVFSFileSize n;
		if (gnome_vfs_create (&handle, filename.c_str (), GNOME_VFS_OPEN_WRITE, true, 0644) == GNOME_VFS_OK)
			gnome_vfs_write (handle, fout.str ().c_str (), (GnomeVFSFileSize) fout.str ().size (), &n);*/
	} else if (!strcmp (type, "svg")) {
		xmlDocPtr doc = BuildSVG ();
		xmlIndentTreeOutput = true;
		xmlKeepBlanksDefault (0);
		xmlSaveFormatFile (filename.c_str (), doc, true);
		xmlFreeDoc (doc);
	} else {
		GdkPixbuf *pixbuf = BuildPixbuf (resolution);
		GnomeVFSHandle *handle = NULL;
		if (gnome_vfs_create (&handle, filename.c_str (), GNOME_VFS_OPEN_WRITE, true, 0644) == GNOME_VFS_OK) {
			GError *error = NULL;
			gdk_pixbuf_save_to_callbackv (pixbuf, do_save_image, handle, type, NULL, NULL, &error);
			if (error) {
				cerr << _("Unable to save image file: ") << error->message << endl;
				g_error_free (error);
			}
			gnome_vfs_close (handle); // hope there will be no error there
		}
		g_object_unref (pixbuf);
	}
	m_pData->ShowSelection (true);
}

xmlDocPtr View::BuildSVG ()
{
	ArtDRect rect;
	m_pData->GetObjectBounds (m_pDoc, &rect);
	xmlDocPtr doc = xmlNewDoc ((const xmlChar*)"1.0");
	char *old_num_locale = g_strdup (setlocale (LC_NUMERIC, NULL));
	char *buf;
	setlocale (LC_NUMERIC, "C");
	xmlNewDtd (doc,
		   (const xmlChar*)"svg", (const xmlChar*)"-//W3C//DTD SVG 1.1//EN",
		   (const xmlChar*)"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd");
	xmlDocSetRootElement (doc, xmlNewDocNode (doc, NULL, (const xmlChar*)"svg", NULL));
	xmlNsPtr ns = xmlNewNs (doc->children, (const xmlChar*)"http://www.w3.org/2000/svg", NULL);
	xmlSetNs (doc->children, ns);
	xmlNewProp (doc->children, (const xmlChar*)"version", (const xmlChar*)"1.1");
	rect.x0 = floor (rect.x0);
	rect.y0 = floor (rect.y0);
	rect.x1 = ceil (rect.x1);
	rect.y1 = ceil (rect.y1);
	buf = g_strdup_printf ("%g", rect.x1 - rect.x0);
	xmlNewProp (doc->children, (const xmlChar*)"width", (const xmlChar*)buf);
	g_free (buf);
	buf = g_strdup_printf ("%g", rect.y1 - rect.y0);
	xmlNewProp (doc->children, (const xmlChar*)"height", (const xmlChar*)buf);
	g_free (buf);
	xmlNodePtr node;
	node = xmlNewDocNode (doc, NULL, (const xmlChar*)"rect", NULL);
	xmlAddChild (doc->children, node);
	buf = g_strdup_printf ("%g", rect.x1 - rect.x0);
	xmlNewProp (node, (const xmlChar*)"width", (const xmlChar*)buf);
	g_free (buf);
	buf = g_strdup_printf ("%g", rect.y1 - rect.y0);
	xmlNewProp (node, (const xmlChar*)"height", (const xmlChar*)buf);
	g_free (buf);
	xmlNewProp (node, (const xmlChar*)"stroke", (const xmlChar*)"none");
	xmlNewProp (node, (const xmlChar*)"fill", (const xmlChar*)"white");
	if (rect.x0 != 0. || rect.y0 != 0) {
		node = xmlNewDocNode (doc, NULL, (const xmlChar*)"g", NULL);
		xmlAddChild (doc->children, node);
		buf = g_strdup_printf ("translate(%g,%g)", - rect.x0, - rect.y0);
		xmlNewProp (node, (const xmlChar*)"transform", (const xmlChar*)buf);
		g_free (buf);
	} else
		node = doc->children;
	g_printable_export_svg (G_PRINTABLE (m_pData->Group), doc, node);
	setlocale (LC_NUMERIC, old_num_locale);
	g_free (old_num_locale);
	return doc;
}

GdkPixbuf *View::BuildPixbuf (int resolution)
{
	ArtDRect rect;
	m_pData->GetObjectBounds (m_pDoc, &rect);
	m_pData->ShowSelection (false);
	int w = (int) (ceil (rect.x1) - floor (rect.x0)), h = (int) (ceil (rect.y1) - floor (rect.y0));
	double zoom;
	if (resolution > 0) {
		int screenres = m_pDoc->GetApp ()->GetScreenResolution ();
		zoom = (double) resolution / screenres;
		w = (int) rint ((double) w * zoom);
		h = (int) rint ((double) h * zoom);
	} else
		zoom = 1.;
	gnome_canvas_set_pixels_per_unit (GNOME_CANVAS (m_pWidget), zoom);
	gnome_canvas_update_now (GNOME_CANVAS (m_pWidget));
	GdkPixbuf *pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, w, h);
	gdk_pixbuf_fill (pixbuf, 0xffffffff);
	GnomeCanvasBuf buf;
	buf.buf = gdk_pixbuf_get_pixels (pixbuf);
	buf.rect.x0 = (int) floor (rect.x0 * zoom);
	buf.rect.x1 = (int) ceil (rect.x1 * zoom);
	buf.rect.y0 = (int) floor (rect.y0 * zoom);
	buf.rect.y1 = (int) ceil (rect.y1 * zoom);
	buf.buf_rowstride = gdk_pixbuf_get_rowstride (pixbuf);
	buf.bg_color = 0xffffff;
	buf.is_buf = 1;
	(* GNOME_CANVAS_ITEM_GET_CLASS (m_pData->Group)->render) (GNOME_CANVAS_ITEM (m_pData->Group), &buf);
	// restore zoom level
	gnome_canvas_set_pixels_per_unit (GNOME_CANVAS (m_pWidget), m_pData->Zoom);
	return pixbuf;
}

void View::EnsureSize ()
{
	GnomeCanvas *canvas = GNOME_CANVAS (m_pWidget);
	gnome_canvas_update_now (canvas);
	if (GTK_WIDGET_REALIZED (m_pWidget))
		g_signal_emit_by_name (m_pWidget, "update_bounds");
}

void View::Zoom (double zoom)
{
	m_pData->Zoom = zoom;
	gnome_canvas_set_pixels_per_unit (GNOME_CANVAS (m_pWidget), zoom);
	EnsureSize ();
	// Call OnSize to be certain that the canvas scroll region will be correct
	OnSize (m_pWidget, m_width, m_height);
}

void View::ShowCursor (bool show)
{
	if (m_ActiveRichText)
		g_object_set (G_OBJECT (m_ActiveRichText), "editing", show, NULL);
}

void View::UpdateTheme ()
{
	if (m_sFontName)
		g_free (m_sFontName);
	if (m_sSmallFontName)
		g_free (m_sSmallFontName);
	pango_font_description_free (m_PangoFontDesc);
	pango_font_description_free (m_PangoSmallFontDesc);
	Theme *pTheme = m_pDoc->GetTheme ();
	m_PangoFontDesc = pango_font_description_new ();
	pango_font_description_set_family (m_PangoFontDesc, pTheme->GetFontFamily ());
	pango_font_description_set_style (m_PangoFontDesc, pTheme->GetFontStyle ());
	pango_font_description_set_weight (m_PangoFontDesc, pTheme->GetFontWeight ());
	pango_font_description_set_variant (m_PangoFontDesc, pTheme->GetFontVariant ());
	pango_font_description_set_stretch (m_PangoFontDesc, pTheme->GetFontStretch ());
	pango_font_description_set_size (m_PangoFontDesc, pTheme->GetFontSize ());
	m_sFontName = pango_font_description_to_string (m_PangoFontDesc);
	m_PangoSmallFontDesc = pango_font_description_new ();
	pango_font_description_set_family (m_PangoSmallFontDesc, pTheme->GetFontFamily ());
	pango_font_description_set_style (m_PangoSmallFontDesc, pTheme->GetFontStyle ());
	pango_font_description_set_weight (m_PangoSmallFontDesc, pTheme->GetFontWeight ());
	pango_font_description_set_variant (m_PangoSmallFontDesc, pTheme->GetFontVariant ());
	pango_font_description_set_stretch (m_PangoSmallFontDesc, pTheme->GetFontStretch ());
	pango_font_description_set_size (m_PangoSmallFontDesc, pTheme->GetFontSize () * 2 / 3);
	m_sSmallFontName = pango_font_description_to_string (m_PangoSmallFontDesc);
	Update (m_pDoc);
}

void View::Render (cairo_t *cr)
{
	m_pData->ShowSelection(false);
	Object* pObj = NULL;
	if (m_ActiveRichText) {
		pObj = (Object*) g_object_get_data (G_OBJECT (m_ActiveRichText), "object");
		if (pObj) pObj->SetSelected (m_pWidget, SelStateUnselected);
	}
	g_printable_draw_cairo (G_PRINTABLE (m_pData->Group), cr);
	m_pData->ShowSelection (true);
	if (pObj)
		pObj->SetSelected (m_pWidget, SelStateUpdating);
}

}	//	namespace gcp
