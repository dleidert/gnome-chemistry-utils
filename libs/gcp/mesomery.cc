// -*- C++ -*-

/* 
 * GChemPaint library
 * mesomery.cc 
 *
 * Copyright (C) 2002-2008 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "document.h"
#include "mesomer.h"
#include "mesomery.h"
#include "mesomery-arrow.h"
#include "molecule.h"
#include "theme.h"
#include "view.h"
#include "widgetdata.h"
#include <glib/gi18n-lib.h>
#include <set>
#include <cmath>
#include <cstring>

using namespace gcu;
using namespace std;

namespace gcp {

Mesomery::Mesomery (): Object (MesomeryType)
{
	SetId ("msy1");
}

Mesomery::~Mesomery ()
{
	if (IsLocked ())
		return;
	map<string, Object*>::iterator i;
	Object *pObj;
	MesomeryArrow *arrow;
	Document *pDoc = reinterpret_cast<Document *> (GetDocument ());
	Operation *pOp = pDoc->GetCurrentOperation ();
	while ((pObj = GetFirstChild (i))) {
		if (pObj->GetType () == MesomeryArrowType) {
			arrow = reinterpret_cast<MesomeryArrow*> (pObj);
			arrow->SetStartMesomer (NULL);
			arrow->SetEndMesomer (NULL);
			arrow->SetParent (GetParent ());
			if (pOp)
				pOp->AddObject (arrow, 1);
			
		} else
			delete pObj;
	}
}

bool Mesomery::Load (xmlNodePtr node)
{
	xmlChar* tmp;
	xmlNodePtr child;
	Object* pObject;
	list<xmlNodePtr> arrows;

	Lock ();
	tmp = xmlGetProp (node, (xmlChar*) "id");
	if (tmp) {
		SetId ((char*) tmp);
		xmlFree (tmp);
	}
	child = node->children;
	while (child) {
		if (!strcmp ((const char*) child->name, "mesomery-arrow"))
			arrows.push_front (child);
		else {
			pObject = CreateObject ((const char*) child->name, this);
			if (pObject) {
				if (!pObject->Load (child))
					delete pObject;
			} else {
				Lock (false);
				return false;
			}
		}
		child = child->next;
	}
	while (!arrows.empty ()) {
		child = arrows.back ();
		pObject = CreateObject ("mesomery-arrow", this);
		if (pObject) {
			if (!pObject->Load (child))
				delete pObject;
		} else {
			Lock (false);
			return false;
		}
		arrows.pop_back ();
	}
	Lock (false);
	return true;
}

typedef struct
{
	double x, y;
	gccv::Rect r;
	Mesomer *mes;
} ObjectData;

bool Mesomery::Build (list<Object*>& Children) throw (invalid_argument)
{
	Document *pDoc = dynamic_cast<Document *> (GetDocument ());
	Theme *pTheme = pDoc->GetTheme ();
	WidgetData  *pData= reinterpret_cast<WidgetData *> (g_object_get_data (G_OBJECT (pDoc->GetWidget ()), "data"));
	list<Object *>::iterator i, iend = Children.end ();
	map<Object *, ObjectData> Objects;
	list<Object *> Arrows;
	double minright, minleft, x, y, x0, y0, x1, y1, l, d, ps;
	Object *Left, *Right;
	ObjectData od;
	MesomeryArrow *arrow;
	unsigned narrows = 0, nmol = 0;
	for (i = Children.begin (); i != iend; i++) {
		pData->GetObjectBounds (*i, &od.r);
		od.x = (od.r.x0 + od.r.x1) / 2.;
		od.y = (*i)->GetYAlign () * pTheme->GetZoomFactor ();
		switch ((*i)->GetType ()) {
		case MoleculeType:
			od.mes = new Mesomer (this, dynamic_cast<Molecule *>(*i));
			Objects[*i] = od;
			nmol++;
			break;
		case MesomeryArrowType:
			narrows++;
			Arrows.push_back (*i);
			AddChild (*i);
			break;
		default:
			throw  invalid_argument (_("Something wrong happened, please file a bug report."));
		}
	}
	// now, for each arrow, search closiest object on both sides and verify it's a molecule
	list<Object *>::iterator j, jend = Arrows.end ();
	map<Object *, ObjectData>::iterator k, kend = Objects.end ();
	for (j = Arrows.begin (); j != jend; j++) {
		arrow = reinterpret_cast<MesomeryArrow *>(*j);
		arrow->GetCoords (&x0, &y0, &x1, &y1);
		//x0 and y0 should be the center of the arrow, not the beginning, so we must transform them
		x0 = (x0 + x1) / 2;
		y0 = (y0 + y1) / 2;
		// x1, y1 will now be the coordinates of a normalized vector:
		x1 -= x0;
		y1 -= y0;
		x0 *= pTheme->GetZoomFactor ();
		y0 *= pTheme->GetZoomFactor ();
		l = sqrt (x1 * x1 + y1 * y1);
		x1 /= l;
		y1 /= l;
		l *= pTheme->GetZoomFactor (); // half length of the arrow on the screen
		// No molecule should be nearer than that
		minright = minleft = DBL_MAX;
		Left = Right = NULL;
		for (k = Objects.begin (); k != kend; k++) {
			od = (*k).second;
			x = od.x - x0;
			y = od.y - y0;
			d = sqrt (x * x + y * y);
			ps = (x * x1 + y * y1) / d;
			if (ps >= -.71 && ps <= .71)
				continue;
			if (d < l) {
				Left = (*k).first;
				Right = *j;
				pData->UnselectAll ();
				pData->SetSelected (Left);
				pData->SetSelected (Right);
				throw invalid_argument (_("No space left between molecule and arrow!"));
			}
			if (ps < 0) {
				if (d < minleft) {
					Left = od.mes;
					minleft = d;
				}
			} else {
				if (d < minright) {
					Right = od.mes;
					minright = d;
				}
			}
		}
		if (!Left || !Right) { // Do not accept arrows with only one mesomer (?)
			Left = *j;
			pData->UnselectAll ();
			pData->SetSelected (Left);
			throw invalid_argument (_("Isolated arrows are not allowed!"));
		}
		reinterpret_cast<MesomeryArrow *> (*j)->SetStartMesomer (reinterpret_cast<Mesomer *> (Left));
		reinterpret_cast<MesomeryArrow *> (*j)->SetEndMesomer (reinterpret_cast<Mesomer *> (Right));
		reinterpret_cast<Mesomer *> (Left)->AddArrow (reinterpret_cast<MesomeryArrow *> (*j), reinterpret_cast<Mesomer *> (Right));
		reinterpret_cast<Mesomer *> (Right)->AddArrow (reinterpret_cast<MesomeryArrow *> (*j), reinterpret_cast<Mesomer *> (Left));
	}
	// now, check if each mesomer has at least one arrow, may be we should add missing arrows?
	for (k = Objects.begin (); k != kend; k++) {
		od = (*k).second;
		if (!od.mes->Validate ()) {
			Left = (*k).first;
			pData->UnselectAll ();
			pData->SetSelected (Left);
			throw invalid_argument (_("Isolated molecule!\n Please add missing arrows."));
		}
	}
	// Check if all mesomers are related (only connectivity is checked for now)
	if (!Validate (false))
		throw invalid_argument (_("Please add missing arrows."));
	// Align the children
	Align ();
	return true;
}

static void BuildConnectivity ( set<Object *> &Objects, Mesomer* pMesomer)
{
	map<Mesomer *, MesomeryArrow *> *Arrows = pMesomer->GetArrows ();
	map<Mesomer *, MesomeryArrow *>::iterator i, end = Arrows->end ();
	for (i = Arrows->begin (); i != end; i++) {
		Objects.insert ((*i).second);
		if (Objects.find ((*i).first) == Objects.end ()) {
			Objects.insert ((*i).first);
			BuildConnectivity (Objects, (*i).first);
		}
	}
}

bool Mesomery::Validate (bool split)
{
	map<string, Object*>::iterator i;
	Object *pObj = GetFirstChild (i);
	while (pObj && pObj->GetType () != MesomerType)
		pObj = GetNextChild (i);
	if (pObj == NULL)
		return false;
	set<Object *> Objects;
	Objects.insert (pObj);
	BuildConnectivity (Objects, reinterpret_cast<Mesomer *> (pObj));
	while (Objects.size () < GetChildrenNumber ()) {
		if (!split)
			return 2;
		pObj = GetFirstChild (i);
		while (pObj && (pObj->GetType () != MesomerType))
			pObj = GetNextChild (i);
		if (reinterpret_cast<Mesomer *> (pObj)->Validate ()) {
			Mesomery *ms = new Mesomery (GetParent (), 
								reinterpret_cast<Mesomer *> (pObj));
			Document *pDoc = reinterpret_cast<Document*> (GetDocument ());
			Operation *pOp = pDoc->GetCurrentOperation ();
			pOp->AddObject (ms, 1);
		} else
			delete pObj;
	}
	return true;
}

typedef struct MC
{
	list<Mesomer*> mesomers;
	struct MC *prev;
	list<struct MC*> children;
} MesomeryChain;

typedef struct
{
	double x, y, dx, dy;
	gccv::Rect r;
	Mesomer *m;
	MesomeryChain *chain;
} MesomerData;

static void TerminateChain (MesomeryChain *chain, set<Object*> &core,
		set<MesomeryChain*> &terminated_chains, map<Object*, MesomerData> &positions)
{
	if (terminated_chains.find (chain) != terminated_chains.end ())
		return;	// job has already been done
	list<Mesomer*>::iterator m, mend;
	MesomerData md;
	if (chain->prev != NULL)
		TerminateChain (chain->prev, core, terminated_chains, positions);
	mend = chain->mesomers.end ();
	for (m = chain->mesomers.begin (), m++; m != mend; m++) {
		md = positions[*m];
		md.chain = NULL;
		positions[*m] = md;
		core.insert (*m);
	}
	terminated_chains.insert (chain);
	list<struct MC*>::iterator i, iend = chain->children.end ();
	for (i = chain->children.begin (); i != iend; i++)
		(*i)->prev = NULL;
}

static void DoAlign (MesomeryArrow *arrow, MesomerData &start, MesomerData &end, double ArrowPadding, double ZoomFactor)
{
	double x0, y0, x1, y1, x, y, l, dx, dy;
	bool horiz;
	arrow->GetCoords (&x0, &y0, &x1, &y1);
	x = x1 - x0;
	y = y1 - y0;
	l = sqrt (x * x + y * y);
	x /= l;
	y /= l;
	if ((fabs (x) > 1e-5) && (fabs (y) > 1e-5))
		horiz = (fabs (x) > fabs (y));
	else if (fabs (x) > 1e-5)
		horiz = true;
	else
		horiz = false;
	if (horiz) {
		if (x > 0)
			dx = start.r.x1 - start.x + ArrowPadding;
		else
			dx = start.r.x0 - start.x - ArrowPadding;
		dy = dx * y / x;
	} else {
		if (y > 0)
			dy = start.r.y1 - start.y + ArrowPadding;
		else
			dy = start.r.y0 - start.y - ArrowPadding;
		dx = dy * x / y;
	}
	x1 += x0 = (dx + start.x) / ZoomFactor - x0;
	y1 += y0 = (dy + start.y) / ZoomFactor - y0;
	arrow->Move (x0, y0);
	if (horiz) {
		if (x > 0)
			dx = end.x - end.r.x0 + ArrowPadding;
		else
			dx = end.x - end.r.x1 - ArrowPadding;
		dy = dx * y / x;
	} else {
		if (y > 0)
			dy = end.y - end.r.y0 + ArrowPadding;
		else
			dy = end.y - end.r.y1 - ArrowPadding;
		dx = dy * x / y;
	}
	dx = x1 * ZoomFactor - (end.x - dx);
	dy = y1 * ZoomFactor - (end.y - dy);
	end.r.x0 += dx;
	end.r.x1 += dx;
	end.x += dx;
	end.dx += dx;
	end.r.y0 += dy;
	end.r.y1 += dy;
	end.y += dy;
	end.dy += dy;
}

static double GetProjectionRatio (MesomerData &md, MesomerData &md0, MesomerData &md1, MesomeryArrow *arrow)
{
	double x0, y0, x1, y1, d;
	arrow->GetCoords (&x0, &y0, &x1, &y1);
	x1 -= x0;
	y1 -= y0;
	x0 = md1.x - md0.x;
	y0 = md1.y - md0.y;
	d = (x0 * y1 - x1 * y0);
	if (d == 0.)
		return -1.; // everything is aligned, and this should not occur, but who knows?
	return ((md.x - md0.x) * y1 - (md.y - md0.y) * x1)/ d;
}

static void AlignArrow (MesomeryArrow *arrow, MesomerData &md0, MesomerData &md1, double ArrowPadding, double ZoomFactor)
{
	double dx, dy, x, y, x0, y0, x1, y1, l;
	bool horiz;
	if (arrow->GetStartMesomer () != md0.m)
		arrow->Reverse ();
	x = md1.x - md0.x;
	y = md1.y - md0.y;
	l = sqrt (x * x + y * y);
	x /= l;
	y /= l;
	if ((fabs (x) > 1e-5) && (fabs (y) > 1e-5))
		horiz = (fabs (x) > fabs (y));
	else if (fabs (x) > 1e-5)
		horiz = true;
	else
		horiz = false;
	if (horiz) {
		if (x > 0)
			dx = md0.r.x1 - md0.x + ArrowPadding;
		else
			dx = md0.r.x0 - md0.x - ArrowPadding;
		dy = dx * y / x;
	} else {
		if (y > 0)
			dy = md0.r.y1 - md0.y + ArrowPadding;
		else
			dy = md0.r.y0 - md0.y - ArrowPadding;
		dx = dy * x / y;
	}
	x0 = (dx + md0.x) / ZoomFactor;
	y0 = (dy + md0.y) / ZoomFactor;
	if (horiz) {
		if (x > 0)
			dx = md1.x - md1.r.x0 + ArrowPadding;
		else
			dx = md1.x - md1.r.x1 - ArrowPadding;
		dy = dx * y / x;
	} else {
		if (y > 0)
			dy = md1.y - md1.r.y0 + ArrowPadding;
		else
			dy = md1.y - md1.r.y1 - ArrowPadding;
		dx = dy * x / y;
	}
	x1 = (md1.x - dx) / ZoomFactor;
	y1 = (md1.y - dy) / ZoomFactor;
	arrow->SetCoords (x0, y0, x1, y1);
}

static void ChainMove (MesomeryChain *chain, double dx, double dy, map<Object*, MesomerData> &positions)
{
	list<Mesomer*>::iterator i, iend;
	list<struct MC*>::iterator c, cend;
	MesomerData md;
	iend = chain->mesomers.end ();
	for (i = chain->mesomers.begin (), i++; i!= iend; i++) {
		md = positions [*i];
		md.r.x0 += dx;
		md.r.x1 += dx;
		md.x += dx;
		md.dx += dx;
		md.r.y0 += dy;
		md.r.y1 += dy;
		md.y += dy;
		md.dy += dy;
		positions[*i] = md;
	}
	cend = chain->children.end ();
	for (c = chain->children.begin (); c != cend; c++)
		ChainMove (*c, dx, dy, positions);
}

static void ChainAdjust (MesomerData &md0, MesomerData &md1, double ratio, map<Object*, MesomerData> &positions)
{
	MesomeryChain *chain = md0.chain, *parent;
	MesomerData md;
	double x = 0., y = 0.;
	ratio -= 1.;
	list<Mesomer*>::reverse_iterator m, mend;
	list<struct MC*>::iterator c, cend;
	while (chain) {
		for (m = chain->mesomers.rbegin (); m != mend; m++) {
			if (*m == md1.m)
				return;
			md = positions[*m];
			x = (md.x - md1.x) * ratio;
			y = (md.y - md1.y) * ratio;
			md.r.x0 += x;
			md.r.x1 += x;
			md.x += x;
			md.dx += x;
			md.r.y0 += y;
			md.r.y1 += y;
			md.y += y;
			md.dy += y;
			positions[md.m] = md;
		}
		parent = chain->prev;
		if (parent == NULL)
			return; // This should not occur, may be we should fire an exception there.
		cend = parent->children.end ();
		for (c = parent->children.begin (); c != cend; c++) {
			if (*c == chain)
				continue;
			ChainMove (*c, x, y, positions);
		}
		parent = chain;
	}
}

// The following value is quite arbitrary, it is there to avoid getting very large images
#define CUTOFF 4.0

void Mesomery::Align ()
{
	Document *pDoc = dynamic_cast <Document *> (GetDocument ());
	View *pView = pDoc->GetView ();
	Theme *pTheme = pDoc->GetTheme ();
	WidgetData  *pData = reinterpret_cast <WidgetData *> (g_object_get_data (G_OBJECT (pDoc->GetWidget ()), "data"));
	/* Update the canvas if necessary */
