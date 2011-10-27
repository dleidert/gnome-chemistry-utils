// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gcu/glview.cc
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
#include <gcu/application.h>
#include <gcu/gldocument.h>
#include <gcu/glview.h>
#include <gcu/macros.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <glib/gi18n-lib.h>
#include <cmath>
#include <cstring>

#define ROOTDIR "/apps/gchemutils/gl/"

double DefaultPsi = 70.;
double DefaultTheta = 10.;
double DefaultPhi = -90.;

using namespace std;

namespace gcu
{

// GLView implementation

GLView::GLView (GLDocument* pDoc) throw (std::runtime_error)
{
	m_Doc = pDoc;
	m_Red = m_Green = m_Blue = 0.;
	m_Alpha = 1.;
	m_Angle = 10.;
	SetRotation (DefaultPsi, DefaultTheta, DefaultPhi);
}

GLView::~GLView ()
{
}

void GLView::Update()
{
}

void GLView::SetRotation(double psi, double theta, double phi)
{
	m_Psi = psi;
	m_Theta = theta;
	m_Phi = phi;
	Matrix m (m_Psi / 180 * M_PI, m_Theta / 180 * M_PI, m_Phi / 180 * M_PI, euler);
	m_Euler = m;
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
	GOutputStream *output = (GOutputStream *) data;
	while (count) {
		count -= g_output_stream_write (output, buf, count, NULL, error);
		if (*error)
			return false;
	}
	return true;
}

void GLView::SaveAsImage (string const &filename, char const *type, map<string, string>& options, unsigned width, unsigned height, bool use_bg) const
{
	if (width == 0 || height == 0)
		return;

	GdkPixbuf *pixbuf = BuildPixbuf (width, height, use_bg);

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
		GFile *file = g_vfs_get_file_for_uri (g_vfs_get_default (), filename.c_str ());
		GFileOutputStream *output = g_file_create (file, G_FILE_CREATE_NONE, NULL, &error);
		if (!error)
			gdk_pixbuf_save_to_callbackv (pixbuf, do_save_image, output, type, (char**) keys, (char**) values, &error);
		if (error) {
			fprintf (stderr, _("Unable to save image file: %s\n"), error->message);
			g_error_free (error);
		}
		g_object_unref (file);
		g_free (keys);
		g_free (values);
		g_object_unref (pixbuf);
	}
}

GdkPixbuf *GLView::BuildPixbuf (G_GNUC_UNUSED unsigned width, G_GNUC_UNUSED unsigned height, G_GNUC_UNUSED bool use_bg) const
{
	GdkPixbuf *pixbuf = NULL;
	g_warning ("Off-screen rendering not supported in this context");
	// TODO: implement rendering using an external program and osmesa
	return pixbuf;
}

void GLView::RenderToCairo (cairo_t *cr, unsigned width, unsigned height, bool use_bg) const
{
	double scale = 72. / 300.;
	GdkPixbuf *pixbuf = BuildPixbuf (width / scale, height / scale, use_bg);
	GOImage *img = GO_IMAGE (go_pixbuf_new_from_pixbuf (pixbuf));
	cairo_scale (cr, scale, scale);
	go_image_draw (img, cr);
	g_object_unref (img);
	g_object_unref (pixbuf);
}

}	//	namespace gcu
