// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * canvas/canvas.cc 
 *
 * Copyright (C) 2008 Jean Bréfort <jean.brefort@normalesup.org>
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

typedef struct {
	GtkDrawingAreaClass base;
	void (* update_bounds) (GccvCanvas *canvas);
} GccvCanvasClass;

GType gccv_canvas_get_type (void);

enum {
  UPDATE_BOUNDS,
  LAST_SIGNAL
};
static guint gccv_canvas_signals[LAST_SIGNAL] = { 0 };

static void gccv_canvas_update_bounds (GccvCanvas *canvas)
{
	canvas->canvas->UpdateBounds ();
}

static void gccv_canvas_class_init (GccvCanvasClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	gccv_canvas_signals[UPDATE_BOUNDS] =
	g_signal_new ("update_bounds",
				  G_TYPE_FROM_CLASS (gobject_class),
				  G_SIGNAL_RUN_LAST,
				  G_STRUCT_OFFSET (GccvCanvasClass, update_bounds),
				  NULL, NULL,
				  g_cclosure_marshal_VOID__VOID,
				  G_TYPE_NONE, 0
				  );
	klass->update_bounds = gccv_canvas_update_bounds;
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
						   GDK_BUTTON_RELEASE_MASK
						   );
	return w;
}

// The C++ class

namespace gccv {

static void on_button_pressed (Canvas *canvas, GdkEventButton *event)
{
	canvas->OnButtonPressed (event);
}

static void on_button_released (Canvas *canvas, GdkEventButton *event)
{
	canvas->OnButtonReleased (event);
}

static void on_motion (Canvas *canvas, GdkEventMotion *event)
{
	canvas->OnMotion (event);
}

static void on_destroy (Canvas *canvas)
{
	delete canvas;
}

static bool on_expose_event (Canvas *canvas, GdkEventExpose *event)
{
	return canvas->OnExpose (event);
}

Canvas::Canvas (Client *client):
	m_Client (client),
	m_Dragging (false),
	m_Root (NULL),
	m_Gap (0.)
{
	m_Root = new Group (this);
	m_Widget = GTK_WIDGET (gccv_canvas_new (this));
	GdkColor color;
	color.red = color.green = color.blue = 0xffff;
	gtk_widget_modify_bg (m_Widget, GTK_STATE_NORMAL, &color);
	g_signal_connect_swapped (G_OBJECT (m_Widget), "button-press-event", G_CALLBACK (on_button_pressed), this);
	g_signal_connect_swapped (G_OBJECT (m_Widget), "button-release-event", G_CALLBACK (on_button_released), this);
	g_signal_connect_swapped (G_OBJECT (m_Widget), "motion-notify-event", G_CALLBACK (on_motion), this);
	g_signal_connect_swapped (G_OBJECT (m_Widget), "destroy", G_CALLBACK (on_destroy), this);
	g_signal_connect_swapped (G_OBJECT (m_Widget), "expose-event", G_CALLBACK (on_expose_event), this);
}

Canvas::~Canvas()
{
	delete m_Root;
}

void Canvas::SetScrollRegion (double xmin, double ymin, double xmax, double ymax)
{
}

bool Canvas::OnButtonPressed (GdkEventButton *event)
{
	Item *item = NULL;
	if (m_Root->Distance (static_cast <double> (event->x), static_cast <double> (event->y), &item) > m_Gap)
		item = NULL;
	ItemClient *client = (item)? item->GetClient (): NULL;
	if (event->button == 1)
		m_Dragging = true;
	return (m_Client)? m_Client->OnButtonPressed (client, event->button, event->x, event->y, event->state): true;
}

bool Canvas::OnButtonReleased (GdkEventButton *event)
{
	Item *item = NULL;
	if (m_Root->Distance (static_cast <double> (event->x), static_cast <double> (event->y), &item) > m_Gap)
		item = NULL;
	ItemClient *client = (item)? item->GetClient (): NULL;
	if (event->button == 1)
		m_Dragging = false;
	return (m_Client)? m_Client->OnButtonReleased (client, event->button, event->x, event->y, event->state): true;
}

bool Canvas::OnMotion (GdkEventMotion *event)
{
	Item *item = NULL;
	if (m_Root->Distance (static_cast <double> (event->x), static_cast <double> (event->y), &item) > m_Gap)
		item = NULL;
	ItemClient *client = (item)? item->GetClient (): NULL;
	return (m_Client)? (m_Dragging? m_Client->OnDrag (client, event->x, event->y, event->state): m_Client->OnMotion (client, event->x, event->y, event->state)): true;
}

void Canvas::Invalidate (double x0, double y0, double x1, double y1)
{
	gtk_widget_queue_draw_area (m_Widget, (int) floor (x0), (int) floor (y0), (int) ceil (x1), (int) ceil (y1));
}

bool Canvas::OnExpose (GdkEventExpose *event)
{
	double x0, y0, x1, y1;
	m_Root->GetBounds (x0, y0, x1, y1);
	if (x0 <= event->area.x + event->area.width && x1 >= event->area.x && y0 <= event->area.y + event->area.height && y1 >= event->area.y) {
		cairo_t *cr = gdk_cairo_create (m_Widget->window);
		m_Root->Draw (cr, event->area.x, event->area.y, event->area.x + event->area.width, event->area.y + event->area.height, false);
		cairo_destroy (cr);
	}
	return true;
}

void Canvas::UpdateBounds ()
{
}

}
