// -*- C++ -*-

/* 
 * GChemPaint atoms plugin
 * electrontool.cc 
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
#include "electrontool.h"
#include <gcp/application.h>
#include <gcp/atom.h>
#include <gcp/document.h>
#include <gcp/electron.h>
#include <gcp/molecule.h>
#include <gcp/operation.h>
#include <gcp/settings.h>
#include <gcp/view.h>
#include <gcp/widgetdata.h>
#include <gccv/circle.h>
#include <gccv/group.h>
#include <glib/gi18n-lib.h>
#include <cmath>
#include <stdexcept>

using namespace gcu;
using namespace std;

gcpElectronTool::gcpElectronTool (gcp::Application *App, string Id): gcp::Tool (App, Id)
{
	if (Id == string ("ElectronPair"))
		m_bIsPair = true;
	else if (Id == string ("UnpairedElectron"))
		m_bIsPair = false;
	else throw logic_error ("Unknown tool Id!"); // This should not happen.
}

gcpElectronTool::~gcpElectronTool ()
{
}

bool gcpElectronTool::OnClicked ()
{
	if (!m_pObject || (m_pObject->GetType () != AtomType))
		return false;
	/* explicit electrons will be authorized in fragments only when they will
	be fully implemented */
	if (m_pObject->GetParent ()->GetType () == FragmentType)
		return false;
	double x, y;
	gcp::Atom *pAtom = (gcp::Atom*) m_pObject;
	if (m_bIsPair) {
		if (!pAtom->HasImplicitElectronPairs ())
			return false;
	} else {
		if (!pAtom->MayHaveImplicitUnpairedElectrons ())
			return false;
	}
	pAtom->GetCoords (&m_x0, &m_y0);
	m_Pos = pAtom->GetAvailablePosition (x, y);
	m_x = x - m_x0;
	m_y = y - m_y0;
	gccv::Rect rect;
	m_pData->GetObjectBounds (m_pObject, &rect);
	m_x0 *= m_dZoomFactor;
	m_y0 *= m_dZoomFactor;
	m_dDistMax = min (sqrt (square (rect.x0 - m_x0) + square (rect.y0 - m_y0)),
				sqrt (square (rect.x1 - m_x0) + square (rect.y0 - m_y0)));
	m_dAngle = atan (- m_y / m_x);
		if (m_x < 0) m_dAngle += M_PI;
	x *= m_dZoomFactor;
	y *= m_dZoomFactor;
	x += 2. * cos (m_dAngle);
	y -= 2. * sin (m_dAngle);
	if (m_bIsPair) {
		double deltax = 3. * sin (m_dAngle);
		double deltay = 3. * cos (m_dAngle);
		gccv::Group *group = new gccv::Group (m_pView->GetCanvas ());
		m_Item = group;
		gccv::Circle *circle = new gccv::Circle (group, x + deltax, y + deltay, 2);
		circle->SetLineWidth (0.);
		circle->SetLineColor (0);
		circle->SetFillColor (gcp::AddColor);
		circle = new gccv::Circle (group, x - deltax, y - deltay, 2);
		circle->SetLineWidth (0.);
		circle->SetLineColor (0);
		circle->SetFillColor (gcp::AddColor);
	} else {
		gccv::Circle *circle = new gccv::Circle (m_pView->GetCanvas (), x, y, 2);
		circle->SetLineWidth (0.);
		circle->SetLineColor (0);
		circle->SetFillColor (gcp::AddColor);
		m_Item = circle;
	}
	char tmp[32];
	snprintf (tmp, sizeof (tmp) - 1, _("Orientation: %g"), m_dAngle * 180. / M_PI);
	m_pApp->SetStatusText (tmp);
	m_bChanged = true;
	return true;
}

void gcpElectronTool::OnDrag ()
{
	if (!m_Item)
		return;
	int old_pos = m_Pos;
	m_x -= m_x0;
	m_y -= m_y0;
	m_dDist = sqrt (square (m_x) + square (m_y));
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
				m_bChanged = true;
			}
		} else {
			if (m_bChanged) {
				m_Item->SetVisible (false);
				m_bChanged = false;
			}
		}
	} else {
		double x, y, x1, y1, x2, y2;
		gcp::Atom *pAtom = (gcp::Atom*) m_pObject;
		if (!(m_nState & GDK_SHIFT_MASK) && (m_dDist >= m_dDistMax) && m_bChanged) {
			m_Item->SetVisible (false);
			m_bChanged = false;
		} else if (pAtom->GetPosition (Angle * 180. / M_PI, x, y)) {
			m_dAngle = Angle;
			if (m_Item) {
				delete m_Item;
				m_Item = NULL;
			}
			if (m_nState & GDK_SHIFT_MASK) {
				x = m_x0 + m_dDist * cos (m_dAngle);
				y = m_y0 - m_dDist * sin (m_dAngle);
			} else {
				x = x * m_dZoomFactor;
				y = y * m_dZoomFactor;
				x += 2. * cos (m_dAngle);
				y -= 2. * sin (m_dAngle);
			}
			if (m_bIsPair) {
				double deltax = 3. * sin (m_dAngle);
				double deltay = 3. * cos (m_dAngle);
				gccv::Group *group = new gccv::Group (m_pView->GetCanvas ());
				m_Item = group;
				gccv::Circle *circle = new gccv::Circle (group, x + deltax, y + deltay, 2);
				circle->SetLineWidth (0.);
				circle->SetLineColor (0);
				circle->SetFillColor (gcp::AddColor);
				circle = new gccv::Circle (group, x - deltax, y - deltay, 2);
				circle->SetLineWidth (0.);
				circle->SetLineColor (0);
				circle->SetFillColor (gcp::AddColor);
			} else {
				gccv::Circle *circle = new gccv::Circle (m_pView->GetCanvas (), x, y, 2);
				circle->SetLineWidth (0.);
				circle->SetLineColor (0);
				circle->SetFillColor (gcp::AddColor);
				m_Item = circle;
			}
			m_bChanged = true;
		} else
			m_Pos = old_pos;
	}
	char tmp[32];
	snprintf (tmp, sizeof (tmp) - 1, _("Orientation: %g"), m_dAngle * 180. / M_PI);
	m_pApp->SetStatusText (tmp);
}

void gcpElectronTool::OnRelease ()
{
	if (!m_bChanged)
		return;
	gcp::Atom *pAtom = (gcp::Atom*) m_pObject;
	Object* pObj = m_pObject->GetGroup ();
	gcp::Document* pDoc = m_pView->GetDoc ();
	gcp::Operation* pOp = pDoc-> GetNewOperation (gcp::GCP_MODIFY_OPERATION);
	pOp->AddObject (pObj, 0);
	gcp::Electron *electron = new gcp::Electron (pAtom, m_bIsPair);
	double Angle = m_dAngle * 180. / M_PI;
	if (!(m_nState & GDK_SHIFT_MASK))
		m_dDist = 0.;
	electron->SetPosition (m_Pos, Angle, m_dDist);
	m_pObject->EmitSignal (gcp::OnChangedSignal);
	pOp->AddObject (pObj, 1);
	pDoc->FinishOperation ();
	m_pView->AddObject (electron);
	m_pView->Update (pAtom);
}
