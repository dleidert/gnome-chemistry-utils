// -*- C++ -*-

/* 
 * Gnome Crystal
 * globals.h 
 *
 * Copyright (C) 2001-2009 Jean Bréfort <jean.brefort@normalesup.org>
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
#include <glib.h>
#include <goffice/goffice.h>

bool IsEmbedded();

class gcDocument;
class gcView;
gcDocument* GetNewDocument();
void RemoveDocument(gcDocument* pDoc);
bool RequestApp(gcView* pView);

extern guint PrintWidth, PrintHeight, PrintResolution;
extern guint FoV;
extern gdouble Phi, Theta, Psi;
extern gdouble Red, Green, Blue;
extern GOConfNode *node;