/*	GnomeCanvas* w = GNOME_CANVAS (((Document*) GetDocument ())->GetWidget ());
	while (w->idle_id)
		gtk_main_iteration();
	gnome_canvas_update_now (w);*/
	map<string, Object*>::iterator i;
	Object *pObj = GetFirstChild (i);
	Mesomer *pStart = NULL, *mes0, *mes;
	map<Object*, MesomerData> positions;
	MesomerData md, md0, md1, md2;
	double d;
	md.dx = md.dy = 0.;
	MesomeryChain *ch;
	while (pObj) {
		if (pObj->GetType () == MesomerType) {
			pData->GetObjectBounds (pObj, &md.r);
			md.x = (md.r.x0 + md.r.x1) / 2.;
			md.y = pObj->GetYAlign () * pTheme->GetZoomFactor ();
			md.chain = NULL;
			md.m = dynamic_cast<Mesomer*> (pObj);
			positions[pObj] = md;
			if (!pStart)
				pStart = md.m;
		}
		pObj = GetNextChild (i);
	}
	if (pStart == NULL)
		throw  invalid_argument (_("Something wrong happened, please file a bug report."));
	set<Object*> Core;
	Core.insert (pStart);
	// initialize chains starting from pStart
	list<MesomeryChain*> chains;
	set<MesomeryChain*> terminated_chains;
	list<MesomeryChain*>::iterator c, cend;
	list<Mesomer*>::iterator m, mend;
	list<Mesomer*>::reverse_iterator mr;
	MesomeryChain *chain;
	map<Mesomer *, MesomeryArrow *> *arrows = pStart->GetArrows ();
	map<Mesomer *, MesomeryArrow *>::iterator j, jend = arrows->end ();
	md0 = positions[pStart];
	for (j = arrows->begin (); j != jend; j++) {
		// At this point we don't need to check anything, just create the chains
		chain = new MesomeryChain;
		chain->prev = NULL;
		chain->mesomers.push_front (pStart);
		chain->mesomers.push_back ((*j).first);
		md = positions[(*j).first];
		md.chain = chain;
		chains.push_back (chain);
		// align the molecule
		if ((*j).second->GetStartMesomer () != pStart)
			(*j).second->Reverse ();
		DoAlign ((*j).second, md0, md, pTheme->GetArrowPadding (), pTheme->GetZoomFactor ());
		positions[(*j).first] = md;
	}
	/* now add one mesomer to each growing chain and terminate chains when the end is
	reached or when a cycle is found */
	while (!chains.empty ()) {
		cend = chains.end ();
		for (c = chains.begin (); c != cend; c++) {
			// here we need the end of the chain and the previous mesomer
			mr = (*c)->mesomers.rbegin ();
			mes0 = *mr;
			mr++;
			mes = *mr;
			arrows = mes0->GetArrows ();
			switch (arrows->size ()) {
			case 1:
				// this is the end of the chain
				TerminateChain (*c, Core, terminated_chains, positions);
			break;
			case 2:
				arrows = mes0->GetArrows ();
				j = arrows->begin ();
				if ((*j).first == mes)
					j++;
				if ((*j).second->GetStartMesomer () != mes0)
					(*j).second->Reverse ();
				md0 = positions[mes0];
				md = positions[(*j).first];
				if (Core.find ((*j).first) != Core.end ()) {
					// we reach a mesomer already in the core: cycle
					if (Core.find (mes0) == Core.end ()) {
						// find start atom for the chain
						ch = *c;
						while (ch->prev)
							ch = ch->prev;
						md1 = positions[ch->mesomers.front ()];
						d = GetProjectionRatio (md, md1, md0, (*j).second);
						if (d > 0.999999 && d < CUTOFF)
							ChainAdjust (md0, md1, d, positions);
					}
					TerminateChain (*c, Core, terminated_chains, positions);
				} else if (md.chain != NULL) {
					// we have just found a new cycle
					// find start atom for both chains
					ch = *c;
					while (ch->prev)
						ch = ch->prev;
					md1 = positions[ch->mesomers.front ()];
					ch = md.chain;
					while (ch->prev)
						ch = ch->prev;
					md2 = positions[ch->mesomers.front ()];
					// Check if mes0 are in core or not (this might happen)
					if (Core.find (mes0) != Core.end ()) {
						/* we are already in core, try to align the other chain
						and change the arrow if not possible */
						d = GetProjectionRatio (md0, md2, md, (*j).second);
						if (d > 0.999999)
							ChainAdjust (md, md2, d, positions);
					} else {
						d = GetProjectionRatio (md, md1, md0, (*j).second);
						if (d > 0.999999 && d < CUTOFF)
							ChainAdjust (md0, md1, d, positions);
						else {
							d = GetProjectionRatio (md0, md2, md, (*j).second);
							if (d > 0.999999 && d < CUTOFF)
								ChainAdjust (md, md2, d, positions);
						}
					}
					TerminateChain (md.chain, Core, terminated_chains, positions);
					TerminateChain (*c, Core, terminated_chains, positions);
				} else {
					md.chain = md0.chain;
					md.chain->mesomers.push_back ((*j).first);
					DoAlign ((*j).second, md0, md, pTheme->GetArrowPadding (), pTheme->GetZoomFactor ());
					positions[(*j).first] = md;
				}
				break;
			default:
				arrows = mes0->GetArrows ();
				jend = arrows->end ();
				for (j = arrows->begin (); j != jend; j++) {
					if ((*j).first == mes)
						continue;
					if ((*j).second->GetStartMesomer () != mes0)
						(*j).second->Reverse ();
					md0 = positions[mes0];
					md = positions[(*j).first];
					if (Core.find ((*j).first) != Core.end ()) {
						// we reach a mesomer already in the core: cycle
						if (Core.find (mes0) == Core.end ()) {
							// find start atom for the chain
							ch = *c;
							while (ch->prev)
								ch = ch->prev;
							md1 = positions[ch->mesomers.front ()];
							d = GetProjectionRatio (md, md1, md0, (*j).second);
							if (d > 0.999999 && d < CUTOFF)
								ChainAdjust (md0, md1, d, positions);
						}
						TerminateChain (*c, Core, terminated_chains, positions);
					} else if (md.chain != NULL) {
						// we have just found a new cycle
						// we have just found a new cycle
						// find start atom for both chains
						ch = *c;
						while (ch->prev)
							ch = ch->prev;
						md1 = positions[ch->mesomers.front ()];
						ch = md.chain;
						while (ch->prev)
							ch = ch->prev;
						md2 = positions[ch->mesomers.front ()];
						// Check if mes0 are in core or not (this might happen)
						if (Core.find (mes0) != Core.end ()) {
							/* we are already in core, try to align the other chain
							and change the arrow if not possible */
							d = GetProjectionRatio (md0, md2, md, (*j).second);
							if (d > 0.999999 && d < CUTOFF)
								ChainAdjust (md, md2, d, positions);
						} else {
							d = GetProjectionRatio (md, md1, md0, (*j).second);
							if (d > 0.999999 && d < CUTOFF)
								ChainAdjust (md0, md1, d, positions);
							else {
								d = GetProjectionRatio (md0, md2, md, (*j).second);
								if (d > 0.999999)
									ChainAdjust (md, md2, d, positions);
							}
						}
						TerminateChain (md.chain, Core, terminated_chains, positions);
						TerminateChain (*c, Core, terminated_chains, positions);
					} else {
						md.chain = md0.chain;
						md.chain->mesomers.push_back ((*j).first);
						DoAlign ((*j).second, md0, md, pTheme->GetArrowPadding (), pTheme->GetZoomFactor ());
						positions[(*j).first] = md;
					}
				}
				break;
			}
		}
		set<MesomeryChain*>::iterator d, dend = terminated_chains.end ();
		for (d = terminated_chains.begin (); d != dend; d++) {
			chains.remove (*d);
			delete *d;
		}
		terminated_chains.clear ();
	}
	// really move mesomers
	map<Object*, MesomerData>::iterator p, pend= positions.end ();
	for (p = positions.begin (); p != pend; p++)
		(*p).first->Move ((*p).second.dx / pTheme->GetZoomFactor (), (*p).second.dy / pTheme->GetZoomFactor ());
	MesomeryArrow *arrow;
	for (pObj = GetFirstChild (i); pObj; pObj = GetNextChild (i))
		if (pObj->GetType () == MesomeryArrowType) {
			arrow = static_cast<MesomeryArrow*> (pObj);
			md = positions[arrow->GetStartMesomer ()];
			md0 = positions[arrow->GetEndMesomer ()];
			AlignArrow (arrow, md, md0, pTheme->GetArrowPadding (), pTheme->GetZoomFactor ());
		}
	pView->Update (this);
}

