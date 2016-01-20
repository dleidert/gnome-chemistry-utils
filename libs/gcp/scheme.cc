// -*- C++ -*-

/*
 * GChemPaint library
 * scheme.cc
 *
 * Copyright (C) 2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "arrow.h"
#include "document.h"
#include "scheme.h"
#include "step.h"
#include "theme.h"
#include "view.h"
#include "widgetdata.h"
#include <glib/gi18n-lib.h>

namespace gcp {

Scheme::Scheme (gcu::TypeId type): gcu::Object (type)
{
}

Scheme::~Scheme ()
{
}

typedef struct SC
{
	std::list < Step * > steps;
	struct SC *prev;
	std::list < struct SC * > children;
} SchemeChain;

typedef struct
{
	double x, y, dx, dy;
	gccv::Rect r;
	Step *s;
	SchemeChain *chain;
} StepData;

static void TerminateChain ( SchemeChain *chain, std::set <gcu::Object * > &core,
		std::set < SchemeChain * > &terminated_chains, std::map < gcu::Object *, StepData > &positions)
{
	if (terminated_chains.find (chain) != terminated_chains.end ())
		return;	// job has already been done
	std::list < Step * >::iterator s, send;
	StepData sd;
	if (chain->prev != NULL)
		TerminateChain (chain->prev, core, terminated_chains, positions);
	send = chain->steps.end ();
	for (s = chain->steps.begin (), s++; s != send; s++) {
		sd = positions[*s];
		sd.chain = NULL;
		positions[*s] = sd;
		core.insert (*s);
	}
	terminated_chains.insert (chain);
	std::list < struct SC *>::iterator i, iend = chain->children.end ();
	for (i = chain->children.begin (); i != iend; i++)
		(*i)->prev = NULL;
}

static void DoAlign (Arrow *arrow, StepData &start, StepData &end, double ArrowPadding, double ZoomFactor)
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

static double GetProjectionRatio (StepData &sd, StepData &sd0, StepData &sd1, Arrow *arrow)
{
	double x0, y0, x1, y1, d;
	arrow->GetCoords (&x0, &y0, &x1, &y1);
	x1 -= x0;
	y1 -= y0;
	x0 = sd1.x - sd0.x;
	y0 = sd1.y - sd0.y;
	d = (x0 * y1 - x1 * y0);
	if (d == 0.)
		return -1.; // everything is aligned, and this should not occur, but who knows?
	return ((sd.x - sd0.x) * y1 - (sd.y - sd0.y) * x1)/ d;
}

static void AlignArrow (Arrow *arrow, StepData &sd0, StepData &sd1, double ArrowPadding, double ZoomFactor)
{
	double dx, dy, x, y, x0, y0, x1, y1, l;
	bool horiz, reversed;
	if (arrow->GetStartStep () != sd0.s) {
		arrow->Reverse ();
		reversed = true;
	} else
		reversed = false;
	if (sd1.s) {
		x = sd1.x - sd0.x;
		y = sd1.y - sd0.y;
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
				dx = sd0.r.x1 - sd0.x + ArrowPadding;
			else
				dx = sd0.r.x0 - sd0.x - ArrowPadding;
			dy = dx * y / x;
		} else {
			if (y > 0)
				dy = sd0.r.y1 - sd0.y + ArrowPadding;
			else
				dy = sd0.r.y0 - sd0.y - ArrowPadding;
			dx = dy * x / y;
		}
		x0 = (dx + sd0.x) / ZoomFactor;
		y0 = (dy + sd0.y) / ZoomFactor;
		if (horiz) {
			if (x > 0)
				dx = sd1.x - sd1.r.x0 + ArrowPadding;
			else
				dx = sd1.x - sd1.r.x1 - ArrowPadding;
			dy = dx * y / x;
		} else {
			if (y > 0)
				dy = sd1.y - sd1.r.y0 + ArrowPadding;
			else
				dy = sd1.y - sd1.r.y1 - ArrowPadding;
			dx = dy * x / y;
		}
		x1 = (sd1.x - dx) / ZoomFactor;
		y1 = (sd1.y - dy) / ZoomFactor;
		arrow->SetCoords (x0, y0, x1, y1);
	} else {
		// FIXME: do we need to do something in that case?
	}
	if (reversed)
		arrow->Reverse ();
}

static void ChainMove (SchemeChain *chain, double dx, double dy, std::map < gcu::Object *, StepData > &positions)
{
	std::list < Step * >::iterator i, iend;
	std::list < struct SC * >::iterator c, cend;
	StepData sd;
	iend = chain->steps.end ();
	for (i = chain->steps.begin (), i++; i!= iend; i++) {
		sd = positions [*i];
		sd.r.x0 += dx;
		sd.r.x1 += dx;
		sd.x += dx;
		sd.dx += dx;
		sd.r.y0 += dy;
		sd.r.y1 += dy;
		sd.y += dy;
		sd.dy += dy;
		positions[*i] = sd;
	}
	cend = chain->children.end ();
	for (c = chain->children.begin (); c != cend; c++)
		ChainMove (*c, dx, dy, positions);
}

static void ChainAdjust (StepData &sd0, StepData &sd1, double ratio, std::map < gcu::Object *, StepData > &positions)
{
	SchemeChain *chain = sd0.chain, *parent;
	StepData sd;
	double x = 0., y = 0.;
	ratio -= 1.;
	std::list < Step * >::reverse_iterator s, send;
	std::list < struct SC * >::iterator c, cend;
	while (chain) {
		for (s = chain->steps.rbegin (); s != send; s++) {
			if (*s == sd1.s)
				return;
			sd = positions[*s];
			x = (sd.x - sd1.x) * ratio;
			y = (sd.y - sd1.y) * ratio;
			sd.r.x0 += x;
			sd.r.x1 += x;
			sd.x += x;
			sd.dx += x;
			sd.r.y0 += y;
			sd.r.y1 += y;
			sd.y += y;
			sd.dy += y;
			positions[sd.s] = sd;
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

void Scheme::Align () throw (std::invalid_argument)
{
	Document *doc = static_cast < Document * > (GetDocument ());
	View *view = doc->GetView ();
	Theme *theme = doc->GetTheme ();
	WidgetData  *data = reinterpret_cast < WidgetData * > (g_object_get_data (G_OBJECT (doc->GetWidget ()), "data"));
	std::map < std::string, gcu::Object* >::iterator i;
	Object *obj = GetFirstChild (i);
	Step *start = NULL, *step0, *step;
	std::map < Object*, StepData > positions;
	StepData sd, sd0, sd1, sd2;
	double d;
	sd.dx = sd.dy = 0.;
	SchemeChain *ch;
	bool reversed;
	while (obj) {
		if ((sd.s = dynamic_cast < Step * > (obj)) != NULL) {
			data->GetObjectBounds (obj, &sd.r);
			sd.x = (sd.r.x0 + sd.r.x1) / 2.;
			sd.y = obj->GetYAlign () * theme->GetZoomFactor ();
			sd.chain = NULL;
			positions[obj] = sd;
			if (!start)
				start = sd.s;
		}
		obj = GetNextChild (i);
	}
	if (start == NULL)
		throw  std::invalid_argument (_("Something wrong happened, please file a bug report."));
	std::set < gcu::Object* > core;
	core.insert (start);
	// initialize chains starting from start
	std::list < SchemeChain * > chains;
	std::set < SchemeChain * > terminated_chains;
	std::list < SchemeChain * >::iterator c, cend;
	std::list < Step * >::iterator s, send;
	std::list < Step * >::reverse_iterator sr;
	SchemeChain *chain;
	std::map < Step *, Arrow * > *arrows = start->GetArrows ();
	std::map < Step *, Arrow  *>::iterator j, jend = arrows->end ();
	sd0 = positions[start];
	for (j = arrows->begin (); j != jend; j++) {
		// At this point we don't need to check anything, just create the chains
		chain = new SchemeChain;
		chain->prev = NULL;
		chain->steps.push_front (start);
		chain->steps.push_back ((*j).first);
		sd = positions[(*j).first];
		sd.chain = chain;
		chains.push_back (chain);
		// align the molecule
		if ((*j).second->GetStartStep () != start) {
			(*j).second->Reverse ();
			reversed = true;
		} else
			reversed = false;
		DoAlign ((*j).second, sd0, sd, theme->GetArrowPadding (), theme->GetZoomFactor ());
		if (reversed)
			(*j).second->Reverse ();
		positions[(*j).first] = sd;
	}
	/* now add one step to each growing chain and terminate chains when the end is
	reached or when a cycle is found */
	while (!chains.empty ()) {
		cend = chains.end ();
		for (c = chains.begin (); c != cend; c++) {
			// here we need the end of the chain and the previous step
			sr = (*c)->steps.rbegin ();
			step0 = *sr;
			while (step0 == NULL) { // only the last step might be NULL
				sr++;
				step0 = *sr;
			}
			step = *sr;
			arrows = step0->GetArrows ();
			switch (arrows->size ()) {
			case 1:
				// this is the end of the chain
				TerminateChain (*c, core, terminated_chains, positions);
			break;
			case 2:
				arrows = step0->GetArrows ();
				j = arrows->begin ();
				if ((*j).first == step)
					j++;
				if ((*j).second->GetStartStep () != step0) {
					(*j).second->Reverse ();
					reversed = true;
				} else
					reversed = false;
				sd0 = positions[step0];
				sd = positions[(*j).first];
				if (core.find ((*j).first) != core.end ()) {
					// we reach a step already in the core: cycle
					if (core.find (step0) == core.end ()) {
						// find start atom for the chain
						ch = *c;
						while (ch->prev)
							ch = ch->prev;
						sd1 = positions[ch->steps.front ()];
						d = GetProjectionRatio (sd, sd1, sd0, (*j).second);
						if (d > 0.999999 && d < CUTOFF)
							ChainAdjust (sd0, sd1, d, positions);
					}
					TerminateChain (*c, core, terminated_chains, positions);
				} else if (sd.chain != NULL) {
					// we have just found a new cycle
					// find start atom for both chains
					ch = *c;
					while (ch->prev)
						ch = ch->prev;
					sd1 = positions[ch->steps.front ()];
					ch = sd.chain;
					while (ch->prev)
						ch = ch->prev;
					sd2 = positions[ch->steps.front ()];
					// Check if mes0 are in core or not (this might happen)
					if (core.find (step0) != core.end ()) {
						/* we are already in core, try to align the other chain
						and change the arrow if not possible */
						d = GetProjectionRatio (sd0, sd2, sd, (*j).second);
						if (d > 0.999999)
							ChainAdjust (sd, sd2, d, positions);
					} else {
						d = GetProjectionRatio (sd, sd1, sd0, (*j).second);
						if (d > 0.999999 && d < CUTOFF)
							ChainAdjust (sd0, sd1, d, positions);
						else {
							d = GetProjectionRatio (sd0, sd2, sd, (*j).second);
							if (d > 0.999999 && d < CUTOFF)
								ChainAdjust (sd, sd2, d, positions);
						}
					}
					TerminateChain (sd.chain, core, terminated_chains, positions);
					TerminateChain (*c, core, terminated_chains, positions);
				} else {
					sd.chain = sd0.chain;
					sd.chain->steps.push_back ((*j).first);
					DoAlign ((*j).second, sd0, sd, theme->GetArrowPadding (), theme->GetZoomFactor ());
					positions[(*j).first] = sd;
				}
				if (reversed)
					(*j).second->Reverse ();
				break;
			default:
				arrows = step0->GetArrows ();
				jend = arrows->end ();
				for (j = arrows->begin (); j != jend; j++) {
					if ((*j).first == step)
						continue;
					if ((*j).second->GetStartStep () != step0) {
						(*j).second->Reverse ();
						reversed = true;
					} else
						reversed = false;
					if ((*j).first == NULL)
						continue;
					sd0 = positions[step0];
					sd = positions[(*j).first];
					if (core.find ((*j).first) != core.end ()) {
						// we reach a mesomer already in the core: cycle
						if (core.find (step0) == core.end ()) {
							// find start step for the chain
							ch = *c;
							while (ch->prev)
								ch = ch->prev;
							sd1 = positions[ch->steps.front ()];
							d = GetProjectionRatio (sd, sd1, sd0, (*j).second);
							if (d > 0.999999 && d < CUTOFF)
								ChainAdjust (sd0, sd1, d, positions);
						}
						TerminateChain (*c, core, terminated_chains, positions);
					} else if (sd.chain != NULL) {
						// we have just found a new cycle
						// find start atom for both chains
						ch = *c;
						while (ch->prev)
							ch = ch->prev;
						sd1 = positions[ch->steps.front ()];
						ch = sd.chain;
						while (ch->prev)
							ch = ch->prev;
						sd2 = positions[ch->steps.front ()];
						// Check if step0 are in core or not (this might happen)
						if (core.find (step0) != core.end ()) {
							/* we are already in core, try to align the other chain
							and change the arrow if not possible */
							d = GetProjectionRatio (sd0, sd2, sd, (*j).second);
							if (d > 0.999999 && d < CUTOFF)
								ChainAdjust (sd, sd2, d, positions);
						} else {
							d = GetProjectionRatio (sd, sd1, sd0, (*j).second);
							if (d > 0.999999 && d < CUTOFF)
								ChainAdjust (sd0, sd1, d, positions);
							else {
								d = GetProjectionRatio (sd0, sd2, sd, (*j).second);
								if (d > 0.999999)
									ChainAdjust (sd, sd2, d, positions);
							}
						}
						TerminateChain (sd.chain, core, terminated_chains, positions);
						TerminateChain (*c, core, terminated_chains, positions);
					} else {
						sd.chain = sd0.chain;
						sd.chain->steps.push_back ((*j).first);
						DoAlign ((*j).second, sd0, sd, theme->GetArrowPadding (), theme->GetZoomFactor ());
						positions[(*j).first] = sd;
					}
					if (reversed)
						(*j).second->Reverse ();
				}
				break;
			}
		}
		std::set < SchemeChain * >::iterator d, dend = terminated_chains.end ();
		for (d = terminated_chains.begin (); d != dend; d++) {
			chains.remove (*d);
			delete *d;
		}
		terminated_chains.clear ();
	}
	// really move mesomers
	std::map < gcu::Object*, StepData >::iterator p, pend= positions.end ();
	for (p = positions.begin (); p != pend; p++)
		if ((*p).first != NULL)
			(*p).first->Move ((*p).second.dx / theme->GetZoomFactor (), (*p).second.dy / theme->GetZoomFactor ());
	Arrow *arrow;
	StepData dummy;
	dummy.s = NULL;
	dummy.x = go_nan;
	for (obj = GetFirstChild (i); obj; obj = GetNextChild (i))
		if ((arrow = dynamic_cast < Arrow * > (obj)) != NULL) {
			if (arrow->GetStartStep ())
				sd = positions[arrow->GetStartStep ()];
			if (arrow->GetEndStep ())
				sd0 = positions[arrow->GetEndStep ()];
			else
				sd0 = dummy;
			if (arrow->GetStartStep ())
				AlignArrow (arrow, sd, sd0, theme->GetArrowPadding (), theme->GetZoomFactor ());
			else
				AlignArrow (arrow, sd0, dummy, theme->GetArrowPadding (), theme->GetZoomFactor ());
		}
	view->Update (this);
}

}	//	namespace gcp
