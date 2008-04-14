// -*- C++ -*-

/* 
 * GChemPaint library
 * electron.cc
 *
 * Copyright (C) 2004-2007 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "electron.h"
#include "atom.h"
#include "document.h"
#include "settings.h"
#include "theme.h"
#include "view.h"
#include "widgetdata.h"
#include <canvas/gcp-canvas-rect-ellipse.h>
#include <canvas/gcp-canvas-group.h>
#include <cmath>
#include <cstring>

#define POSITION_NE 1
#define POSITION_NW 2
#define POSITION_N 4
#define POSITION_SE 8
#define POSITION_SW 16
#define POSITION_S 32
#define POSITION_E 64
#define POSITION_W 128

using namespace gcu;

namespace gcp {

TypeId ElectronType;

Electron::Electron (Atom *pAtom, bool IsPair): Object ()
{
	m_IsPair = IsPair;
	m_pAtom = pAtom;
	m_Pos = 1;
	if (pAtom)
		pAtom->AddElectron (this);
}

Electron::~Electron ()
{
	if (m_pAtom && (GetParent () == m_pAtom))
		// If not, this destructor is called from the Atom destructor and nothing should
		// be done in that case.
		m_pAtom->RemoveElectron (this);
}

char Electron::GetPosition (double *angle, double *distance)
{
	*angle = m_Angle;
	*distance = m_Dist;
	return m_Pos;
}

void Electron::SetPosition (unsigned char Pos, double angle, double distance)
{
	m_Dist = distance;
	if (!Pos)
		m_Angle = angle;
	else {
		switch (Pos) {
		case POSITION_NE:
			m_Angle = 45.;
			break;
		case POSITION_NW:
			m_Angle = 135.;
			break;
		case POSITION_N:
			m_Angle = 90.;
			break;
		case POSITION_SE:
			m_Angle = 315.;
			break;
		case POSITION_SW:
			m_Angle = 225.;
			break;
		case POSITION_S:
			m_Angle = 270.;
			break;
		case POSITION_E:
			m_Angle = 0.;
			break;
		case POSITION_W:
			m_Angle = 180.;
			break;
		}
		if (m_pAtom) {
			m_pAtom->NotifyPositionOccupation (m_Pos, false);
			m_pAtom->NotifyPositionOccupation (Pos, true);
		}
	}
	m_Pos = Pos;
}

void Electron::Add (GtkWidget* w) const
{
	WidgetData* pData = (WidgetData*) g_object_get_data (G_OBJECT (w), "data");
	if (pData->Items[this] != NULL)
		return;
	Theme* pTheme = pData->m_View->GetDoc ()->GetTheme ();
	GnomeCanvasGroup* group = GNOME_CANVAS_GROUP(gnome_canvas_item_new(pData->Group, gnome_canvas_group_ext_get_type(), NULL));
	GnomeCanvasItem* item;
	const char *color = (m_pAtom)? ((pData->IsSelected(m_pAtom))? SelectColor: Color): "white";
	double x, y, angle = m_Angle / 180. * M_PI;
	if (m_Dist != 0.){
		m_pAtom->GetCoords (&x, &y);
		x += m_Dist * cos (angle);
		y -= m_Dist * sin (angle);
		x *= pTheme->GetZoomFactor ();
		y *= pTheme->GetZoomFactor ();
	} else {
		m_pAtom->GetPosition (m_Angle, x, y);
		x *= pTheme->GetZoomFactor ();
		y *= pTheme->GetZoomFactor ();
		x += 2. * cos (angle);
		y -= 2. * sin (angle);
	}
	if (m_IsPair) {
		double deltax = 3. * sin (angle);
		double deltay = 3. * cos (angle);
		item = gnome_canvas_item_new (
						group,
						gnome_canvas_ellipse_ext_get_type (),
						"width_units", 0.0,
						"fill_color", color,
						"x1", x + deltax - 2. ,
						"x2", x + deltax + 2.,
						"y1", y + deltay - 2.,
						"y2", y + deltay + 2.,
						NULL);
		g_signal_connect (G_OBJECT (item), "event", G_CALLBACK (on_event), w);
		g_object_set_data (G_OBJECT (item), "object", (void *) this);
		g_object_set_data (G_OBJECT (group), "0", item);
		item = gnome_canvas_item_new (
						group,
						gnome_canvas_ellipse_ext_get_type (),
						"width_units", 0.0,
						"fill_color", color,
						"x1", x - deltax - 2. ,
						"x2", x - deltax + 2.,
						"y1", y - deltay - 2.,
						"y2", y - deltay + 2.,
						NULL);
		g_object_set_data (G_OBJECT (item), "object", (void *) this);
		g_object_set_data (G_OBJECT (group), "1", item);
	} else {
		item = gnome_canvas_item_new (
						group,
						gnome_canvas_ellipse_ext_get_type (),
						"width_units", 0.0,
						"fill_color", color,
						"x1", x - 2. ,
						"x2", x + 2.,
						"y1", y - 2.,
						"y2", y + 2.,
						NULL);
		g_object_set_data (G_OBJECT (item), "object", (void *) this);
		g_object_set_data (G_OBJECT (group), "0", item);
	}
	g_object_set_data (G_OBJECT (group), "object", (void *) this);
	g_signal_connect (G_OBJECT (item), "event", G_CALLBACK (on_event), w);
	pData->Items[this] = group;
}

void Electron::Update (GtkWidget* w) const
{
	WidgetData* pData = (WidgetData*) g_object_get_data (G_OBJECT (w), "data");
	if (pData->Items[this] == NULL)
		return;
	Theme* pTheme = pData->m_View->GetDoc ()->GetTheme ();
	GnomeCanvasGroup *group = pData->Items[this];
	double x, y, angle = m_Angle / 180. * M_PI;
	if (m_Dist != 0.){
		m_pAtom->GetCoords (&x, &y);
		x += m_Dist * cos (angle);
		y -= m_Dist * sin (angle);
		x *= pTheme->GetZoomFactor ();
		y *= pTheme->GetZoomFactor ();
	} else {
		m_pAtom->GetPosition (m_Angle, x, y);
		x *= pTheme->GetZoomFactor ();
		y *= pTheme->GetZoomFactor ();
		x += 2. * cos (angle);
		y -= 2. * sin (angle);
	}
	if (m_IsPair) {
		double deltax = 3. * sin (angle);
		double deltay = 3. * cos (angle);
		g_object_set(G_OBJECT(g_object_get_data(G_OBJECT(group), "0")),
						"x1", x + deltax - 2. ,
						"x2", x + deltax + 2.,
						"y1", y + deltay - 2.,
						"y2", y + deltay + 2.,
						NULL);
		g_object_set(G_OBJECT(g_object_get_data(G_OBJECT(group), "1")),
						"x1", x - deltax - 2. ,
						"x2", x - deltax + 2.,
						"y1", y - deltay - 2.,
						"y2", y - deltay + 2.,
						NULL);
	} else {
		g_object_set(G_OBJECT(g_object_get_data(G_OBJECT(group), "0")),
						"x1", x - 2. ,
						"x2", x + 2.,
						"y1", y - 2.,
						"y2", y + 2.,
						NULL);
	}
}

void Electron::SetSelected(GtkWidget* w, int state)
{
	WidgetData* pData = (WidgetData*) g_object_get_data (G_OBJECT (w), "data");
	GnomeCanvasGroup* group = pData->Items[this];
	gchar const *color;
	
	switch (state) {	
	case SelStateUnselected:
		color = "black";
		break;
	case SelStateSelected:
		color = SelectColor;
		break;
	case SelStateUpdating:
		color = AddColor;
		break;
	case SelStateErasing:
		color = DeleteColor;
		break;
	default:
		color = "black";
		break;
	}
	g_object_set (G_OBJECT (g_object_get_data (G_OBJECT (group), "0")),
				"fill_color", color, NULL);
	if (m_IsPair)
		g_object_set (G_OBJECT (g_object_get_data (G_OBJECT (group), "1")),
					"fill_color", color, NULL);
}
	
xmlNodePtr Electron::Save (xmlDocPtr xml) const
{
	xmlNodePtr node = xmlNewDocNode (xml, NULL, (xmlChar*) ((m_IsPair)? "electron-pair": "electron"), NULL);
	char *buf;
	if (m_Pos) {
		char const *buf;
		switch (m_Pos) {
		case POSITION_NE:
			buf = "ne";
			break;
		case POSITION_NW:
			buf = "nw";
			break;
		case POSITION_N:
			buf = "n";
			break;
		case POSITION_SE:
			buf = "se";
			break;
		case POSITION_SW:
			buf = "sw";
			break;
		case POSITION_S:
			buf = "s";
			break;
		case POSITION_E:
			buf = "e";
			break;
		case POSITION_W:
			buf = "w";
			break;
		default:
			buf = "def"; // should not occur
		}
		xmlNewProp (node, (xmlChar*) "position", (xmlChar*) buf);
	} else {
		buf = g_strdup_printf ("%g", m_Angle);
		xmlNewProp (node, (xmlChar*) "angle", (xmlChar*) buf);
		g_free (buf);
	}
	if (m_Dist != 0.) {
		buf = g_strdup_printf ("%g", m_Dist);
		xmlNewProp (node, (xmlChar*) "dist", (xmlChar*) buf);
		g_free (buf);
	}
	return node;
}
	
bool Electron::Load (xmlNodePtr node)
{
	char *buf = (char*) xmlGetProp (node, (xmlChar*) "position");
	m_Pos = 0;
	if (buf) {
		if (! strcmp (buf, "ne")) {
			m_Pos = POSITION_NE;
			m_Angle = 45.;
		} else if (! strcmp (buf, "nw")) {
			m_Pos = POSITION_NW;
			m_Angle = 135.;
		} else if (! strcmp (buf, "n")) {
			m_Pos = POSITION_N;
			m_Angle = 90.;
		} else if (! strcmp (buf, "se")) {
			m_Pos = POSITION_SE;
			m_Angle = 315.;
		} else if (! strcmp (buf, "sw")) {
			m_Pos = POSITION_SW;
			m_Angle = 225.;
		} else if (! strcmp (buf, "s")) {
			m_Pos = POSITION_S;
			m_Angle = 270.;
		} else if (! strcmp (buf, "e")) {
			m_Pos = POSITION_E;
			m_Angle = 0.;
		} else if (! strcmp (buf, "w")) {
			m_Pos = POSITION_W;
			m_Angle = 180.;
		}
		xmlFree (buf);
	} else {
		buf = (char*) xmlGetProp (node, (xmlChar*) "angle");
		if (!buf)
			return false;
		sscanf (buf, "%lg", &m_Angle);
		xmlFree (buf);
	}
	buf = (char*) xmlGetProp (node, (xmlChar*) "dist");
	if (buf) {
		sscanf (buf, "%lg", &m_Dist);
		xmlFree (buf);
	} else
		m_Dist = 0.;
	return true;
}

bool Electron::OnSignal (SignalId Signal, Object *Child)
{
	if (Signal != OnDeleteSignal)
		return true;
	Document *pDoc = (Document*) GetDocument ();
	Object *pMol = GetMolecule ();
	Operation *pOp = pDoc->GetNewOperation (GCP_MODIFY_OPERATION);
	pOp->AddObject(pMol, 0);
	SetParent (NULL);
	pDoc->GetView ()->Remove (this);
	if (m_pAtom)
		m_pAtom->Update ();
	pOp->AddObject(pMol, 1);
	pDoc->FinishOperation ();
	return false;
}

void Electron::Transform2D (Matrix2D& m, double x, double y)
{
	double a = m_Angle * M_PI / 180.;
	double xc = cos (a), yc = - sin (a);
	m.Transform (xc, yc);
	a = atan2 (- yc, xc) * 180. / M_PI;
	if (a < 0)
		a += 360;
	SetPosition (0, a, m_Dist);
}

}	//	namespace gcp
