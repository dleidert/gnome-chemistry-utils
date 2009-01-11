// -*- C++ -*-

/* 
 * Gnome Crystal
 * globals.h 
 *
 * Copyright (C) 2001-2003 Jean Bréfort <jean.brefort@normalesup.org>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02111-1307
 * USA
 */
#include <glib.h>
#ifdef HAVE_GO_CONF_SYNC
#	include <goffice/app/go-conf.h>
#else
#	include <gconf/gconf-client.h>
#endif

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
#ifdef HAVE_GO_CONF_SYNC
	extern GOConfNode *node;
#else
	extern GConfClient *conf_client;
#endif
