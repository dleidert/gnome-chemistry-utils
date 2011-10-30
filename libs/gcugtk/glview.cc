// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gcugtk/glview.cc
 *
 * Copyright (C) 2006-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "application.h"
#include "glview.h"
#include <gcu/gldocument.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <gdk/gdkx.h>
#include <cstring>

#define ROOTDIR "/apps/gchemutils/gl/"

namespace gcugtk {

#define GCU_CONF_DIR_GL "gl"

class GLViewPrivate
{
public:
	static bool OnInit (GLView* View);
	static bool OnReshape (GLView* View, GdkEventConfigure *event);
	static bool OnDraw (GLView* View, cairo_t *cr);
	static bool OnMotion (G_GNUC_UNUSED GtkWidget *widget, GdkEventMotion *event, GLView* View);
	static bool OnPressed (G_GNUC_UNUSED GtkWidget *widget, GdkEventButton *event, GLView* View);
};

// Callbacks
bool GLViewPrivate::OnInit (GLView* View)
{
	// Initialize the GLX stuff
	gtk_widget_set_double_buffered (View->m_Widget, false);
	View->m_Window = gtk_widget_get_window (View->m_Widget);
	int const attr_list[] = {
		GLX_RGBA,
		GLX_DOUBLEBUFFER,
		GLX_RED_SIZE, 1,
		GLX_GREEN_SIZE, 1,
		GLX_BLUE_SIZE, 1,
		GLX_DEPTH_SIZE, 1,
		0
	};
	View->m_VisualInfo = glXChooseVisual (GDK_WINDOW_XDISPLAY (View->m_Window), gdk_screen_get_number (gdk_window_get_screen (View->m_Window)), const_cast < int * > (attr_list));
	View->m_Context = glXCreateContext (GDK_WINDOW_XDISPLAY (View->m_Window), View->m_VisualInfo, NULL, true);
	if (View->GLBegin ()) {
	    glEnable (GL_LIGHTING);
		glEnable (GL_LIGHT0);
		glEnable (GL_DEPTH_TEST);
		glEnable (GL_CULL_FACE);
		glEnable (GL_COLOR_MATERIAL);
		float shiny = 25.0, spec[4] = {1.0, 1.0, 1.0, 1.0};
		glMaterialfv (GL_FRONT_AND_BACK, GL_SHININESS, &shiny);
		glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, spec);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glShadeModel (GL_SMOOTH);
		glPolygonMode (GL_FRONT, GL_FILL);
		glEnable(GL_BLEND);
		View->m_bInit = true;
		View->GLEnd ();
		View->Update ();
	}
	return true;
}

bool GLViewPrivate::OnReshape (GLView* View, GdkEventConfigure *event)
{
	View->Reshape (event->width, event->height);
	return true;
}

bool GLViewPrivate::OnDraw (GLView* View, G_GNUC_UNUSED cairo_t *cr)
{
	// Draw only last expose.
	GdkEventExpose *event = reinterpret_cast < GdkEventExpose * > (gtk_get_current_event ());
	if (event && event->type == GDK_EXPOSE && event->count > 0)
		return TRUE;

	if (!View->m_bInit)
		return true;
	if (View->GLBegin ()) {
		glClearColor (View->GetRed (), View->GetGreen (), View->GetBlue (), View->GetAlpha ());
		glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		View->m_Doc->Draw (View->m_Euler);
		// Swap backbuffer to front
		// FIXME: make this compatible with non X11 backends
		glXSwapBuffers (GDK_WINDOW_XDISPLAY (View->m_Window), GDK_WINDOW_XID (View->m_Window));
		View->GLEnd ();
	}
	return true;
}

