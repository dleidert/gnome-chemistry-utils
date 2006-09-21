/* 
 * Gnome Chemisty Utils
 * crystalview.cc 
 *
 * Copyright (C) 2002-2005 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "crystalview.h"
#include "crystaldoc.h"
#include "matrix.h"
#include "xml-utils.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <gtk/gtkgl.h>
#include <math.h>
#include <glib/gi18n-lib.h>
#include <libgnomevfs/gnome-vfs-ops.h>

using namespace gcu;

/*typedef struct
{
	unsigned glList;
} WidgetData;

static bool on_init(GtkWidget *widget, CrystalView* View) 
{
	View->Init(widget);
	return true;
}

bool on_reshape(GtkWidget *widget, GdkEventConfigure *event, CrystalView* View) 
{
	View->Reshape(widget);
	return true;
}

static bool on_draw(GtkWidget *widget, GdkEventExpose *event, CrystalView* View) 
{*/
	/* Draw only last expose. */
/*	if (event->count > 0) return TRUE;

	View->Draw(widget);
	return true;
}

static bool on_motion(GtkWidget *widget, GdkEventMotion *event, CrystalView* View) 
{
	View->OnMotion(widget, event);
	return true;
}

static bool on_pressed(GtkWidget *widget, GdkEventButton *event, CrystalView* View) 
{
	return View->OnPressed(widget, event);
}

static void on_destroyed(GtkWidget *widget, CrystalView *pView)
{
	pView->OnDestroyed(widget);
}*/

CrystalView::CrystalView(CrystalDoc* pDoc): GLView (pDoc)
{
	SetRed (1.);
	SetGreen (1.);
	SetBlue (1.);
}

CrystalView::~CrystalView()
{
}

bool CrystalView::Load (xmlNodePtr node)
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
	if (!ReadColor (node, "background", &r, &g, &b, &a))
		return false;
	SetRed (r);
	SetGreen (g);
	SetBlue (b);
	SetAlpha (a);
	return true;
}

xmlNodePtr CrystalView::Save(xmlDocPtr xml)
{
	xmlNodePtr parent, child;
	gchar buf[256];
	parent = xmlNewDocNode(xml, NULL, (xmlChar*)"view", NULL);
	if (!parent) return NULL;
	
	child = xmlNewDocNode(xml, NULL, (xmlChar*)"orientation", NULL);
	if (child) xmlAddChild(parent, child);
	else {xmlFreeNode(parent); return NULL;}
	snprintf(buf, sizeof(buf), "%g", GetPsi ());
	xmlNewProp(child, (xmlChar*)"psi", (xmlChar*)buf);
	snprintf(buf, sizeof(buf), "%g", GetTheta ());
	xmlNewProp(child, (xmlChar*)"theta", (xmlChar*)buf);
	snprintf(buf, sizeof(buf), "%g", GetPhi ());
	xmlNewProp(child, (xmlChar*)"phi", (xmlChar*)buf);
	
	g_snprintf(buf, sizeof(buf) - 1, "%g", GetAngle ());
	child = xmlNewDocNode(xml, NULL, (xmlChar*)"fov", (xmlChar*)buf);
	if (child) xmlAddChild(parent, child);
	else {xmlFreeNode(parent); return NULL;}
	
	if (!WriteColor(xml, parent, "background", GetRed (), GetGreen (), GetBlue (), GetAlpha ())) {xmlFreeNode(parent); return NULL;}
	
	return parent;
}
