// -*- C++ -*-

/* 
 * GChemPaint atoms plugin
 * chargetool.cc 
 *
 * Copyright (C) 2003-2007 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "chargetool.h"
#include <gcp/settings.h>
#include <gcp/document.h>
#include <gcp/application.h>
#include <gcp/theme.h>
#include <canvas/gcp-canvas-group.h>
#include <canvas/gcp-canvas-pango.h>
#include <glib/gi18n-lib.h>
#include <cmath>

using namespace gcu;
using namespace std;

gcpChargeTool::gcpChargeTool (gcp::Application *App, string Id): gcp::Tool (App, Id)
{
	if (Id == string ("ChargePlus"))
		m_glyph = "\xE2\x8a\x95";
	else if (Id == string ("ChargeMinus"))
		m_glyph = "\xE2\x8a\x96";
	else m_glyph = 0;
}

gcpChargeTool::~gcpChargeTool ()
{
}

bool gcpChargeTool::OnClicked ()
{
	if (!m_pObject || (m_pObject->GetType () != AtomType))
		return false;
	gcp::Atom *pAtom = (gcp::Atom*) m_pObject;
	gcp::Theme *Theme = m_pView->GetDoc ()->GetTheme ();
	m_Charge = pAtom->GetCharge () + ((GetName() == string ("ChargePlus"))? 1: -1);
	if (!pAtom->AcceptCharge (m_Charge))
		return false;
	m_bDragged = false;
	GObject *obj;
	ArtDRect rect;
	pAtom->GetCoords (&m_x0, &m_y0);
	m_x0 *= m_dZoomFactor;
	m_y0 *= m_dZoomFactor;
	if (m_pObject->GetParent ()->GetType () == FragmentType) {
		obj = G_OBJECT (m_pData->Items[m_pObject->GetParent ()]);
		gnome_canvas_item_get_bounds (GNOME_CANVAS_ITEM (g_object_get_data (obj, "fragment")),
				&rect.x0, &rect.y0, &rect.x1, &rect.y0);
	} else {
		obj = G_OBJECT (m_pData->Items[m_pObject]);
		GnomeCanvasItem *sym = (GnomeCanvasItem*) g_object_get_data (obj, "symbol");
		if (sym)
			gnome_canvas_item_get_bounds (sym,
					&rect.x0, &rect.y0, &rect.x1, &rect.y0);
		else
			rect.y0 = m_y0 + 6;
	}
	GnomeCanvasItem *item = (GnomeCanvasItem*) g_object_get_data (obj, "charge");
	m_dDist = 0;
	m_pData->GetObjectBounds (m_pObject, &rect);
	m_dDistMax = 1.5 * fabs (rect.y0 - m_y0);
	if (m_Charge) {
		if (item)
			gnome_canvas_item_hide (item);
		double x, y, xc = 0., yc;
		m_DefaultPos = 0xff;
		int align = ((gcp::Atom*) m_pObject)->GetChargePosition (m_DefaultPos, 0., x, y);
		if (!align)
			return false;
		m_Pos = m_DefaultPos;
		x *= m_dZoomFactor;
		y *= m_dZoomFactor;
		if (!m_Pos) {
			m_x = x - m_x0;
			m_y = y - m_y0;
			m_dAngle = atan (- m_y / m_x);
			if (m_x < 0)
				m_dAngle += M_PI;
			m_dDist = sqrt (m_x * m_x + m_y * m_y);
		} else {
			switch (m_Pos) {
			case POSITION_NE:
				m_dAngle = M_PI / 4;
				break;
			case POSITION_NW:
				m_dAngle = 3 * M_PI / 4;
				break;
			case POSITION_N:
				m_dAngle = M_PI / 2;
				break;
			case POSITION_SE:
				m_dAngle = 7 * M_PI / 4;
				break;
			case POSITION_SW:
				m_dAngle = 5 * M_PI / 4;
				break;
			case POSITION_S:
				m_dAngle = 3 * M_PI / 2;
				break;
			case POSITION_E:
				m_dAngle = 0.;
				break;
			case POSITION_W:
				m_dAngle = M_PI;
				break;
			}
		}
		char* markup = NULL;
		PangoLayout* pl = NULL;
		if (abs (m_Charge) > 1) {
			markup = g_strdup_printf ("%d", abs (m_Charge));
			PangoContext* pc = m_pView->GetPangoContext ();
			PangoRectangle rect;
			pl = pango_layout_new (pc);
			pango_layout_set_text (pl, markup, -1);
			pango_layout_get_extents (pl, NULL, &rect);
			m_ChargeWidth = rect.width / PANGO_SCALE;
			pango_layout_set_font_description (pl, m_pView->GetPangoSmallFontDesc ());
			m_ChargeTWidth = m_ChargeWidth + 1. + Theme->GetChargeSignSize ();
		} else {
			m_ChargeWidth = 0.;
			m_ChargeTWidth = Theme->GetChargeSignSize ();
		}
		switch (align) {
		case -2:
			xc = x + m_ChargeTWidth / 2. - Theme->GetChargeSignSize ();
			y += Theme->GetChargeSignSize () / 2.;
			break;
		case -1:
			xc = x - Theme->GetChargeSignSize () - Theme->GetPadding ();
			break;
		case 0:
		case -3:
			xc = x + m_ChargeTWidth / 2. - Theme->GetChargeSignSize ();
			break;
		case 1:
			xc = x + m_ChargeWidth + Theme->GetPadding ();
			break;
		case 2:
			xc = x + m_ChargeTWidth / 2. - Theme->GetChargeSignSize ();
			y -= Theme->GetChargeSignSize () / 2.;
			break;
		}
		x = xc - 1.;
		yc = y - Theme->GetChargeSignSize () / 2.;
		m_x1 = x;
		m_y1 = y;
		m_pItem = gnome_canvas_item_new (
							m_pGroup,
							gnome_canvas_group_get_type(),
							NULL);
		if (markup) {
			gnome_canvas_item_new(
						GNOME_CANVAS_GROUP (m_pItem),
						gnome_canvas_pango_get_type(),
						"fill_color", gcp::AddColor,
						"layout", pl,
						"anchor", GTK_ANCHOR_EAST,
						"x", x,
						"y", y,
						NULL);
		}
		gnome_canvas_item_new (
					GNOME_CANVAS_GROUP (m_pItem),
					gnome_canvas_ellipse_get_type (),
					"x1", xc,
					"y1", yc,
					"x2", xc + Theme->GetChargeSignSize (),
					"y2", yc + Theme->GetChargeSignSize (),
					"outline_color", gcp::AddColor,
					"width_units", 0.5,
					NULL
				);
		ArtBpath *path = art_new (ArtBpath, 5);
		path[0].code = ART_MOVETO_OPEN;
		path[0].x3 = xc + 1.;
		path[1].code = ART_LINETO;
		path[1].x3 = xc + Theme->GetChargeSignSize () - 1.;
		path[0].y3 = path[1].y3 = yc + Theme->GetChargeSignSize () / 2.;
		if (m_Charge > 0) {
			path[2].code = ART_MOVETO_OPEN;
			path[2].y3 = yc + 1.;
			path[3].code = ART_LINETO;
			path[3].y3 = yc + Theme->GetChargeSignSize () - 1.;
			path[2].x3 = path[3].x3 = xc + Theme->GetChargeSignSize () / 2.;
			path[4].code = ART_END;
		} else
			path[2].code = ART_END;
		GnomeCanvasPathDef *cpd = gnome_canvas_path_def_new_from_bpath (path);
		item = gnome_canvas_item_new (
					GNOME_CANVAS_GROUP (m_pItem),
					gnome_canvas_bpath_get_type (),
					"bpath", cpd,
					"outline_color", gcp::AddColor,
					"width_units", .75,
					NULL
				);
		gnome_canvas_path_def_unref (cpd);
		if (pl)
			g_object_unref (G_OBJECT (pl));
	} else {
		void *child = g_object_get_data (obj, "figure");
		if (child)
			g_object_set (G_OBJECT (child), "fill-color", gcp::DeleteColor, NULL);
		child = g_object_get_data (obj, "circle");
		g_object_set (G_OBJECT (child), "outline-color", gcp::DeleteColor, NULL);
		child = g_object_get_data (obj, "sign");
		g_object_set (G_OBJECT (child), "outline-color", gcp::DeleteColor, NULL);
	}
	char buf[32];
	snprintf (buf, sizeof (buf) - 1, _("Orientation: %g"), m_dAngle * 180. / M_PI);
	m_pApp->SetStatusText (buf);
	m_bChanged = true;
	return true;
}

void gcpChargeTool::OnDrag ()
{
	if (m_Charge && !m_pItem)
		return;
	m_bDragged = true;
	GObject *obj = G_OBJECT ((m_pObject->GetParent ()->GetType () == FragmentType)?
		m_pData->Items[m_pObject->GetParent ()]: m_pData->Items[m_pObject]);
	GnomeCanvasItem *item = (GnomeCanvasItem*) g_object_get_data (obj, "charge");
	int align, old_pos = m_Pos;
	m_x -= m_x0;
	m_y -= m_y0;
	m_dDist = sqrt (square (m_x) + square (m_y));
	if (!m_pItem) {
		void *child;
		if (m_dDist < m_dDistMax) {
			if (!m_bChanged) {
				child = g_object_get_data (obj, "figure");
				if (child)
					g_object_set (G_OBJECT (child), "fill-color", gcp::DeleteColor, NULL);
				child = g_object_get_data (obj, "circle");
				g_object_set (G_OBJECT (child), "outline-color", gcp::DeleteColor, NULL);
				child = g_object_get_data (obj, "sign");
				g_object_set (G_OBJECT (child), "outline-color", gcp::DeleteColor, NULL);
				m_bChanged = true;
			}
		} else {
			if (m_bChanged) {
				child = g_object_get_data (obj, "figure");
				if (child)
					g_object_set (G_OBJECT (child), "fill-color", "black", NULL);
				child = g_object_get_data (obj, "circle");
				g_object_set (G_OBJECT (child), "outline-color", "black", NULL);
				child = g_object_get_data (obj, "sign");
				g_object_set (G_OBJECT (child), "outline-color", "black", NULL);
				m_bChanged = false;
			}
		}
		return;
	}
	double Angle = atan (- m_y / m_x);
	if (isnan (Angle))
		Angle = m_dAngle;
	else if (m_x < 0)
		Angle += M_PI;
	if (!(m_nState & GDK_CONTROL_MASK)) {
		int pos = (int) rint (Angle * 4. / M_PI);
		Angle = (double) pos * M_PI / 4.;
		if (m_nState & GDK_SHIFT_MASK)
			pos = 8;
		else if (pos < 0)
			pos += 8;
		switch (pos) {
		case 0:
			m_Pos = POSITION_E;
			break;
		case 1:
			m_Pos = POSITION_NE;
			break;
		case 2:
			m_Pos = POSITION_N;
			break;
		case 3:
			m_Pos = POSITION_NW;
			break;
		case 4:
			m_Pos = POSITION_W;
			break;
		case 5:
			m_Pos = POSITION_SW;
			break;
		case 6:
			m_Pos = POSITION_S;
			break;
		case 7:
			m_Pos = POSITION_SE;
			break;
		default:
			m_Pos = 0;
		}
	} else
		m_Pos = 0;
	if ((Angle == m_dAngle) && !(m_nState & GDK_SHIFT_MASK)) {
		if (m_dDist < m_dDistMax) {
			if (!m_bChanged) {
				gnome_canvas_item_show (m_pItem);
				if (item)
					gnome_canvas_item_hide (item);
				m_bChanged = true;
			}
		} else {
			if (m_bChanged) {
				if (item)
					gnome_canvas_item_show (item);
				gnome_canvas_item_hide (m_pItem);
				m_bChanged = false;
			}
		}
	} else {
		double x, y;
		gcp::Atom *pAtom = (gcp::Atom*) m_pObject;
		gcp::Theme *Theme = m_pView->GetDoc ()->GetTheme ();
		if (!(m_nState & GDK_SHIFT_MASK) && (m_dDist >= m_dDistMax) && m_bChanged) {
			gnome_canvas_item_hide (m_pItem);
			m_bChanged = false;
		} else if ((align = pAtom->GetChargePosition (m_Pos, Angle * 180. / M_PI, x, y))) {
			m_dAngle = Angle;
			if (m_nState & GDK_SHIFT_MASK) {
				align = 0;
				x = m_x0 + m_dDist * cos (m_dAngle);
				y = m_y0 - m_dDist * sin (m_dAngle);
			} else {
				x = x * m_dZoomFactor;
				y = y * m_dZoomFactor;
			}
			switch (align) {
			case -2:
				x += m_ChargeTWidth / 2. - Theme->GetChargeSignSize () - 1.;
				y += Theme->GetChargeSignSize () / 2.;
				break;
			case -1:
				x-= Theme->GetChargeSignSize () + Theme->GetPadding ();
				break;
			case -3:
				x += m_ChargeTWidth / 2. - Theme->GetChargeSignSize () - 1.;
				break;
			case 1:
				x += m_ChargeWidth + Theme->GetPadding ();
				break;
			case 2:
				x += m_ChargeTWidth / 2. - Theme->GetChargeSignSize () - 1.;
				y -= Theme->GetChargeSignSize () / 2.;
				break;
			}
			gnome_canvas_item_move (m_pItem, x - m_x1, y - m_y1);
			m_x1 = x;
			m_y1 = y;
			gnome_canvas_item_show (m_pItem);
			if (item)
				gnome_canvas_item_hide (item);
			m_bChanged = true;
		} else
			m_Pos = old_pos;
	}
	char tmp[32];
	snprintf(tmp, sizeof(tmp) - 1, _("Orientation: %g"), m_dAngle * 180. / M_PI);
	m_pApp->SetStatusText(tmp);
}

void gcpChargeTool::OnRelease ()
{
	if (m_bChanged) {
		gcp::Atom* pAtom = (gcp::Atom*) m_pObject;
		gcp::Document* pDoc = m_pView->GetDoc ();
		gcp::Operation* pOp = pDoc-> GetNewOperation(gcp::GCP_MODIFY_OPERATION);
		GObject *obj = G_OBJECT ((m_pObject->GetParent ()->GetType () == FragmentType)?
			m_pData->Items[m_pObject->GetParent ()]: m_pData->Items[m_pObject]);
		GnomeCanvasItem *item = (GnomeCanvasItem*) g_object_get_data (obj, "charge");
		if (item)
			gnome_canvas_item_show (item);
		m_pObject = m_pObject->GetGroup ();
		pOp->AddObject (m_pObject, 0);
		pAtom->SetCharge (m_Charge);
		if (!m_bDragged) {
			double x, y;
			m_DefaultPos = 0xff;
			pAtom->GetChargePosition (m_DefaultPos, 0., x, y);
			if (m_Pos && (m_Pos != m_DefaultPos))
				m_Pos = m_DefaultPos;
		}
		if (!(m_nState & GDK_SHIFT_MASK))
			m_dDist = 0.;
		pAtom->SetChargePosition (m_Pos, m_Pos == m_DefaultPos,
								m_dAngle, m_dDist / m_dZoomFactor);
		pAtom->Update ();
		m_pView->Update (m_pObject);
		pAtom->EmitSignal (gcp::OnChangedSignal);
		pOp->AddObject (m_pObject, 1);
		pDoc->FinishOperation ();
	}
}
