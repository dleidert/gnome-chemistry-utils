// -*- C++ -*-

/* 
 * Gnome Crystal
 * view.cc 
 *
 * Copyright (C) 2000-2005 Jean Bréfort <jean.brefort@normalesup.org>
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include "config.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <gtk/gtkgl.h>
#include <math.h>
#include <libxml/parserInternals.h>
#include <libxml/xmlmemory.h>
#include "view.h"
#include "document.h"
#include "globals.h"

guint FoV;
gdouble Psi, Theta, Phi;
gdouble Red, Green, Blue;
extern GtkWidget *vbox1;

typedef struct
{
	unsigned glList;
} WidgetData;//FIXME:The same structure is defines in crystalview.cc

extern "C" {
	
	gint on_init(GtkWidget *widget, void *data) 
	{
		((gcView*)data)->Init(widget);
		return TRUE;
	}
	
	gint on_reshape(GtkWidget *widget, GdkEventConfigure *event, void *data) 
	{
		((gcView*)data)->Reshape(widget);
		return TRUE;
	}
	
	gint on_draw(GtkWidget *widget, GdkEventExpose *event, void *data) 
	{
		/* Draw only last expose. */
		if (event->count > 0) return TRUE;

		((gcView*)data)->Draw(widget);
		return TRUE;
	}

	gint on_motion(GtkWidget *widget, GdkEventMotion *event, void *data) 
	{
		((gcView*)data)->OnMotion(widget, event);
		return TRUE;
	}

	gint on_pressed(GtkWidget *widget, GdkEventButton *event, void *data) 
	{
		return ((gcView*)data)->OnPressed(widget, event);
	}

}//end of extern "C" section

static void on_destroyed(GtkWidget *widget, gcView *pView)
{
	pView->OnDestroyed(widget);
}

gcView::gcView (gcDocument *pDoc): CrystalView ((CrystalDoc*) pDoc)
{
	m_fAngle = FoV;
	SetRotation (Psi, Theta, Phi);
	m_nGLList = 0;
	m_fBlue = Blue;
	m_fRed = Red;
	m_fGreen = Green;
	m_fAlpha = 1.0;
	m_pLabel = NULL;
	m_Window = NULL;
}

gcView::gcView (gcView*pView): CrystalView ((CrystalDoc*) (pView->m_pDoc))
{
	m_fGreen = pView->m_fGreen;
	m_fAngle = pView->m_fAngle;
	SetRotation(pView->m_psi, pView->m_theta, pView->m_phi);
	m_nGLList = 0;
	m_fBlue = pView->m_fBlue;
	m_fRed = pView->m_fRed;
	m_fGreen = pView->m_fGreen;
	m_fAlpha = pView->m_fAlpha;
	((gcDocument*) m_pDoc)->AddView (this);
	m_pLabel = NULL;
	m_Window = NULL;
}

gcView::~gcView ()
{
	gtk_widget_destroy(GTK_WIDGET(m_pMenu));
	Dialog *dialog;
	while (!m_Dialogs.empty ()) {
		dialog = m_Dialogs.front();
		m_Dialogs.pop_front();
		dialog->Destroy();
	}
}

void gcView::SetDocument(gcDocument *pDoc)
{
	m_pDoc = pDoc;
}
void gcView::SetBackgroundColor(float red, float green, float blue, float alpha)
{
	m_fRed = red ;
	m_fGreen = green ;
	m_fBlue = blue ;
	m_fAlpha = alpha ;
}

void gcView::GetBackgroundColor(double *red, double *green, double *blue, double *alpha)
{
	*red = m_fRed ;
	*green = m_fGreen ;
	*blue = m_fBlue ;
	*alpha = m_fAlpha ;
}

void gcView::GetRotation(double *psi, double *theta, double *phi)
{
	*psi = m_psi;
	*theta = m_theta;
	*phi = m_phi;
}

void gcView::SetRotation(double psi, double theta, double phi)
{
	m_psi = psi;
	m_theta = theta;
	m_phi = phi;
	Matrix m(m_psi / 180 * M_PI, m_theta / 180 * M_PI, m_phi / 180 * M_PI, euler);
	m_Euler = m;
}

