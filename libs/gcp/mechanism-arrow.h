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

#include <gccv/item-client.h>
#include <gcu/object.h>

namespace gcp {

extern gcu::TypeId MechanismArrowType;

class MechanismArrow: public gcu::Object, public gccv::ItemClient
{
public:
	MechanismArrow ();
	virtual ~MechanismArrow ();

	void SetSource (gcu::Object *source);
	void SetSourceAux (gcu::Object *aux);
	void SetTarget (gcu::Object *target);
	void SetControlPoint (int num, double dx, double dy);
	void SetShowControls (bool show);
	void SetPair (bool is_pair);
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

Used to load amechanism arrow in memory. The arrow must already exist.
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

	std::string Name ();

private:
	double m_CPx1, m_CPy1, m_CPx2, m_CPy2;

GCU_RO_PROP (gcu::Object *, Source)
GCU_RO_PROP (gcu::Object *, SourceAux)
GCU_RO_PROP (gcu::Object *, Target)
GCU_RO_PROP (bool, ShowControls)
GCU_RO_PROP (bool, Pair)
GCU_RO_PROP (bool, EndAtNewBondCenter)
};

}	//	namespace gcp

#endif	//	GCHEMPAINT_MECHANISM_ARROW_H