// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gccv/canvas.cc
 *
 * Copyright (C) 2008-2014 Jean Bréfort <jean.brefort@normalesup.org>
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

#include "config.h"
#include "canvas.h"
#include "client.h"
#include "group.h"
#include "item-client.h"
#include <cmath>

#include <gsf/gsf-impl-utils.h>

// The gtk+ widget

#define GCCV_CANVAS_TYPE	(gccv_canvas_get_type ())
#define GCCV_CANVAS(o)	(G_TYPE_CHECK_INSTANCE_CAST ((o), GCCV_CANVAS_TYPE, GccvCanvas))
#define GCCV_IS_CANVAS(o)	(G_TYPE_CHECK_INSTANCE_TYPE ((o), GCCV_CANVAS_TYPE))

typedef struct {
	GtkDrawingArea base;

	gccv::Canvas *canvas;
} GccvCanvas;

typedef GtkDrawingAreaClass GccvCanvasClass;

GType gccv_canvas_get_type (void);

static void gccv_canvas_class_init (GObjectClass *klass)
{
#if GTK_CHECK_VERSION(3,20,0)
	gtk_widget_class_set_css_name (reinterpret_cast < GtkWidgetClass * > (klass), "canvas");
#endif
}

GSF_CLASS (GccvCanvas, gccv_canvas,
	   gccv_canvas_class_init, NULL,
	   gtk_drawing_area_get_type ())

static GtkWidget *gccv_canvas_new (gccv::Canvas *owner)
{
	GccvCanvas *canvas = GCCV_CANVAS (g_object_new (GCCV_CANVAS_TYPE, NULL));
	canvas->canvas = owner;
	GtkWidget *w = GTK_WIDGET (canvas);
	gtk_widget_add_events (w,
						   GDK_POINTER_MOTION_MASK |
						   GDK_BUTTON_MOTION_MASK |
						   GDK_BUTTON_PRESS_MASK |
						   GDK_BUTTON_RELEASE_MASK |
    					   GDK_LEAVE_NOTIFY_MASK
						   );
	return w;
}

// The C++ class

