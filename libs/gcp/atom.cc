// -*- C++ -*-

/* 
 * GChemPaint library
 * atom.cc
 *
 * Copyright (C) 2001-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "atom.h"
#include "bond.h"
#include "document.h"
#include "electron.h"
#include "molecule.h"
#include "settings.h"
#include "theme.h"
#include "view.h"
#include "widgetdata.h"
#include "Hposdlg.h"
#include <gccv/canvas.h>
#include <gccv/circle.h>
#include <gccv/group.h>
#include <gccv/text.h>
#include <gccv/text-tag.h>
#include <gcu/chain.h>
#include <gcu/element.h>
#include <gcu/objprops.h>
#include <glib/gi18n-lib.h>
#include <cstdlib>
#include <cstring>
#include <sstream>

using namespace gcu;
using namespace std;

#define ATOM_EPSILON 0.1

namespace gcp {

Atom::Atom ():
	gcu::Atom (),
	ItemClient (),
	m_ShowSymbol (false),
	m_HPosStyle (AUTO_HPOS)
{
	m_Valence = -1; //unspecified
	m_nlp = m_nlu = 0;
	m_nH = 0;
	m_HPos = GetBestSide();
	m_ChargeAuto = false;
	m_ascent = 0;
	m_CHeight = 0.;
	m_Changed = 0;
	m_AvailPosCached = false;
	m_OccupiedPos = 0;
	m_ChargePos = 0xff;
	m_ChargeAngle = 0.;
	m_ChargeDist = 0.;
	m_ChargeAutoPos = true;
	m_Layout = m_ChargeLayout = m_HLayout = NULL;
	m_DrawCircle = false;
	m_SWidth = 0.;
	m_ChargeItem = NULL;
	m_ShowCharge = true;
}

Atom::~Atom ()
{
	Document *pDoc = (Document*) GetDocument ();
	if (!pDoc)
		return;
	View *pView = pDoc->GetView ();
	map<string, Object*>::iterator i;
	Object* obj = GetFirstChild (i);
	while (obj) {
		pView->Remove (obj);
		obj->SetParent (NULL); // avoids a call to RemoveElectron()
		delete obj;
		obj = GetFirstChild (i);
	}
	if (m_Layout)
		g_object_unref (G_OBJECT (m_Layout));
	if (m_ChargeLayout)
		g_object_unref (G_OBJECT (m_ChargeLayout));
}

Atom::Atom (int Z, double x, double y, double z):
	gcu::Atom (Z, x, y, z),
	ItemClient (),
	m_ShowSymbol (false),
	m_HPosStyle (AUTO_HPOS)
{
	m_ChargeAuto = false;
	m_HPos = GetBestSide ();
	m_nlp = m_nlu = 0;
	m_ascent = 0;
	m_CHeight = 0.;
	m_Changed = 0;
	m_AvailPosCached = false;
	m_OccupiedPos = 0;
	m_ChargePos = 0xff;
	m_ChargeAngle = 0.;
	m_ChargeDist = 0.;
	m_ChargeAutoPos = true;
	m_Layout = m_ChargeLayout = m_HLayout = NULL;
	m_DrawCircle = false;
	m_SWidth = 0.;
	m_ChargeItem = NULL;
	m_ShowCharge = true;
	SetZ(Z);
}

void Atom::SetZ (int Z)
{
	gcu::Atom::SetZ (Z);
	if (Z < 1)
		return;
	m_Element = Element::GetElement (m_Z);
	if ((m_Valence = (m_Element)? m_Element->GetDefaultValence (): 0))
		m_HPos = (m_HPosStyle == AUTO_HPOS)? GetBestSide(): m_HPosStyle;
	else
		m_nH = 0;
	int max = (m_Element)? m_Element->GetMaxValenceElectrons (): 0;
	int diff = (m_Element)? m_Element->GetTotalValenceElectrons () - m_Element->GetValenceElectrons (): 0;
	switch (max) {
	case 2:
		m_ValenceOrbitals = 1;
		break;
	case 8:
		m_ValenceOrbitals = 4;
		break;
	case 18:
		if (!diff)
			m_ValenceOrbitals = 6;
		else
			m_ValenceOrbitals = 4;
		break;
	case 32:
		if (!diff)
			m_ValenceOrbitals = 8;
		else if (diff == 14)
			m_ValenceOrbitals = 6;
		else
			m_ValenceOrbitals = 4;
		break;
	default:
		m_ValenceOrbitals = 0; //should not occur
	}
	Update();
	EmitSignal (OnChangedSignal);
}

int Atom::GetTotalBondsNumber () const
{
	std::map<gcu::Atom*, gcu::Bond*>::const_iterator i, end = m_Bonds.end ();
	int n = 0;
	for (i = m_Bonds.begin(); i != end; i++)
		n += (*i).second->GetOrder ();
	return n;
}

void Atom::AddBond (gcu::Bond* pBond)
{
	gcu::Atom::AddBond (pBond);
	m_Changed = true;
}

void Atom::RemoveBond (gcu::Bond* pBond)
{
	gcu::Atom::RemoveBond (pBond);
	Update ();
}

HPos Atom::GetBestSide ()
{
	size_t nb_bonds = m_Bonds.size ();
	if (nb_bonds == 0)
		return (Element::BestSide (m_Z))? RIGHT_HPOS: LEFT_HPOS;
	std::map<gcu::Atom*, gcu::Bond*>::iterator i, end = m_Bonds.end();
	double sumc = 0.0, sums = 0.0, a;
	for (i = m_Bonds.begin(); i != end; i++) {
		a = ((Bond*) (*i).second)->GetAngle2DRad (this);
		sumc += cos (a);
		sums += sin (a);
	}
	if (fabs (sums) > fabs (sumc) && nb_bonds > 1)
		return (fabs (sums) > .1)? ((sums >= 0.)? BOTTOM_HPOS: TOP_HPOS): ((Element::BestSide (m_Z))? RIGHT_HPOS: LEFT_HPOS);
	else
		return (fabs (sumc) > .1)? ((sumc >= 0.)? LEFT_HPOS: RIGHT_HPOS): ((Element::BestSide (m_Z))? RIGHT_HPOS: LEFT_HPOS);
}

void Atom::Update ()
{
	if (m_ChargeAuto) {
		m_Charge = 0;
		m_ChargeAuto = false;
	}
	if (m_ChargeAutoPos) {
		NotifyPositionOccupation (m_ChargePos, false);
		m_ChargePos = 0xff;
	}
	int nb, nexplp = 0, nexplu = 0; //nexplp is the number of explicit lone pairs
	//nexplu is the number of explicit unpaired electrons
	map<string, Object*>::iterator i;
	Object *obj = GetFirstChild (i);
	while (obj) {
		Electron *electron = dynamic_cast <Electron *> (obj);
		if (electron) {
			if (electron->IsPair ())
				nexplp++;
			else
				nexplu++;
		}
		obj = GetNextChild (i);
	}
	int nbonds = GetTotalBondsNumber ();
	if (m_Valence > 0 && !m_Element->IsMetallic ()) {
		m_nlp = (m_Element->GetValenceElectrons () - ((nbonds > m_Valence)? nbonds: m_Valence)) / 2;
		if ((m_Charge > 0) && (m_nlp > 0)) m_nlp -= (m_Charge + 1) / 2;
		else if (m_Charge < 0)
			m_nlp -= m_Charge;
		if (m_nlp < nexplp) // Can this occur ?
			m_nlp = nexplp;
		else if (m_nlp > m_ValenceOrbitals - nbonds - nexplu) {
			int max_bonds = Element::GetElement (m_Z)->GetMaxBonds ();
			if (max_bonds > m_ValenceOrbitals) {
				if (m_nlp > max_bonds - nbonds - nexplu)
					m_nlp = max_bonds - nbonds - nexplu;// FIXME, might be wrong, but will this occur?
			} else
				m_nlp = m_ValenceOrbitals - nbonds - nexplu;
		}
		if (m_nlp < 0)
			m_nlp = 0;
		nb = m_Element->GetValenceElectrons () - 2 * m_nlp - m_Charge;
		if (nb + m_nlp > 4) nb -= 2; //octet rule
		m_nH = nb - nbonds - nexplu;
		if (!m_Charge && m_nH == -1 && m_nlp > 0)
		{
			m_Charge = m_Element->GetValenceElectrons () - nbonds
						- m_nlp * 2 - nexplu;
			m_ChargeAuto = true;
			m_nH = 0;
		}
		if (m_nH < 0) { // extended octet or missing core electrons
			m_nH = 0;
			if (m_nlp || nexplu || nbonds) {
				m_Charge = m_Element->GetValenceElectrons () - 2 * m_nlp - nexplu - nbonds;
				m_ChargeAuto = true;
			}
		}
		m_HPos = (m_HPosStyle == AUTO_HPOS)? GetBestSide(): m_HPosStyle;
	} else {
		m_nH = 0;
		if (m_ChargeAuto || !m_Charge) {
			m_Charge = (m_Element)? m_Element->GetValenceElectrons () - 2 * nexplp - nexplu - nbonds: 0;
			if (m_Charge > 0)
				m_Charge = 0;
			m_ChargeAuto = true;
		}
	}
	m_AvailPosCached = false;
	map<gcu::Atom*, gcu::Bond*>::iterator j = m_Bonds.begin(), jend = m_Bonds.end ();
	if (nbonds && GetZ () == 6) {
		// update large bonds ends
		Bond *bond;
		BondType type;
		bool DrawCircle;
		int nb = 0;
		j = m_Bonds.begin();
		while (j != jend)
		{
			bond = dynamic_cast<Bond*> ((Bond*)(*j).second);
			type = bond->GetType ();
			if (type == ForeBondType || (type == UpBondType && bond->GetAtom (1) == this))
				nb++;
			j++;
		}
		DrawCircle = nb > 1;
		if (!DrawCircle && GetBondsNumber () == 2) {
			j = m_Bonds.begin();
			double angle = static_cast<Bond*> ((*j).second)->GetAngle2D (this);
			j++;
			angle -= static_cast<Bond*> ((*j).second)->GetAngle2D (this);
			if (go_finite (angle)) {
				while (angle < 0)
					angle += 360.;
				while (angle > 360.)
					angle -= 360;
				if (fabs (angle - 180.) < 1)
					DrawCircle = true;
			}
		}
		if (DrawCircle != m_DrawCircle)
			m_DrawCircle = DrawCircle;
	}
	// Update all double bonds
	for (j = m_Bonds.begin(); j != jend; j++)
		if (((*j).second)->GetOrder () == 2)
			static_cast<Bond*> ((*j).second)->SetDirty ();
}

void Atom::UpdateAvailablePositions ()
{
	list<double>::iterator n;
	double angle, delta, dir;
	m_AngleList.clear ();
	if (((GetZ() != 6 || m_Bonds.size() == 0)) && m_nH) {
		switch (m_HPos) {
		case LEFT_HPOS:
			m_AvailPos = 0x6D;
			m_AngleList.push_front(225.0);
			m_AngleList.push_front(135.0);
			break;
		case RIGHT_HPOS:
			m_AvailPos = 0xB6;
			m_AngleList.push_front(315.0);
			m_AngleList.push_front(45.0);
			break;
		case TOP_HPOS:
			m_AvailPos = 0xF8;
			m_AngleList.push_front(135.0);
			m_AngleList.push_front(45.0);
			break;
			break;
		case BOTTOM_HPOS:
			m_AvailPos = 0xC7;
			m_AngleList.push_front(315.0);
			m_AngleList.push_front(225.0);
			break;
			break;
		default:
			break;
		}
	} else
		m_AvailPos = 0xff;
	m_AvailPos &= ~m_OccupiedPos;
	map<gcu::Atom*, gcu::Bond*>::iterator i = m_Bonds.begin();
	while (i != m_Bonds.end()) {
		n = m_AngleList.begin ();
		angle = ((Bond*) (*i).second)->GetAngle2D (this);
		if (angle < 0)
			angle += 360.;
		while ((n != m_AngleList.end ()) && (*n < angle)) n++;
		m_AngleList.insert (n, angle);
		i++;
		if ((m_AvailPos & POSITION_SW) && (angle >= 180.0 - ATOM_EPSILON) &&
			(angle <= 270.0 + ATOM_EPSILON))
			m_AvailPos -= POSITION_SW;
		if ((m_AvailPos & POSITION_SE) && (((angle >= 270.0 - ATOM_EPSILON) &&
			(angle <= 360.0 + ATOM_EPSILON)) || (fabs(angle) < ATOM_EPSILON)))
			m_AvailPos -= POSITION_SE;
		if ((m_AvailPos & POSITION_S) && (angle >= 225.0 - ATOM_EPSILON) &&
			(angle <= 315.0 + ATOM_EPSILON))
			m_AvailPos -= POSITION_S;
		if ((m_AvailPos & POSITION_NW) && (angle >= 90.0 - ATOM_EPSILON) &&
			(angle <= 180.0 + ATOM_EPSILON))
			m_AvailPos -= POSITION_NW;
		if ((m_AvailPos & POSITION_NE) && (((angle >= 0.0 - ATOM_EPSILON) &&
			(angle <= 90.0 + ATOM_EPSILON)) || (fabs(angle - 360.0) < ATOM_EPSILON)))
			m_AvailPos -= POSITION_NE;
		if ((m_AvailPos & POSITION_N) && (angle >= 45.0 - ATOM_EPSILON) &&
			(angle <= 135.0 + ATOM_EPSILON))
			m_AvailPos -= POSITION_N;
		if ((m_AvailPos & POSITION_W) && ((angle <= 225.0 + ATOM_EPSILON) &&
			(angle >= 135.0 - ATOM_EPSILON)))
			m_AvailPos -= POSITION_W;
		if ((m_AvailPos & POSITION_E) && ((angle >= 315.0 - ATOM_EPSILON) ||
			(angle <= 45.0 + ATOM_EPSILON)))
			m_AvailPos -= POSITION_E;
	}
	m_AngleList.push_back ((angle = m_AngleList.front ()) + 360.0);
	m_InterBonds.clear ();
	for (n = m_AngleList.begin (), n++; n != m_AngleList.end (); n++) {
		delta = *n - angle;
		while (m_InterBonds.find (delta) != m_InterBonds.end ())
			delta -= 1e-8;
		dir = (*n + angle) / 2.;
		if ((m_AvailPos == 0xff) || (m_HPos && (dir < 135. || dir > 225.)) ||
			(!m_HPos && (dir > 45. && dir < 315.)))
			m_InterBonds[delta] = dir;
		angle = *n;
	}
	m_AvailPosCached = true;
}

gccv::Anchor Atom::GetChargePosition (unsigned char& Pos, double Angle, double& x, double& y)
{
	list<double>::iterator n, end;
	double angle;
	if (m_ChargePos != 0xff)
		m_OccupiedPos &= ~m_ChargePos;
	if (m_ChargeAutoPos) // reset the charge position
		m_ChargePos = 0xff;
	if (!m_AvailPosCached)
		UpdateAvailablePositions ();
	if (m_ChargePos != 0xff)
		m_OccupiedPos |= m_ChargePos;
	if (!m_ChargeAutoPos && Pos == 0xff) {
		Pos = m_ChargePos;
		if (!Pos)
			Angle = m_ChargeAngle * 180 / M_PI;
	} else if (Pos == 0xff) {
		if (m_AvailPos) {
			if (m_AvailPos & POSITION_NE)
				Pos = POSITION_NE;
			else if (m_AvailPos & POSITION_NW)
				Pos = POSITION_NW;
			else if (m_AvailPos & POSITION_N)
				Pos = POSITION_N;
			else if (m_AvailPos & POSITION_SE)
				Pos = POSITION_SE;
			else if (m_AvailPos & POSITION_SW)
				Pos = POSITION_SW;
			else if (m_AvailPos & POSITION_S)
				Pos = POSITION_S;
			else if (m_AvailPos & POSITION_E)
				Pos = POSITION_E;
			else if (m_AvailPos & POSITION_W)
				Pos = POSITION_W;
		} else {
			Pos = 0;
			angle = m_AngleList.front();
			double max = 0.0;
			end = m_AngleList.end ();
			//if we are there, there are at least two bonds
			for (n = m_AngleList.begin (), n++; n != end; n++) {
				if (*n - angle > max) {
					if (*n - angle - max > 0.1) x = (*n + angle) / 2;
					if (m_nH) {
						if (m_HPos && ((x > 225.0) || (x < 135.0)))
							Angle = x;
						else if (m_HPos && (x > 45.0) && (x < 315.0))
							Angle = x;
					}
					else Angle = x;
					max = *n - angle;
				}
				angle = *n;
			}
		} 
	} else if (Pos) {
		if (!(Pos & m_AvailPos) && (Pos != m_ChargePos))
			return gccv::AnchorCenter;
	} else {
		if (Angle > 360.)
			Angle -= 360;
		else if (Angle < 0.)
			Angle += 360;
		if (!(((GetZ() == 6) && (m_Bonds.size() != 0)) ||
			 !m_nH || ((!m_HPos && (Angle < 135. || Angle > 225.)) ||
				(m_HPos && (Angle > 45. && Angle < 315.)))))
			return gccv::AnchorCenter;
	}
	switch (Pos) {
	case POSITION_NE:
		x = m_x + m_width / 2.0;
		y = m_y - m_height / 2.0;
		return gccv::AnchorWest;
	case POSITION_NW:
		x = m_x - m_width / 2.0;
		y = m_y - m_height / 2.0;
		return gccv::AnchorEast;
	case POSITION_N:
		x = m_x;
		y = m_y - m_height / 2.0;
		return gccv::AnchorSouth;
	case POSITION_SE:
		x = m_x + m_width / 2.0;
		y = m_y + m_height / 2.0;
		return gccv::AnchorWest;
	case POSITION_SW:
		x = m_x - m_width / 2.0;
		y = m_y + m_height / 2.0;
		return gccv::AnchorEast;
	case POSITION_S:
		x = m_x;
		y = m_y + m_height / 2.0;
		return gccv::AnchorNorth;
	case POSITION_E:
		x = m_x + m_width / 2.0;
		y = m_y;
		return gccv::AnchorWest;
	case POSITION_W:
		x = m_x - m_width / 2.0;
		y = m_y;
		return gccv::AnchorEast;
	default: {
			double t = tan (Angle / 180. * M_PI);
			double limit = atan (m_height / m_width) * 180. / M_PI;
			if (Angle < limit) {
				x = m_x /*+  12. */+ m_width / 2.;
				y = m_y - m_width / 2. * t;
				return gccv::AnchorWest;
			} else if (Angle < 180. - limit) {
				if (!isnan (t))
					x = m_x + m_height / 2. / t;
				else
					x = m_x;
				y = m_y - m_height / 2.;
				return gccv::AnchorSouth;
			} else if (Angle < 180. + limit) {
				x = m_x /*- 12.*/ - m_width / 2.;
				y = m_y + m_width / 2. * t;
				return gccv::AnchorEast;
			} else if (Angle < 360. - limit) {
				if (!isnan (t))
					x = m_x - m_height / 2. / t;
				else
					x = m_x;
				y = m_y + m_height / 2.;
				return gccv::AnchorNorth;
			} else {
				x = m_x /*+  12.*/ + m_width / 2.;
				y = m_y - m_width / 2. * t;
				return gccv::AnchorWest;
			}
		}			
	}
	return gccv::AnchorCenter; // should not occur
}

