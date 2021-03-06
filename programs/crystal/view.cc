// -*- C++ -*-

/*
 * Gnome Crystal
 * view.cc
 *
 * Copyright (C) 2000-2011 Jean Bréfort <jean.brefort@normalesup.org>
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

#include "config.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <libxml/parserInternals.h>
#include <libxml/xmlmemory.h>
#include "view.h"
#include "document.h"
#include <cmath>
#include <cstring>

extern GtkWidget *vbox1;

gcView::gcView (gcDocument *pDoc): gcr::View (pDoc)
{
}

gcView::gcView (gcView *pView):
	gcr::View (dynamic_cast <gcr::Document *> (pView->GetDoc ()))
{
	SetAngle (pView->GetAngle ());
	SetRotation (pView->GetPsi (), pView->GetTheta (), pView->GetPhi ());
	SetBlue (pView->GetBlue ());
	SetRed (pView->GetRed ());
	SetGreen (pView->GetGreen ());
	SetAlpha (pView->GetAlpha ());
	dynamic_cast <gcDocument*> (GetDoc())->AddView (this);
}

gcView::~gcView ()
{
	gtk_widget_destroy (GTK_WIDGET(m_pMenu));
}

bool gcView::LoadOld (xmlNodePtr node)
{
	char *txt;
	xmlNodePtr child = node->children;
	while(child) {
		if (!strcmp ((char *) child->name, "orientation")) {
			txt = (char*) xmlNodeGetContent (child);
			if (txt) {
				double y, t, h;
				sscanf (txt, "%lg %lg %lg", &y, &t, &h);
				SetRotation (y, t, h);
				xmlFree (txt);
			}
		} else if (!strcmp ((char *) child->name, "background")) {
			txt = (char*) xmlNodeGetContent (child);
			if (txt) {
				float b, r, g, a;
				sscanf (txt, "%g %g %g %g", &b, &r, &g, &a);
				SetRed (r);
				SetGreen (g);
				SetBlue (b);
				SetAlpha (a);
				xmlFree(txt);
			}
		} else if (!strcmp((char *)child->name, "fov")) {
			txt = (char*) xmlNodeGetContent (child);
			if (txt) {
				double x;
				sscanf (txt, "%lg", &x);
				SetAngle (x);
				xmlFree (txt);
			}
		}
		child = child->next;
	}
	return true;
}
