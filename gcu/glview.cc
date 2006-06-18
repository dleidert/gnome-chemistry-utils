// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gcu/glview.cc 
 *
 * Copyright (C) 2006 Jean Bréfort <jean.brefort@normalesup.org>
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
#include <gcu/gldocument.h>
#include <gcu/glview.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <gtk/gtkgl.h>
#include <math.h>

static GdkGLConfig *glconfig = NULL;
double DefaultPsi = 70.;
double DefaultTheta = 10.;
double DefaultPhi = -90.;
using namespace gcu;

// Callbacks
static bool on_init(GtkWidget *widget, GLView* View) 
{
	View->Init ();
	return true;
}

bool on_reshape(GtkWidget *widget, GdkEventConfigure *event, GLView* View) 
{
	View->Reshape ();
	return true;
}

static bool on_draw(GtkWidget *widget, GdkEventExpose *event, GLView* View) 
{
	/* Draw only last expose. */
	if (event->count > 0) return TRUE;

	View->Draw ();
	return true;
}

static bool on_motion(GtkWidget *widget, GdkEventMotion *event, GLView* View) 
{
	View->OnMotion (event);
	return true;
}

static bool on_pressed(GtkWidget *widget, GdkEventButton *event, GLView* View) 
{
	return View->OnPressed (event);
}

// GLView implementation

GLView::GLView (GLDocument* pDoc)
{
	m_bInit = false;
	m_pDoc = pDoc;
	m_nGLList = 0;
	m_Red = m_Green = m_Blue = 0.;
	m_Alpha = 1.;
	m_Angle = 10.;
	SetRotation (DefaultPsi, DefaultTheta, DefaultPhi);
/* Create new OpenGL widget. */
	if (glconfig == NULL)
	{
		/* Check if OpenGL is supported. */
		if (!gdk_gl_query_extension())
		{
			g_print("\n*** OpenGL is not supported.\n");
			exit(1);
		}
	
		/* Configure OpenGL-capable visual. */
	
		/* Try double-buffered visual */
		glconfig = gdk_gl_config_new_by_mode (GdkGLConfigMode (GDK_GL_MODE_RGB |
											GDK_GL_MODE_DEPTH |
											GDK_GL_MODE_DOUBLE));
		if (glconfig == NULL)
		{
			g_print("*** Cannot find the double-buffered visual.\n");
			  exit(1);
		}
	}
	/* create new OpenGL widget */
	m_pWidget = GTK_WIDGET(gtk_drawing_area_new());

	/* Set OpenGL-capability to the widget. */
	gtk_widget_set_gl_capability(m_pWidget,
                                glconfig,
                                NULL,
                                TRUE,
                                GDK_GL_RGBA_TYPE);

	gtk_widget_set_events(GTK_WIDGET(m_pWidget),
			GDK_EXPOSURE_MASK|
			GDK_BUTTON_MOTION_MASK|
			GDK_POINTER_MOTION_HINT_MASK|
			GDK_BUTTON_PRESS_MASK);

// Connect signal handlers
// Do initialization when widget has been realized.
	g_signal_connect (G_OBJECT (m_pWidget), "realize",
		     G_CALLBACK (on_init), this);
// When window is resized viewport needs to be resized also.
	g_signal_connect (G_OBJECT (m_pWidget), "configure_event",
		     G_CALLBACK (on_reshape), this);
// Redraw image when exposed. 
	g_signal_connect (G_OBJECT (m_pWidget), "expose_event",
		     G_CALLBACK (on_draw), this);
// When moving mouse 
  g_signal_connect (G_OBJECT (m_pWidget), "motion_notify_event",
		      G_CALLBACK (on_motion), this);
// When a mouse button is pressed
  g_signal_connect (G_OBJECT (m_pWidget), "button_press_event",
		      G_CALLBACK (on_pressed), this);

	gtk_widget_show (GTK_WIDGET (m_pWidget));
}

GLView::~GLView ()
{
}

