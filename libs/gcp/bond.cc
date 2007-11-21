// -*- C++ -*-

/* 
 * GChemPaint library
 * bond.cc 
 *
 * Copyright (C) 2001-2007 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "bond.h"
#include "atom.h"
#include "fragment.h"
#include "cycle.h"
#include "settings.h"
#include "document.h"
#include "theme.h"
#include "view.h"
#include "widgetdata.h"
#include <canvas/gcp-canvas-group.h>
#include <canvas/gcp-canvas-line.h>
#include <canvas/gcp-canvas-bpath.h>
#include <canvas/gcp-canvas-polygon.h>
#include <glib/gi18n-lib.h>
#include <math.h>

using namespace gcu;
using namespace std;

namespace gcp {

Bond::Bond (): gcu::Bond ()
{
	m_CoordsCalc = false;
	m_type = NormalBondType;
	m_level = 0;
}

Bond::Bond (Atom* first, Atom* last, unsigned char order):
		gcu::Bond (first, last, order)
{
	m_CoordsCalc = false;
	m_type = NormalBondType;
	m_level = 0;
}

Bond::~Bond ()
{
}

void Bond::SetType (BondType type)
{
	m_type = type;
	m_CoordsCalc = false;
	if (m_type != NormalBondType)
		m_order = 1;
}

double Bond::GetAngle2D (Atom* pAtom)
{
	double x1, y1, x2, y2;
	m_Begin->GetCoords (&x1, &y1);
	m_End->GetCoords (&x2, &y2);
	x2 -= x1;
	y2 -= y1;
	double length = square (x2) + square (y2);
	if (length == 0.0)
		return HUGE_VAL;
	if (pAtom == m_Begin)
		return atan2 (-y2, x2) * 180. / M_PI;
	else if (pAtom == m_End)
		return atan2 (y2, -x2) * 180. / M_PI;
	return HUGE_VAL;
}

double Bond::GetAngle2DRad (Atom* pAtom)
{
	double x1, y1, x2, y2;
	m_Begin->GetCoords (&x1, &y1);
	m_End->GetCoords (&x2, &y2);
	x2 -= x1;
	y2 -= y1;
	double length = square (x2) + square (y2);
	if (length == 0.0)
		return HUGE_VAL;
	if (pAtom == m_Begin)
		return atan2 (-y2, x2);
	else if (pAtom == m_End)
		return atan2 (y2, -x2);
	return HUGE_VAL;
}

Cycle* Bond::GetFirstCycle (std::list<Cycle*>::iterator& i, Cycle * pCycle)
{
	i = m_Cycles.begin ();
	return GetNextCycle (i, pCycle);
}

Cycle* Bond::GetNextCycle (std::list<Cycle*>::iterator& i, Cycle * pCycle)
{
	if (*i == pCycle)
		i++;
	if (i == m_Cycles.end ())
		return NULL;
	pCycle = *i;
	i++;
	return pCycle;
}

bool Bond::IsInCycle (Cycle* pCycle)
{
	std::list<Cycle*>::iterator i, end = m_Cycles.end ();
	for (i = m_Cycles.begin (); i != end; i++)
		if ((*i) == pCycle)
			return true;
	return false;
}

bool Bond::GetLine2DCoords (unsigned Num, double* x1, double* y1, double* x2, double* y2)
{
	if ((Num == 0) || (Num > m_order))
		return false;
	if (!m_CoordsCalc) {
		Theme *Theme = dynamic_cast <Document *> (GetDocument ())->GetTheme ();
		m_Begin->GetCoords (x1, y1);
		m_End->GetCoords (x2, y2);
		double dx = *x2 - *x1, dy = *y2 - *y1;
		double l = sqrt (square (dx) + square (dy));
		double BondDist = Theme->GetBondDist () / Theme->GetZoomFactor ();
		dx *= (BondDist / l);
		dy *= (BondDist / l);
		if (m_order & 1) {
			m_coords[0] = *x1;
			m_coords[1] = *y1;
			m_coords[2] = *x2;
			m_coords[3] = *y2;
			if (m_order == 3) {
				m_coords[4] = *x1 - dy;
				m_coords[5] = *y1 + dx;
				m_coords[6] = *x2 - dy;
				m_coords[7] = *y2 + dx;
				m_coords[8] = *x1 + dy;
				m_coords[9] = *y1 - dx;
				m_coords[10] = *x2 + dy;
				m_coords[11] = *y2 - dx;
			}
		} else if ((m_order == 2) && IsCyclic ()) {
			m_coords[0] = *x1;
			m_coords[1] = *y1;
			m_coords[2] = *x2;
			m_coords[3] = *y2;
			Cycle* pCycle;
			if (IsCyclic() > 1) {
				//Search prefered cycle
				std::list<Cycle*>::iterator i = m_Cycles.begin (), end = m_Cycles.end ();
				pCycle = *i;
				for (; i != end; i++)
					if (pCycle->IsBetterForBonds (*i))
						pCycle = *i;
			} else
				pCycle = m_Cycles.front();
			double a0 = atan2 (*y1 - *y2, *x2 - *x1), a1, a2;
			pCycle->GetAngles2D (this, &a1, &a2);
			if (sin(a0 - a1) * sin (a0 - a2) > 0) {
				double sign = sin (a0 - a1) > 0.0 ? 1.0 : -1.0;
				double tanb = fabs (tan ((M_PI - a0 + a1) / 2)), cosa = cos (a0), sina = sin (a0);
				m_coords[4] = *x1 + BondDist * cosa * tanb - dy * sign;
				m_coords[5] = *y1 + dx * sign - BondDist * sina * tanb;
				tanb = fabs (tan ((a2 - a0) / 2));
				m_coords[6] = *x2 - BondDist * cosa * tanb - dy * sign;
				m_coords[7] = *y2 + dx * sign + BondDist * sina * tanb;
			} else {
				m_coords[0] = *x1 - dy / 2;
				m_coords[1] = *y1 + dx / 2;
				m_coords[2] = *x2 - dy / 2;
				m_coords[3] = *y2 + dx / 2;
				m_coords[4] = *x1 + dy / 2;
				m_coords[5] = *y1 - dx / 2;
				m_coords[6] = *x2 + dy / 2;
				m_coords[7] = *y2 - dx / 2;
			}
		} else {
			m_coords[0] = *x1 - dy / 2;
			m_coords[1] = *y1 + dx / 2;
			m_coords[2] = *x2 - dy / 2;
			m_coords[3] = *y2 + dx / 2;
			m_coords[4] = *x1 + dy / 2;
			m_coords[5] = *y1 - dx / 2;
			m_coords[6] = *x2 + dy / 2;
			m_coords[7] = *y2 - dx / 2;
			if (m_order == 4) {
				m_coords[8] = *x1 - dy * 1.5;
				m_coords[9] = *y1 + dx * 1.5;
				m_coords[10] = *x2 - dy * 1.5;
				m_coords[11] = *y2 + dx * 1.5;
				m_coords[12] = *x1 + dy * 1.5;
				m_coords[13] = *y1 - dx * 1.5;
				m_coords[14] = *x2 + dy * 1.5;
				m_coords[15] = *y2 - dx * 1.5;
			}
		}
		m_CoordsCalc = true;
	}
	Num--;
	Num *= 4;
	*x1 = m_coords[Num++];
	*y1 = m_coords[Num++];
	*x2 = m_coords[Num++];
	*y2 = m_coords[Num++];
	return true;
}

Object* Bond::GetAtomAt(double x, double y, double z)
{
	double x1, y1;
	m_Begin->GetCoords (&x1, &y1);
	if ((fabs (x - x1) < 10) && (fabs (y - y1) < 10))
		return m_Begin;
	m_End->GetCoords (&x1, &y1);
	if ((fabs (x - x1) < 10) && (fabs (y - y1) < 10))
		return m_End;
	return NULL;
}

bool Bond::SaveNode (xmlDocPtr xml, xmlNodePtr node)
{
	switch(m_type) {
	case UpBondType:
		xmlNewProp (node, (xmlChar*) "type", (xmlChar*) "up");
		break;
	case DownBondType:
		xmlNewProp (node, (xmlChar*) "type", (xmlChar*) "down");
		break;
	case ForeBondType:
		xmlNewProp (node, (xmlChar*) "type", (xmlChar*) "fore");
		break;
	case UndeterminedBondType:
		xmlNewProp (node, (xmlChar*) "type", (xmlChar*) "undetermined");
		break;
	default:
		break;
	}
	if (m_level != 0) {
		char *buf = g_strdup_printf ("%d", m_level);
		xmlNewProp (node, (xmlChar*) "level", (xmlChar*) buf);
		g_free (buf);
	}
	return true;
}

bool Bond::LoadNode (xmlNodePtr node)
{
	char* buf;
	buf = (char*) xmlGetProp (node, (xmlChar*) "type");
	if (!buf)
		SetType (NormalBondType);
	else if (!strcmp (buf, "up"))
		SetType (UpBondType);
	else if (!strcmp (buf, "down"))
		SetType (DownBondType);
	else if (!strcmp (buf, "fore"))
		SetType (ForeBondType);
	else if (!strcmp (buf, "undetermined"))
		SetType (UndeterminedBondType);
	else
		SetType (NormalBondType);
	if (buf)
		xmlFree (buf);
	buf = (char*) xmlGetProp (node, (xmlChar*) "level");
	if (buf) {
		m_level = atoi (buf);
		xmlFree (buf);
	}
	return true;
}

void Bond::IncOrder (int n)
{
	if (!((Atom*) GetAtom(0))->AcceptNewBonds () ||
		!((Atom*) GetAtom (1))->AcceptNewBonds ())
		m_order = 1;
	else {
		gcu::Bond::IncOrder (n);
		if (m_order == 4)
			m_order = 1;	//avoid quadruple bonds for now
	}
	m_CoordsCalc = false;
	((Atom*) m_Begin)->Update ();
	((Atom*) m_End)->Update ();
}

double Bond::GetDist (double x, double y)
{
	double x1, y1, x2, y2, l, d1, d2;
	Theme *Theme = dynamic_cast <Document *> (GetDocument ())->GetTheme ();
	double BondDist = Theme->GetBondDist () / Theme->GetZoomFactor ();
	m_Begin->GetCoords (&x1, &y1);
	m_End->GetCoords (&x2, &y2);
	d1 = (x2 - x1) * (x1 - x) + (y2 - y1) * (y1 - y);
	d2 = (x2 - x1) * (x2 - x) + (y2 - y1) * (y2 - y);
	if ((d1 < 0.0) && (d2 < 0.0))
		return sqrt (square (x2 - x) + square (y2 - y));
	if ((d1 > 0.0) && (d2 > 0.0))
		return sqrt (square (x1 - x) + square (y1 - y));
	x2 -= x1;
	y2 -= y1;
	x -= x1;
	y -= y1;
	l = fabs (x2 * y - y2 * x) / sqrt(square (x2) + square (y2));
	return (l < BondDist * (m_order - 1)) ? 0 : l - BondDist * (m_order - 1);
}

void Bond::AddCycle (Cycle* pCycle)
{
	m_Cycles.push_back (pCycle);
	if ((m_order == 2) && m_CoordsCalc)
		SetDirty();
}

void Bond::RemoveCycle (Cycle* pCycle)
{
	m_Cycles.remove (pCycle);
	if ((m_order == 2) && m_CoordsCalc)
		SetDirty();
}

void Bond::SetDirty ()
{
	Document *pDoc = (Document*) GetDocument ();
	if (pDoc)
		pDoc->NotifyDirty (this);
	m_CoordsCalc = false;
}

void Bond::RemoveAllCycles ()
{
	m_Cycles.clear ();
	if (m_order == 2) {
		Document *pDoc = (Document*) GetDocument ();
		if (pDoc)
			pDoc->NotifyDirty (this);
		m_CoordsCalc = false;
	}
}

void Bond::Move (double x, double y, double z)
{
	m_CoordsCalc = false;
}

void Bond::Transform2D (Matrix2D& m, double x, double y)
{
	m_CoordsCalc = false;
}

void Bond::Revert ()
{
	m_CoordsCalc = false;
	gcu::Atom* pAtom = m_Begin;
	m_Begin = m_End;
	m_End = pAtom;
}

double Bond::Get2DLength ()
{
	double x1, y1, x2, y2;
	m_Begin->GetCoords (&x1, &y1);
	m_End->GetCoords (&x2, &y2);
	return sqrt (square (x1 - x2) + square (y1 - y2));
}

void Bond::SetSelected (GtkWidget* w, int state)
{
	if (!m_order)
		return;
	WidgetData* pData = (WidgetData*) g_object_get_data (G_OBJECT (w), "data");
	GnomeCanvasGroup* group = pData->Items[this];
	gchar const *color;
	switch (state) {	
	case SelStateUnselected:
		color = Color;
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
		color = Color;
		break;
	}
	void *obj = g_object_get_data (G_OBJECT (group), "path");
	switch (GetType ())
	{
		case NormalBondType:
		case UndeterminedBondType:
			g_object_set (obj, "outline_color", color, NULL);
			break;
		case ForeBondType:
		case UpBondType:
		case DownBondType:
			g_object_set (obj, "fill_color", color, NULL);
			break;
	}
}

void Bond::Add (GtkWidget* w)
{
	if (!w)
		return;
	WidgetData* pData = (WidgetData*) g_object_get_data (G_OBJECT (w), "data");
	Theme *pTheme = pData->m_View->GetDoc ()->GetTheme ();
	gcu::Atom *pAtom0, *pAtom1;
	if (!(pAtom0 = GetAtom (0)))
		return; 
	if (!(pAtom1 = GetAtom (1)))
		return;
	unsigned char order = GetOrder ();
	if (order == 0)
		return;
	GnomeCanvasGroup* group;
	GnomeCanvasItem* item = NULL;
	pData = (WidgetData*) g_object_get_data (G_OBJECT (w), "data");
	group = GNOME_CANVAS_GROUP (gnome_canvas_item_new (pData->Group, gnome_canvas_group_ext_get_type (), NULL));
	g_signal_connect (G_OBJECT (group), "event", G_CALLBACK (on_event), w);
	g_object_set_data (G_OBJECT (group), "object", this);
	bool result = false;
	if (m_Crossing.size () > 0) {
		map<Bond*, BondCrossing>::iterator i, iend = m_Crossing.end ();
		for (i = m_Crossing.begin (); i != iend; i++)
			if ((result |= (*i).second.is_before))
				break;
	}
	GnomeCanvasPathDef *path = NULL;
	if (result) {
		path = BuildCrossingPathDef (pData);
		if (path) {
			GnomeCanvasItem *item = (m_type == NormalBondType || m_type == UndeterminedBondType)?
				gnome_canvas_item_new (
					group,
					gnome_canvas_bpath_ext_get_type (),
					"bpath", path,
					"outline_color", "white",
					"width_units", pTheme->GetBondWidth () * 3.,
					NULL):
				gnome_canvas_item_new (
					group,
					gnome_canvas_bpath_ext_get_type (),
					"bpath", path,
					"fill_color", "white",
					"width_units", 0.,
					NULL);
			g_object_set_data (G_OBJECT (group), "back", item);
			g_object_set_data (G_OBJECT (item), "object", this);
			g_signal_connect (G_OBJECT (item), "event", G_CALLBACK (on_event), w);
			/* now bring to front, FIXME: not secure ! */
			gnome_canvas_item_lower_to_bottom (item);
			gnome_canvas_item_raise_to_top (GNOME_CANVAS_ITEM (group));
			gcu::Atom *pAtom = GetAtom (0);
			if (pAtom->GetZ () != 6 || static_cast <gcp::Atom*> (pAtom)->GetShowSymbol ())
				gnome_canvas_item_raise_to_top (GNOME_CANVAS_ITEM (pData->Items[pAtom]));
			pAtom = GetAtom (1);
			if (pAtom->GetZ () != 6 || static_cast <gcp::Atom*> (pAtom)->GetShowSymbol ())
				gnome_canvas_item_raise_to_top (GNOME_CANVAS_ITEM (pData->Items[pAtom]));
			gnome_canvas_path_def_unref (path);
		}
	}
	path = BuildPathDef (pData);
	switch (GetType ())
	{
		case NormalBondType:
		case UndeterminedBondType:
			item = gnome_canvas_item_new(
									group,
									gnome_canvas_bpath_ext_get_type (),
									"bpath", path,
									"outline_color", (pData->IsSelected (this))? SelectColor: Color,
									"width_units", pTheme->GetBondWidth (),
									NULL);
			break;
		case ForeBondType:
		case UpBondType:
		case DownBondType:
			item = gnome_canvas_item_new (
									group,
									gnome_canvas_bpath_ext_get_type (),
									"bpath", path,
									"fill_color", (pData->IsSelected (this))? SelectColor: Color,
									"width_units", 0.,
									NULL);
			break;
	}
	gnome_canvas_path_def_unref (path);
	g_object_set_data (G_OBJECT (group), "path", item);
	g_object_set_data (G_OBJECT (item), "object", this);
	g_signal_connect (G_OBJECT (item), "event", G_CALLBACK (on_event), w);
	pData->Items[this] = group;
	if (pAtom0->GetParent ()->GetType () == FragmentType)
		gnome_canvas_item_raise_to_top (GNOME_CANVAS_ITEM (pData->Items[pAtom0->GetParent ()]));
	else if (pAtom0->GetZ() != 6 || static_cast <Atom*> (pAtom0)->GetShowSymbol ())
		gnome_canvas_item_raise_to_top (GNOME_CANVAS_ITEM (pData->Items[pAtom0]));
	else
		gnome_canvas_item_lower_to_bottom (GNOME_CANVAS_ITEM (pData->Items[pAtom0]));
	map<string, Object*>::iterator i;
	Object* electron = pAtom0->GetFirstChild (i);
	while (electron) { 
		gnome_canvas_item_raise_to_top (GNOME_CANVAS_ITEM (pData->Items[electron]));
		electron = pAtom0->GetNextChild (i);
	}
	if (pAtom1->GetParent ()->GetType () == FragmentType)
		gnome_canvas_item_raise_to_top (GNOME_CANVAS_ITEM (pData->Items[pAtom1->GetParent ()]));
	else if (pAtom1->GetZ () != 6 || static_cast <Atom*> (pAtom1)->GetShowSymbol ())
		gnome_canvas_item_raise_to_top (GNOME_CANVAS_ITEM (pData->Items[pAtom1]));
	else
		gnome_canvas_item_lower_to_bottom (GNOME_CANVAS_ITEM (pData->Items[pAtom1]));
	electron = pAtom1->GetFirstChild (i);
	while (electron) { 
		gnome_canvas_item_raise_to_top (GNOME_CANVAS_ITEM (pData->Items[electron]));
		electron = pAtom1->GetNextChild (i);
	}
}

