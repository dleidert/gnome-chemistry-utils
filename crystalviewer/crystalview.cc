/* 
 * Gnome Chemisty Utils
 * crystaldoc.cc 
 *
 * Copyright (C) 2002
 *
 * Developed by Jean Bréfort <jean.brefort@ac-dijon.fr>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA  02111-1307, USA.
 */

#include "config.h"
#include "crystalview.h"
#include "crystaldoc.h"
#include "chemistry/matrix.h"
#include "chemistry/xml-utils.h"
#include <GL/gl.h>
#include <GL/glu.h>
#ifdef HAVE_GTKGLAREA
#	include <gtkgl/gtkglarea.h>
#else
#	include <gtk/gtkgl.h>
#endif
#include <math.h>

/* Attribute list for gtkglarea widget. Specifies a
     list of Boolean attributes and enum/integer
     attribute/value pairs. The last attribute must be
     GDK_GL_NONE. See glXChooseVisual manpage for further
     explanation.
*/

#ifdef HAVE_GTKGLAREA
	static int attrlist[] = {
		GDK_GL_RGBA,
		GDK_GL_BUFFER_SIZE,1,
		GDK_GL_RED_SIZE,1,
		GDK_GL_GREEN_SIZE,1,
		GDK_GL_BLUE_SIZE,1,
		GDK_GL_DEPTH_SIZE,1,
		GDK_GL_DOUBLEBUFFER,
		GDK_GL_NONE
	};
#else
static GdkGLConfig *glconfig = NULL;
#endif

using namespace gcu;

typedef struct
{
	unsigned glList;
} WidgetData;

static bool on_init(GtkWidget *widget, CrystalView* View) 
{
	View->Init(widget);
	return true;
}

bool on_reshape(GtkWidget *widget, GdkEventConfigure *event, CrystalView* View) 
{
	View->Reshape(widget);
	return true;
}

static bool on_draw(GtkWidget *widget, GdkEventExpose *event, CrystalView* View) 
{
	/* Draw only last expose. */
	if (event->count > 0) return TRUE;

	View->Draw(widget);
	return true;
}

static bool on_motion(GtkWidget *widget, GdkEventMotion *event, CrystalView* View) 
{
	View->OnMotion(widget, event);
	return true;
}

static bool on_pressed(GtkWidget *widget, GdkEventButton *event, CrystalView* View) 
{
	return View->OnPressed(widget, event);
}

static void on_destroyed(GtkWidget *widget, CrystalView *pView)
{
	pView->OnDestroyed(widget);
}

CrystalView::CrystalView(CrystalDoc* pDoc)
{
	m_pDoc = pDoc;
	m_bInit = false;
}

CrystalView::~CrystalView()
{
}

void CrystalView::Init(GtkWidget *widget)
{
#ifdef HAVE_GTKGLAREA
	if (gtk_gl_area_make_current(GTK_GL_AREA(widget)))
#else
	GdkGLContext *glcontext = gtk_widget_get_gl_context(widget);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(widget);
	if (gdk_gl_drawable_gl_begin(gldrawable, glcontext))
#endif
	{
	    glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glEnable(GL_COLOR_MATERIAL);
		float shiny = 25.0, spec[4] = {1.0, 1.0, 1.0, 1.0};
		glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &shiny);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec);
		Update(widget);
		m_bInit = true;
    }
}

bool CrystalView::Load(xmlNodePtr node)
{
	char *txt;
	xmlNodePtr child = node->children;
	while(child)
	{
		if (!strcmp((gchar*)child->name, "orientation"))
		{
			txt = (char*)xmlGetProp(child, (xmlChar*)"psi");
			if (txt) sscanf(txt, "%lg", &m_psi); else return false;
			txt = (char*)xmlGetProp(child, (xmlChar*)"theta");
			if (txt) sscanf(txt, "%lg", &m_theta); else return false;
			txt = (char*)xmlGetProp(child, (xmlChar*)"phi");
			if (txt) sscanf(txt, "%lg", &m_phi); else return false;
			Matrix m(m_psi/90*1.570796326794897, m_theta/90*1.570796326794897, m_phi/90*1.570796326794897, euler);
			m_Euler = m;
		}
		else if (!strcmp((gchar*)child->name, "fov"))
		{
			txt = (char*)xmlNodeGetContent(child);
			int result = sscanf(txt, "%lg", &m_fAngle);
		}
		child = child->next;
	}
	if (!ReadColor(node, "background", &m_fRed, &m_fGreen, &m_fBlue, &m_fAlpha)) return false;
	return true;
}

