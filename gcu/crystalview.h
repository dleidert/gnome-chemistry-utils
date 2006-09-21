// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * crystalview.h 
 *
 * Copyright (C) 2002-2004 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef CRYSTAL_VIEW_H
#define CRYSTAL_VIEW_H

#include <libxml/parser.h>
#include <gcu/matrix.h>
#include <gcu/glview.h>
#include <list>
#include <map>
#include <gtk/gtkwidget.h>

using namespace std;

namespace gcu
{
class CrystalDoc;
	
/*!\class CrystalView gcu/crystalview.h
The class representing a view of the model. Each document
might have several views.
Most methods are automatically called by the framework and should not be explicitely used in programs.
*/
class CrystalView: public GLView
{
public:
//!Constructor.
/*!
@param pDoc: a pointer to the CrystalDoc instance.

Creates a new view for the document.
*/
	CrystalView (CrystalDoc* pDoc);
//!Destructor.
/*!
The destructor of CrystalView.
*/
	virtual ~CrystalView ();

/*!
@param widget: a pointer to a widget created by CreateNewWidget().

Initialize the widget. Automatically called by the framework.
*/
//	void Init (GtkWidget *widget);
/*!
@param widget: a pointer to a widget created by CreateNewWidget().

Automatically called by the framework when the  the widget size changes.
*/
//	void Reshape (GtkWidget *widget);
/*!
@param widget: a pointer to a widget created by CreateNewWidget().

Draws the contents of the widget. Automatically called by the framework.
*/
	//void Draw (GtkWidget *widget);
/*!
Updates all widgets of the view. This method must be called each time the document or the view are modified.
*/
//	void Update ();
/*!
@param widget: a pointer to a widget created by CreateNewWidget().

Update the contents of widget. Automatically called by the framework when Update() is executed.
*/
//	void Update (GtkWidget *widget);
/*!
@param widget: a pointer to a widget created by CreateNewWidget().
@param event: a pointer to a GdkEvent.

Automatically called by the framework when a left button click occurs in the widget drawing area.
*/
//	bool OnPressed (GtkWidget *widget, GdkEventButton *event);
/*!
@param widget: a pointer to a widget created by CreateNewWidget().
@param event: a pointer to a GdkEvent.

Automatically called by the framework when the mouse cursor moves over the widget drawing area.
*/
//	void OnMotion (GtkWidget *widget, GdkEventMotion *event);
/*!
@param widget: a pointer to a widget created by CreateNewWidget().

Automatically called by the framework when the widget is destroyed.
*/
//	void OnDestroyed (GtkWidget *widget);

/*!
@param node: a pointer to the xmlNode containing the serialized view.

Loads the parameters of the view from an xmlNode.
*/
	virtual bool Load (xmlNodePtr node);
/*!
@param xml: the xmlDoc used to save the document.
@return a pointer to the xmlNode containig the view parameters or NULL if an error occured.
*/
	virtual xmlNodePtr Save (xmlDocPtr xml);

private:
/*!
@param x: the x component of the rotation.
@param y: the y component of the rotation.

Called by OnMotion(). x and y are the displacement coordinates of the mouse.
*/
//	void Rotate (gdouble x, gdouble y);

protected:
/*!
The field of view.
*/
//	gdouble m_fAngle;
/*!
The distance of the center of the model from the viewer.
*/
//	gdouble m_fRadius;
/*!
Euler's Psi angle giving the orientaion of the crystal in the view.
*/
//	gdouble m_psi;
/*!
Euler's Theta angle giving the orientaion of the crystal in the view.
*/
//	gdouble m_theta;
/*!
Euler's Phi angle giving the orientaion of the crystal in the view.
*/
//	gdouble m_phi;
/*!
The height of the widget.
*/
	gdouble m_height;
/*!
The width of the widget.
*/
	gdouble m_width;
/*!
The distance of the fore plane delimiting the volume active in the OpenGL representation to the viewer.
*/
//	gdouble m_near;
/*
The distance of the back plane delimiting the volume active in the OpenGL representation to the viewer.
*/
//	gdouble m_far;
/*!
The gcu::Matrix used to calculate the absolute coordiantes from the positions relative to the crystal.
*/
//	Matrix m_Euler;
/*!
A GLList used when drawing the model.
*/
//	unsigned m_nGLList;
	//background color
/*!
The blue component of the background color.
*/
//	float m_fBlue;
/*!
The red component of the background color.
*/
//	float m_fRed;
/*!
The green component of the background color.
*/
//	float m_fGreen;
/*!
The alpha component of the background color.
*/
//	float m_fAlpha;

private:
//	bool m_bInit;
//	gdouble m_lastx, m_lasty;
};

} //namespace gcu

#endif //CRYSTAL_VIEW_H
