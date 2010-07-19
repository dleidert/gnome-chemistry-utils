// -*- C++ -*-

/* 
 * GChemPaint library
 * mechanism-arrow.h 
 *
 * Copyright (C) 2009-2010 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_MECHANISM_ARROW_H
#define GCHEMPAINT_MECHANISM_ARROW_H

/*!\file*/

#include <gccv/item-client.h>
#include <gcu/object.h>

namespace gcp {

/*!
The gcu::TypeId for MechanismArrow.
*/
	extern gcu::TypeId MechanismArrowType;

/*!
@brief Mechanism curved arrows.

Curved arrows used to represent electrons movements during a mechanim step.
*/
class MechanismArrow: public gcu::Object, public gccv::ItemClient
{
public:
/*!
Constructs a new MechanismArrow.
*/
	MechanismArrow ();
/*!
The destructor.
*/
	virtual ~MechanismArrow ();

/*!
@param source the source of the electrons.

Sets the initial owner, atom or bond, of the electrons. An Electron instance is
also allowed there.
*/
	void SetSource (gcu::Object *source);
/*!
@param aux an object.

Used when a new bond is created by the electron move, using initially bonding
electrons. In that case the source is the initial bond, the target, the not
bonded atom of the new bond, and the auxilliary source, the atom of the initial
bond which will be bonded to the target.
*/
	void SetSourceAux (gcu::Object *aux);
/*!
@param target the target of the electron move.

Sets where the arrow ends. Might be an atom or a bond.
*/
	void SetTarget (gcu::Object *target);
/*!
@param num the control point numbre, should be 1 or 2.
@param dx the x coordinate of the vector.
@param dy the y coordinate of the vector.

Sets the position of the intermediate control points used to construct a Bezier
cubic curve for the arrow. If \a num is 1, the vector starts at the start
point of the arrow, and if \a num is 2 it starts at the arrow end.
*/
	void SetControlPoint (int num, double dx, double dy);
/*!
@param show whether to show the control points.

Show or hide the control points. Showing the control points is useful when
editing.
*/
	void SetShowControls (bool show);
/*!
@param is_pair whether the arrow represents an electrons pair move or a single
electron move.

Sets the type of electron move.
*/
	void SetPair (bool is_pair);
/*!
@param end_at_new_bond_center the position of the arrow end for new bonds.

For arrows representing an electrons pair movement creating a new bond, the
might end either at the target atom or at the centeer of the created bond.
*/
	void SetEndAtNewBondCenter (bool end_at_new_bond_center);

	// virtual gcu::Object methods
/*!
	@param xml the xmlDoc used to save the document.
	
	Used to save the mechanism arrow to the xmlDoc.
	@return the xmlNode containing the serialized arrow.
*/
	xmlNodePtr Save (xmlDocPtr xml) const;
/*!
@param node a pointer to the xmlNode containing the serialized arrow.

Used to load a mechanism arrow in memory. The arrow must already exist.
@return true on succes, false otherwise.
*/
	bool Load (xmlNodePtr node);
/*!
@param m the Matrix2D of the transformation.
@param x the x component of the center of the transformation.
@param y the y component of the center of the transformation.

Used to move and/or transform a mechanism arrow.
*/
	void Transform2D (gcu::Matrix2D& m, double x, double y);

	// virtual gccv::ItemClient methods
/*!
Used to add a representation of the mechanism arrow in the view.
*/
	void AddItem ();
/*!
@param state the selection state of the text.

Used to set the selection state of text inside.
The values of state might be gcp::SelStateUnselected, gcp::SelStateSelected,
gcp::SelStateUpdating, or gcp::SelStateErasing.
*/
	void SetSelected (int state);

/*!
@param object the object just unlinked by Object::Unlink.
*/
	void OnUnlink (Object *object);

/*!
Ensure that links are correctly set.
*/
	void OnLoaded ();

/*!
@return the localized object generic name.
*/
	std::string Name ();

/*!
@return true if the MechanismArrow can be safely selected.
*/
	bool CanSelect () const;

private:
	double m_CPx1, m_CPy1, m_CPx2, m_CPy2;

/*!\fn GetSource()
@return the initial owner, atom or bond, of the electrons. Might be an Electron
instance.
*/
GCU_RO_PROP (gcu::Object *, Source)
/*!\fn GetSourceAux()
@return the auxilliary source, see SetSourceAux() for details.
*/
GCU_RO_PROP (gcu::Object *, SourceAux)
/*!\fn GetTarget()
@return the target of the electron move.
*/
GCU_RO_PROP (gcu::Object *, Target)
/*!\fn GetShowControls()
@return whether to show the control points.
*/
GCU_RO_PROP (bool, ShowControls)
/*!\fn GetPair()
@return whether the elctron movement concerns a pair or a single electron.
*/
GCU_RO_PROP (bool, Pair)
/*!\fn GetEndAtNewBondCenter()
@return whether to end the arrow at the center of the new bond. The value is
not significant when no new bond is created.
*/
GCU_RO_PROP (bool, EndAtNewBondCenter)
};

}	//	namespace gcp

#endif	//	GCHEMPAINT_MECHANISM_ARROW_H