bool GLViewPrivate::OnMotion (G_GNUC_UNUSED GtkWidget *widget, GdkEventMotion *event, GLView* View)
{
	gint x, y;
	GdkModifierType state;

	if (event->is_hint)
		gdk_window_get_pointer (event->window, &x, &y, &state);
	else {
	    x = (gint) event->x;
	    y = (gint) event->y;
	    state = (GdkModifierType) event->state;
	}
	if (state & GDK_BUTTON1_MASK) {
		if ((x == View->m_Lastx) && (y == View->m_Lasty))
			return false;
		View->m_Doc->SetDirty (true);
		View->Rotate (x - View->m_Lastx, y - View->m_Lasty);
		View->m_Lastx = x;
		View->m_Lasty = y;
		gtk_widget_queue_draw_area(View->m_Widget, 0, 0, View->m_WindowWidth, View->m_WindowHeight);
	}
	return true;
}

bool GLViewPrivate::OnPressed (G_GNUC_UNUSED GtkWidget *widget, GdkEventButton *event, GLView* View)
{
  if (event->button == 1) {
    // beginning of drag, reset mouse position
    View->m_Lastx = event->x;
    View->m_Lasty = event->y;
    return true;
  }
  return false;
}

GLView::GLView (gcu::GLDocument* pDoc) throw (std::runtime_error): gcu::GLView (pDoc), Printable ()
{
	m_bInit = false;
/* Create new OpenGL widget. */
	static bool inited = false;
	if (!inited) {
		inited = true;
		/* Check if OpenGL is supported. */
		if (!glXQueryExtension (GDK_DISPLAY_XDISPLAY (gdk_display_get_default ()), NULL, NULL))
			throw  std::runtime_error ("*** OpenGL is not supported.\n");

	}
	/* create new widget */
	m_Widget = GTK_WIDGET (gtk_drawing_area_new ());

	gtk_widget_set_events (GTK_WIDGET (m_Widget),
		GDK_EXPOSURE_MASK |
		GDK_POINTER_MOTION_MASK |
		GDK_POINTER_MOTION_HINT_MASK |
		GDK_BUTTON_PRESS_MASK |
	    GDK_BUTTON_RELEASE_MASK);

	// Connect signal handlers
	// Do initialization when widget has been realized.
	g_signal_connect_swapped (G_OBJECT (m_Widget), "realize",
				G_CALLBACK (GLViewPrivate::OnInit), this);
	// When window is resized viewport needs to be resized also.
	g_signal_connect_swapped (G_OBJECT (m_Widget), "configure_event",
				G_CALLBACK (GLViewPrivate::OnReshape), this);
	// Redraw image when exposed.
	g_signal_connect_swapped (G_OBJECT (m_Widget), "draw",
				G_CALLBACK (GLViewPrivate::OnDraw), this);
	// When moving mouse
	g_signal_connect (G_OBJECT (m_Widget), "motion_notify_event",
				G_CALLBACK (GLViewPrivate::OnMotion), this);
	// When a mouse button is pressed
	g_signal_connect (G_OBJECT (m_Widget), "button_press_event",
				G_CALLBACK (GLViewPrivate::OnPressed), this);

	gtk_widget_show (GTK_WIDGET (m_Widget));
	SetHasBackground (true);
	m_Window = NULL;
}

GLView::~GLView ()
{
	glXDestroyContext (GDK_WINDOW_XDISPLAY (m_Window), m_Context);
	XFree (m_VisualInfo);
}

void GLView::Update()
{
	if (!m_bInit)
		return;
	if (GLBegin ()) {
		m_Doc->Draw (m_Euler);
		GLEnd ();
    }
	Reshape (m_WindowWidth, m_WindowHeight);
	gtk_widget_queue_draw (m_Widget);
}