int Atom::GetAvailablePosition (double& x, double& y)
{
	list<double>::iterator n, end;
	double angle;
	if (!m_AvailPosCached)
		UpdateAvailablePositions ();
	if (m_AvailPos) {
		if (m_AvailPos & POSITION_N) {
			x = m_x;
			y = m_y - m_height / 2.0;
			return POSITION_N;
		}
		if (m_AvailPos & POSITION_S) {
			x = m_x;
			y = m_y + m_height / 2.0;
			return POSITION_S;
		}
		if (m_AvailPos & POSITION_E) {
			x = m_x + m_width / 2.0;
			y = m_y;
			return POSITION_E;
		}
		if (m_AvailPos & POSITION_W) {
			x = m_x - m_width / 2.0;
			y = m_y;
			return POSITION_W;
		}
		if (m_AvailPos & POSITION_NE) {
			x = m_x + m_width / 2.0;
			y = m_y - m_height / 2.0;
			return POSITION_NE;
		}
		if (m_AvailPos & POSITION_NW) {
			x = m_x - m_width / 2.0;
			y = m_y - m_height / 2.0;
			return POSITION_NW;
		}
		if (m_AvailPos & POSITION_SE) {
			x = m_x + m_width / 2.0;
			y = m_y + m_height / 2.0;
			return POSITION_SE;
		}
		if (m_AvailPos & POSITION_SW) {
			x = m_x - m_width / 2.0;
			y = m_y + m_height / 2.0;
			return POSITION_SW;
		}
	}
	angle = m_AngleList.front ();
	double dir = 0.0, max = 0.0;
	end = m_AngleList.end ();
	//if we are there, there are at least two bonds
	for (n = m_AngleList.begin (), n++; n != end; n++) {
		if (*n - angle > max) {
			if (*n - angle - max > 0.1)
				x = (*n + angle) / 2;
			if (m_nH) {
				if (m_HPos && ((x > 225.0) || (x < 135.0)))
					dir = x;
				else if (m_HPos && (x > 45.0) && (x < 315.0))
					dir = x;
			} else
				dir = x;
			max = *n - angle;
		}
		angle = *n;
	}
	max = sqrt (square (m_width) + square (m_height)) / 2.0 + 24.;//Could do better, should replace 24 by something more intelligent
	x = m_x + max * cos (dir / 180.0 * M_PI);
	y = m_y - max * sin (dir / 180.0 * M_PI);
	return 0;
}

