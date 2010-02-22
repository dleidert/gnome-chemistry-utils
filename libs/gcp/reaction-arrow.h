// -*- C++ -*-

/* 
 * GChemPaint library
 * reaction-arrow.h 
 *
 * Copyright (C) 2004-2010 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_REACTION_ARROW_H
#define GCHEMPAINT_REACTION_ARROW_H

#include "arrow.h"

namespace gcp {

class ReactionStep;
class Reaction;
class ReactionProp;

/*!\class ReactionArrow gcp/reaction-arrow.h
Arrow class for arrows used in chemical reactions.*/
class ReactionArrow: public Arrow
{
public:
/*!
*/
	ReactionArrow (Reaction* react, unsigned Type = SimpleArrow);
/*!
The destructor.
*/
	virtual ~ReactionArrow ();

/*!
@param xml the xmlDoc used to save the document.

Used to save the arrow to the xmlDoc.
@return the xmlNode containing the serialized arrow.
*/
	xmlNodePtr Save (xmlDocPtr xml) const;
/*!
@param node: a pointer to the xmlNode containing the serialized arrow.

Used to load an arrow in memory.
@return true on succes, false otherwise.
*/
	bool Load (xmlNodePtr node);
/*!
Used to add a representation of the arrow in the view.
*/
	void AddItem ();
/*!
Used to update the representation of the arrow in the view.
*/
	void UpdateItem ();
/*!
@param Step the reaction step before the reaction

Sets the initial reaction step.
*/
	void SetStartStep (ReactionStep *Step) {m_Start = Step;}
/*!
@return the initial reaction step.
*/
	ReactionStep* GetStartStep () {return m_Start;}
/*!
@param Step the reaction step after the reaction

Sets the final reaction step.
*/
	void SetEndStep (ReactionStep *Step) {m_End = Step;}
/*!
@return the final reaction step.
*/
	ReactionStep* GetEndStep () {return m_End;}
/*!
@param Step the reaction step to remove from the reaction

Removes the reaction step, which might be either the initial or final step.
If it not one of these, nothing is done.
*/
	void RemoveStep (ReactionStep *Step);
/*!
@param UIManager the GtkUIManager to populate.
@param object the Object on which occured the mouse click.
@param x x coordinate of the mouse click.
@param y y coordinate of the mouse click.

This method is called to build a contextual menu for the arrow.
@return true if something is added to the UIManager, false otherwise.
*/
	bool BuildContextualMenu (GtkUIManager *UIManager, gcu::Object *object, double x, double y);
/*!
@param x the x component of the transation vector.
@param y the y component of the transation vector.
@param z the z component of the transation vector.

Used to move a reaction arrow. It will also move the objects attached to the
arrow if any.
*/
	void Move (double x, double y, double z = 0);
/*!
@param state: the selection state of the arrow.

Used to set the selection state of the arrow inside the widget.
The values of state might be gcp::SelStateUnselected, gcp::SelStateSelected,
gcp::SelStateUpdating, or gcp::SelStateErasing. Children will be selected too.
*/
	void SetSelected (int state);
/*!
@param object the object to attach to the arrow.

Attach an object to the arrow and pops up the reaction property dialog box.
*/
	void AddProp (gcu::Object *object);
/*!
@param prop the attached property to position.

Position an object at the right place near the arrow.
*/
	void PositionChild (ReactionProp *prop);
/*!
@param Signal the appropriate SignalId
@param Child the child which emitted the signal or NULL

This function is called by the framework when a signal has been emitted for
the arrow, such as when an attached object size changed.
It should not be called by a program; call Object::EmitSignal instead.

@return true if the signal should be propagated to the parent, false otherwise.
*/
	bool OnSignal (gcu::SignalId Signal, gcu::Object *Child);
/*!
@param property the property id as defined in objprops.h
@param value the property value as a string

Used when loading to set common properties to arrows. Only one property is
currently supported: gcu::GCU_PROP_REACTION_ARROW_TYPE.
@return true if the property could be set, or if the property is not relevant,
false otherwise.
*/
	bool SetProperty (unsigned property, char const *value);

	std::string Name ();

private:
	unsigned m_Type;
	bool m_TypeChanged;
	ReactionStep *m_Start, *m_End;
};

}	//	namespace gcp

#endif	//	GCHEMPAINT_REACTION_ARROW_H
