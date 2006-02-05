// -*- C++ -*-

/* 
 * Gnome Crystal
 * view.cc 
 *
 * Copyright (C) 2000-2005
 *
 * Developed by Jean Bréfort <jean.brefort@normalesup.org>
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
	m_bLocked = false;
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
	m_bLocked = false;
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

void gcView::ExportPNG(const gchar* filename)
{
/*	FILE *fp;
	png_structp png_ptr;
	png_infop info_ptr;
	unsigned char *tmp = NULL, **lines = NULL;
	fp = fopen(filename, "wb");
	try
	{
		if (!fp) throw (int) 0;
		if (!(png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)))
			throw (int) 1;
		if (!(info_ptr = png_create_info_struct(png_ptr))) throw (int) 2;
		if (setjmp(png_jmpbuf(png_ptr))) throw (int) 3;
		png_init_io(png_ptr, fp);
		png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);
		png_set_IHDR(png_ptr, info_ptr, m_pWidget->allocation.width, m_pWidget->allocation.height,
						8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
						PNG_FILTER_TYPE_DEFAULT);
		png_write_info(png_ptr, info_ptr);
		unsigned LineWidth, n = sizeof(int);
		if (m_pWidget->allocation.width & (n - 1))
		{
			LineWidth = ((~(n - 1)) & (m_pWidget->allocation.width * 3)) + n;
		}
		else LineWidth = m_pWidget->allocation.width * 3;
		unsigned size = LineWidth * m_pWidget->allocation.height;
		tmp = new unsigned char[size];
		if (!tmp) throw (int) 4;
		GdkGLContext *glcontext = gtk_widget_get_gl_context(m_pWidget);
		GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(m_pWidget);
		if (gdk_gl_drawable_gl_begin(gldrawable, glcontext))
		{
			glPixelStorei(GL_PACK_ALIGNMENT, n);
			glReadBuffer(GL_BACK_LEFT);
			glReadPixels(0, 0, m_pWidget->allocation.width, m_pWidget->allocation.height , GL_RGB,
							GL_UNSIGNED_BYTE, tmp);
		}
		else throw (int) 6;
		lines = new unsigned char*[m_pWidget->allocation.height];
		if (!lines) throw (int) 5;
		for (int i = 0, j = m_pWidget->allocation.height - 1; j >= 0; i++, j--)
		{
			lines[i] = tmp + LineWidth * j;
		}
		png_write_image(png_ptr, lines);
		png_write_end(png_ptr, info_ptr);
		delete [] tmp;
		delete [] lines;
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(fp);
	}
	catch (int num)
	{
		if (tmp) delete [] tmp;
		if (lines) delete [] lines;
		if (num > 0) fclose(fp);
		switch(num)
		{
		case 0: break;
		case 1: break;
		case 2:
			png_destroy_write_struct(&png_ptr, NULL);
			break;
		default:
			png_destroy_write_struct(&png_ptr, &info_ptr);
			break;
		}
	}*/
}

void gcView::ExportJPG(const gchar* filename)
{
/*	FILE *fp;
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	unsigned char *tmp = NULL, **lines = NULL;
	fp = fopen(filename, "wb");
	try
	{
		if (!fp) throw (int) 0;
		cinfo.err = jpeg_std_error(&jerr);
		jpeg_create_compress(&cinfo);
		jpeg_stdio_dest(&cinfo, fp);
		cinfo.image_width = m_pWidget->allocation.width;
		cinfo.image_height = m_pWidget->allocation.height;
		cinfo.input_components = 3;
		cinfo.in_color_space = JCS_RGB;
		jpeg_set_defaults(&cinfo);
		unsigned LineWidth, n = sizeof(int);
		if (m_pWidget->allocation.width & (n - 1))
		{
			LineWidth = ((~(n - 1)) & (m_pWidget->allocation.width * 3)) + n;
		}
		else LineWidth = m_pWidget->allocation.width * 3;
		unsigned size = LineWidth * m_pWidget->allocation.height;
		tmp = new unsigned char[size];
		if (!tmp) throw (int) 2;
		GdkGLContext *glcontext = gtk_widget_get_gl_context(m_pWidget);
		GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(m_pWidget);
		if (gdk_gl_drawable_gl_begin(gldrawable, glcontext))
		{
			glPixelStorei(GL_PACK_ALIGNMENT, n);
			glReadBuffer(GL_BACK_LEFT);
			glReadPixels(0, 0, m_pWidget->allocation.width, m_pWidget->allocation.height , GL_RGB,
							GL_UNSIGNED_BYTE, tmp);
		}
		else throw (int) 4;
		lines = new unsigned char*[m_pWidget->allocation.height];
		if (!lines) throw (int) 3;
		for (int i = 0, j = m_pWidget->allocation.height - 1; j >= 0; i++, j--)
		{
			lines[i] = tmp + LineWidth * j;
		}
		jpeg_start_compress(&cinfo, true);
		jpeg_write_scanlines(&cinfo, lines, m_pWidget->allocation.height);
		jpeg_finish_compress(&cinfo);
		delete [] tmp;
		delete [] lines;
		fclose(fp);
		jpeg_destroy_compress(&cinfo);
	}
	catch (int num)
	{
		if (tmp) delete [] tmp;
		if (lines) delete [] lines;
		if (num > 0) fclose(fp);
		switch(num)
		{
		case 0: break;
		case 1: break;
		default:
			jpeg_destroy_compress(&cinfo);
			break;
		}
	}*/
}