void GLView::Init ()
{
	GdkGLContext *glcontext = gtk_widget_get_gl_context (m_pWidget);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (m_pWidget);
	if (gdk_gl_drawable_gl_begin (gldrawable, glcontext)) {
	    glEnable (GL_LIGHTING);
		glEnable (GL_LIGHT0);
		glEnable (GL_DEPTH_TEST);
		glEnable (GL_CULL_FACE);
		glEnable (GL_COLOR_MATERIAL);
		float shiny = 25.0, spec[4] = {1.0, 1.0, 1.0, 1.0};
		glMaterialfv (GL_FRONT_AND_BACK, GL_SHININESS, &shiny);
		glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, spec);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		m_bInit = true;
		gdk_gl_drawable_gl_end (gldrawable);
		Update ();
    }
}

void GLView::Reshape ()
{
	if (!m_bInit)
		return;
	float fAspect;
	GdkGLContext *glcontext = gtk_widget_get_gl_context (m_pWidget);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (m_pWidget);
	if (gdk_gl_drawable_gl_begin (gldrawable, glcontext)) {
		if (m_pWidget->allocation.height) {
			fAspect = (GLfloat) m_pWidget->allocation.width / (GLfloat) m_pWidget->allocation.height;
			if (fAspect == 0.0)
				fAspect = 1.0;
		} else	// don't divide by zero, not that we should ever run into that...
			fAspect = 1.0f;
		double x = m_pDoc->GetMaxDist ();
		if (x == 0)
			x = 1;
		m_Radius = (float) (x / sin (m_Angle / 360 * M_PI)) ;
		glViewport (0,0, m_pWidget->allocation.width, m_pWidget->allocation.height);
		if (fAspect > 1.0) {
			m_Height = x * (1 - tan (m_Angle / 360 * M_PI));
			m_Width = m_Height * fAspect;
		} else {
			m_Width = x * (1 - tan (m_Angle / 360 * M_PI));
			m_Height = m_Width / fAspect;
		}
		m_Near = m_Radius - x;
		m_Far = m_Radius + x;
	    glMatrixMode (GL_PROJECTION);
	    glLoadIdentity();
		glFrustum (- m_Width, m_Width, - m_Height, m_Height, m_Near , m_Far);
	    glMatrixMode (GL_MODELVIEW);
		glLoadIdentity ();
		glTranslatef (0, 0, -m_Radius);
		gdk_gl_drawable_gl_end (gldrawable);
	}
}

void GLView::Draw ()
{
	if (!m_bInit)
		return;
	GdkGLContext *glcontext = gtk_widget_get_gl_context (m_pWidget);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (m_pWidget);
	if (gdk_gl_drawable_gl_begin (gldrawable, glcontext)) {
		glClearColor (m_Red, m_Green, m_Blue, m_Alpha);
		glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		if (m_nGLList) {
			glPushMatrix ();
			glRotated (m_Psi, 0.0, 1.0, 0.0);
			glRotated (m_Theta, 0.0, 0.0, 1.0);
			glRotated (m_Phi, 0.0, 1.0, 0.0);
			glEnable (GL_BLEND);
			glCallList (m_nGLList);
			glDisable (GL_BLEND);
			glPopMatrix ();
		}
		gdk_gl_drawable_gl_end (gldrawable);
		/* Swap backbuffer to front */
		gdk_gl_drawable_swap_buffers (gldrawable);
    }
}

void GLView::Update()
{
	if (!m_bInit)
		return;
	GdkGLContext *glcontext = gtk_widget_get_gl_context (m_pWidget);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (m_pWidget);
	if (gdk_gl_drawable_gl_begin (gldrawable, glcontext)) {
		if (m_nGLList)
			glDeleteLists (m_nGLList,1);
		m_nGLList = glGenLists (1);
		glNewList (m_nGLList, GL_COMPILE);
		m_pDoc->Draw ();
		glEndList ();
		gdk_gl_drawable_gl_end (gldrawable);
	}
	Reshape ();
	Draw ();
}

void GLView::SetRotation(double psi, double theta, double phi)
{
	m_Psi = psi;
	m_Theta = theta;
	m_Phi = phi;
	Matrix m (m_Psi / 180 * M_PI, m_Theta / 180 * M_PI, m_Phi / 180 * M_PI, euler);
	m_Euler = m;
}

bool GLView::OnPressed (GdkEventButton *event)
{
  if (event->button == 1) {
    // beginning of drag, reset mouse position
    m_Lastx = event->x;
    m_Lasty = event->y;
    return true;
  }
  return false;
}