/* Transform2D is just here to inhibit rotation of retrosynthesis while we do not have 
real 2D alignment (only vertical at the moment */
void Mesomery::Transform2D (G_GNUC_UNUSED Matrix2D& m, G_GNUC_UNUSED double x, G_GNUC_UNUSED double y)
{
}

bool Mesomery::OnSignal (G_GNUC_UNUSED SignalId Signal, G_GNUC_UNUSED Object *Child)
{
	Validate (true);
	if (GetChildrenNumber () < 3)
		delete this;
	else
		Align ();
	return true;
}

static void do_destroy_mesomery (void *data)
{
	Mesomery *ms = reinterpret_cast<Mesomery *> (data);
	Document *pDoc = reinterpret_cast<Document *> (ms->GetDocument ());
	WidgetData *pData = reinterpret_cast<WidgetData *> (g_object_get_data (G_OBJECT (pDoc->GetWidget ()), "data"));
	pData->Unselect (ms);
	Operation *pOp = pDoc->GetNewOperation (GCP_MODIFY_OPERATION);
	pOp->AddObject (ms, 0);
	delete ms;
	pDoc->FinishOperation ();
}

bool Mesomery::BuildContextualMenu (GtkUIManager *UIManager, Object *object, double x, double y)
{
	GtkActionGroup *group = gtk_action_group_new ("mesomery");
	GtkAction *action = gtk_action_new ("destroy-ms", _("Destroy the mesomery relationship"), NULL, NULL);
	gtk_action_group_add_action (group, action);
	g_object_unref (action);
	gtk_ui_manager_insert_action_group (UIManager, group, 0);
	g_object_unref (group);
	char buf[] = "<ui><popup><menuitem action='destroy-ms'/></popup></ui>";
	gtk_ui_manager_add_ui_from_string (UIManager, buf, -1, NULL);
	GtkWidget *w = gtk_ui_manager_get_widget (UIManager, "/popup/destroy-ms");
	g_signal_connect_swapped (w, "activate", G_CALLBACK (do_destroy_mesomery), this);
	GetParent ()->BuildContextualMenu (UIManager, object, x, y);
	return true;
}

Mesomery::Mesomery (Object* parent, Mesomer *mesomer): Object (MesomeryType)
{
	SetId ("msy1");
	SetParent (parent);
	AddChild (mesomer);
	set<Object *> Objects;
	BuildConnectivity (Objects, mesomer);
	set<Object *>::iterator i, end = Objects.end ();
	for (i = Objects.begin (); i != end; i++)
		AddChild (*i);
	Align ();
}

double Mesomery::GetYAlign ()
{
	map<string, Object*>::iterator i;
	Object *pObj;
	pObj = GetFirstChild (i);
	double y = DBL_MAX, new_y;
	while (pObj) {
		if (pObj->GetType () == MesomerType)
			if ((new_y = pObj->GetYAlign ()) < y)
				y = new_y;			
		pObj = GetNextChild (i);
	}
	return y;
}

}	//	namespace gcp
