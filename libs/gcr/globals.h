// -*- C++ -*-

/*
 * Gnome Crystal library
 * globals.h
 *
 * Copyright (C) 2001-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
 * USA
 */

#ifndef GCR_GLOBALS_H
#define GCR_GLOBALS_H

#include <glib.h>
#include <goffice/goffice.h>

bool IsEmbedded();

namespace gcr {

class Document;
class View;
Document* GetNewDocument();
void RemoveDocument(Document* pDoc);
bool RequestApp(View* pView);

extern guint PrintWidth, PrintHeight, PrintResolution;
extern guint FoV;
extern gdouble Phi, Theta, Psi;
extern gdouble Red, Green, Blue;

}	//	namespace gcr

#endif	//	GCR_GLOBALS_H