void gcView::Print(GnomePrintContext *pc, gdouble width, gdouble height)
{
	double hstep, vstep, dxStep, dyStep;
	hstep = m_width;
	vstep = m_height;
	dxStep = m_near;
	dyStep = m_far;
	WidgetData* pData = (WidgetData*)g_object_get_data(G_OBJECT(m_pWidget), "gldata");
	while(gtk_events_pending()) gtk_main_iteration();
	m_width = hstep;
	m_height = vstep;
	m_near = dxStep;
	m_far = dyStep;
	unsigned char *tmp;
	unsigned LineWidth, s = sizeof(int);
	if (m_pWidget->allocation.width & (s - 1))
	{
		LineWidth = ((~(s - 1)) & (m_pWidget->allocation.width * 3)) + s;
	}
	else LineWidth = m_pWidget->allocation.width * 3;
	unsigned size = LineWidth * m_pWidget->allocation.height;
	int i, j;
	//hstep, vstep: size of the printing window in gnome-print units
	hstep = (double) m_pWidget->allocation.width *72 / PrintResolution;
	vstep = (double) m_pWidget->allocation.height *72 / PrintResolution;
	tmp = new unsigned char[size];
	if (!tmp) return;
	double Width, Height;//size of print area
	int n, m, imax, jmax;
	Width = m_pWidget->allocation.width;
	Height = m_pWidget->allocation.height;
	imax = int(Width / hstep);
	jmax = int(Height / vstep);
	dxStep = hstep / Width * 2; 
	dyStep = vstep / Height *2;
	double matrix [6] = {hstep, 0, 0, - vstep,
						0, (height + Height )/ 2};
	for (j = 0; j <= jmax; j++)
	{
		matrix[4] = (width - Width)/ 2;
		matrix[3] = (j < jmax) ? - vstep : - Height + vstep * jmax;
		for (i = 0; i <= imax; i++)
		{
			GdkGLContext *glcontext = gtk_widget_get_gl_context(m_pWidget);
			GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(m_pWidget);
			if (gdk_gl_drawable_gl_begin(gldrawable, glcontext))
			{
				glMatrixMode(GL_PROJECTION);
				glLoadIdentity();
				glFrustum(m_width * ( -1 + i * dxStep), m_width * ( -1 + (i + 1)* dxStep),
							m_height * ( 1 - (j + 1)* dyStep), m_height * ( 1 - j* dyStep), m_near , m_far);
				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity();
				glTranslatef(0, 0, -m_fRadius);
				glClearColor(m_fRed, m_fGreen, m_fBlue, m_fAlpha);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				if  (pData->glList)
				{
					glPushMatrix();
					glRotated(m_psi, 0.0, 1.0, 0.0);
					glRotated(m_theta, 0.0, 0.0, 1.0);
					glRotated(m_phi, 0.0, 1.0, 0.0);
					glCallList(pData->glList);
					glPopMatrix();
				}
				glPixelStorei(GL_PACK_ALIGNMENT, s);
				glReadBuffer(GL_BACK_LEFT);
				glReadPixels(0, 0, m_pWidget->allocation.width, m_pWidget->allocation.height, GL_RGB,
									GL_UNSIGNED_BYTE, tmp);
			}
			gnome_print_gsave(pc);
			matrix[0] = (i < imax) ? hstep : Width - imax * hstep;
			gnome_print_concat (pc, matrix);
			m = (i < imax) ? m_pWidget->allocation.width : (int(Width) * PrintResolution / 72)  % m_pWidget->allocation.width;
			n = (j < jmax) ? m_pWidget->allocation.height: (int(Height) * PrintResolution / 72) % m_pWidget->allocation.height;
			gnome_print_rgbimage(pc, (const guchar *) tmp + (m_pWidget->allocation.height - n) * LineWidth, m, n, LineWidth);
			gnome_print_grestore(pc);
			matrix[4] += hstep;
		}
		matrix[5] -= vstep;
	}
	Update(m_pWidget);
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
