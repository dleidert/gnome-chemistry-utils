// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gcu/glview.cc 
 *
 * Copyright (C) 2006-2007 Jean Bréfort <jean.brefort@normalesup.org>
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
#include <gcu/application.h>
#include <gcu/gldocument.h>
#include <gcu/glview.h>
#include <gcu/macros.h>
#include <goffice/goffice-features.h>
#include <goffice/utils/go-image.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <gtk/gtkgl.h>
#include <libgnomevfs/gnome-vfs-ops.h>
#include <glib/gi18n-lib.h>
#include <cmath>
#include <cstring>

#define ROOTDIR "/apps/gchemutils/gl/"

static GdkGLConfig *glconfig = NULL;
double DefaultPsi = 70.;
double DefaultTheta = 10.;
double DefaultPhi = -90.;
bool OffScreenRendering = false;

using namespace std;

namespace gcu
{
#ifdef HAVE_GO_CONF_SYNC
GOConfNode *GLView::m_ConfNode = NULL;
#else
GConfClient *GLView::m_ConfClient = NULL;
#endif
guint GLView::m_NotificationId = 0;
int GLView::nbViews = 0;

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

#ifdef HAVE_GO_CONF_SYNC
static void on_config_changed (GOConfNode *node, gchar const *key, gpointer data)
{
	if (!strcmp (key, ROOTDIR"off-screen-rendering"))
		OffScreenRendering = go_conf_get_bool (node, key);
}
#else
static void on_config_changed (GConfClient *client, guint cnxn_id, GConfEntry *entry, gpointer data)
{
	if (!strcmp (gconf_entry_get_key (entry),ROOTDIR"off-screen-rendering"))
		OffScreenRendering = gconf_value_get_bool (gconf_entry_get_value (entry));
}
#endif

// GLView implementation
#define GCU_CONF_DIR_GL "gl"

GLView::GLView (GLDocument* pDoc): Printable ()
{
	m_bInit = false;
	m_Doc = pDoc;
	m_Red = m_Green = m_Blue = 0.;
	m_Alpha = 1.;
	m_Angle = 10.;
	nbViews++;
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
#ifdef HAVE_GO_CONF_SYNC
		m_ConfNode = go_conf_get_node (Application::GetConfDir (), GCU_CONF_DIR_GL);
#else
		GError *error = NULL;
		m_ConfClient = gconf_client_get_default ();
		gconf_client_add_dir (m_ConfClient, "/apps/gchemutils/gl", GCONF_CLIENT_PRELOAD_ONELEVEL, NULL);
#endif
		GCU_GCONF_GET_NO_CHECK ("off-screen-rendering", bool, OffScreenRendering, true)
#ifdef HAVE_GO_CONF_SYNC
		m_NotificationId = go_conf_add_monitor (m_ConfNode, "off-screen-rendering", (GOConfMonitorFunc) on_config_changed, NULL);
#else
		m_NotificationId = gconf_client_notify_add (m_ConfClient, "/apps/gchemutils/gl", (GConfClientNotifyFunc) on_config_changed, NULL, NULL, NULL);
#endif
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
	nbViews--;
	if (!nbViews) {
#ifdef HAVE_GO_CONF_SYNC
		go_conf_remove_monitor (m_NotificationId);
		go_conf_free_node (m_ConfNode);
		m_ConfNode = NULL;
#else
		gconf_client_notify_remove (m_ConfClient, m_NotificationId);
		gconf_client_remove_dir (m_ConfClient, "/apps/gchemutils/gl", NULL);
		g_object_unref (m_ConfClient);
		m_ConfClient = NULL;
#endif
		m_NotificationId = 0;
	}
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
		glShadeModel (GL_SMOOTH);
		glPolygonMode (GL_FRONT, GL_FILL);
		glEnable(GL_BLEND);
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
		double x = m_Doc->GetMaxDist ();
		if (x == 0)
			x = 1;
		glViewport (0,0, m_pWidget->allocation.width, m_pWidget->allocation.height);
		if (fAspect > 1.0) {
			m_Height = x * (1 - tan (m_Angle / 360 * M_PI));
			m_Width = m_Height * fAspect;
		} else {
			m_Width = x * (1 - tan (m_Angle / 360 * M_PI));
			m_Height = m_Width / fAspect;
		}
		glMatrixMode (GL_PROJECTION);
		glLoadIdentity();
		if (m_Angle > 0.) {
			m_Radius = (float) (x / sin (m_Angle / 360 * M_PI)) ;
			m_Near = m_Radius - x;
			m_Far = m_Radius + x;
			glFrustum (- m_Width, m_Width, - m_Height, m_Height, m_Near, m_Far);
		} else {
			m_Radius = 2 * x;
			m_Near = m_Radius - x;
			m_Far = m_Radius + x;
			glOrtho (- m_Width, m_Width, - m_Height, m_Height, m_Near, m_Far);
		}
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
		m_Doc->Draw (m_Euler);
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
		m_Doc->Draw (m_Euler);
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

bool GLView::OnMotion(GdkEventMotion *event)
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
			return false;
		m_Doc->SetDirty (true);
		Rotate (x - m_Lastx, y - m_Lasty);
		m_Lastx = x;
		m_Lasty = y;
		gtk_widget_queue_draw_area(m_pWidget, 0, 0, m_pWidget->allocation.width, m_pWidget->allocation.height);
	}
	return true;
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

static gboolean do_save_image (const gchar *buf, gsize count, GError **error, gpointer data)
{
	GnomeVFSHandle *handle = (GnomeVFSHandle*) data;
	GnomeVFSFileSize written = 0;
	GnomeVFSResult res;
	while (count) {
		res = gnome_vfs_write (handle, buf, count, &written);
		if (res != GNOME_VFS_OK) {
			g_set_error (error, g_quark_from_static_string ("gchemutils"), res, gnome_vfs_result_to_string (res));
			return false;
		}
		count -= written;
	}
	return true;
}

void GLView::SaveAsImage (string const &filename, char const *type, map<string, string>& options, unsigned width, unsigned height)
{
	if (width == 0 || height == 0)
		return;

	GdkPixbuf *pixbuf = BuildPixbuf (width, height);

	if (pixbuf != NULL) {
		char const **keys = g_new0 (char const*, options.size () + 1);
		char const **values = g_new0 (char const*, options.size ());
		GError *error = NULL;
		map<string, string>::iterator i, iend = options.end ();
		int j = 0;
		for (i = options.begin (); i != iend; i++) {
			keys[j] = (*i).first.c_str ();
			values[j++] = (*i).second.c_str ();
		}
		GnomeVFSHandle *handle = NULL;
		if (gnome_vfs_create (&handle, filename.c_str (), GNOME_VFS_OPEN_WRITE, true, 0644) == GNOME_VFS_OK) {
			gdk_pixbuf_save_to_callbackv (pixbuf, do_save_image, handle, type, (char**) keys, (char**) values, &error);
			if (error) {
				fprintf (stderr, _("Unable to save image file: %s\n"), error->message);
				g_error_free (error);
			}
			gnome_vfs_close (handle); // hope there will be no error there
		}
		g_free (keys);
		g_free (values);
		g_object_unref (pixbuf);
	}
}

GdkPixbuf *GLView::BuildPixbuf (unsigned width, unsigned height)
{
	GdkGLConfig *glconfig = gdk_gl_config_new_by_mode (
		GdkGLConfigMode (GDK_GL_MODE_RGBA | GDK_GL_MODE_DEPTH));
	GdkPixmap *pixmap = gdk_pixmap_new (NULL, width, height, 24);
	GdkGLPixmap *gl_pixmap = gdk_pixmap_set_gl_capability (pixmap,
							       glconfig,
							       NULL );
	GdkGLDrawable *drawable = NULL;
	GdkGLContext *context = NULL;
	if (gl_pixmap != NULL) {
		drawable = gdk_pixmap_get_gl_drawable (pixmap);
		context = gdk_gl_context_new (drawable,
						     NULL,
						     TRUE,
						     GDK_GL_RGBA_TYPE);
	}
	double aspect = (GLfloat) width / height;
	double x = m_Doc->GetMaxDist (), w, h;
	if (x == 0)
		x = 1;
	if (aspect > 1.0) {
		h = x * (1 - tan (m_Angle / 360 * M_PI));
		w = h * aspect;
	} else {
		w = x * (1 - tan (m_Angle / 360 * M_PI));
		h = w / aspect;
	}
	GdkPixbuf *pixbuf = NULL;
	gdk_error_trap_push ();
	bool result = OffScreenRendering && gl_pixmap && gdk_gl_drawable_gl_begin (drawable, context);
	gdk_flush ();
	if (gdk_error_trap_pop ())
		result = false;
	
	if (result) {
	    glEnable (GL_LIGHTING);
		glEnable (GL_LIGHT0);
		glEnable (GL_DEPTH_TEST);
		glEnable (GL_CULL_FACE);
		glEnable (GL_COLOR_MATERIAL);
		float shiny = 25.0, spec[4] = {1.0, 1.0, 1.0, 1.0};
		glMaterialfv (GL_FRONT_AND_BACK, GL_SHININESS, &shiny);
		glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, spec);
		glViewport (0, 0, width, height);
	    glMatrixMode (GL_PROJECTION);
	    glLoadIdentity ();
		GLfloat radius, near, far;
		if (m_Angle > 0.) {
			radius = (float) (x / sin (m_Angle / 360 * M_PI)) ;
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
		glTranslatef (0, 0, -m_Radius);
		glClearColor (m_Red, m_Green, m_Blue, m_Alpha);
		glClearDepth (1.0);
		glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable (GL_BLEND);
		GetDoc ()->Draw(m_Euler);
		glDisable (GL_BLEND);
		glFlush ();
		gdk_gl_drawable_gl_end (drawable);
		pixbuf = gdk_pixbuf_get_from_drawable (NULL,
			(GdkDrawable*) pixmap, NULL, 0, 0, 0, 0, -1, -1);
	} else if (m_bInit) {
		unsigned hstep, vstep;
		double dxStep, dyStep;
		unsigned char *tmp, *dest, *src, *dst;
		unsigned LineWidth, s = sizeof(int);
		gtk_window_present (GTK_WINDOW (gtk_widget_get_toplevel (m_pWidget))); 
		while (gtk_events_pending ())
			gtk_main_iteration ();
		if (m_pWidget->allocation.width & (s - 1))
			LineWidth = ((~(s - 1)) & (m_pWidget->allocation.width * 3)) + s;
		else
			LineWidth = m_pWidget->allocation.width * 3;
		unsigned size = LineWidth * m_pWidget->allocation.height;
		int i, j;
		hstep = m_pWidget->allocation.width;
		vstep = m_pWidget->allocation.height;
		tmp = new unsigned char[size];
		if (!tmp)
			goto osmesa;
		pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, false, 8, (int) width, (int) height);
		dest = gdk_pixbuf_get_pixels (pixbuf);
		int n, m, imax, jmax, rowstride =  gdk_pixbuf_get_rowstride (pixbuf);
		imax = width / hstep;
		jmax = height / vstep;
		dxStep = ((double) hstep) / width * 2; 
		dyStep = ((double) vstep) / height * 2;
		for (j = 0; j <= jmax; j++)
		{
			for (i = 0; i <= imax; i++)
			{
				GdkGLContext *glcontext = gtk_widget_get_gl_context (m_pWidget);
				GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (m_pWidget);
				if (gdk_gl_drawable_gl_begin (gldrawable, glcontext))
				{
					glMatrixMode (GL_PROJECTION);
					glLoadIdentity ();
					if (m_Angle > 0.)
						glFrustum (w * ( -1 + i * dxStep), w * ( -1 + (i + 1)* dxStep),
								h * ( 1 - (j + 1)* dyStep), h * ( 1 - j* dyStep), m_Near , m_Far);
					else
						glOrtho (w * ( -1 + i * dxStep), w * ( -1 + (i + 1)* dxStep),
								h * ( 1 - (j + 1)* dyStep), h * ( 1 - j* dyStep), m_Near , m_Far);
					glMatrixMode(GL_MODELVIEW);
					glLoadIdentity();
					glTranslatef(0, 0, - m_Radius);
					glClearColor (m_Red, m_Green, m_Blue, m_Alpha);
					glClearDepth (1.0);
					glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
					m_Doc->Draw (m_Euler);
					glFlush ();
					gdk_gl_drawable_gl_end (gldrawable);
					glPixelStorei (GL_PACK_ALIGNMENT, s);
					glReadBuffer (GL_BACK_LEFT);
					glReadPixels (0, 0, m_pWidget->allocation.width, m_pWidget->allocation.height, GL_RGB,
										GL_UNSIGNED_BYTE, tmp);
					// copy the data to the pixbuf.
					// linesize
					m = (i < imax)? hstep * 3: (width - imax * hstep) * 3;
					src = tmp + (vstep - 1) * LineWidth;
					dst = dest + j * vstep * rowstride + i * hstep * 3;
					for (n = 0; n < (int) ((j < jmax)? vstep: height - jmax * vstep); n++) {
						memcpy (dst, src, m);
						src -= LineWidth;
						dst += rowstride;
					}
				} else {
					g_object_unref (pixbuf);
					pixbuf = NULL;
					goto osmesa;
				}
			}
		}
		delete [] tmp;
	} else {
osmesa:
		g_warning ("Off-screen rendering not supported in this context");
		// TODO: implement rendering using an external program and osmesa
	}
	if (context)
		gdk_gl_context_destroy (context);
	if (gl_pixmap)
		gdk_gl_pixmap_destroy (gl_pixmap);
	// destroying pixmap gives a CRITICAL and destroying glconfig leeds to a crash.
	Update ();
	return pixbuf;
}