xmlNodePtr CrystalView::Save(xmlDocPtr xml)
{
	xmlNodePtr parent, child;
	gchar buf[256];
	parent = xmlNewDocNode(xml, NULL, (xmlChar*)"view", NULL);
	if (!parent) return NULL;
	
	child = xmlNewDocNode(xml, NULL, (xmlChar*)"orientation", NULL);
	if (child) xmlAddChild(parent, child);
	else {xmlFreeNode(parent); return NULL;}
	snprintf(buf, sizeof(buf), "%g", m_psi);
	xmlNewProp(child, (xmlChar*)"psi", (xmlChar*)buf);
	snprintf(buf, sizeof(buf), "%g", m_theta);
	xmlNewProp(child, (xmlChar*)"theta", (xmlChar*)buf);
	snprintf(buf, sizeof(buf), "%g", m_phi);
	xmlNewProp(child, (xmlChar*)"phi", (xmlChar*)buf);
	
	g_snprintf(buf, sizeof(buf) - 1, "%g", m_fAngle);
	child = xmlNewDocNode(xml, NULL, (xmlChar*)"fov", (xmlChar*)buf);
	if (child) xmlAddChild(parent, child);
	else {xmlFreeNode(parent); return NULL;}
	
	if (!WriteColor(xml, parent, "background", m_fRed, m_fGreen, m_fBlue, m_fAlpha)) {xmlFreeNode(parent); return NULL;}
	
	return parent;
}

