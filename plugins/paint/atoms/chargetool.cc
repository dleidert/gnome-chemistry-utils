// -*- C++ -*-

/*
 * GChemPaint atoms plugin
 * chargetool.cc
 *
 * Copyright (C) 2003-2009 Jean Bréfort <jean.brefort@normalesup.org>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include "config.h"
#include "chargetool.h"
#include <gcp/application.h>
#include <gcp/atom.h>
#include <gcp/document.h>
#include <gcp/settings.h>
#include <gcp/theme.h>
#include <gcp/view.h>
#include <gcp/widgetdata.h>
#include <gccv/canvas.h>
#include <gccv/group.h>
#include <gccv/text.h>
#include <gccv/text-tag.h>
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
	gcp::Atom* pAtom = static_cast <gcp::Atom*> (m_pObject);
	gcp::Theme *Theme = m_pView->GetDoc ()->GetTheme ();
	m_Charge = pAtom->GetCharge () + ((GetName() == string ("ChargePlus"))? 1: -1);
	if (!pAtom->AcceptCharge (m_Charge))
		return false;
	m_bDragged = false;
	pAtom->GetCoords (&m_x0, &m_y0);
	gccv::Rect rect;
	m_pData->GetObjectBounds (((m_pObject->GetParent ()->GetType () == FragmentType)? m_pObject->GetParent (): m_pObject), &rect);
	m_x0 *= m_dZoomFactor;
	m_y0 *= m_dZoomFactor;
	m_dDistMax = 1.5 * fabs (rect.y0 - m_y0);
	m_dDist = 0;
	gccv::Item *item = pAtom->GetChargeItem ();

	if (m_Charge) {
		if (item)
			item->SetVisible (false);
		double x, y;
		m_DefaultPos = 0xff;
		gccv::Anchor anchor = pAtom->GetChargePosition (m_DefaultPos, 0., x, y);
		if (anchor == gccv::AnchorCenter)
			return false;
		m_Pos = m_DefaultPos;
		m_x1 = x *= m_dZoomFactor;
		m_y1 = y *= m_dZoomFactor;
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
		if (abs (m_Charge) > 1)
			markup = g_strdup_printf ("%d%s", abs (m_Charge), m_glyph);
		else
			markup = g_strdup_printf ("%s", m_glyph);
		gccv::Text *text = new gccv::Text (m_pView->GetCanvas ()->GetRoot (), x, y, NULL);
		text->SetFillColor (0);
		text->SetPadding (Theme->GetPadding ());
		text->SetLineColor (0);
		text->SetLineWidth (0.);
		text->SetAnchor (anchor);
		text->SetFontDescription (m_pView->GetPangoSmallFontDesc ());
		text->SetText (markup);
		g_free (markup);
		text->SetColor (gcp::AddColor);
		m_Item = text;
	} else
		static_cast <gccv::Text *> (pAtom->GetChargeItem ())->SetColor (gcp::DeleteColor);
	char buf[32];
	snprintf (buf, sizeof (buf) - 1, _("Orientation: %g"), m_dAngle * 180. / M_PI);
	m_pApp->SetStatusText (buf);
	m_bChanged = true;
	return true;
}

void gcpChargeTool::OnDrag ()
{
	if (m_Charge && !m_Item)
		return;
	m_bDragged = true;
	gcp::Atom* pAtom = static_cast <gcp::Atom*> (m_pObject);
	gccv::Item *item = pAtom->GetChargeItem ();
	int old_pos = m_Pos;
	gccv::Anchor anchor;
	m_x -= m_x0;
	m_y -= m_y0;
	m_dDist = sqrt (square (m_x) + square (m_y));
	if (!m_Item) {
		gccv::Text *text = static_cast <gccv::Text *> (item);
		if (m_dDist < m_dDistMax) {
			if (!m_bChanged) {
				text->SetColor (gcp::DeleteColor);
				m_bChanged = true;
			}
		} else {
			if (m_bChanged) {
				text->SetColor (gcp::Color);
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
				m_Item->SetVisible (true);
				if (item)
					item->SetVisible (false);
				m_bChanged = true;
			}
		} else {
			if (m_bChanged) {
				if (item)
					item->SetVisible (true);
				m_Item->SetVisible (false);
				m_bChanged = false;
			}
		}
	} else {
		double x, y;
		if (!(m_nState & GDK_SHIFT_MASK) && (m_dDist >= m_dDistMax) && m_bChanged) {
			m_Item->SetVisible (false);
			m_bChanged = false;
		} else if ((anchor = pAtom->GetChargePosition (m_Pos, Angle * 180. / M_PI, x, y)) != gccv::AnchorCenter) {
			m_dAngle = Angle;
			if (m_nState & GDK_SHIFT_MASK) {
				anchor = gccv::AnchorCenter;
				x = m_x0 + m_dDist * cos (m_dAngle);
				y = m_y0 - m_dDist * sin (m_dAngle);
			} else {
				x = x * m_dZoomFactor;
				y = y * m_dZoomFactor;
			}
			static_cast<gccv::Text *> (m_Item)->SetAnchor (anchor);
			m_Item->Move (x - m_x1, y - m_y1);
			m_x1 = x;
			m_y1 = y;
			m_Item->SetVisible (true);
			if (item)
				item->SetVisible (false);
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
		gcp::Atom* pAtom = static_cast <gcp::Atom*> (m_pObject);
		gcp::Document* pDoc = m_pView->GetDoc ();
		gcp::Operation* pOp = pDoc-> GetNewOperation(gcp::GCP_MODIFY_OPERATION);
		gccv::Item *item = pAtom->GetChargeItem ();
		if (item)
			item->SetVisible (true);
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
