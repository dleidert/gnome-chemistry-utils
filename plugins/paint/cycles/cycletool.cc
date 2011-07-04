// -*- C++ -*-

/*
 * GChemPaint cycles plugin
 * cycletool.cc
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
#include "cycletool.h"
#include <gcp/application.h>
#include <gcp/atom.h>
#include <gcp/bond.h>
#include <gcp/document.h>
#include <gcp/molecule.h>
#include <gcp/settings.h>
#include <gcp/theme.h>
#include <gcp/view.h>
#include <gcu/chain.h>
#include <gcu/cycle.h>
#include <gccv/canvas.h>
#include <gccv/group.h>
#include <gccv/line.h>
#include <gcugtk/ui-builder.h>
#include <glib/gi18n-lib.h>
#include <cmath>
#include <list>
#include <vector>

using namespace gcu;
using namespace std;

static char const *ToolNames [] = {
	"Cycle3",
	"Cycle4",
	"Cycle5",
	"Cycle6",
	"Cycle7",
	"Cycle8",
	"CycleN",
};

gcpCycleTool::gcpCycleTool (gcp::Application *App, unsigned char size): gcp::Tool (App, ToolNames[size - 3])
{
	m_Points = NULL;
	if ((m_size = size))
		Init();
	m_Chain = NULL;
}

gcpCycleTool::~gcpCycleTool ()
{
	if (m_Points)
		delete [] m_Points;
	if (m_Chain)
		delete m_Chain;
}

bool gcpCycleTool::OnClicked ()
{
	if (!m_size)
		return false;
	double x1, y1, x2, y2;
	double a0, a1, a2, b1, b2, m1, m2;
	gcp::Atom* pAtom, *pAtom1;
	gcp::Bond* pBond, *pBond1;
	gcp::Document *pDoc = m_pView->GetDoc ();
	m_dLength = pDoc->GetBondLength () * m_dZoomFactor;
	map<Atom*, Bond*>::iterator j;
	int i;
	bool bDone = false;
	m_dDefAngle = 0.0;
	if (m_pObject) {
		bDone = true;
		switch (m_pObject->GetType ()) {
		case BondType:
			pBond = (gcp::Bond*) m_pObject;
			pAtom = (gcp::Atom*) pBond->GetAtom (0);
			pAtom->GetCoords (&x1, &y1);
			x1 *= m_dZoomFactor;
			y1 *= m_dZoomFactor;
			pAtom1 = (gcp::Atom*) pBond->GetAtom (1);
			pAtom1->GetCoords (&x2, &y2);
			x2 *= m_dZoomFactor;
			y2 *= m_dZoomFactor;
			m_dLength = sqrt (square (x2 - x1) + square (y2 - y1));
			if (pBond->IsCyclic () == 1) {
				Cycle *pCycle = NULL;
				list<Cycle*>::iterator i;
				pCycle = pBond->GetFirstCycle (i, pCycle);
				a0 = atan2 (y1 - y2, x2 - x1);
				pCycle->GetAngles2D (pBond, &a1, &a2);
				if (sin (a0 - a1) * sin (a0 - a2) > 0) {
					if (sin (a0 - a1) < 0.0) {
						m_Points->x = m_x0 = x1;
						m_Points->y = m_y0 = y1;
						m_dAngle = a0;
						m_Start = pAtom;
						m_End = pAtom1;
						m_Direct = true;
					} else {
						m_Points->x = m_x0 = x1 = x2;
						m_Points->y = m_y0 = y1 = y2;
						m_dAngle = a0 - M_PI;
						m_Start = pAtom1;
						m_End = pAtom;
						m_Direct = false;
					}
				}
			} else {
				m_Start = pAtom;
				m_End = pAtom1;
				a0 = pBond->GetAngle2DRad (pAtom);
				b1 = a0 + m_dDev;
				b2 = a0 - m_dDev;
				m1 = m2 = M_PI;
				pBond1 = (gcp::Bond*) pAtom->GetFirstBond (j);
				while (pBond1) {
					if (pBond == pBond1) {
						pBond1 = (gcp::Bond*) pAtom->GetNextBond (j);
						continue;
					}
					a1 = pBond1->GetAngle2DRad (pAtom);
					a2 = fabs (b2 - a1);
					if (a2 > M_PI)
						a2 = 2 * M_PI - a2;
					if (m2 > a2)
						m2 = a2;
					a1 = fabs (a1 - b1);
					if (a1 > M_PI)
						a1 = 2 * M_PI - a1;
					if (m1 > a1)
						m1 = a1;
					pBond1 = (gcp::Bond*) pAtom->GetNextBond (j);
				}
				pBond1 = (gcp::Bond*) pAtom1->GetFirstBond (j);
				a2 = b2;
				b2 = - b1;
				b1 = - b2;
				while (pBond1) {
					if (pBond == pBond1) {
						pBond1 = (gcp::Bond*) pAtom1->GetNextBond (j);
						continue;
					}
					a1 = pBond1->GetAngle2DRad (pAtom1);
					a2 = fabs (b2 - a1);
					if (a2 > M_PI)
						a2 = 2 * M_PI - a2;
					if (m2 > a2)
						m2 = a2;
					a1 = fabs (a1 - b1);
					if (a1 > M_PI)
						a1 = 2 * M_PI - a1;
					if (m1 > a1)
						m1 = a1;
					pBond1 = (gcp::Bond*) pAtom1->GetNextBond (j);
				}
				if (m2 > m1) {
					m_Points->x = m_x0 = x1;
					m_Points->y = m_y0 = y1;
					m_dAngle = a0;
					m_Direct = true;
				} else {
					m_Points->x = m_x0 = x1 = x2;
					m_Points->y = m_y0 = y1 = y2;
					m_dAngle = a0 - M_PI;
					m_Direct = false;
				}
			}
			if (m_Chain)
				delete m_Chain;
			if (m_nState & GDK_SHIFT_MASK)
				m_Chain = new Chain (pBond, m_Start);
			break;
		case AtomType:
			pAtom = (gcp::Atom*) m_pObject;
			pAtom->GetCoords (&x1, &y1);
			x1 *= m_dZoomFactor;
			y1 *= m_dZoomFactor;
			i = pAtom->GetBondsNumber ();
			switch (i) {
				case 0:
					m_x0 = x1;
					m_y0 = y1;
					 bDone = false;
					break;
				case 1:
					pBond = (gcp::Bond*) pAtom->GetFirstBond (j);
					pAtom1 = (gcp::Atom*) pBond->GetAtom (pAtom);
					a0 = pBond->GetAngle2DRad (pAtom);
					m_Points->x = m_x0 = x1;
					m_Points->y = m_y0 = y1;
					m_dDefAngle = m_dAngle = - M_PI / 2 + a0 - m_dDev / 2;
					break;
				case 2:
					pBond = (gcp::Bond*) pAtom->GetFirstBond (j);
					pBond1 = (gcp::Bond*) pAtom->GetNextBond (j);
					a1 = pBond->GetAngle2DRad (pAtom);
					a2 = pBond1->GetAngle2DRad (pAtom);
					a0 = (a1 + a2) / 2;
					if (fabs (a1 - a2) > M_PI)
						a0 += M_PI;
					m_Points->x = m_x0 = x1;
					m_Points->y = m_y0 = y1;
					m_dDefAngle = m_dAngle = - M_PI / 2 + a0 - m_dDev / 2;
					break;
				default:
					vector<double> orientations;
					orientations.reserve (i);
					pBond = (gcp::Bond*) pAtom->GetFirstBond (j);
					orientations.push_back (pBond->GetAngle2DRad (pAtom));
					unsigned k;
					while ((pBond = (gcp::Bond*) pAtom->GetNextBond (j))) {
						k = 0;
						a0 = pBond->GetAngle2DRad (pAtom);
						while ((a0 > orientations[k]) && (k < orientations.size ()))
							k++;
						orientations.insert (orientations.begin () + k, a0);
					}
					a0 = 2 * M_PI - orientations[i - 1] + orientations[0];
					i = 0;
					for (k = 1; k < orientations.size (); k++) {
						a1 =  orientations[k] - orientations[k - 1];
						if (a0 < a1) {
							a0 = a1;
							i = k;
						}
					}
					if (a0 < M_PI - m_dDev + M_PI / 18) {
						bDone = false;
						break;
					}
					m_dAngle = (orientations[(i)? i - 1: orientations.size () - 1] + orientations[i] + M_PI - m_dDev) / 2;
					if (!i)
						m_dAngle += M_PI;
					m_dDefAngle = m_dAngle;
					m_Points->x = m_x0 = x1;
					m_Points->y = m_y0 = y1;
					orientations.clear ();
			}
			break;
		default:
			m_pObject = NULL;
			return false;
		}
	}
	if (!bDone) {
		m_dAngle = M_PI / 2;
		m_Points->x = x1 = m_x0;
		m_Points->y = y1 = m_y0;
	}
	for (i = 1; i < m_size; i++) {
		m_Points[i].x = x1 += m_dLength * cos (m_dAngle - m_dDev * (i - 1));
		m_Points[i].y = y1 -= m_dLength * sin (m_dAngle - m_dDev * (i - 1));
	}

	m_bAllowed = CheckIfAllowed ();
	Draw ();
	return true;
}

void gcpCycleTool::OnDrag ()
{
	if (!m_size)
		return;
	int i;
	double x1, y1, x2, y2;
	bool bDone = false;
	gccv::Item *item = m_pView->GetCanvas ()->GetItemAt (m_x, m_y);
	gcp::Document *pDoc = m_pView->GetDoc ();
	gcp::Theme *pTheme = pDoc->GetTheme ();
	Object* pObject = (item)? dynamic_cast <Object *> (item->GetClient ()): NULL;
	if (m_pObject) {
		if (m_pObject->GetType() == BondType) {
			if (((gcp::Bond*) m_pObject)->GetDist (m_x / m_dZoomFactor, m_y / m_dZoomFactor) < (pTheme->GetPadding () + pTheme->GetBondWidth () / 2) * m_dZoomFactor) {
				if (m_Item) {
					delete m_Item;
					m_Item = NULL;
				}
				return;
			}
			x1 = (m_x - m_Points->x) * (m_Points[1].y - m_Points->y) + (m_Points->y - m_y) * (m_Points[1].x - m_Points->x);
			if (m_nState & GDK_SHIFT_MASK) {
				if (m_Chain->GetLength () == (unsigned) m_size - 2)
					return;
				gcp::Bond* pBond;
				if (pObject) {
					if (pObject->GetType () == AtomType) {
						if (m_Chain->Contains ((gcp::Atom*) pObject))
							return;
						pBond = (gcp::Bond*) m_Start->GetBond ((gcp::Atom*) pObject);
						if (pBond) {
							m_Chain->AddBond ((gcp::Atom*)pObject, m_Start);
							m_Start = (gcp::Atom*) pObject;
						} else {
							pBond = (gcp::Bond*) m_End->GetBond ((gcp::Atom*) pObject);
							if (pBond) {
								m_Chain->AddBond (m_End, (gcp::Atom*) pObject);
								m_End = (gcp::Atom*) pObject;
							}
						}
						if (pBond)
							bDone = true;
					} else if (pObject->GetType() == BondType) {
						m_pAtom = (gcp::Atom*) ((gcp::Bond*) pObject)->GetAtom (m_Start);
						if (m_pAtom) {
							if (m_Chain->Contains (m_pAtom))
								return;
							m_Chain->AddBond (m_pAtom, m_Start);
							m_Start = m_pAtom;
						} else {
							m_pAtom = (gcp::Atom*)((gcp::Bond*) pObject)->GetAtom (m_End);
							if (m_pAtom) {
								if (m_Chain->Contains (m_pAtom))
									return;
								m_Chain->AddBond (m_End, m_pAtom);
								m_End = m_pAtom;
							}
						}
						if (m_pAtom)
							bDone = true;
					}
					if (!bDone)
						return;
					m_Start->GetCoords (&x1, &y1);
					x1 *= m_dZoomFactor;
					y1 *= m_dZoomFactor;
					m_End->GetCoords (&x2, &y2);
					x2 *= m_dZoomFactor;
					y2 *= m_dZoomFactor;
					double d = sqrt (square (x2 - x1) + square (y2 - y1)), L = pDoc->GetBondLength () * m_dZoomFactor, t;
					unsigned n = m_size - m_Chain->GetLength ();
					if (d < L * n) {
						//FIXME: this algorithm is far from optimal!!!
						double t1, k = d/L, p = M_PI /n;
						t = 0;
						while (1) {
							 t1 = t + (p - t) * (1 - k / ((t != 0.0)? (sin (t * n) / sin (t)): n));
							if (fabs (t1 - t) < 1e-15)
								break;	//An infinite loop might be possible here!
							t = t1;
						}
					} else t = 0;
					if (t < fabs(m_dDev) / 4)
						//too large, change the bond length
						t = fabs (m_dDev) / 4;
					//Find center
					double dx, dy;
					if (m_dDev > 0) {
						dx = ((y1 - y2) / tan (M_PI - t * n) + x2 + x1) / 2;
						dy = ((x1 - x2) / tan (t * n) + y1 + y2) / 2;
						t =  - t;
					} else {
						dx = ((y2 - y1) / tan (M_PI - t * n) + x2 + x1) / 2;
						dy = ((x2 - x1) / tan (t * n) + y1 + y2) / 2;
					}
					double a0 = atan2 (dy - y2, x2 - dx);
					d = sqrt (square (dy - y2) + square (dx - x2));
					i = 0;
					m_pAtom = m_Start;
					do {
						m_pAtom->GetCoords (&x1, &y1);
						m_Points[i].x = x1 * m_dZoomFactor;
						m_Points[i++].y = y1 * m_dZoomFactor;
					} while ((m_pAtom != m_End) && (m_pAtom = reinterpret_cast <gcp::Atom *> (m_Chain->GetNextAtom (m_pAtom))));
					while (i < m_size) {
						a0 += 2 * t;
						m_Points[i].x = dx + d * cos (a0);
						m_Points[i++].y= dy - d * sin (a0);
					}
				}
				bDone = true;
			} else if (x1 * m_dDev > 0)  {
				m_dDev = - m_dDev;
				x1 = m_Points[1].x;
				y1 = m_Points[1].y;
				for (i = 2; i < m_size; i++) {
					m_Points[i].x = x1 += (pDoc->GetBondLength () * m_dZoomFactor) * cos (m_dAngle - m_dDev * (i - 1));
					m_Points[i].y = y1 -= (pDoc->GetBondLength () * m_dZoomFactor) * sin (m_dAngle - m_dDev * (i - 1));
				}
				bDone = true;
			}
			else if (m_Item)
				return;
			else bDone = true;
		}
	}
	if (m_Item) {
		delete m_Item;
		m_Item = NULL;
	}
	if (!bDone) {
		double dAngle;
		m_pAtom = NULL;
		if (pObject) {
			if (pObject->GetType () == BondType)
				m_pAtom = (gcp::Atom*) pObject->GetAtomAt (m_x / m_dZoomFactor, m_y / m_dZoomFactor);
			else if (pObject->GetType() == AtomType)
				m_pAtom = (gcp::Atom*) pObject;
		}
		if (m_pAtom) {
			if (m_pAtom == m_pObject)
				return;
			m_pAtom->GetCoords (&m_x, &m_y);
			m_x *= m_dZoomFactor;
			m_y *= m_dZoomFactor;
			m_x -= m_x0;
			m_y -= m_y0;
			double BondLength = sqrt (square (m_x) + square (m_y));
			if (m_x == 0) {
				if (m_y == 0)
					return;
				dAngle = (m_y < 0) ? 90 : 270;
			} else {
				m_dAngle = atan (-m_y / m_x);
				if (m_x < 0)
					m_dAngle += M_PI;
			}
			x1 = m_x0;
			y1 = m_y0;
			for (i = 1; i < m_size; i++) {
				m_Points[i].x = x1 += (BondLength) * cos (m_dAngle - m_dDev * (i - 1));
				m_Points[i].y = y1 -= (BondLength) * sin (m_dAngle - m_dDev * (i - 1));
			}
		} else {
			m_x -= m_x0;
			m_y -= m_y0;
			if (m_x == 0) {
				if (m_y == 0)
					return;
				dAngle = (m_y < 0) ? 90 : 270;
			} else {
				dAngle = atan (-m_y/m_x) * 180 / M_PI;
				if (!(m_nState & GDK_CONTROL_MASK))
					dAngle = rint (dAngle / 5) * 5;
				if (m_x < 0)
					dAngle += 180;
			}
			m_dAngle = dAngle * M_PI / 180;
			x1 = m_x0;
			y1 = m_y0;
			double d = (m_nState & GDK_SHIFT_MASK)? sqrt (square (m_x) + square (m_y)): pDoc->GetBondLength () * m_dZoomFactor;
			for (i = 1; i < m_size; i++) {
				m_Points[i].x = x1 += d * cos (m_dAngle - m_dDev * (i - 1));
				m_Points[i].y = y1 -= d * sin (m_dAngle - m_dDev * (i - 1));
			}
			char tmp[32];
			if (dAngle < 0)
				dAngle += 360;
			snprintf (tmp, sizeof (tmp) - 1, _("Orientation: %g"), dAngle);
			m_pApp->SetStatusText (tmp);
		}
	}
	m_bAllowed = CheckIfAllowed ();
	Draw ();
}

void gcpCycleTool::OnRelease ()
{
	if (!m_size)
		return;
	if (m_Chain) {
		delete m_Chain;
		m_Chain = NULL;
	}
	if (m_Item) {
		delete m_Item;
		m_Item = NULL;
	} else
		return;
	m_pApp->ClearStatus ();
	if (!m_bAllowed)
		return;
	gcp::Atom* pAtom[m_size];
	gcp::Bond* pBond;
	Object *pObject;
	char const *Id;
	gccv::Item *item;
	gccv::Canvas *canvas = m_pView->GetCanvas ();
	gcp::Document *pDoc = m_pView->GetDoc ();
	gcp::Operation *pOp = NULL;
	gcp::Molecule *pMol = NULL;
	std::string MolId;
	int i;
	// first save groups if any will be modified
	for (i = 0; i < m_size; i++) {
		m_x = m_Points[i].x;
		m_y = m_Points[i].y;
		pAtom[i] = NULL;
		item = canvas->GetItemAt (m_x, m_y);
		m_pObject = (item)? dynamic_cast <Object *> (item->GetClient ()): NULL;
		if (i && m_pObject == pAtom[i - 1]) // this might happen if the symbol is large enough
			m_pObject = NULL;
		if (gcp::MergeAtoms && m_pObject) {
			// At this point, we don't need to check if things are allowed since otherwise,
			// m_Item would have been NULL on entry and the method would have already returned.
			if (m_pObject->GetType () == BondType)
				pAtom[i] = (gcp::Atom*) m_pObject->GetAtomAt (m_x / m_dZoomFactor, m_y / m_dZoomFactor);
			else if (m_pObject->GetType () == AtomType)
				pAtom[i] = (gcp::Atom*) m_pObject;
			if (pAtom[i]) {
				if (pMol == NULL) {
					pMol = reinterpret_cast<gcp::Molecule *> (pAtom[i]->GetMolecule ());
					pMol->Lock (true);
					// we must store the id, the molecule might be destroyed
					MolId = pMol->GetId ();
				}
				// if group not in ModifiedObjects, add it and save it
				pObject = pAtom[i]->GetGroup ();
				Id = pObject->GetId ();
				if (ModifiedObjects.find (Id) == ModifiedObjects.end ()) {
					if (!pOp)
						pOp = pDoc->GetNewOperation (gcp::GCP_MODIFY_OPERATION);
					pOp->AddObject (pObject);
					ModifiedObjects.insert (Id);
				}
			}
		}
	}
	// now add missing atoms and bonds
	for (i = 0; i < m_size; i++) {
		if (!pAtom[i]) {
			pAtom[i] = new gcp::Atom (m_pApp->GetCurZ (), m_Points[i].x / m_dZoomFactor, m_Points[i].y / m_dZoomFactor, 0);
			pDoc->AddAtom (pAtom[i]);
		}
		if (i) {
			pBond = (gcp::Bond*) pAtom[i]->GetBond (pAtom[i - 1]);
			if (!pBond) {
				pBond = new gcp::Bond (pAtom[i - 1], pAtom[i], 1);
				pDoc->AddBond (pBond);
			}
		}
	}
	pBond = (gcp::Bond*) pAtom[m_size - 1]->GetBond (pAtom[0]);
	if (!pBond) {
		pBond = new gcp::Bond (pAtom[m_size - 1], pAtom[0], 1);
		pDoc->AddBond (pBond);
	}
	pObject = pBond->GetGroup ();
	if (pOp) {
		ModifiedObjects.insert (pObject->GetId ());
		set<string>::iterator it, end = ModifiedObjects.end ();
		for (it = ModifiedObjects.begin (); it != end; it++) {
			pObject = pDoc->GetDescendant ((*it).c_str ());
			if (pObject)
				pOp->AddObject (pObject, 1);
		}
	} else {
		pOp = pDoc->GetNewOperation (gcp::GCP_ADD_OPERATION);
		pOp->AddObject (pObject);
	}
	pDoc->FinishOperation ();
	if (pMol)
		pMol = static_cast <gcp::Molecule*> (pDoc->GetDescendant (MolId.c_str ()));
	if (pMol) {
		pMol->Lock (false);
		pMol->EmitSignal (gcp::OnChangedSignal);
	}
	ModifiedObjects.clear ();
}

void gcpCycleTool::OnChangeState ()
{
	if (m_pObject && (m_pObject->GetType() == BondType)) {
		if (m_nState & GDK_SHIFT_MASK) {
			if (!m_Chain) {
				if (m_Direct) {
					m_Start = (gcp::Atom*) ((gcp::Bond*) m_pObject)->GetAtom (0);
					m_End = (gcp::Atom*) ((gcp::Bond*) m_pObject)->GetAtom (1);
				} else {
					m_Start = (gcp::Atom*) ((gcp::Bond*) m_pObject)->GetAtom (1);
					m_End = (gcp::Atom*) ((gcp::Bond*) m_pObject)->GetAtom (0);
				}
				m_Chain = new Chain ((gcp::Bond*) m_pObject, m_Start);
			}
		} else {
			double x1, y1, x2, y2;
			if (m_Item) {
				delete m_Item;
				m_Item = NULL;
			}
			if (m_Direct) {
				m_Start = (gcp::Atom*) ((gcp::Bond*) m_pObject)->GetAtom (0);
				m_End = (gcp::Atom*) ((gcp::Bond*) m_pObject)->GetAtom (1);
			} else {
				m_Start = (gcp::Atom*) ((gcp::Bond*) m_pObject)->GetAtom (1);
				m_End = (gcp::Atom*) ((gcp::Bond*) m_pObject)->GetAtom (0);
			}
			m_Start->GetCoords (&x1, &y1);
			m_End->GetCoords (&x2, &y2);
			m_Points->x = x1 * m_dZoomFactor;
			m_Points->y = y1 * m_dZoomFactor;
			m_Points[1].x = x1 = x2 * m_dZoomFactor;
			m_Points[1].y = y1 = y2 * m_dZoomFactor;
			for (int i = 2; i < m_size; i++) {
				gcp::Document *pDoc = m_pView->GetDoc ();
				m_Points[i].x = x1 += (pDoc->GetBondLength () * m_dZoomFactor) * cos (m_dAngle - m_dDev * (i - 1));
				m_Points[i].y = y1 -= (pDoc->GetBondLength () * m_dZoomFactor) * sin (m_dAngle - m_dDev * (i - 1));
			}
			 if (m_Chain) {
				 delete m_Chain;
				m_Chain = NULL;
			 }
		}
	}
	m_bAllowed = CheckIfAllowed ();
	gcp::Tool::OnChangeState ();
}

void gcpCycleTool::Init ()
{
	m_dDev = 2 * M_PI / m_size;
	m_Points = new gccv::Point[m_size];
}

bool gcpCycleTool::CheckIfAllowed ()
{
//Search atoms at the positions of the vertices and check if adding bonds to them is allowed
	gcp::Atom* pAtom[m_size];
	gccv::Item *item;
	gccv::Canvas *canvas = m_pView->GetCanvas ();
	Object* pObject, *group, *other;
	gcp::Document *pDoc = m_pView->GetDoc ();
	group = (m_pObject)? m_pObject->GetMolecule ()->GetParent (): NULL;
	if (group == pDoc)
		group = NULL;
	int i, n;
	for (i = 0; i < m_size; i++) {
		m_x = m_Points[i].x;
		m_y = m_Points[i].y;
		item = canvas->GetItemAt (m_x, m_y);
		pObject = (item)? dynamic_cast <Object *> (item->GetClient ()): NULL;
		if (gcp::MergeAtoms && pObject) {
			TypeId Id = pObject->GetType ();
			switch (Id) {
				case BondType:
				case FragmentType:
					pAtom[i] = (gcp::Atom*) pObject->GetAtomAt (m_x / m_dZoomFactor, m_y / m_dZoomFactor);
					break;
				case AtomType:
					pAtom[i] = (gcp::Atom*) pObject;
					break;
				default:
					pAtom[i] = NULL;
			}
		} else
			pAtom[i] = NULL;
	}
	for (i = 0; i < m_size; i++) {
		if (!pAtom[i])
			continue;
		other = pAtom[i]->GetMolecule ()->GetParent ();
		if (other != pDoc) {
			if (group == NULL)
				group = other;
			else if (other != group)
				return false;
		}
		n = 0;
		if (!pAtom[i]->GetBond (pAtom[(i)? i - 1: m_size -1]))
			n++;
		if (!pAtom[i]->GetBond (pAtom[(i < m_size - 1)? i + 1: 0]))
			n++;
		if (n && (!pAtom[i]->AcceptNewBonds (n)))
			return false;
		pAtom[i]->GetCoords (&m_Points[i].x, &m_Points[i].y);
		m_Points[i].x *= m_dZoomFactor;
		m_Points[i].y *= m_dZoomFactor;
	}
	return true;
}

static void on_length_changed (GtkSpinButton *btn, gcpCycleTool *tool)
{
	tool->SetLength (gtk_spin_button_get_value (btn));
}

static void on_merge_toggled (GtkToggleButton *btn)
{
	gcp::MergeAtoms = gtk_toggle_button_get_active (btn);
}

void gcpCycleTool::SetLength (double length)
{
	m_pApp->GetActiveDocument ()->SetBondLength (length);
}

GtkWidget *gcpCycleTool::GetPropertyPage ()
{
	gcugtk::UIBuilder *builder = new gcugtk::UIBuilder (UIDIR"/cycle.ui", GETTEXT_PACKAGE);
	m_LengthBtn = GTK_SPIN_BUTTON (builder->GetWidget ("bond-length"));
	g_signal_connect (m_LengthBtn, "value-changed", G_CALLBACK (on_length_changed), this);
	m_MergeBtn = GTK_TOGGLE_BUTTON (builder->GetWidget ("merge"));
	g_signal_connect (m_MergeBtn, "toggled", G_CALLBACK (on_merge_toggled), NULL);
	GtkWidget *res = builder->GetRefdWidget ("cycle");
	delete builder;
	return res;
}

void gcpCycleTool::Activate ()
{
	gcp::Document *pDoc = m_pApp->GetActiveDocument ();
	gtk_spin_button_set_value (m_LengthBtn, pDoc->GetBondLength ());
	gtk_toggle_button_set_active (m_MergeBtn, gcp::MergeAtoms);
}

void gcpCycleTool::Draw ()
{
	gcp::Theme *pTheme = m_pView->GetDoc ()->GetTheme ();
	m_Item = new gccv::Group (m_pView->GetCanvas ());
	gccv::Item *item;
	unsigned i;
	for (i = 1; i < m_size; i++) {
		item = new gccv::Line (static_cast <gccv::Group *> (m_Item), m_Points[i-1].x, m_Points[i-1].y, m_Points[i].x, m_Points[i].y);
		static_cast <gccv::LineItem *> (item)->SetLineColor (m_bAllowed? gcp::AddColor: gcp::DeleteColor);
		static_cast <gccv::LineItem *> (item)->SetLineWidth (pTheme->GetBondWidth ());
	}
	item = new gccv::Line (static_cast <gccv::Group *> (m_Item), m_Points[m_size-1].x, m_Points[m_size-1].y, m_Points[0].x, m_Points[0].y);
	static_cast <gccv::LineItem *> (item)->SetLineColor (m_bAllowed? gcp::AddColor: gcp::DeleteColor);
	static_cast <gccv::LineItem *> (item)->SetLineWidth (pTheme->GetBondWidth ());
}

gcpNCycleTool::gcpNCycleTool (gcp::Application* App, unsigned char size): gcpCycleTool (App, 9)
{
	SetSize(size);
}

gcpNCycleTool::~gcpNCycleTool ()
{
}

void gcpNCycleTool::SetSize (unsigned char size)
{
	if (m_Points) {
		delete [] m_Points;
	}
	if ((m_size = size))
		Init();
}

static void on_size_changed (GtkSpinButton *button, gcpNCycleTool *tool)
{
	tool->SetSize ((unsigned char) gtk_spin_button_get_value_as_int (button));
}

GtkWidget *gcpNCycleTool::GetPropertyPage ()
{
	gcugtk::UIBuilder *builder = new gcugtk::UIBuilder (UIDIR"/cyclen.ui", GETTEXT_PACKAGE);
	m_LengthBtn = GTK_SPIN_BUTTON (builder->GetWidget ("bond-length"));
	g_signal_connect (m_LengthBtn, "value-changed", G_CALLBACK (on_length_changed), this);
	m_MergeBtn = GTK_TOGGLE_BUTTON (builder->GetWidget ("merge"));
	g_signal_connect (m_MergeBtn, "toggled", G_CALLBACK (on_merge_toggled), NULL);
	m_SizeBtn = GTK_SPIN_BUTTON (builder->GetWidget ("sizebtn"));
	gtk_spin_button_set_value (m_SizeBtn, m_size);
	g_signal_connect (m_SizeBtn, "value-changed", G_CALLBACK (on_size_changed), this);
	GtkWidget *res = builder->GetRefdWidget ("cycle");
	delete builder;
	return res;
}