namespace gccv {
class CanvasPrivate {
public:
	// Event related functions
	static bool OnButtonPressed (Canvas *canvas, GdkEventButton *event);
	static bool OnButtonReleased (Canvas *canvas, GdkEventButton *event);
	static bool OnMotion (Canvas *canvas, GdkEventMotion *event);
	static bool OnDraw (Canvas *canvas, cairo_t *cr);
	static bool OnLeaveNotify (Canvas *canvas, GdkEventCrossing *event);
};


bool CanvasPrivate::OnDraw (Canvas *canvas, cairo_t *cr)
{
	double x0, y0, x1, y1;
	GdkEvent *ev = gtk_get_current_event ();
	canvas->m_Root->GetBounds (x0, y0, x1, y1);
	cairo_save (cr);
	cairo_scale (cr, canvas->m_Zoom, canvas->m_Zoom);
	// get the default color
	GtkStateFlags state = gtk_widget_get_state_flags (canvas->m_Widget);
	GtkStyleContext *ctxt = gtk_widget_get_style_context (canvas->m_Widget);
	GdkRGBA rgba;
	gtk_style_context_get_color (ctxt, state, &rgba);
	canvas->m_Color = GO_COLOR_FROM_GDK_RGBA (rgba);
	gtk_style_context_get (ctxt, state, GTK_STYLE_PROPERTY_FONT, &canvas->m_Font, NULL);
	if (ev && ev->type == GDK_EXPOSE) {
		double clip_x0, clip_y0, clip_x1, clip_y1;
		cairo_clip_extents (cr, &clip_x0, &clip_y0, &clip_x1, &clip_y1);
		if (canvas->m_BackgroundColor != 0) {
			cairo_save (cr);
			cairo_set_source_rgba (cr, GO_COLOR_TO_CAIRO (canvas->m_BackgroundColor));
			cairo_rectangle (cr, clip_x0, clip_y0, clip_x1 - clip_x0, clip_y1 - clip_y0);
			cairo_fill (cr);
			cairo_restore (cr);
		} else
			gtk_render_background (ctxt, cr, clip_x0, clip_y0, clip_x1 - clip_x0, clip_y1 - clip_y0);
		if (x0 <= clip_x1 && x1 >= clip_x0 && y0 <= clip_y1 && y1 >= clip_y0)
			canvas->m_Root->Draw (cr, clip_x0, clip_y0, clip_x1, clip_y1, false);
	} else {
		if (canvas->m_BackgroundColor != 0) {
			cairo_save (cr);
			cairo_set_source_rgba (cr, GO_COLOR_TO_CAIRO (canvas->m_BackgroundColor));
			cairo_rectangle (cr, x0, y0, x1 - x0, y1 - y0);
			cairo_fill (cr);
			cairo_restore (cr);
		}
		canvas->m_Root->Draw (cr, x0, y0, x1, y1, true);
	}
	cairo_restore (cr);
	pango_font_description_free (canvas->m_Font);
	canvas->m_Font = NULL;
	return true;
}

bool CanvasPrivate::OnButtonPressed (Canvas *canvas, GdkEventButton *event)
{
	Item *item = NULL;
	canvas->m_LastEventState = event->state;
	double x = event->x / canvas->m_Zoom, y = event->y / canvas->m_Zoom;
	if (canvas->m_Root->Distance (x, y, &item) > canvas->m_Gap)
		item = NULL;
	ItemClient *client = (item)? item->GetClient (): NULL;
	if (event->button == 1)
		canvas->m_Dragging = true;
	return (canvas->m_Client)? canvas->m_Client->OnButtonPressed (client, event->button, x, y, event->state): true;
}

bool CanvasPrivate::OnButtonReleased (Canvas *canvas, GdkEventButton *event)
{
	Item *item = NULL;
	canvas->m_LastEventState = event->state;
	double x = event->x / canvas->m_Zoom, y = event->y / canvas->m_Zoom;
	if (canvas->m_Root->Distance (x, y, &item) > canvas->m_Gap)
		item = NULL;
	ItemClient *client = (item)? item->GetClient (): NULL;
	if (event->button == 1)
		canvas->m_Dragging = false;
	return (canvas->m_Client)? canvas->m_Client->OnButtonReleased (client, event->button, x, y, event->state): true;
}

bool CanvasPrivate::OnMotion (Canvas *canvas, GdkEventMotion *event)
{
	Item *item = NULL;
	canvas->m_LastEventState = event->state;
	double x = event->x / canvas->m_Zoom, y = event->y / canvas->m_Zoom;
	if (canvas->m_Root->Distance (x, y, &item) > canvas->m_Gap)
		item = NULL;
	ItemClient *client = (item)? item->GetClient (): NULL;
	return (canvas->m_Client)? (canvas->m_Dragging? canvas->m_Client->OnDrag (client, x, y, event->state): canvas->m_Client->OnMotion (client, x, y, event->state)): true;
}

bool CanvasPrivate::OnLeaveNotify (Canvas *canvas, GdkEventCrossing *event)
{
	canvas->m_LastEventState = event->state;
	return (canvas->m_Client)? canvas->m_Client->OnLeaveNotify (event->state): true;
}

static void on_destroy (Canvas *canvas)
{
	delete canvas;
}

Canvas::Canvas (Client *client):
	m_Client (client),
	m_Dragging (false),
	m_Zoom (1.),
	m_Root (NULL),
	m_Gap (0.),
	m_Font (NULL),
	m_BackgroundColor (0)
{
	m_Root = new Group (this);
	m_Widget = GTK_WIDGET (gccv_canvas_new (this));
	g_signal_connect_swapped (G_OBJECT (m_Widget), "button-press-event", G_CALLBACK (CanvasPrivate::OnButtonPressed), this);
	g_signal_connect_swapped (G_OBJECT (m_Widget), "button-release-event", G_CALLBACK (CanvasPrivate::OnButtonReleased), this);
	g_signal_connect_swapped (G_OBJECT (m_Widget), "motion-notify-event", G_CALLBACK (CanvasPrivate::OnMotion), this);
	g_signal_connect_swapped (G_OBJECT (m_Widget), "leave-notify-event", G_CALLBACK (CanvasPrivate::OnLeaveNotify), this);
	g_signal_connect_swapped (G_OBJECT (m_Widget), "destroy", G_CALLBACK (on_destroy), this);
	g_signal_connect_swapped (G_OBJECT (m_Widget), "draw", G_CALLBACK (CanvasPrivate::OnDraw), this);
}

Canvas::~Canvas()
{
	delete m_Root;
}

Item *Canvas::GetItemAt (double x, double y)
{
	Item *item = NULL;
	return (m_Root->Distance (x, y, &item) <= m_Gap)? item: NULL;
}


void Canvas::Invalidate (double x0, double y0, double x1, double y1)
{
	if (x0 < 0) {
		x0 = 0;
		if (x1 < 0.)
			x1 = 0.;
	}
	if (y0 < 0) {
		y0 = 0;
		if (y1 < 0.)
			y1 = 0.;
	}
	if (x1 > x0 && y1 > y0)
		gtk_widget_queue_draw_area (m_Widget, (int) floor (x0 * m_Zoom), (int) floor (y0 * m_Zoom), (int) (ceil (x1 * m_Zoom) - floor (x0 * m_Zoom)), (int) (ceil (y1 * m_Zoom) - floor (y0 * m_Zoom)));
}

void Canvas::SetBackgroundColor (GOColor color)
{
	m_BackgroundColor = color;
}

void Canvas::SetZoom (double zoom)
{
	m_Root->Invalidate ();
	m_Zoom = zoom;
	m_Root->Invalidate ();
}

void Canvas::Render (cairo_t* cr, bool is_vector)
{
	double x0, y0, x1, y1;
	m_Root->GetBounds (x0, y0, x1, y1);
	m_Root->Draw (cr, x0, y0, x1, y1, is_vector);
}

}

