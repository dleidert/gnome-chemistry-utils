// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gcu/chem3ddoc.h
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

#ifndef GCU_CHEM3D_DOCUMENT_H
#define GCU_CHEM3D_DOCUMENT_H

#include <gcu/macros.h>
#include <gcu/gldocument.h>
#include <openbabel/mol.h>

using namespace OpenBabel;

namespace gcu {
	
typedef enum
{
	BALL_AND_STICK,
	SPACEFILL
} Display3DMode;

class Application;

class Chem3dDoc: public GLDocument
{
public:
	Chem3dDoc ();
	Chem3dDoc (Application *App, GLView *View);
	virtual ~Chem3dDoc ();

	void Draw ();
	bool IsEmpty () {return m_Mol.NumNodes () == 0;}
	void Load (char const *uri, char const *mime_type);
	void LoadData (char const *data, char const *mime_type);
	const char *GetTitle () {return m_Mol.GetTitle ();}

private:
	OBMol m_Mol;

GCU_PROP (Display3DMode, Display3D);
};

}	// namespace gcu

#endif	//	GCU_CHEM3D_DOCUMENT_H