void GLView::Reshape (int width, int height)
{
	m_WindowWidth = width;
	m_WindowHeight = height;
	if (!m_bInit)
		return;
	float fAspect;
	if (GLBegin ()) {
		if (height) {
			fAspect = (GLfloat)width / (GLfloat) height;
			if (fAspect == 0.0)
				fAspect = 1.0;
		} else	// don't divide by zero, not that we should ever run into that...
			fAspect = 1.0f;
		double x = m_Doc->GetMaxDist ();
		if (x == 0)
			x = 1;
		glViewport (0,0, width, height);
		if (fAspect > 1.0) {
			m_Height = x * (1 - tan (GetAngle () / 360 * M_PI));
			m_Width = m_Height * fAspect;
		} else {
			m_Width = x * (1 - tan (GetAngle () / 360 * M_PI));
			m_Height =m_Width / fAspect;
		}
		glMatrixMode (GL_PROJECTION);
		glLoadIdentity();
		if (GetAngle () > 0.) {
			m_Radius = (float) (x / sin (GetAngle () / 360 * M_PI)) ;
			m_Near = m_Radius - x;
			m_Far = m_Radius + x;
			glFrustum (-m_Width, m_Width, -m_Height, m_Height, m_Near, m_Far);
		} else {
			m_Radius = 2 * x;
			m_Near = m_Radius - x;
			m_Far = m_Radius + x;
			glOrtho (-m_Width, m_Width, -m_Height, m_Height, m_Near, m_Far);
		}
		glMatrixMode (GL_MODELVIEW);
		glLoadIdentity ();
		glTranslatef (0, 0, -m_Radius);
		GLEnd ();
	}
}

void GLView::DoPrint (G_GNUC_UNUSED GtkPrintOperation *print, GtkPrintContext *context, G_GNUC_UNUSED int page) const
{
	cairo_t *cr;
	gdouble width, height;

	cr = gtk_print_context_get_cairo_context (context);
	width = gtk_print_context_get_width (context);
	height = gtk_print_context_get_height (context);
	int w, h; // size in points
	w = m_WindowWidth;
	h = m_WindowHeight;
	switch (GetScaleType ()) {
	case GCU_PRINT_SCALE_NONE:
		break;
	case GCU_PRINT_SCALE_FIXED:
		w *= GetScale ();
		h *= GetScale ();
		break;
	case GCU_PRINT_SCALE_AUTO:
		if (GetHorizFit ())
			w = width;
		if (GetVertFit ())
			h = height;
		break;
	}
	double scale = 72. / 300.;
	GdkPixbuf *pixbuf = BuildPixbuf (w / scale, h / scale, GetPrintBackground ());
	GOImage *img = GO_IMAGE (go_pixbuf_new_from_pixbuf (pixbuf));
	double x = 0., y = 0.;
	if (GetHorizCentered ())
		x = (width - w) / 2.;
	if (GetVertCentered ())
		y = (height - h) / 2.;
	cairo_scale (cr, scale, scale);
	cairo_translate (cr, x, y);
	go_image_draw (img, cr);
	g_object_unref (img);
	g_object_unref (pixbuf);
}

