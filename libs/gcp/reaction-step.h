// -*- C++ -*-

/* 
 * GChemPaint library
 * reaction-step.h 
 *
 * Copyright (C) 2004-2008 Jean Bréfort <jean.brefort@normalesup.org>
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
#include <gccv/structs.h>
#include <set>

/*!\file*/
namespace gcp {

class Reaction;
class ReactionArrow;

extern gcu::TypeId ReactionStepType;

/*!\class ReactionStep gcp/reaction-step.h
\brief the list of reactants before or after a reaction arrow.

The ReactionStep class is a group class which owns a list of reactants and
the operators betwwen them. This class is misnamed, since the step is more
generally associated with the arrow. It might be renamed ReactionStage in the
future if it is possible without making old files unreadable.
*/
class ReactionStep: public gcu::Object
{
public:
/*!
The default constructor.
*/
	ReactionStep ();
/*!
The destructor.
*/
	virtual ~ReactionStep ();

/*!
@param reaction the parent reaction.
@param Children the reactants from which to build the new instance.
@param Objects the rectangles bounding the reactants.

Buils a new reaction step from the children and adds as many eaction operators
as necessary. All children will be horizontally aligned.
*/
	ReactionStep (Reaction *reaction, std::map<double, gcu::Object*>& Children, std::map<gcu::Object*, gccv::Rect> Objects);
/*!
@param xml the xmlDoc used to save the document.

Used to save the reaction step to the xmlDoc.
@return the xmlNode containing the serialized step.
*/
	xmlNodePtr Save (xmlDocPtr xml) const;
/*!
@param node: a pointer to the xmlNode containing the serialized step.

Used to load a reaction step in memory.
@return true on succes, false otherwise.
*/
	bool Load (xmlNodePtr node);
/*!
Used to retrieve the y coordinate for alignment. 
@return y coordinate used for the reaction step alignment.
*/
	double GetYAlign ();
/*!
@param Signal the appropriate SignalId
@param Child the child which emitted the signal or NULL

This function is called by the framework when a signal has been emitted for
the reaction step, when one of its children changed.
It should not be called by a program; call Object::EmitSignal instead.

@return true if the signal should be propagated to the parent, false otherwise.
*/
	bool OnSignal (gcu::SignalId Signal, gcu::Object *Child);

/*!
@param arrow an arrow related to this instance.

Adds an arrow to the list of arrows related to this instance.
*/
	void AddArrow (ReactionArrow *arrow) {m_Arrows.insert (arrow);}
/*!
@param arrow an arrow related to this instance.

Removes an arrow from the list of arrows related to this instance when it
not anymore relevant.
*/
	void RemoveArrow (ReactionArrow *arrow);

private:
	bool m_bLoading;
	std::set<ReactionArrow *> m_Arrows;
};

}	//	namespace gcp

#endif	//	GCHEMPAINT_REACTION_STEP_H
