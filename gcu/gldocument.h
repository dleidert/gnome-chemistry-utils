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

/*!\class GLDocument gcu/gldocument.h
This class is a base class for documents representing 3d objects.
*/

class GLDocument: public Document
{
public:
	GLDocument (Application *App);
	virtual ~GLDocument ();

	virtual void Draw () = 0;

// Properties
GCU_PROT_PROP (double, MaxDist);
GCU_PROT_PROP (GLView*, View);
};

};	//	namespace gcu

#endif	//	GCU_GL_DOCUMENT_H
