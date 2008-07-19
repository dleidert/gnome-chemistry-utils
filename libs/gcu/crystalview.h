// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * crystalview.h 
 *
 * Copyright (C) 2002-2008 Jean Bréfort <jean.brefort@normalesup.org>
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

/*!\file*/
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
@param node: a pointer to the xmlNode containing the serialized view.

Loads the parameters of the view from an xmlNode.
*/
	virtual bool Load (xmlNodePtr node);
/*!
@param xml: the xmlDoc used to save the document.
@return a pointer to the xmlNode containig the view parameters or NULL if an error occured.
*/
	virtual xmlNodePtr Save (xmlDocPtr xml) const;

protected:
/*!
The height of the widget.
*/
	gdouble m_height;
/*!
The width of the widget.
*/
	gdouble m_width;
};

} //namespace gcu

#endif //CRYSTAL_VIEW_H