GdkPixbuf *GLView::BuildPixbuf (unsigned width, unsigned height, bool use_bg) const
{
	GdkPixbuf *pixbuf = NULL;
	// Create the pixmap
	int const attr_list[] = {
		GLX_RGBA,
		GLX_RED_SIZE, 1,
		GLX_GREEN_SIZE, 1,
		GLX_BLUE_SIZE, 1,
		GLX_ALPHA_SIZE, 1,
		GLX_DEPTH_SIZE, 1,
		0
	};
	GdkWindow *window = (m_Window)? m_Window: gdk_get_default_root_window ();
	XVisualInfo *xvi = glXChooseVisual (GDK_WINDOW_XDISPLAY (window), gdk_screen_get_number (gdk_window_get_screen (window)), const_cast < int * > (attr_list));
	Pixmap pixmap = XCreatePixmap (GDK_WINDOW_XDISPLAY (window), GDK_WINDOW_XID (window), width, height, xvi->depth);
	GLXContext ctxt = glXCreateContext (GDK_WINDOW_XDISPLAY (window), xvi, NULL, false);
	GLXPixmap glxp = glXCreateGLXPixmap (GDK_WINDOW_XDISPLAY (window), xvi, pixmap);
	// draw
	if (glXMakeCurrent (GDK_WINDOW_XDISPLAY (window), glxp, ctxt)) {
		double aspect = (GLfloat) width / height;
		double x = m_Doc->GetMaxDist (), w, h;
		if (x == 0)
			x = 1;
		if (aspect > 1.0) {
			h = x * (1 - tan (GetAngle () / 360 * M_PI));
			w = h * aspect;
		} else {
			w = x * (1 - tan (GetAngle () / 360 * M_PI));
			h = w / aspect;
		}
	    glEnable (GL_LIGHTING);
		glEnable (GL_LIGHT0);
		glEnable (GL_DEPTH_TEST);
		glEnable (GL_CULL_FACE);
		glEnable (GL_COLOR_MATERIAL);
		float shiny = 25.0, spec[4] = {1.0, 1.0, 1.0, 1.0};
		glMaterialfv (GL_FRONT_AND_BACK, GL_SHININESS, &shiny);
		glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, spec);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glShadeModel (GL_SMOOTH);
		glPolygonMode (GL_FRONT, GL_FILL);
		glEnable(GL_BLEND);
		glViewport (0, 0, width, height);
	    glMatrixMode (GL_PROJECTION);
	    glLoadIdentity ();
		GLfloat radius, near, far;
		if (GetAngle () > 0.) {
			radius = (float) (x / sin (GetAngle () / 360 * M_PI)) ;
			near = radius - x;
			far = radius + x;
			glFrustum (- w, w, - h, h, near, far);
		} else {
			radius = 2 * x;
			near = radius - x;
			far = radius + x;
			glOrtho (- w, w, - h, h, near, far);
		}
	    glMatrixMode (GL_MODELVIEW);
		glLoadIdentity ();
		glTranslatef (0, 0, -radius);
		if (use_bg)
			glClearColor (GetRed (), GetGreen (), GetBlue (), GetAlpha ());
		else
			glClearColor (0., 0., 0., 0.);
		glClearDepth (1.0);
		glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable (GL_BLEND);
		GetDoc ()->Draw(m_Euler);
		glDisable (GL_BLEND);
		glFlush ();
	}
	// get the pixels and copy to a GdkPixbuf
	XImage *image = XGetImage (GDK_WINDOW_XDISPLAY (window), pixmap, 0, 0, width, height, AllPlanes, ZPixmap);
	unsigned i = 4 * width * height, j;
	guchar *data = reinterpret_cast < guchar * > (g_malloc (i)), *dst = data;
	guchar *src_line = reinterpret_cast < guchar * > (image->data), *src;
	// FIXME: we might have bit or byte order issues there, looks like the XImage is in BGRA format
	for (j = 0; j < height; j++) {
		src = src_line;
		src_line += image->bytes_per_line;
		for (i = 0; i < width; i++) {
			dst[2] = *(src++);
			dst[1] = *(src++);
			dst[0] = *(src++);
			dst[3] = *(src++);
			dst += 4;
		}
	}
	pixbuf = gdk_pixbuf_new_from_data (data, GDK_COLORSPACE_RGB, true, 8, width, height, 4 * width, reinterpret_cast < GdkPixbufDestroyNotify > (g_free), NULL);
	// reset the current context
	glXMakeCurrent (GDK_WINDOW_XDISPLAY (window), None, NULL);
	// now free things
	XDestroyImage (image);
	// now free things
	glXDestroyGLXPixmap (GDK_WINDOW_XDISPLAY (window), glxp);
	glXDestroyContext (GDK_WINDOW_XDISPLAY (window), ctxt);
	XFree (xvi);
	XFreePixmap (GDK_WINDOW_XDISPLAY (window), pixmap);
	return pixbuf;
}

bool GLView::GLBegin ()
{
	return glXMakeCurrent (GDK_WINDOW_XDISPLAY (m_Window), GDK_WINDOW_XID (m_Window), m_Context);
}

void GLView::GLEnd ()
{
	glXMakeCurrent (GDK_WINDOW_XDISPLAY (m_Window), None, NULL);
}

}	//	namespace gcugtk
