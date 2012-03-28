// -*- C++ -*-

/*
 * GChemPaint library
 * reactant.h
 *
 * Copyright (C) 2004-2011 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_REACTANT_H
#define GCHEMPAINT_REACTANT_H

#include <gcu/object.h>

/*!\file*/

namespace gcu {
class UIManager;
}

namespace gcp {

class ReactionStep;

/*!\class Reactant gcp/reactant.h
\brief Class for reactants and products of a chemical reaction.

Objects of these class embed either a molecule or a text representing a reactant
or a product for a chemical reaction. They can have a stoichiometry coefficient.
*/
class Reactant: public gcu::Object
{
public:
/*!
The default constructor.
*/
	Reactant ();
/*!
The destructor.
*/
	virtual ~Reactant ();

/*!
@param step the parent reaction step.
@param object the molecule formula or text to use as reactant.
*/
	Reactant (ReactionStep* step, gcu::Object* object) throw (std::invalid_argument);

/*!
@param xml the xmlDoc used to save the document.

Used to save the reactant to the xmlDoc.
@return the xmlNode containing the serialized reactant.
*/
	virtual xmlNodePtr Save (xmlDocPtr xml) const;
/*!
@param node a pointer to the xmlNode containing the serialized reactant.

Used to load a reactant in memory. The Reactant instance must already exist.

@return true on succes, false otherwise.
*/
	virtual bool Load (xmlNodePtr node);

/*!
@return the reactants stoichiometry coefficient.
*/
	unsigned GetStoichiometry () const {return m_Stoich;}
/*!
@param coef the new stoichiometry coefficient.

Sets the stoichiometry coefficient for the reactant.
*/
	void SetStoichiometry (unsigned coef) {m_Stoich = coef;}
/*!
Used to retrieve the y coordinate for alignment.
@return y coordinate used for reactant alignment.
*/
	virtual double GetYAlign ();
/*!
@param UIManager the gcu::UIManager to populate.
@param object the Object on which occured the mouse click.
@param x x coordinate of the mouse click.
@param y y coordinate of the mouse click.

This method is called to build a contextual menu for the reactant.
@return true if something is added to the UIManager, false otherwise.
*/
	bool BuildContextualMenu (gcu::UIManager *UIManager, gcu::Object *object, double x, double y);
/*!
@param Signal the appropriate SignalId
@param Child the child which emitted the signal or NULL

This function is called by the framework when a signal has been emitted for
the reactant, when the embedded text or molecule changed.
It should not be called by a program; call Object::EmitSignal instead.

@return true to propagate the signal to the parent.
*/
	bool OnSignal (gcu::SignalId Signal, gcu::Object *Child);

/*!
Adds a text item to display the reactants stoichiometry coefficient. This
coefficient does not need to be an integer, but should always be positive.
*/
	void AddStoichiometry ();
/*!
@return the molecule or text embedded in this instance.
*/
	gcu::Object *GetChild () {return Child;}
/*!
@return the text representing the stoichiometry coefficient of this instance.
*/
	gcu::Object *GetStoichChild () {return Stoichiometry;}

/*!
@return the localized object generic name.
*/
	std::string Name ();

private:
	unsigned m_Stoich;	//always positive
	gcu::Object *Stoichiometry;
	gcu::Object *Child;
};

}	//	namespace gcp

#endif //	GCHEMPAINT_REACTANT_H
