// -*- C++ -*-

/*
 * Gnome Crystal
 * document.h
 *
 * Copyright (C) 2000-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
 * USA
 */

#ifndef GCRYSTAL_DOCUMENT_H
#define GCRYSTAL_DOCUMENT_H

#include <libxml/parser.h>
#include <gcr/document.h>
#include <gcu/dialog.h>
#include <gcu/macros.h>
#include "atom.h"
#include "line.h"
#include "cleavage.h"

#ifdef HAVE_OPENBABEL_2_2
namespace OpenBabel {
	class OBMol;
}
#endif

class gcView;
class gcApplication;

class gcDocument: public gcr::Document
{
	//Constructor and destructor
public:
	gcDocument (gcApplication *App);
	~gcDocument ();

	//Interface
public:
	void ParseXMLTree(xmlNode* xml);
	void OnNewDocument();
	gcr::View* CreateNewView();
	gcr::Atom* CreateNewAtom();
	gcr::Line* CreateNewLine();
	gcr::Cleavage* CreateNewCleavage();
	const char* GetProgramId() const;
};

#endif //GCRYSTAL_DOCUMENT_H
