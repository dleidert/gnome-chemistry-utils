// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gcu/gldocument.h 
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

#ifndef GCU_GL_DOCUMENT_H
#define GCU_GL_DOCUMENT_H

#include "document.h"
#include "macros.h"

namespace gcu
{

class GLView;
class Matrix;

/*!\class GLDocument gcu/gldocument.h
This class is a base class for documents representing 3d objects.
*/

class GLDocument: public Document
{
public:
/*!
@param App the application owning the new document.

Default constructor.
*/
	GLDocument (Application *App);
/*!
Default destructor.
*/
	virtual ~GLDocument ();

/*!
@param m the Matrix giving the current model orientation

Called by GLView::Update to render the model.
*/
	virtual void Draw (Matrix &m) = 0;

// Properties
/*!\var m_MaxDist
The longest distance between any object and the center of the model.
*/
/*!\fn GetMaxDist()
@return the longest distance between any object and the center of the model.
*/
GCU_PROT_PROP (double, MaxDist);
/*!\var m_View
The associated GLView instance.
*/
/*!\fn GetView()
@return the associated GLView instance.
*/
GCU_PROT_PROP (GLView*, View);
};

};	//	namespace gcu

#endif	//	GCU_GL_DOCUMENT_H