GtkWidget* CrystalView::CreateNewWidget()
{
/* Create new OpenGL widget. */
#ifdef HAVE_GTKGLAREA
	m_pWidget = GTK_WIDGET(gtk_gl_area_new(attrlist));
#else
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
		glconfig = gdk_gl_config_new_by_mode(GdkGLConfigMode(GDK_GL_MODE_RGB |
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

#endif
	WidgetData* pData = new WidgetData;
	pData->glList = 0;
	g_object_set_data(G_OBJECT(m_pWidget), "gldata", pData);
	m_Widgets.push_back(m_pWidget);
// Events for widget must be set before X Window is created
	gtk_widget_set_events(GTK_WIDGET(m_pWidget),
			GDK_EXPOSURE_MASK|
			GDK_BUTTON_MOTION_MASK|
			GDK_POINTER_MOTION_HINT_MASK|
			GDK_BUTTON_PRESS_MASK);

// Connect signal handlers
// Do initialization when widget has been realized.
	g_signal_connect(G_OBJECT(m_pWidget), "realize",
		     GTK_SIGNAL_FUNC(on_init), this);
// When window is resized viewport needs to be resized also.
	g_signal_connect(G_OBJECT(m_pWidget), "configure_event",
		     GTK_SIGNAL_FUNC(on_reshape), this);
// Redraw image when exposed. 
	g_signal_connect(G_OBJECT(m_pWidget), "expose_event",
		     GTK_SIGNAL_FUNC(on_draw), this);
// When moving mouse 
  g_signal_connect (G_OBJECT(m_pWidget), "motion_notify_event",
		      GTK_SIGNAL_FUNC(on_motion), this);
// When a mouse button is pressed
  g_signal_connect (G_OBJECT(m_pWidget), "button_press_event",
		      GTK_SIGNAL_FUNC(on_pressed), this);
// When a widget is destroyed
	g_signal_connect (G_OBJECT(m_pWidget), "destroy", GTK_SIGNAL_FUNC(on_destroyed), this);

	gtk_widget_show(GTK_WIDGET(m_pWidget));
	return m_pWidget;
}

bool CrystalView::OnPressed(GtkWidget *widget, GdkEventButton *event)
{
  if (event->button == 1) {
    // beginning of drag, reset mouse position
    m_lastx = event->x;
    m_lasty = event->y;
    return true;
  }
  return false;
}

void CrystalView::OnMotion(GtkWidget *widget, GdkEventMotion *event)
{
	gint x, y;
	GdkModifierType state;
	
	if (event->is_hint)
		gdk_window_get_pointer(event->window, &x, &y, &state);
	else
	{
	    x = (gint) event->x;
	    y = (gint) event->y;
	    state = (GdkModifierType) event->state;
	}
	if (state & GDK_BUTTON1_MASK)
	{
		if ((x == m_lastx) && (y == m_lasty)) return;
			m_pDoc->SetDirty();
		Rotate(x - m_lastx, y - m_lasty);
		m_lastx = x;
		m_lasty = y;
		std::list<GtkWidget*>::iterator i;
		for (i = m_Widgets.begin(); i!= m_Widgets.end(); i++)
		{
			gtk_widget_queue_draw_area(*i, 0, 0, widget->allocation.width, widget->allocation.height);
		}
	}
}

void CrystalView::Reshape(GtkWidget *widget)
{
	float fAspect;
#ifdef HAVE_GTKGLAREA
	if (gtk_gl_area_make_current(GTK_GL_AREA(widget)))
#else
	GdkGLContext *glcontext = gtk_widget_get_gl_context(widget);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(widget);
	if (gdk_gl_drawable_gl_begin(gldrawable, glcontext))
#endif
    {
		if (widget->allocation.height)
		{
			fAspect = (GLfloat)widget->allocation.width/(GLfloat)widget->allocation.height;
			if (fAspect == 0.0) fAspect = 1.0;
		}
		else	// don't divide by zero, not that we should ever run into that...
			fAspect = 1.0f;
		double x = m_pDoc->GetMaxDist();
		if (x == 0) x = 1;
		m_fRadius = (float) (x / sin(m_fAngle / 360 * M_PI)) ;
		glViewport(0,0, widget->allocation.width, widget->allocation.height);
		if (fAspect > 1.0)
		{
			m_height = x * (1 - tan(m_fAngle / 360 * M_PI));
			m_width = m_height * fAspect;
		}
		else
		{
			m_width = x * (1 - tan(m_fAngle / 360 * M_PI));
			m_height = m_width / fAspect;
		}
		m_near = m_fRadius - x;
		m_far = m_fRadius + x;
	    glMatrixMode(GL_PROJECTION);
	    glLoadIdentity();
		glFrustum(- m_width, m_width, - m_height, m_height, m_near , m_far);
	    glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glTranslatef(0, 0, -m_fRadius);
	}
}

void CrystalView::Draw(GtkWidget *widget)
{
#ifdef HAVE_GTKGLAREA
	if (gtk_gl_area_make_current(GTK_GL_AREA(widget)))
#else
	GdkGLContext *glcontext = gtk_widget_get_gl_context(widget);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(widget);
	if (gdk_gl_drawable_gl_begin(gldrawable, glcontext))
#endif
    {
		glClearColor(m_fRed, m_fGreen, m_fBlue, m_fAlpha);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		WidgetData* pData = (WidgetData*)g_object_get_data(G_OBJECT(widget), "gldata");
		m_nGLList = pData->glList;
		if  (m_nGLList)
		{
			glPushMatrix();
			glRotated(m_psi, 0.0, 1.0, 0.0);
			glRotated(m_theta, 0.0, 0.0, 1.0);
			glRotated(m_phi, 0.0, 1.0, 0.0);
			glCallList(m_nGLList);
			glPopMatrix();
		}
	/* Swap backbuffer to front */
#ifdef HAVE_GTKGLAREA
		gtk_gl_area_swapbuffers(GTK_GL_AREA(widget));
#else
		gdk_gl_drawable_swap_buffers(gldrawable);
#endif
    }
}

void CrystalView::Update()
{
	std::list<GtkWidget*>::iterator i;
	for (i = m_Widgets.begin(); i!= m_Widgets.end(); i++) Update(*i);
}

void CrystalView::Update(GtkWidget* widget)
{
	Reshape(widget);
#ifdef HAVE_GTKGLAREA
	if (gtk_gl_area_make_current(GTK_GL_AREA(widget)))
#else
	GdkGLContext *glcontext = gtk_widget_get_gl_context(widget);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(widget);
	if (gdk_gl_drawable_gl_begin(gldrawable, glcontext))
#endif
 	{
		WidgetData* pData = (WidgetData*)g_object_get_data(G_OBJECT(widget), "gldata");
		m_nGLList = pData->glList;
		if (m_nGLList) glDeleteLists(m_nGLList,1);
		pData->glList = m_nGLList = glGenLists(1);
		glNewList(m_nGLList, GL_COMPILE);
		m_pDoc->Draw();
		glEndList();
	}
	Draw(widget);
}

void CrystalView::OnDestroyed(GtkWidget *widget)
{
	delete (WidgetData*) g_object_get_data(G_OBJECT(widget), "gldata");
	m_Widgets.remove(widget);
}

void CrystalView::Rotate(gdouble x, gdouble y)
{
	gdouble z = sqrt(x*x + y*y);
	Matrix Mat(0, (y > 0) ? - acos(x/z) : acos(x/z), z * 0.00349065850398866, rotation);
	m_Euler = Mat * m_Euler;
	m_Euler.Euler(m_psi, m_theta, m_phi);
	m_psi /= 0.0174532925199433;
	m_theta /= 0.0174532925199433;
	m_phi /= 0.0174532925199433;
}
