// -*- C++ -*-

/* 
 * GChemPaint library
 * reaction-step.h 
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

#ifndef GCHEMPAINT_REACTION_STEP_H
#define GCHEMPAINT_REACTION_STEP_H

#include <gcu/object.h>
#include <set>

using namespace gcu;

namespace gcp {

class Reaction;
class ReactionArrow;

extern TypeId ReactionStepType;

class ReactionStep: public Object
{
public:
	ReactionStep ();
	virtual ~ReactionStep ();

	ReactionStep (Reaction *reaction, map<double, Object*>& Children, map<Object*, ArtDRect> Objects);

	void Add (GtkWidget* w);
	virtual xmlNodePtr Save (xmlDocPtr xml);
	virtual bool Load (xmlNodePtr);
	virtual double GetYAlign ();
	virtual bool OnSignal (SignalId Signal, Object *Child);

	void AddArrow (ReactionArrow *arrow) {m_Arrows.insert (arrow);}
	void RemoveArrow (ReactionArrow *arrow);

private:
	bool m_bLoading;
	set<ReactionArrow *> m_Arrows;
};

}	//	namespace gcp

#endif	//	GCHEMPAINT_REACTION_STEP_H