bool Atom::LoadNode (xmlNodePtr)
{
	SetZ (GetZ ());
	return true;
}

void Atom::SetSelected (int state)
{
	GOColor textcolor, othercolor;
	
	switch (state) {	
	default:
	case SelStateUnselected:
		textcolor = 0;
		othercolor = GO_COLOR_BLACK;
		break;
	case SelStateSelected:
		othercolor = textcolor = SelectColor;
		break;
	case SelStateUpdating:
		othercolor = textcolor = AddColor;
		break;
	case SelStateErasing:
		othercolor = textcolor = DeleteColor;
		break;
	}
	gccv::Group *group = static_cast <gccv::Group *> (m_Item);
	std::list<gccv::Item *>::iterator it;
	gccv::Item *item = group->GetFirstChild (it);
	while (item) {
		if (item->GetClient () == this) {
			gccv::FillItem *fill;
			gccv::Text *text;
			if ((text = dynamic_cast <gccv::Text *> (item)))
				text->SetColor (othercolor);
			else if ((fill = dynamic_cast <gccv::Rectangle *> (item)))
				fill->SetFillColor (textcolor);
			else if ((fill = dynamic_cast <gccv::FillItem *> (item)))
				fill->SetFillColor (othercolor);
			else
				static_cast <gccv::LineItem *> (item)->SetLineColor (othercolor);
		}
		item = group->GetNextChild (it);
	}
	// select children
	map<string, Object*>::iterator ic;
	gccv::ItemClient *client;
	for (Object* obj = GetFirstChild (ic); obj; obj = GetNextChild (ic)) {
		client = dynamic_cast <gccv::ItemClient *> (obj);
		if (client)
			client->SetSelected (state);
	}
}

