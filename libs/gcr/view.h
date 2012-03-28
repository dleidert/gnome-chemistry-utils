// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gcr/view.h
 *
 * Copyright (C) 2002-2011 Jean Bréfort <jean.brefort@normalesup.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the
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

#ifndef GCR_VIEW_H
#define GCR_VIEW_H

#include <libxml/parser.h>
#include <gcu/matrix.h>
#include <gcugtk/glview.h>
#include <list>
#include <map>
#include <gtk/gtk.h>

/*!\file*/
namespace gcr
{
class Document;
class Window;

/*!\class View gcr/view.h
The class representing a view of the model. Each document
might have several views.
Most methods are automatically called by the framework and should not be explicitely used in programs.
*/
class View: public gcugtk::GLView
{
public:
//!Constructor.
/*!
@param pDoc: a pointer to the Document instance.

Creates a new view for the document.
*/
	View (Document* pDoc);
//!Destructor.
/*!
The destructor of View.
*/
	virtual ~View ();

/*!
@param node: a pointer to the xmlNode containing the serialized view.

Loads the parameters of the view from an xmlNode.
*/
	virtual bool Load (xmlNodePtr node);
/*!
@param xml: the xmlDoc used to save the document.
@return a pointer to the xmlNode containig the view parameters or NULL if an error occured.
*/
	virtual xmlNodePtr Save (xmlDocPtr xml) const;

	void SetBackgroundColor (float red, float green, float blue, float alpha);
	void GetBackgroundColor (double *red, double *green, double *blue, double *alpha);
	gdouble& GetFoV () {return GetRefAngle ();}
	gdouble& GetPos () {return m_Radius;}
	void GetRotation (double *psi, double *theta, double *phi);

protected:
/*!
The height of the widget.
*/
	gdouble m_height;
/*!
The width of the widget.
*/
	gdouble m_width;

GCU_POINTER_PROP (Window, Window);
};

}	//	namespace gcr

#endif	//	GCR_VIEW_H