void GLView::OnMotion(GdkEventMotion *event)
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
		if ((x == m_Lastx) && (y == m_Lasty))
			return;
		Rotate (x - m_Lastx, y - m_Lasty);
		m_Lastx = x;
		m_Lasty = y;
		gtk_widget_queue_draw_area(m_pWidget, 0, 0, m_pWidget->allocation.width, m_pWidget->allocation.height);
	}
}

void GLView::Rotate (gdouble x, gdouble y)
{
	gdouble z = sqrt (x*x + y*y);
	Matrix Mat (0, (y > 0) ? - acos (x/z) : acos (x/z), z * M_PI / 900., rotation);
	m_Euler = Mat * m_Euler;
	m_Euler.Euler (m_Psi, m_Theta, m_Phi);
	m_Psi /= M_PI / 180.;
	m_Theta /= M_PI / 180.;
	m_Phi /= M_PI / 180.;
}

void GLView::Print (GnomePrintContext *pc, gdouble width, gdouble height)
{
	int Width = m_pWidget->allocation.width;
	int Height = m_pWidget->allocation.height;
	if (Width > width) {
		Height = (int) (Height * width / Width);
		Width = (int) width;
	}
	if (Height > height) {
		Width = (int) (Width * height / Height);
		Height = (int) height;
	}
	GdkGLConfig *glconfig = gdk_gl_config_new_by_mode (
		GdkGLConfigMode (GDK_GL_MODE_RGB | GDK_GL_MODE_DEPTH));
	double matrix [6] = {Width, 0, 0, Height,
						(width - Width) / 2, (height - Height )/ 2};
	int w= (int) (Width * 300. / 72.), h = (int) (Height * 300. / 72.);
	GdkPixmap *pixmap = gdk_pixmap_new (
			(GdkDrawable*) (m_pWidget->window),
			w, h, -1);
	GdkGLPixmap *gl_pixmap = gdk_pixmap_set_gl_capability (pixmap,
							       glconfig,
							       NULL );
	GdkGLDrawable * drawable = gdk_pixmap_get_gl_drawable (pixmap);
	GdkGLContext * context = gdk_gl_context_new (drawable,
						     NULL,
						     FALSE,
						     GDK_GL_RGBA_TYPE);
	if (gdk_gl_drawable_gl_begin (drawable, context)) {
	    glEnable (GL_LIGHTING);
		glEnable (GL_LIGHT0);
		glEnable (GL_DEPTH_TEST);
		glEnable (GL_CULL_FACE);
		glEnable (GL_COLOR_MATERIAL);
		float shiny = 25.0, spec[4] = {1.0, 1.0, 1.0, 1.0};
		glMaterialfv (GL_FRONT_AND_BACK, GL_SHININESS, &shiny);
		glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, spec);
		glViewport (0, 0, w, h);
	    glMatrixMode (GL_PROJECTION);
	    glLoadIdentity ();
		glFrustum(- m_Width, m_Width, - m_Height, m_Height, m_Near , m_Far);
	    glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glTranslatef(0, 0, - m_Radius);
		glClearColor (m_Red, m_Green, m_Blue, m_Alpha);
		glClearDepth (1.0);
		glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glPushMatrix ();
		glRotated (m_Psi, 0.0, 1.0, 0.0);
		glRotated (m_Theta, 0.0, 0.0, 1.0);
		glRotated (m_Phi, 0.0, 1.0, 0.0);
		m_pDoc->Draw ();
		glPopMatrix ();
		glFlush ();
		gdk_gl_drawable_gl_end (drawable);
		GdkPixbuf* pixbuf = gdk_pixbuf_get_from_drawable    (NULL,
			(GdkDrawable*) pixmap, NULL, 0, 0, 0, 0, -1, -1);
		gnome_print_gsave (pc);
		gnome_print_concat (pc, matrix);
		gnome_print_rgbimage (pc, (const guchar*) gdk_pixbuf_get_pixels (pixbuf), w, h, gdk_pixbuf_get_rowstride (pixbuf));
		gnome_print_grestore (pc);
		g_object_unref (pixbuf);
	}

	gdk_gl_context_destroy (context);
	gdk_gl_pixmap_destroy (gl_pixmap);
	// destroying pixmap gives a CRITICAL and destroying glconfig leeds to a crash.
}