bool Atom::AcceptNewBonds (int nb)
{
	if ((m_Valence > 0) || m_ChargeAuto)
		return Element::GetMaxBonds (m_Z) >= (GetTotalBondsNumber () + GetChildrenNumber () + nb);
	map<string, Object*>::iterator i;
	Electron* electron;
	unsigned nel = 0;
	for (Object *obj = GetFirstChild (i); obj; obj = GetNextChild (i)) {
		electron = dynamic_cast <Electron *> (obj);
		if (electron == NULL)
			continue;
		if (electron->IsPair ())
			nel += 2;
		else
			nel++;
	}
	nel += GetTotalBondsNumber ();
	return (m_ValenceOrbitals - GetTotalBondsNumber () - GetChildrenNumber () > 0)
			&& (((m_Element->GetValenceElectrons() - m_Charge) > nel) || m_ChargeAuto);
}

void Atom::AddToMolecule (Molecule* Mol)
{
	Mol->AddAtom (this);
}
	
double Atom::GetYAlign ()
{
	return m_y;
}

bool Atom::HasImplicitElectronPairs ()
{
	map<string, Object*>::iterator i;
	Object *obj = GetFirstChild (i);
	Electron* electron = NULL;
	if (m_Valence > 0) {
		int nexplp = 0; //nexplp is the number of explicit lone pairs
		while (obj) {
			electron = dynamic_cast <Electron *> (obj);
			if (electron && electron->IsPair ())
				nexplp++;
			obj = GetNextChild (i);
		}
		return (m_nlp > nexplp);
	}
	unsigned nel = 0;
	while (electron) { 
		electron = dynamic_cast <Electron *> (obj);
		if (electron) {
			if (electron->IsPair ())
				nel += 2;
			else
				nel++;
		}
		obj = GetNextChild (i);
	}
	nel += GetTotalBondsNumber ();
	int nocc = GetChildrenNumber () + GetTotalBondsNumber ();
	return (nocc < m_ValenceOrbitals) && (((m_Element->GetValenceElectrons() - m_Charge) > nel + 1) || m_ChargeAuto);
}

bool Atom::MayHaveImplicitUnpairedElectrons ()
{
	map<string, Object*>::iterator i;
	Electron* electron = (Electron*) GetFirstChild (i);
	unsigned nel = 0;
	while (electron) { 
		if (electron->IsPair ())
			nel += 2;
		else
			nel++;
		electron = (Electron*) GetNextChild (i);
	}
	nel += GetTotalBondsNumber ();
	return (m_ValenceOrbitals - GetTotalBondsNumber () - GetChildrenNumber () > 0)
			&& (((m_Element->GetValenceElectrons() - m_Charge) > nel) || m_ChargeAuto);
}

bool Atom::GetRelativePosition (double angle, double& x, double& y)
{
	if (angle > 360.)
		angle -= 360;
	else if (angle < 0.)
		angle += 360;
	if (((GetZ() == 6) && (m_Bonds.size() != 0)) ||
		 !m_nH || ((m_HPos == LEFT_HPOS && (angle < 135. || angle > 225.)) ||
			(m_HPos == RIGHT_HPOS && (angle > 45. && angle < 315.)) ||
			(m_HPos == TOP_HPOS && (angle > 45. || angle > 135.)) ||
			(m_HPos == BOTTOM_HPOS && (angle < 225. || angle > 315.)))) {
		double t = tan (angle / 180. * M_PI);
		double limit = atan (m_height / m_width) * 180. / M_PI;
		if (angle < limit) {
			x = m_width / 2.;
			y = -m_width / 2. * t;
		} else if (angle < 180. - limit) {
			if (!isnan (t))
				x = m_height / 2. / t;
			else
				x = 0.;
			y = -m_height / 2.;
		} else if (angle < 180. + limit) {
			x = -m_width / 2.;
			y = m_width / 2. * t;
		} else if (angle < 360. - limit) {
			if (!isnan (t))
				x = -m_height / 2. / t;
			else
				x = m_x;
			y = m_height / 2.;
		} else {
			x = m_width / 2.;
			y = -m_width / 2. * t;
		}			
		return true;
	}
	return false;
}

bool Atom::GetPosition (double angle, double& x, double& y)
{
	if (GetRelativePosition (angle, x, y)) {
		x += m_x;
		y += m_y;
		return true;
	}
	return false;
}

void Atom::AddElectron (Electron* electron)
{
	AddChild (electron);
	Update ();
}

void Atom::RemoveElectron (Electron* electron)
{
	// remove the electron from children so that it is not taken into account when
	// updating.
	electron->SetParent (NULL);
	Update ();
	// Force view update.
	Document *pDoc = reinterpret_cast<Document*> (GetDocument ());
	if (pDoc)
		pDoc->GetView ()->Update (this);
}

void Atom::NotifyPositionOccupation (unsigned char pos, bool occupied)
{
	if (occupied)
		m_OccupiedPos |= pos;
	else
		m_OccupiedPos &= ~pos;
}
	
