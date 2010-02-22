// -*- C++ -*-

/* 
 * GChemPaint library
 * reaction.h 
 *
 * Copyright (C) 2002-2010 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_REACTION_H
#define GCHEMPAINT_REACTION_H

#include <gcu/object.h>
#include <stdexcept>

/*!\file*/
namespace gcp {

/*!\class Reaction gcp/reaction.h
\brief Chemical reaction class.

This class is used for chemical reactions in GChemPaint. It can represent a
whole reactions scheme with several steps, although only one step is currently
really supported. Acceptable children for an instance of this class are
instances of ReactionArrow and ReactionStep.
*/
class Reaction: public gcu::Object
{
public:
/*!
Constructs a new empty Reaction instance.
*/
	Reaction ();
/*!
The destructor.
*/
	virtual ~Reaction ();

/*!
@param Children the list of objects used as children to build the reaction

This method is called to build a reactiont from its children. It might fail if
reactants are not properly aligned with the reaction arrows. Accepted
children are molecules and texts to be used as reactants or products, and
reaction arrows.
@return true in case of success and false if failed.
*/
	bool Build (std::list<gcu::Object*>& Children) throw (std::invalid_argument);
/*!
*/
	void Transform2D (gcu::Matrix2D& m, double x, double y);
/*!
@param UIManager the GtkUIManager to populate.
@param object the Object on which occured the mouse click.
@param x x coordinate of the mouse click.
@param y y coordinate of the mouse click.

This method is called to build a contextual menu for the reaction.
@return true if something is added to the UIManager, false otherwise.
*/
	bool BuildContextualMenu (GtkUIManager *UIManager, gcu::Object *object, double x, double y);
/*!
@param Signal the appropriate SignalId
@param Child the child which emitted the signal or NULL

This function is called by the framework when a signal has been emitted for
the reaction, when one of its children changed.
It should not be called by a program; call Object::EmitSignal instead.

@return true if the signal should be propagated to the parent, false otherwise.
*/
	bool OnSignal (gcu::SignalId Signal, gcu::Object *Child);
/*!
@param node: a pointer to the xmlNode containing the serialized reaction.

Used to load an reaction in memory.
@return true on succes, false otherwise.
*/
	bool Load (xmlNodePtr node);
/*!
Used to retrieve the y coordinate for alignment. 
@return y coordinate used for the reaction alignment.
*/
	double GetYAlign ();

	std::string Name ();
};

}	//	namespace gcp

#endif	//GCHEMPAINT_REACTION_H