void GLView::DoPrint (GtkPrintOperation *print, GtkPrintContext *context)
{
	cairo_t *cr;
	gdouble width, height;

	cr = gtk_print_context_get_cairo_context (context);
	width = gtk_print_context_get_width (context);
	height = gtk_print_context_get_height (context);
	int w, h; // size in points
	w = m_pWidget->allocation.width;
	h = m_pWidget->allocation.height;
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
	double scale = 300. / 72.;
	GdkPixbuf *pixbuf = BuildPixbuf (w * scale, h * scale);
	GOImage *img = go_image_new_from_pixbuf (pixbuf);
	cairo_pattern_t *cr_pattern = go_image_create_cairo_pattern (img);
	cairo_matrix_t cr_matrix;
	double x = 0., y = 0.;
	if (GetHorizCentered ())
		x = (width - w) / 2.;
	if (GetVertCentered ())
		y = (height - h) / 2.;
	cairo_matrix_init_scale (&cr_matrix, scale, scale);
	cairo_matrix_translate (&cr_matrix, -x, -y);
	cairo_pattern_set_matrix (cr_pattern, &cr_matrix);
	cairo_rectangle (cr, x, y, w, h);
	cairo_set_source (cr, cr_pattern);
	cairo_fill (cr);
	cairo_pattern_destroy (cr_pattern);
	g_object_unref (img);
	g_object_unref (pixbuf);
}

}	//	namespace gcu