xmlNodePtr Atom::Save (xmlDocPtr xml) const
{
	xmlNodePtr node = gcu::Atom::Save (xml);
	if (node)
		SaveChildren (xml, node);
	if (m_Charge && !m_ChargeAutoPos) {
		char *buf;
		if (m_ChargePos) {
			char const *buf;
			switch (m_ChargePos) {
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
			xmlNewProp (node, (xmlChar*) "charge-position", (xmlChar*) buf);
		} else {
			buf = g_strdup_printf ("%g", m_ChargeAngle * 180. / M_PI);
			xmlNewProp (node, (xmlChar*) "charge-angle", (xmlChar*) buf);
			g_free (buf);
		}
		if (m_ChargeDist != 0.) {
			buf = g_strdup_printf ("%g", m_ChargeDist);
			xmlNewProp (node, (xmlChar*) "charge-dist", (xmlChar*) buf);
			g_free (buf);
		}
	}
	if (GetZ () == 6 && m_ShowSymbol) {
		xmlNewProp (node, (xmlChar*) "show-symbol", (xmlChar*) "true");
	}
	if (m_HPosStyle != AUTO_HPOS) {
		char const *pos;
		switch (m_HPosStyle) {
		default:
		case RIGHT_HPOS:
			pos = "right";
			break;
		case LEFT_HPOS:
			pos = "left";
			break;
		case TOP_HPOS:
			pos = "top";
			break;
		case BOTTOM_HPOS:
			pos = "bottom";
			break;
		}
		xmlNewProp (node, reinterpret_cast <xmlChar const*> ("H-position"),
					reinterpret_cast <xmlChar const*> (pos));
	}
	if (!m_ShowCharge)
		xmlNewProp (node, reinterpret_cast <xmlChar const*> ("show-charge"),
					reinterpret_cast <xmlChar const*> ("no"));
	return node;
}
	
bool Atom::Load (xmlNodePtr node)
{
	if (!gcu::Atom::Load (node))
		return false;
	//Load electrons
	xmlNodePtr child = node->children;
	Electron *electron;
	while (child) {
		electron = NULL;
		if (!strcmp ((char*) child->name, "electron"))
			electron = new Electron (this, false);
		else if (!strcmp ((char*) child->name, "electron-pair"))
			electron = new Electron (this, true);
		else if (strcmp (reinterpret_cast <char const *> (child->name), "position")
		    && strcmp (reinterpret_cast <char const *> (child->name), "text")) {
			Object *obj = CreateObject (reinterpret_cast <char const *> (child->name));
			if (obj) {
				AddChild (obj);
				if (!obj->Load (child))
						return false;
			}
		}
		if (electron && !electron->Load (child))
			return false;
		child = child->next;
	}
	char *buf = (char*) xmlGetProp (node, (xmlChar*) "charge-position");
	m_ChargePos = 0xff;
	if (buf) {
		if (! strcmp (buf, "ne")) {
			m_ChargePos = POSITION_NE;
			m_ChargeAngle = M_PI / 4.;
		} else if (! strcmp (buf, "nw")) {
			m_ChargePos = POSITION_NW;
			m_ChargeAngle = 3. * M_PI / 4.;
		} else if (! strcmp (buf, "n")) {
			m_ChargePos = POSITION_N;
			m_ChargeAngle = M_PI / 2.;
		} else if (! strcmp (buf, "se")) {
			m_ChargePos = POSITION_SE;
			m_ChargeAngle = 7. * M_PI / 4;
		} else if (! strcmp (buf, "sw")) {
			m_ChargePos = POSITION_SW;
			m_ChargeAngle = 5. * M_PI / 4;
		} else if (! strcmp (buf, "s")) {
			m_ChargePos = POSITION_S;
			m_ChargeAngle = 3 * M_PI / 2.;
		} else if (! strcmp (buf, "e")) {
			m_ChargePos = POSITION_E;
			m_ChargeAngle = 0.;
		} else if (! strcmp (buf, "w")) {
			m_ChargePos = POSITION_W;
			m_ChargeAngle = M_PI;
		}
		m_ChargeAutoPos = false;
		xmlFree (buf);
	} else {
		buf = (char*) xmlGetProp(node, (xmlChar*)"charge-angle");
		if (buf) {
			sscanf(buf, "%lg", &m_ChargeAngle);
			m_ChargeAngle *= M_PI / 180.;
			xmlFree (buf);
			m_ChargePos = 0;
			m_ChargeAutoPos = false;
		}
	}
	buf = (char*) xmlGetProp(node, (xmlChar*)"charge-dist");
	if (buf) {
		sscanf(buf, "%lg", &m_ChargeDist);
		xmlFree (buf);
		m_ChargeAutoPos = false;
	} else
		m_ChargeDist = 0.;
	buf = (char*) xmlGetProp (node, (xmlChar*) "show-symbol");
	if (buf) {
		if (!strcmp (buf, "true"))
			m_ShowSymbol = true;
		xmlFree (buf);
	}
	// Load H atoms position if any
	buf = (char*) xmlGetProp (node, (xmlChar*) "H-position");
	if (buf) {
		if (!strcmp (buf, "left"))
			m_HPosStyle = LEFT_HPOS;
		else if (!strcmp (buf, "right"))
			m_HPosStyle = RIGHT_HPOS;
		else if (!strcmp (buf, "top"))
			m_HPosStyle = TOP_HPOS;
		else if (!strcmp (buf, "bottom"))
			m_HPosStyle = BOTTOM_HPOS;
		else // who know?
			m_HPosStyle = AUTO_HPOS;
		xmlFree (buf);
		Update ();
	}
	buf = reinterpret_cast <char *> (xmlGetProp (node, (xmlChar*) "show-charge"));
	if (buf) {
		if (!strcmp (buf, "no"))
			m_ShowCharge = false;
		xmlFree (buf);
	}
	return true;
}
	
bool Atom::AcceptCharge (int charge) {
	unsigned nb = GetTotalBondsNumber (), ne = 0;
	map<string, Object*>::iterator i;
	Electron* electron = (Electron*) GetFirstChild (i);
	while (electron){ 
		if (electron->IsPair ())
			ne += 2;
		else
			ne++;
		electron = (Electron*) GetNextChild (i);
	}
	if (charge < 0)
		return (m_Element->GetTotalValenceElectrons () <= m_Element->GetMaxValenceElectrons () + charge - nb - 2 * GetChildrenNumber () + ne);
	if (nb)
		return (m_Element->GetValenceElectrons () >= charge + nb + ne);
	return (charge <= GetZ ());
}

void Atom::SetChargePosition (unsigned char Pos, bool def, double angle, double distance)
{
	if (Pos != m_ChargePos) {
		m_ChargeAutoPos = def;
		if (m_ChargePos > 0)
			NotifyPositionOccupation (m_ChargePos, false);
		m_ChargePos = Pos;
		if (m_ChargePos > 0)
			NotifyPositionOccupation (m_ChargePos, true);
	}
	m_ChargeAngle = angle;
	m_ChargeDist = distance;
	m_AvailPosCached = false;
}

char Atom::GetChargePosition (double *Angle, double *Dist) const
{
	if (Angle)
		*Angle = m_ChargeAngle;
	if (Dist)
		*Dist = m_ChargeDist;
	return (m_ChargeAutoPos)? -1: m_ChargePos;
}

void Atom::SetCharge (int charge)
{
	gcu::Atom::SetCharge (charge);
	m_ChargeAuto = false;
	Update ();
}

void Atom::Move (double x, double y, double z)
{
	gcu::Atom::Move (x, y, z);
	map<string, Object*>::iterator i;
	Object* electron = GetFirstChild (i);
	while (electron) {
		electron->Move (x, y, z);
		electron = GetNextChild (i);
	}
	if (GetCharge ()) {
		if (m_ChargeAutoPos) {
			if (m_ChargePos > 0)
				NotifyPositionOccupation (m_ChargePos, false);
			m_ChargePos = 0xff;
			Update ();
		}
	}
}

void Atom::Transform2D (Matrix2D& m, double x, double y)
{
	gcu::Atom::Transform2D (m, x, y);
	// Now transform electrons
	map<string, Object*>::iterator i;
	Object* electron = GetFirstChild (i);
	while (electron) {
		electron->Transform2D (m, x, y);
		electron = GetNextChild (i);
	}
	if (GetCharge ()) {
		if (m_ChargeAutoPos) {
			if (m_ChargePos > 0)
				NotifyPositionOccupation (m_ChargePos, false);
			m_ChargePos = 0xff;
			Update ();
		} else {
			double xc = cos (m_ChargeAngle), yc = - sin (m_ChargeAngle);
			m.Transform (xc, yc);
			m_ChargeAngle = atan2 (- yc, xc);
			if (m_ChargeAngle < 0)
				m_ChargeAngle += 2 * M_PI;
			SetChargePosition (0, FALSE, m_ChargeAngle, m_ChargeDist);
		}
	}
}

static void do_display_symbol (GtkToggleAction *action, Atom *pAtom)
{
	Document *Doc = static_cast <Document *> (pAtom->GetDocument ());
	Operation *Op = Doc->GetNewOperation (GCP_MODIFY_OPERATION);
	Object *Obj = pAtom->GetGroup ();
	Op->AddObject (Obj, 0);
	pAtom->SetShowSymbol (gtk_toggle_action_get_active (action));
	pAtom->Update ();
	pAtom->ForceChanged ();
	pAtom->EmitSignal (OnChangedSignal);
	Op->AddObject (Obj, 1);
	Doc->FinishOperation ();
	View *view = Doc->GetView ();
	view->Update (pAtom);
	// also update the bonds
	map<gcu::Atom*, gcu::Bond*>::iterator i;
	Bond *bond = static_cast <Bond *> (pAtom->GetFirstBond (i));
	while (bond) {
		bond->SetDirty ();
		view->Update (bond);
		bond = static_cast <Bond *> (pAtom->GetNextBond (i));
	}
	
}

static void do_choose_H_pos (Atom* Atom)
{
	new HPosDlg (static_cast<Document*> (Atom->GetDocument ()), Atom);
}

static void do_show_charge (GtkToggleAction *action, Atom *atom)
{
	Document *Doc = static_cast <Document *> (atom->GetDocument ());
	Operation *Op = Doc->GetNewOperation (GCP_MODIFY_OPERATION);
	Object *Obj = atom->GetGroup ();
	Op->AddObject (Obj, 0);
	atom->SetShowCharge (gtk_toggle_action_get_active (action));
	Op->AddObject (Obj, 1);
	Doc->FinishOperation ();
	View *view = Doc->GetView ();
	view->Update (atom);
	
}

static void do_select_child (Object *obj)
{
	Document *Doc = static_cast <Document *> (obj->GetDocument ());
	View *view = Doc->GetView ();
	WidgetData *data = view->GetData ();
	data->UnselectAll ();
	data->SetSelected (obj);
}

static void do_delete_child (Object *obj)
{
	Document *Doc = static_cast <Document *> (obj->GetDocument ());
	View *view = Doc->GetView ();
	WidgetData *data = view->GetData ();
	data->UnselectAll ();
	data->SetSelected (obj);
	view->OnDeleteSelection (view->GetWidget ());
}

static void do_child_properties (Object *obj)
{
	obj->ShowPropertiesDialog ();
}

bool Atom::BuildContextualMenu (GtkUIManager *UIManager, Object *object, double x, double y)
{
	bool result = false;
	GtkActionGroup *group = NULL;
	GtkAction *action;
	if (GetZ () == 6 && GetBondsNumber() != 0) {
		group = gtk_action_group_new ("atom");
		action = gtk_action_new ("Atom", _("Atom"),NULL, NULL);
		gtk_action_group_add_action (group, action);
		g_object_unref (action);
		action = GTK_ACTION (gtk_toggle_action_new ("show-symbol", _("Display symbol"),  _("Whether to display carbon atom symbol"), NULL));
		gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), m_ShowSymbol);
		g_signal_connect (action, "toggled", G_CALLBACK (do_display_symbol), this);
		gtk_action_group_add_action (group, action);
		g_object_unref (action);
		gtk_ui_manager_add_ui_from_string (UIManager, "<ui><popup><menu action='Atom'><menuitem action='show-symbol'/></menu></popup></ui>", -1, NULL);
		result = true;
	}
	if (m_nH) {
		if (!group) {
			group = gtk_action_group_new ("atom");
			action = gtk_action_new ("Atom", _("Atom"),NULL, NULL);
			gtk_action_group_add_action (group, action);
			g_object_unref (action);
		}
		action = GTK_ACTION (gtk_action_new ("H-position", _("Hydrogen atoms position"),  NULL, NULL));
		g_signal_connect_swapped (action, "activate", G_CALLBACK (do_choose_H_pos), this);
		gtk_action_group_add_action (group, action);
		g_object_unref (action);
		gtk_ui_manager_add_ui_from_string (UIManager, "<ui><popup><menu action='Atom'><menuitem action='H-position'/></menu></popup></ui>", -1, NULL);
	}
	if (m_Charge) {
		if (!group) {
			group = gtk_action_group_new ("atom");
			action = gtk_action_new ("Atom", _("Atom"),NULL, NULL);
			gtk_action_group_add_action (group, action);
			g_object_unref (action);
		}
		action = GTK_ACTION (gtk_toggle_action_new ("show-charge", _("Show charge"),   _("Whether to display atom charge"), NULL));
		gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), m_ShowCharge);
		g_signal_connect (action, "toggled", G_CALLBACK (do_show_charge), this);
		gtk_action_group_add_action (group, action);
		g_object_unref (action);
		gtk_ui_manager_add_ui_from_string (UIManager, "<ui><popup><menu action='Atom'><menuitem action='show-charge'/></menu></popup></ui>", -1, NULL);
    }
	if (object == this && HasChildren ()) {
		if (!group)
			group = gtk_action_group_new ("atom");
		// add a submenu for each child
		std::map< std::string, Object * >::iterator i;
		for (Object *obj = GetFirstChild (i); obj; obj = GetNextChild (i)) {
			string id = obj->Identity ();
			string sel = id + "-select";
			string del = id + "-delete";
			string prop;
			action = gtk_action_new (id.c_str (), id.c_str (),NULL, NULL);
			gtk_action_group_add_action (group, action);
			g_object_unref (action);
			action = GTK_ACTION (gtk_action_new (sel.c_str (), _("Select"), _("Select object"), NULL));
			g_signal_connect_swapped (action, "activate", G_CALLBACK (do_select_child), obj);
			gtk_action_group_add_action (group, action);
			g_object_unref (action);
			action = GTK_ACTION (gtk_action_new (del.c_str (), _("Delete"), _("Delete object"), NULL));
			g_signal_connect_swapped (action, "activate", G_CALLBACK (do_delete_child), obj);
			gtk_action_group_add_action (group, action);
			g_object_unref (action);
			bool has_props = obj->HasPropertiesDialog ();
			if (has_props) {
				prop = id + "props";
				action = GTK_ACTION (gtk_action_new (prop.c_str (), _("Properties"), _("Object properties"), NULL));
				g_signal_connect_swapped (action, "activate", G_CALLBACK (do_child_properties), obj);
				gtk_action_group_add_action (group, action);
				g_object_unref (action);
			}
			// now add the ui string
			ostringstream str;
			str << "<ui><popup><menu action='" << id << "'><menuitem action='" << sel <<
				"'/><menuitem action='" << del;
			if (has_props)
				str << "'/><menuitem action='" << prop;
			str << "'/></menu></popup></ui>";
			gtk_ui_manager_add_ui_from_string (UIManager, str.str ().c_str (), -1, NULL);
		}
	}
	if (group) {
		gtk_ui_manager_insert_action_group (UIManager, group, 0);
		g_object_unref (group);
	}
	return result | Object::BuildContextualMenu (UIManager, object, x, y);
}