bool Bond::ReplaceAtom (Atom* oldAtom, Atom* newAtom)
{
	if (oldAtom == m_Begin) {
		if (m_End)
			m_End->RemoveBond (this);
		m_Begin = newAtom;
		if (m_Begin && m_End)
			m_End->AddBond (this);
	} else if (oldAtom == m_End) {
		if (m_Begin)
			m_Begin->RemoveBond (this);
		m_End = newAtom;
		if (m_Begin && m_End)
			m_Begin->AddBond (this);
	}
	return true;
}

GnomeCanvasPathDef* Bond::BuildPathDef (WidgetData* pData)
{
	double x1, y1, x2, y2, x, y, dx, dy, dx1, dy1;
	int i, n;
	GnomeCanvasPathDef* path = gnome_canvas_path_def_new ();
	Theme *pTheme = pData->m_View->GetDoc ()->GetTheme ();
	switch (GetType ())
	{
		case NormalBondType:
			i = 1;
			while (GetLine2DCoords (i++, &x1, &y1, &x2, &y2))
			{
				gnome_canvas_path_def_moveto (path, x1 * pTheme->GetZoomFactor (), y1 * pTheme->GetZoomFactor ());
				gnome_canvas_path_def_lineto (path, x2 * pTheme->GetZoomFactor (), y2 * pTheme->GetZoomFactor ());
			}
			break;
		case UpBondType:
			GetLine2DCoords(1, &x1, &y1, &x2, &y2);
			gnome_canvas_path_def_moveto (path, x1 * pTheme->GetZoomFactor (), y1 * pTheme->GetZoomFactor ());
			x = sqrt (square (x2 - x1) + square (y2 - y1));
			dx = (y1 - y2) / x * pTheme->GetStereoBondWidth () / 2;
			dy = (x2 - x1) / x * pTheme->GetStereoBondWidth () / 2;
			gnome_canvas_path_def_lineto (path, x2 * pTheme->GetZoomFactor () + dx, y2 * pTheme->GetZoomFactor () + dy);
			gnome_canvas_path_def_lineto (path, x2 * pTheme->GetZoomFactor () -dx, y2 * pTheme->GetZoomFactor () -dy);
			gnome_canvas_path_def_closepath (path);
			break;
		case DownBondType:
		{
			double xi[8];
			GetLine2DCoords (1, &x1, &y1, &x2, &y2);
			x1 *= pTheme->GetZoomFactor ();
			y1 *= pTheme->GetZoomFactor ();
			x2 *= pTheme->GetZoomFactor ();
			y2 *= pTheme->GetZoomFactor ();
			x = sqrt (square (x2 - x1) + square (y2 - y1));
			n = int (floor(x / (pTheme->GetHashDist () + pTheme->GetHashWidth ())));
			dx1 = (x2 - x1) / x * pTheme->GetHashWidth ();
			dy1 = (y2 - y1) / x * pTheme->GetHashWidth ();
			dx = (y1 - y2) / x * pTheme->GetStereoBondWidth () / 2;
			dy = (x2 - x1) / x * pTheme->GetStereoBondWidth () / 2;
			gnome_canvas_path_def_moveto (path, xi[0] = x1 + dx, xi[1] = y1 + dy);
			gnome_canvas_path_def_lineto (path, xi[2] = x1 - dx, xi[3] = y1 - dy);
			dx *= (1 - pTheme->GetHashWidth () / x);
			dy *= (1 - pTheme->GetHashWidth () / x);
			gnome_canvas_path_def_lineto (path, xi[4] = x1 + dx1 - dx, xi[5] = y1 + dy1 - dy);
			gnome_canvas_path_def_lineto (path, xi[6] = x1 + dx1 + dx, xi[7] = y1 + dy1 + dy);
			gnome_canvas_path_def_lineto (path, xi[0], xi[1]);
			gnome_canvas_path_def_closepath_current (path);
			dx = (x2 - x1) / x * (pTheme->GetHashDist () + pTheme->GetHashWidth ())
				- (y1 - y2) / x * pTheme->GetStereoBondWidth () / 2 * (pTheme->GetHashDist () + pTheme->GetHashWidth ()) / x;
			dy = (y2 - y1) / x * (pTheme->GetHashDist () + pTheme->GetHashWidth ())
				- (x2 - x1) / x * pTheme->GetStereoBondWidth () / 2 *  (pTheme->GetHashDist () + pTheme->GetHashWidth ()) / x;
			dx1 = (x2 - x1) / x * (pTheme->GetHashDist () + pTheme->GetHashWidth ())
				+ (y1 - y2) / x * pTheme->GetStereoBondWidth () / 2 *  (pTheme->GetHashDist () + pTheme->GetHashWidth ()) / x;
			dy1 = (y2 - y1) / x * (pTheme->GetHashDist () + pTheme->GetHashWidth ())
				+ (x2 - x1) / x * pTheme->GetStereoBondWidth () / 2 *  (pTheme->GetHashDist () + pTheme->GetHashWidth ()) / x;
			for (int i = 1; i < n; i++) {
				gnome_canvas_path_def_moveto (path, xi[0] += dx, xi[1] += dy);
				gnome_canvas_path_def_lineto (path, xi[2] += dx1, xi[3] += dy1);
				gnome_canvas_path_def_lineto (path, xi[4] += dx1, xi[5] += dy1);
				gnome_canvas_path_def_lineto (path, xi[6] += dx, xi[7] += dy);
				gnome_canvas_path_def_lineto (path, xi[0], xi[1]);
				gnome_canvas_path_def_closepath_current (path);
			}
			break;
		}
		case ForeBondType:
			GetLine2DCoords (1, &x1, &y1, &x2, &y2);
			x = sqrt (square (x2 - x1) + square (y2 - y1));
			dx = (y1 - y2) / x * pTheme->GetStereoBondWidth () / 2;
			dy = (x2 - x1) / x * pTheme->GetStereoBondWidth () / 2;
			gnome_canvas_path_def_moveto (path, x1 * pTheme->GetZoomFactor () + dx, y1 * pTheme->GetZoomFactor () + dy);
			gnome_canvas_path_def_lineto (path, x2 * pTheme->GetZoomFactor () + dx, y2 * pTheme->GetZoomFactor () + dy);
			gnome_canvas_path_def_lineto (path, x2 * pTheme->GetZoomFactor () -dx, y2 * pTheme->GetZoomFactor () -dy);
			gnome_canvas_path_def_lineto (path, x1 * pTheme->GetZoomFactor () -dx, y1 * pTheme->GetZoomFactor () -dy);
			gnome_canvas_path_def_closepath (path);
			break;
		case UndeterminedBondType:
		{
			GetLine2DCoords (1, &x1, &y1, &x2, &y2);
			x1 *= pTheme->GetZoomFactor ();
			y1 *= pTheme->GetZoomFactor ();
			x2 *= pTheme->GetZoomFactor ();
			y2 *= pTheme->GetZoomFactor ();
			gnome_canvas_path_def_moveto (path, x1, y1);
			double length;
			length = sqrt (square (x2 - x1) + square (y2 - y1));
			n = (int) length / 3;
			int	s = 1;
			dx = (x2 - x1) / n;
			dy = (y2 - y1) / n;
			x = x1;
			y = y1;
			for (int i = 1; i < n; i++) {
				x1 = x + dx / 3 + dy /1.5 * s;
				y1 = y + dy / 3 - dx /1.5 * s;
				dx1 = x + dx / 1.5 + dy /1.5 * s;
				dy1 = y + dy / 1.5 - dx /1.5 * s;
				x += dx;
				y += dy;
				s *= -1;
				gnome_canvas_path_def_curveto (path, x1, y1, dx1, dy1, x, y);
			}
			x1 = x + dx / 3 + dy /1.5 * s;
			y1 = y + dy / 3 - dx /1.5 * s;
			dx1 = x + dx / 1.5 + dy /1.5 * s;
			dy1 = y + dy / 1.5 - dx /1.5 * s;
			gnome_canvas_path_def_curveto (path, x1, y1, dx1, dy1, x2, y2);
			break;
		}
	}
	return path;
}

