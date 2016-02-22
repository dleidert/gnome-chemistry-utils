// -*- C++ -*-

/*
 * GChemPaint library
 * reaction-prop.h
 *
 * Copyright (C) 2007-2010 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_REACTION_PROP_H
#define GCHEMPAINT_REACTION_PROP_H

#include <gcu/object.h>
#include <gcu/dialog-owner.h>
#include <gcu/macros.h>

/*!\file*/
namespace gcp {

extern gcu::TypeId ReactionPropType;
class ReactionArrow;

/*!
This enumerates the known roles of objects attached to a reaction arrow
*/
enum {
/*!
Unkonw role.
*/
	REACTION_PROP_UNKNOWN,
/*!
Catalyst
*/
	REACTION_PROP_CATALYST,
/*!
Reactant
*/
	REACTION_PROP_REACTANT,
/*!
Product
*/
	REACTION_PROP_PRODUCT,
/*!
Solvent
*/
	REACTION_PROP_SOLVENT,
/*!
Values allowed for molecules are lower than this one
*/
	REACTION_PROP_MAX_MOL,
/*!
Temperature
*/
	REACTION_PROP_TEMPERATURE = REACTION_PROP_MAX_MOL,
/*!
Pressure
*/
	REACTION_PROP_PRESSURE,
/*!
Reaction time.
*/
	REACTION_PROP_TIME,
/*!
Reaction enthalpy
*/
	REACTION_PROP_ENTHALPY,
/*!
The first value greater than all knwo values.
*/
	REACTION_PROP_MAX,
};

/*!
The names associated with the various roles knwon for objects attached to
reaction arrows. These names are used for serialization.
*/
extern char const *ReactionPropRoles[];

/*!\class ReactionProp gcp/reaction-prop.h
This is a container class for objects attached to a reaction arrow.
*/

class ReactionProp: public gcu::Object, public gcu::DialogOwner
{
public:
/*!
The default constructor.
*/
	ReactionProp ();
/*!
@param parent the parent reaction arrow.
@param child the molecule or text to attach to the arrow.

Builds a new reaction property, and attach the child to the arrow.
*/
	ReactionProp (ReactionArrow *parent, gcu::Object *child);
/*!
The destructor.
*/
	~ReactionProp ();

/*!
@param xml the xmlDoc used to save the document.

Used to save the reaction property to the xmlDoc.
@return the xmlNode containing the serialized reaction property.
*/
	xmlNodePtr Save (xmlDocPtr xml) const;
/*!
@param node: a pointer to the xmlNode containing the serialized reaction property.

Used to load a reaction property in memory.
@return true on succes, false otherwise.
*/
	bool Load (xmlNodePtr node);
/*!
@param Signal the appropriate SignalId
@param Child the child which emitted the signal or NULL

This function is called by the framework when a signal has been emitted for
the reaction property, when one its emedded object changed.
It should not be called by a program; call Object::EmitSignal instead.

@return true to be propagate the signal to the parent.
*/
	bool OnSignal (gcu::SignalId Signal, gcu::Object *Child);
/*!
@param UIManager the gcu::UIManager to populate.
@param object the Object on which occured the mouse click.
@param x x coordinate of the mouse click.
@param y y coordinate of the mouse click.

This method is called to build a contextual menu for the arrow attached object.
@return true if something is added to the UIManager, false otherwise.
*/
	bool BuildContextualMenu (gcu::UIManager *UIManager, gcu::Object *object, double x, double y);
/*!
@param property the property id as defined in objprops.h
@param value the property value as a string

Used when loading to set common properties to arrow associated objects.
Currently supported properties:
	gcu::GCU_PROP_REACTION_ARROW_PROP_STEP,
	gcu::GCU_PROP_REACTION_ARROW_PROP_LINE,
	gcu::GCU_PROP_REACTION_ARROW_PROP_POSITION.
@return true if the property could be set, or if the property is not relevant,
false otherwise.
*/
	bool SetProperty (unsigned property, char const *value);
/*!
@param child: the embedded object
Sets the rembedded objecte.
*/
	void SetChild (gcu::Object *child);

/*!
@return the localized object generic name.
*/
	std::string Name ();
/*!
Used to retrieve the y coordinate for alignment.
@return y coordinate used for reaction arrow children alignment.
*/
	double GetYAlign () const;

private:
/*!\fn GetObject()
@return the embedded object.
*/
GCU_RO_PROP (gcu::Object*, Object);
/*!\fn SetRole(unsigned Role)
@param Role the new role for the property.

Sets the role for the property which should be less than REACTION_PROP_MAX.
*/
/*!\fn GetRole()
@return the property role.
*/
/*!\fn GetRefRole()
@return the property role as a reference.
*/
GCU_PROP (unsigned, Role)
GCU_PROP (unsigned, Step)
GCU_PROP (unsigned, Line)
GCU_PROP (unsigned, Rank)
};

}	//	namespace gcp


#endif	//	GCHEMPAINT_REACTION_PROP_H