bool Atom::Match (gcu::Atom *atom, AtomMatchState &state)
{
	if (m_nH != static_cast <Atom*> (atom)->m_nH)
		return false;
	return gcu::Atom::Match (atom, state);
}

void Atom::GetSymbolGeometry (double &width, double &height, double &angle, bool up) const
{
	if ((GetZ() != 6) || (GetBondsNumber () == 0) || m_ShowSymbol) {
		width = m_SWidth;
		if (up) {
			height = m_SHeightH ;
			angle = m_SAngleH;
		} else {
			height = m_SHeightL ;
			angle = m_SAngleL;
		}
	} else
		width = height  = angle = 0.;
}

void Atom::BuildSymbolGeometry (double width, double height, double ascent)
{
	m_SWidth = width / 2.;
	m_SHeightH = ascent + 1.; // we use ink extent vertically and logical extent horizonatlly
	m_SHeightL = height - m_SHeightH + 2.;
	m_SAngleH = atan2 (m_SHeightH, m_SWidth);
	m_SAngleL = atan2 (m_SHeightL, m_SWidth);
}

void Atom::AddItem ()
{
	if (m_Item || GetZ() <= 0)
		return;
	if (m_Changed) {
		Update ();
		m_Changed = false;
	}
	Document *doc = static_cast <Document*> (GetDocument ());
	View *view = doc->GetView ();
	Theme *theme = doc->GetTheme ();
	double x, y;
	GetCoords (&x, &y);
	x *= theme->GetZoomFactor ();
	y *= theme->GetZoomFactor ();
	// always use a group, even if not needed
	gccv::Group *group = new gccv::Group (view->GetCanvas ()->GetRoot (), x, y, this);
	view->GetCanvas ()->GetRoot ()->MoveToFront (group);
	if ((GetZ() != 6) || (GetBondsNumber() == 0) || m_ShowSymbol) {
		gccv::Text *text = new gccv::Text (group, 0., 0., this);
		text->SetColor ((view->GetData ()->IsSelected (this))? SelectColor: GO_COLOR_BLACK);
		text->SetPadding (theme->GetPadding ());
		text->SetLineColor (0);
		text->SetLineWidth (0.);
		text->SetFillColor (0);
		text->SetFontDescription (view->GetPangoFontDesc ());
		text->SetText (GetSymbol ());
		text->SetLineOffset (view->GetCHeight ());
		// build the symbol geometry
		int n = GetAttachedHydrogens ();
		gccv::Rect ink, logical;
		text->GetBounds (&ink, &logical);
		BuildSymbolGeometry (text->GetWidth (), ink.y1 - ink.y0, /*text->GetAscent () - view->GetCHeight ()*/ - ink.y0);		
		m_width = (ink.x1 - ink.x0 + 2 * theme->GetPadding ()) / theme->GetZoomFactor ();
		m_height = (ink.y1 - ink.y0 + 2 * theme->GetPadding ()) / theme->GetZoomFactor ();
		if (n > 0) {
			string hs = "H";
			if (n > 1) {
				char *str = g_strdup_printf ("%d", n);
				hs += str;
				g_free (str);
			}
			text = new gccv::Text (group, 0., 0., this);
			text->SetColor ((view->GetData ()->IsSelected (this))? SelectColor: GO_COLOR_BLACK);
			text->SetPadding (theme->GetPadding ());
			text->SetLineColor (0);
			text->SetLineWidth (0.);
			text->SetFillColor (0);
			text->SetFontDescription (view->GetPangoFontDesc ());
			text->SetText (hs.c_str ());
			if (n >1) {
				gccv::TextTag *tag = new gccv::PositionTextTag (gccv::Subscript, text->GetDefaultFontSize ());
				tag->SetStartIndex (1);
				tag->SetEndIndex (hs.length ());
				text->InsertTextTag (tag);
			}
			text->SetLineOffset (view->GetCHeight ());
			switch (m_HPos) {
			case LEFT_HPOS:
				text->SetAnchor (gccv::AnchorLineEast);
				text->SetPosition (logical.x0, 0.);
				break;
			case RIGHT_HPOS:
				text->SetAnchor (gccv::AnchorLineWest);
				text->SetPosition (logical.x1, 0.);
				break;
			case TOP_HPOS:
				text->SetAnchor (gccv::AnchorLineWest);
				text->SetPosition (-view->GetHWidth (), ink.y0 - ink.y1 - 2.); // 2. is arbitrary
				break;
			case BOTTOM_HPOS:
				text->SetAnchor (gccv::AnchorLineWest);
				text->SetPosition (-view->GetHWidth (), ink.y1 - ink.y0 + 2.);
				break;
			default:
				g_critical ("This should not happen, please file a bug report");
				break;
			}
		}
	} else {
		gccv::FillItem *fill = new gccv::Rectangle (group,  -3., -3., 6., 6., this);
		fill->SetFillColor ((view->GetData ()->IsSelected (this))? SelectColor: 0);
		fill->SetLineColor (0);
		m_width = m_height = 2 * theme->GetPadding () / theme->GetZoomFactor ();
		if (m_DrawCircle) {
			fill = new gccv::Circle ( group, 0., 0., theme->GetStereoBondWidth () / 2., this);
			fill->SetFillColor ((view->GetData ()->IsSelected (this))? SelectColor: Color);
			fill->SetLineColor (0);
		}
	}
	m_Item = group;
	int charge = GetCharge ();
	if (charge && m_ShowCharge) {
		gccv::Anchor anchor = GetChargePosition (m_ChargePos, m_ChargeAngle * 180 / M_PI, x, y);
		if (m_ChargeDist != 0.) {
			anchor = gccv::AnchorCenter;
			x = m_x + m_ChargeDist * cos (m_ChargeAngle);
			y = m_y - m_ChargeDist * sin (m_ChargeAngle);
		}
		x -= m_x;
		x *= theme->GetZoomFactor ();
		y -= m_y;
		y *= theme->GetZoomFactor ();
		char* markup = NULL;
		char const *glyph = (charge > 0)? "\xE2\x8a\x95": "\xE2\x8a\x96";
		if (abs (m_Charge) > 1)
			markup = g_strdup_printf ("%d%s", abs (m_Charge), glyph);
		else
			markup = g_strdup (glyph);
		gccv::Text *text = new gccv::Text (group, x, y, this);
		text->SetColor ((view->GetData ()->IsSelected (this))? SelectColor: GO_COLOR_BLACK);
		text->SetFillColor (0);
		text->SetPadding (theme->GetPadding ());
		text->SetLineColor (0);
		text->SetLineWidth (0.);
		text->SetAnchor (anchor);
		text->SetFontDescription (view->GetPangoSmallFontDesc ());
		text->SetText (markup);
		m_ChargeItem = text;
		g_free (markup);
	} else
		m_ChargeItem = NULL;
	// select children
	map<string, Object*>::iterator ic;
	gccv::ItemClient *client;
	for (Object* obj = GetFirstChild (ic); obj; obj = GetNextChild (ic)) {
		client = dynamic_cast <gccv::ItemClient *> (obj);
		if (client)
			client->AddItem ();
	}
}