GnomeCanvasPathDef *Bond::BuildCrossingPathDef (WidgetData* pData)
{
	Theme *pTheme = pData->m_View->GetDoc ()->GetTheme ();
	GnomeCanvasPathDef* path = NULL;
	double x1, y1, x2, y2, x, dx, dy;
	int i;
	switch (GetType ()) {
	case NormalBondType:
		i = 1;
		path = gnome_canvas_path_def_new ();
		while (GetLine2DCoords (i++, &x1, &y1, &x2, &y2)) {
			dx = (x2 - x1) / 10.;
			dy = (y2 - y1) / 10.;
			x1 += dx;
			x2 -= dx;
			y1 += dy;
			y2 -= dy;
			gnome_canvas_path_def_moveto (path, x1 * pTheme->GetZoomFactor (), y1 * pTheme->GetZoomFactor ());
			gnome_canvas_path_def_lineto (path, x2 * pTheme->GetZoomFactor (), y2 * pTheme->GetZoomFactor ());
		}
		break;
	case UpBondType:
		break;
	case DownBondType:
		break;
	case ForeBondType:
		path = gnome_canvas_path_def_new ();
		GetLine2DCoords (1, &x1, &y1, &x2, &y2);
		dx = (x2 - x1) / 10.;
		dy = (y2 - y1) / 10.;
		x1 += dx;
		x2 -= dx;
		y1 += dy;
		y2 -= dy;
		x = sqrt(square(x2 - x1) + square(y2 - y1));
		dx = (y1 - y2) / x * pTheme->GetStereoBondWidth () / 2;
		dy = (x2 - x1) / x * pTheme->GetStereoBondWidth () / 2;
		dx += (dx > 0.)? 1.: -1.;
		dy += (dy > 0.)? 1.: -1.;
		gnome_canvas_path_def_moveto (path, x1 * pTheme->GetZoomFactor () + dx, y1 * pTheme->GetZoomFactor () + dy);
		gnome_canvas_path_def_lineto (path, x2 * pTheme->GetZoomFactor () + dx, y2 * pTheme->GetZoomFactor () + dy);
		gnome_canvas_path_def_lineto (path, x2 * pTheme->GetZoomFactor () -dx, y2 * pTheme->GetZoomFactor () -dy);
		gnome_canvas_path_def_lineto (path, x1 * pTheme->GetZoomFactor () -dx, y1 * pTheme->GetZoomFactor () -dy);
		gnome_canvas_path_def_closepath (path);
		break;
	case UndeterminedBondType:
		break;
	}
	return path;
}

