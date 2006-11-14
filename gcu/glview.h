// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gcu/glview.h 
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

#ifndef GCU_GL_VIEW_H
#define GCU_GL_VIEW_H

#include "macros.h"
#include "matrix.h"
#include <gtk/gtkwidget.h>
#include <libgnomeprint/gnome-print.h>
#include <map>

extern double DefaultPsi, DefaultTheta, DefaultPhi;

using namespace std;

namespace gcu {

class GLDocument;

class GLView
{
public:
//!Constructor.
/*!
@param pDoc: a pointer to the GLDocument instance.

Creates a view for the document.
*/
	GLView (GLDocument* pDoc);
//!Destructor.
/*!
The destructor of GLView.
*/
	virtual ~GLView ();

	GtkWidget *GetWidget () {return m_pWidget;}
/*!
Initialize the associated widget. Automatically called by the framework.
*/
	void Init ();
/*!
Automatically called by the framework when the associated widget size changes.
*/
	void Reshape ();
/*!
Draws the contents of the associated widget. Automatically called by the framework.
*/
	void Draw ();
/*!
@param event: a pointer to a GdkEvent.

Automatically called by the framework when a left button click occurs in the
associated widget drawing area.
*/
	bool OnPressed (GdkEventButton *event);
/*!
@param event: a pointer to a GdkEvent.

Automatically called by the framework when the mouse cursor moves over the
associated widget drawing area.

@return true if a motion really occured, false otherwise.
*/
	bool OnMotion (GdkEventMotion *event);
/*!
Update the contents of the associated widget. This method must be called
each time the document or the view are modified.
*/
	void Update ();
	void SetRotation (double psi, double theta, double phi);
	void Print (GnomePrintContext *pc, gdouble width, gdouble height);
/*!
@param filename: the name of the file.
@param type: the type as supported by GdkPixbuf (e.g. "png" or "jpeg").
@param options: the pairs of keys/values to pass GdkPixbuf.
@param resolution: the resolution to use. This means that the image size will
be the view size multiplied by resolution divided by the screen resolution.
If 0, the screen resolution is used.

Export the view contents as an image.
*/
	void SaveAsImage (string const &filename, char const *type, map<string, string>& options, int resolution = 0);

private:
/*!
@param x: the x component of the rotation.
@param y: the y component of the rotation.

Called by OnMotion(). x and y are the displacement coordinates of the mouse.
*/
	void Rotate (gdouble x, gdouble y);

protected:
	GtkWidget *m_pWidget;
	unsigned m_nGLList;

private:
	bool m_bInit;
	Matrix m_Euler;
	double m_Height, m_Width, m_Near, m_Far;
	double m_Lastx, m_Lasty;

// Properties
GCU_PROP (double, Angle)
GCU_PROP (double, Psi)
GCU_PROP (double, Phi)
GCU_PROP (double, Theta)
GCU_PROP (float, Red)
GCU_PROP (float, Green)
GCU_PROP (float, Blue)
GCU_PROP (float, Alpha)
GCU_RO_PROP (GLDocument *, Doc)
GCU_PROT_PROP (double, Radius);
};

}	// namespace gcu

#endif	//	GCU_GL_VIEW_H