bool Atom::HasAvailableElectrons (bool paired)
{
	map<string, Object*>::iterator i;
	Object *obj = GetFirstChild (i);
	Electron *electron = NULL;
	while (obj) {
		if ((electron = dynamic_cast <Electron *> (obj)))
			break;
			obj = GetNextChild (i);
	}
	if (paired) {
		if (m_nlp)
			return true;
		while (obj) { 
			if (electron && electron->IsPair ())
				return true;
			obj = GetNextChild (i);
			electron = dynamic_cast <Electron *> (obj);
		}
	} else {
		return electron || m_nlp || m_nlu; // TODO: take curved arrows into account
	}
	return false;
}

Bond *Atom::GetBondAtAngle (double angle)
{
	Bond *res = NULL, *bond;
	double a1, a0 = 2 * M_PI;// something larger that anything we can get
	std::map <gcu::Atom *, gcu::Bond *>::iterator it;
	for (bond = static_cast <gcp::Bond *> (GetFirstBond (it)); bond; bond = static_cast <gcp::Bond *> (GetNextBond (it))) {
		a1 = bond->GetAngle2DRad (this);
		a1 -= angle;
		a1 = fabs (a1);
		if (a1 > M_PI)
			a1 = 2 * M_PI - a1;
		if (a1 < a0) {
			a0 = a1;
			res = bond;
		}
	}
	return res;
}