bool gcView::LoadOld(xmlNodePtr node)
{
	char *txt;
	xmlNodePtr child = node->children;
	while(child)
	{
		if (!strcmp((gchar*)child->name, "orientation"))
		{
			txt = (char*)xmlNodeGetContent(child);
			if (txt)
			{
				sscanf(txt, "%lg %lg %lg", &m_psi, &m_theta, &m_phi);
				Matrix m(m_psi / 180 * M_PI, m_theta / 180 * M_PI, m_phi / 180 * M_PI, euler);
				m_Euler = m;
				xmlFree(txt);
			}
		}
		else if (!strcmp((gchar*)child->name, "background"))
		{
			txt = (char*)xmlNodeGetContent(child);
			if (txt)
			{
				sscanf(txt, "%g %g %g %g", &m_fBlue, &m_fRed, &m_fGreen, &m_fAlpha);
				xmlFree(txt);
			}
		} else if (!strcmp((gchar*)child->name, "fov")) {
			txt = (char*) xmlNodeGetContent (child);
			if (txt) {
				sscanf (txt, "%lg", &m_fAngle);
				xmlFree (txt);
			}
		}
		child = child->next;
	}
	return true;
}

void gcView::Print(GnomePrintContext *pc, gdouble width, gdouble height)
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
	int w= (int) (Width * PrintResolution / 72.), h = (int) (Height * PrintResolution / 72.);
	GdkPixmap *pixmap = gdk_pixmap_new (
			(GdkDrawable*) (m_pWidget->window),
			w, h, -1);
	GdkGLPixmap *gl_pixmap = gdk_pixmap_set_gl_capability( pixmap,
							       glconfig,
							       NULL );
	GdkGLDrawable * drawable = gdk_pixmap_get_gl_drawable (pixmap);
	GdkGLContext * context = gdk_gl_context_new(drawable,
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
		glFrustum (- m_width, m_width, - m_height, m_height, m_near , m_far);
	    glMatrixMode (GL_MODELVIEW);
		glLoadIdentity ();
		glTranslatef (0, 0, -m_fRadius);
		glClearColor (m_fRed, m_fGreen, m_fBlue, m_fAlpha);
		glClearDepth (1.0);
		glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		WidgetData* pData = (WidgetData*) g_object_get_data (G_OBJECT (m_pWidget), "gldata");
		m_nGLList = pData->glList;
		if  (m_nGLList) {
			glPushMatrix ();
			glRotated (m_psi, 0.0, 1.0, 0.0);
			glRotated (m_theta, 0.0, 0.0, 1.0);
			glRotated (m_phi, 0.0, 1.0, 0.0);
			glEnable (GL_BLEND);
			m_pDoc->Draw();
			glDisable (GL_BLEND);
			glPopMatrix ();
		}
		glFlush ();
		gdk_gl_drawable_gl_end (drawable);
		GdkPixbuf* pixbuf = gdk_pixbuf_get_from_drawable    (NULL,
			(GdkDrawable*) pixmap, NULL, 0, 0, 0, 0, -1, -1);
		gnome_print_gsave(pc);
		gnome_print_concat (pc, matrix);
		gnome_print_rgbimage (pc, (const guchar*) gdk_pixbuf_get_pixels (pixbuf), w, h, gdk_pixbuf_get_rowstride (pixbuf));
		gnome_print_grestore(pc);
		g_object_unref (pixbuf);
	}

	gdk_gl_context_destroy (context);
	gdk_gl_pixmap_destroy (gl_pixmap);
	// destroying pixmap gives a CRITICAL and destroying glconfig leeds to a crash.
}

void gcView::SetMenu(GtkMenuItem* item)
{
	m_pMenu = item;
	m_pMenuLabel = NULL;
	GList* l;
	for (l = gtk_container_get_children(GTK_CONTAINER(item)); l != NULL; l = g_list_next(l))
		if (GTK_IS_LABEL(l->data))
		{
			m_pMenuLabel = (GtkLabel*)(l->data);
			break;
		}
}

void gcView::NotifyDialog (Dialog* dialog)
{
	m_Dialogs.push_front (dialog);
}

void gcView::RemoveDialog (Dialog* dialog)
{
	m_Dialogs.remove (dialog);
}