void Bond::Update(GtkWidget* w)
{
	if (!w || !m_order)
		return;
	WidgetData* pData = (WidgetData*) g_object_get_data(G_OBJECT(w), "data");
	Theme *pTheme = pData->m_View->GetDoc ()->GetTheme ();
	bool result = false;
	if (m_Crossing.size () > 0) {
		map<Bond*, BondCrossing>::iterator i, iend = m_Crossing.end ();
		for (i = m_Crossing.begin (); i != iend; i++)
			if ((result |= (*i).second.is_before))
				break;
	}
	GnomeCanvasPathDef* path;
	void * obj;
	GnomeCanvasGroup *group = pData->Items[this];
	obj = g_object_get_data(G_OBJECT(group), "back");
	if (result) {
		path = BuildCrossingPathDef (pData);
		if (path) {
			if (obj)
				g_object_set (obj, "bpath", path, NULL);
			else {
				GnomeCanvasItem *item = (m_type == NormalBondType || m_type == UndeterminedBondType)?
					gnome_canvas_item_new (
						group,
						gnome_canvas_bpath_ext_get_type (),
						"bpath", path,
						"outline_color", "white",
						"width_units", pTheme->GetBondWidth () * 3.,
						NULL):
					gnome_canvas_item_new (
						group,
						gnome_canvas_bpath_ext_get_type(),
						"bpath", path,
						"fill_color", "white",
						"width_units", 0.,
						NULL);
				g_object_set_data (G_OBJECT (group), "back", item);
				g_object_set_data (G_OBJECT (item), "object", this);
				g_signal_connect (G_OBJECT (item), "event", G_CALLBACK (on_event), w);
				/* now bring to front, FIXME: not secure ! */
				gnome_canvas_item_lower_to_bottom (item);
				gnome_canvas_item_raise_to_top (GNOME_CANVAS_ITEM (group));
				gcu::Atom *pAtom = GetAtom (0);
				if (pAtom->GetZ () != 6 || static_cast <gcp::Atom*> (pAtom)->GetShowSymbol ())
					gnome_canvas_item_raise_to_top (GNOME_CANVAS_ITEM (pData->Items[pAtom]));
				pAtom = GetAtom (1);
				if (pAtom->GetZ () != 6 || static_cast <gcp::Atom*> (pAtom)->GetShowSymbol ())
					gnome_canvas_item_raise_to_top (GNOME_CANVAS_ITEM (pData->Items[pAtom]));
			}
			gnome_canvas_path_def_unref (path);
		}
	} else if (obj) {
		g_object_set_data (G_OBJECT(group), "back", NULL);
	}
	path = BuildPathDef (pData);
	obj = g_object_get_data(G_OBJECT(group), "path");
	g_object_set (obj, "bpath", path, NULL);
	if (m_type == NormalBondType || m_type == UndeterminedBondType)
		g_object_set (obj, "width_units", pTheme->GetBondWidth (), NULL);
	gnome_canvas_path_def_unref (path);
}
	