bool Atom::SetProperty (unsigned property, char const *value)
{
	switch (property) {
	case GCU_PROP_ATOM_PARITY: {
		std::istringstream iss (value);
		int p;
		std::string a0, a1, a2, a3;
		iss >> p >> a0 >> a1 >> a2 >> a3;
		if (p == 0)
			return false;
		gcu::Document *doc = GetDocument ();
		doc->SetTarget (a0.c_str (), reinterpret_cast <Object **> (m_Bonded + ((p > 0)? 0: 1)), GetParent (), this);
		doc->SetTarget (a1.c_str (), reinterpret_cast <Object **> (m_Bonded + ((p > 0)? 1: 0)), GetParent (), this);
		doc->SetTarget (a2.c_str (), reinterpret_cast <Object **> (m_Bonded + 2), GetParent (), this);
		if (a3.length () == 0)
			m_Bonded[3] = NULL;
		else
			doc->SetTarget (a3.c_str (), reinterpret_cast <Object **> (m_Bonded + 3), GetParent (), this);
		static_cast < Molecule * > (GetMolecule ())->AddChiralAtom (this);
		break;
	}
	default:
		return gcu::Atom::SetProperty (property, value);
	}
	return  true;
}

bool Atom::UpdateStereoBonds ()
{
	unsigned length[4]; // lengths before end or cycle 0 means cyclic bond
	unsigned cycle_size[4]; // size of the nearest and largest attached cycle
	unsigned cycle_pos[4]; // position of the cycle
	Bond *bond[4];
	double x[4], y[4];
	std::list < unsigned > sorted;
	std::list < unsigned >::iterator s, send;
	// we need to determine wich bonds should be considered as stereobonds
	// the answer is that stereocenters shound not be bonded by a stereobond
	// then the bond should not be cyclic
	// then it shoud be in the shortest possible chain
	// it should be not more that 90° from the nearest bond (which might also be
	// a stereobond)
	for (unsigned i = 0; i < 4; i++) {
		if (!m_Bonded[i]) {
			if (i < 3)
				return false; // atoms are still not fully loaded
			bond[i] = NULL;
			length[i] = 0;
			cycle_size[i] = 0;
			cycle_pos[i] = 0;
			continue;
		}
		bond[i] = static_cast < Bond * > (GetBond (m_Bonded[i]));
		if (!bond[i]) // not everything has been loaded
			return false;
		// search if the bonded atom is a stereocenter
		if (static_cast < Molecule * > (GetMolecule ())->AtomIsChiral (m_Bonded[i]) ||
		    bond[i]->IsCyclic ()) {
			length[i] = cycle_pos[i] = G_MAXUINT; // this will be large enough
			cycle_size[i] = 0;
		} else {
			gcu::Chain *chain = new gcu::Chain (bond[i], this);
			// find the longuest linear chain and the first attached cycle
			length[i] = chain->BuildLength (cycle_size + i, cycle_pos + i);
			// now delete the chain
			delete chain;
		}
		m_Bonded[i]->GetCoords (x + i, y + i);
		// now sort the atoms with the stereobonds preferably first.
		send = sorted.end ();
		for (s = sorted.begin (); s != send; s++) {
			if (length[*s] > length[i] || 
			    (length[*s] == length[i] && (cycle_pos[*s] > cycle_pos[i] ||
			    (cycle_pos[*s] == cycle_pos[i] && (cycle_size[*s] < cycle_size[i] ||
			    (cycle_size[*s] == cycle_size[i] && (m_Bonded[*s]->GetZ () > m_Bonded[i]->GetZ () || m_Bonded[*s]->GetZ () == 6)))))))
				break;
		}
		sorted.insert (s, i);
	}
	unsigned n1, n2;
	s = sorted.begin ();
	n1 = *s;
	s++;
	n2 = *s;
	// check if the bond to the n1th atom is properly placed
	// FIXME

	// parity is evaluated using a determinant as explained in CML reference
	// assume the third atom is at z=1 and evaluate the determinant
	// | x0 x1 x2 x3 |
	// | y0 y1 y2 y3 |
	// | 0  0  0  1  |
	// | 1  1  1  1  |
	// this determinant is the same as (substracting the third column from the
	// two first):
	// | x0-x2 x1-x2 x2 x3 |
	// | y0-y2 y1-y2 y2 y3 |
	// | 0     0     0  1  |
	// | 0     0     1  1  |
	// the determinant is then equal to:
	// (y0-y2)*(x1-x2)-(y1-y2)*(x0-x2)
	double invert;
	if (n1 != 3) {
		x[n1] = x[3];
		y[n1] = y[3];
		invert = -1.;
	} else
		invert = 1.;
	double d = ((y[0] - y[2]) * (x[1] - x[2]) - (y[1] - y[2]) * (x[0] - x[2])) * invert;
	// for now setting the last bond as stereochemical, clearly bad
	// first ensure that this is the bond start
	if (bond[n1]->GetAtom (0) != this)
		bond[n1]->Revert ();
	// now, set the correct stereochemistry
	bond[n1]->SetType ((d > 0)? UpBondType: DownBondType);
	if (length[n2] == length[n1]) {
		double a1, a2;
		a1 = bond[n1]->GetAngle2D (this);
		a2 = bond[n2]->GetAngle2D (this);
		a1 -= a2;
		if (a1 > 360.)
			a1 -= 360;
		else if (a1 < 0)
			a1 += 360.;
		if (a1 > 180.)
			a1 = 360. - a1;
		if (a1 < 90) {
			if (bond[n2]->GetAtom (0) != this)
				bond[n2]->Revert ();
			bond[n2]->SetType ((d > 0)? DownBondType: UpBondType);
		}
	}
	return true;
}

bool Atom::HasStereoBond () const
{
	std::map < gcu::Atom *, gcu::Bond * >::const_iterator i, end = m_Bonds.end ();
	for (i = m_Bonds.begin (); i != end; i++)
		switch (static_cast < Bond * > ((*i).second)->GetType ()) {
		case UpBondType:
		case DownBondType:
		case UndeterminedBondType:
			return true;
		default:
			break;
		}
	return false;
}

}	//	namespace gcp
