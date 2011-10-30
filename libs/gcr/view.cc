/*
 * Gnome Chemisty Utils
 * gcr/view.cc
 *
 * Copyright (C) 2002-2011 Jean Bréfort <jean.brefort@normalesup.org>
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

#include "config.h"
#include "view.h"
#include "document.h"
#include "globals.h"
#include <gcu/matrix.h>
#include <gcu/xml-utils.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <glib/gi18n-lib.h>
#include <cmath>
#include <cstring>

namespace gcr
{

guint FoV;
gdouble Psi, Theta, Phi;
gdouble Red, Green, Blue;

View::View (Document* pDoc): gcugtk::GLView (pDoc)
{
	SetAngle (FoV);
	SetRotation (Psi, Theta, Phi);
	SetBlue (Blue);
	SetRed (Red);
	SetGreen (Green);
	SetAlpha (1.0);
	m_Window = NULL;
}

View::~View ()
{
}

bool View::Load (xmlNodePtr node)
{
	char *txt;
	xmlNodePtr child = node->children;
	double x, y, z;
	while (child) {
		if (!strcmp ((gchar*) child->name, "orientation")) {
			txt = (char*) xmlGetProp (child, (xmlChar*) "psi");
			if (txt) {
				sscanf(txt, "%lg", &x);
				xmlFree (txt);
			}	else
				return false;
			txt = (char*) xmlGetProp (child, (xmlChar*) "theta");
			if (txt) {
				sscanf(txt, "%lg", &y);
				xmlFree (txt);
			}	else
				return false;
			txt = (char*) xmlGetProp (child, (xmlChar*) "phi");
			if (txt) {
				sscanf(txt, "%lg", &z);
				xmlFree (txt);
			}	else
				return false;
			SetRotation (x, y, z);
		} else if (!strcmp ((gchar*) child->name, "fov")) {
			txt = (char*) xmlNodeGetContent (child);
			int result = sscanf (txt, "%lg", &x);
			SetAngle ((result == 0)? 10.: x);
			xmlFree (txt);
		}
		child = child->next;
	}
	float r, g, b, a;
	if (!gcu::ReadColor (node, "background", &r, &g, &b, &a))
		return false;
	SetRed (r);
	SetGreen (g);
	SetBlue (b);
	SetAlpha (a);
	return true;
}

xmlNodePtr View::Save (xmlDocPtr xml) const
{
	xmlNodePtr parent, child;
	gchar buf[256];
	parent = xmlNewDocNode (xml, NULL, (xmlChar*) "view", NULL);
	if (!parent)
		return NULL;

	child = xmlNewDocNode (xml, NULL, (xmlChar*) "orientation", NULL);
	if (child)
		xmlAddChild (parent, child);
	else {
		xmlFreeNode (parent);
		return NULL;
	}
	snprintf (buf, sizeof (buf), "%g", GetPsi ());
	xmlNewProp (child, (xmlChar*) "psi", (xmlChar*) buf);
	snprintf (buf, sizeof (buf), "%g", GetTheta ());
	xmlNewProp (child, (xmlChar*) "theta", (xmlChar*) buf);
	snprintf (buf, sizeof (buf), "%g", GetPhi ());
	xmlNewProp (child, (xmlChar*) "phi", (xmlChar*) buf);

	g_snprintf (buf, sizeof (buf) - 1, "%g", GetAngle ());
	child = xmlNewDocNode (xml, NULL, (xmlChar*) "fov", (xmlChar*) buf);
	if (child)
		xmlAddChild (parent, child);
	else {
		xmlFreeNode (parent);
		return NULL;
	}

	if (!gcu::WriteColor (xml, parent, "background", GetRed (), GetGreen (), GetBlue (), GetAlpha ())) {
		xmlFreeNode(parent);
		return NULL;
	}

	return parent;
}

void View::SetBackgroundColor (float red, float green, float blue, float alpha)
{
	SetRed (red);
	SetGreen (green);
	SetBlue (blue);
	SetAlpha (alpha);
}

void View::GetBackgroundColor (double *red, double *green, double *blue, double *alpha)
{
	*red = GetRed ();
	*green = GetGreen ();
	*blue = GetBlue ();
	*alpha = GetAlpha ();
}

void View::GetRotation (double *psi, double *theta, double *phi)
{
	*psi = GetPsi ();
	*theta = GetTheta ();
	*phi = GetPhi ();
}

}	//	namespace gcr