double Bond::GetYAlign ()
{
	double x1, y1, x2, y2;
	m_Begin->GetCoords(&x1, &y1);
	m_End->GetCoords (&x2, &y2);
	return (y1 + y2) / 2;
}

bool Bond::IsCrossing (Bond *pBond)
{
	double a1, a2, b1, b2, c1, c2, d, d1, d2;
	double x0, x1, x2, x3, y0, y1, y2, y3, z0, z1, z2, z3;
	if (m_Begin == pBond->m_Begin || m_Begin == pBond->m_End ||
			m_End == pBond->m_Begin || m_End == pBond->m_End)
		return false;
	m_Begin->GetCoords (&x0, &y0, &z0);
	m_End->GetCoords (&x1, &y1, &z1);
	pBond->m_Begin->GetCoords (&x2, &y2, &z2);
	pBond->m_End->GetCoords (&x3, &y3, &z3);
	a1 = x1 - x0;
	a2 = y1 - y0;
	b1 = x2 - x3;
	b2 = y2 - y3;
	d = a1 * b2 - a2 * b1;
	if (d == 0.)
		return false;
	c1 = x2 - x0;
	c2 = y2 - y0;
	d1 = c1 * b2 - c2 * b1;
	d2 = c2 * a1 - c1 * a2;
	a1 = d1 / d;
	a2 = d2 / d;
	if ((a1 <= 0.) || (a1 >= 1.))
		return false;
	if ((a2 > 0.) && (a2 < 1.)) {
		double z = z0 + a1 * (z1 - z0), z_ = z2 + a2 * (z3 - z2);
		bool is_before = z > z_ || m_level > pBond->m_level;
		if (z == z_ && m_level == pBond->m_level) {
			if (m_type == ForeBondType && pBond->m_type != ForeBondType) {
				is_before = true;
				pBond->m_level -= 1;
			} else {
				is_before = false;
				pBond->m_level += 1;
			}
		}
		m_Crossing[pBond].a = a1;
		m_Crossing[pBond].is_before = is_before;
		pBond->m_Crossing[this].a = a2;
		pBond->m_Crossing[this].is_before = !is_before;
		return true;
	} else
		return false;
}

static void on_bring_to_front (Bond *pBond)
{
	pBond->BringToFront ();
}

static void on_move_to_back (Bond *pBond)
{
	pBond->MoveToBack ();
}

bool Bond::BuildContextualMenu (GtkUIManager *UIManager, Object *object, double x, double y)
{
	Object *pAtom = GetAtomAt (x, y);
	if (pAtom)
		return pAtom->BuildContextualMenu (UIManager, object, x, y);
	if (m_Crossing.size () == 0)
		return Object::BuildContextualMenu (UIManager, object, x, y);
	bool is_before = false, is_after = false;
	if (m_Crossing.size () > 0) {
		map<Bond*, BondCrossing>::iterator i, iend = m_Crossing.end ();
		for (i = m_Crossing.begin (); i != iend; i++) {
			if (m_level == (*i).first->m_level || m_type != (*i).first->m_type)
				continue;
			if ((*i).second.is_before)
				is_before = true;
			else
				is_after = true;
		}
	}
	if (!(is_before || is_after))
		return Object::BuildContextualMenu (UIManager, object, x, y);
	GtkActionGroup *group = gtk_action_group_new ("bond");
	GtkAction *action;
	action = gtk_action_new ("Bond", _("Bond"), NULL, NULL);
	gtk_action_group_add_action (group, action);
	g_object_unref (action);
	if (is_before) {
		action = gtk_action_new ("MoveBack", _("Move to back"), NULL, NULL);
		g_signal_connect_swapped (action, "activate", G_CALLBACK (on_move_to_back), this);
		gtk_action_group_add_action (group, action);
		g_object_unref (action);
		gtk_ui_manager_add_ui_from_string (UIManager, "<ui><popup><menu action='Bond'><menuitem action='MoveBack'/></menu></popup></ui>", -1, NULL);
	}
	if (is_after) {
		action = gtk_action_new ("BringFront", _("Bring to front"), NULL, NULL);
		g_signal_connect_swapped (action, "activate", G_CALLBACK (on_bring_to_front), this);
		gtk_action_group_add_action (group, action);
		g_object_unref (action);
		gtk_ui_manager_add_ui_from_string (UIManager, "<ui><popup><menu action='Bond'><menuitem action='BringFront'/></menu></popup></ui>", -1, NULL);
	}
	gtk_ui_manager_insert_action_group (UIManager, group, 0);
	g_object_unref (group);
	Object::BuildContextualMenu (UIManager, object, x, y);
	return true;
}

void Bond::MoveToBack ()
{
	View *pView = reinterpret_cast<Document*> (GetDocument ())->GetView ();
	map<Bond*, BondCrossing>::iterator i, iend = m_Crossing.end ();
	for (i = m_Crossing.begin (); i != iend; i++) {
		if (m_level > (*i).first->m_level && m_type == (*i).first->m_type) {
			m_level = (*i).first->m_level - 1;
			(*i).second.is_before = false;
			(*i).first->m_Crossing[this].is_before = true;
			pView->Update ((*i).first);
		}
	}
	pView->Update (this);
}

void Bond::BringToFront ()
{
	View *pView = reinterpret_cast<Document*> (GetDocument ())->GetView ();
	map<Bond*, BondCrossing>::iterator i, iend = m_Crossing.end ();
	for (i = m_Crossing.begin (); i != iend; i++) {
		if (m_level < (*i).first->m_level && m_type == (*i).first->m_type) {
			m_level = (*i).first->m_level + 1;
			(*i).second.is_before = true;
			(*i).first->m_Crossing[this].is_before = false;
			pView->Update ((*i).first);
		}
	}
	pView->Update (this);
}

}	//	namespace gcp