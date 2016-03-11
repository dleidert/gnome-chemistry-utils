// -*- C++ -*-

/*
 * GChemPaint library
 * bond.h
 *
 * Copyright (C) 2001-2012 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_BOND_H
#define GCHEMPAINT_BOND_H

/*!\file*/

#include <gccv/item-client.h>
#include <gcu/bond.h>
#include <list>

namespace gcu {
class UIManager;
}

namespace gcp {

/*!\enum BondType gcp/bond.h
The bond types recognized in GChemPaint. Possible values are:
 - NormalBondType: normal bonds,
 - UpBondType: wedge bond,
 - DownBondType: hash bond,
 - ForeBondType: large bond,
 - UndeterminedBondType: squiggled bond.
 - NewmanBondType: the bond displayed as a circle in a Newman projection.
*/
typedef enum
{
	NormalBondType,
	UpBondType,
	DownBondType,
	ForeBondType,
	UndeterminedBondType,
	NewmanBondType,
	WeakBondType
} BondType;

/*!\enum DoubleBondPosition gcp/bond.h
Used to set the second line position of a double bond.
 - DoubleBondAuto: automatic, GChemPaint places it on the side it thinks the best.
 - DoubleBondCenter: the two lines are symetric relative to the line joining the two ends.
 - DoubleBondLeft: the second line is on the left side when viewing from the start.
 - DoubleBondRight: the second line is on the right side when viewing from the start.

*/
typedef enum
{
	DoubleBondAuto,
	DoubleBondCenter,
	DoubleBondLeft,
	DoubleBondRight
} DoubleBondPosition;

/*!\struct BondCrossing gcp/bond.h
This structure is used for crossing bonds, so that the bond behind the other is
partially hidden.
*/
typedef struct {
/*!
The position of the crossing alog the bond: 0 if at start, 1 at end.
*/
	double a;
/*!
Whether the bond is above the other or not.
*/
	bool is_before;
} BondCrossing;

class Atom;
class WidgetData;

/*!\class Bond gcp/bond.h
This class is used to represent bonds.
*/

class Bond: public gcu::Bond, public gccv::ItemClient
{
public:
/*!
The default constructor.
*/
	Bond ();
/*!
@param first the first bonded atom.
@param last the last bonded atom.
@param order the bond order

Constructs a bond given its two terminal atoms and its order.
*/
	Bond (Atom* first, Atom* last, unsigned char order);
/*!
The destructor.
*/
	virtual ~Bond ();

/*!
@param x the x coordinate
@param y the y coordinate
@param z the z coordinate

@return a pointer to a bonded atom at or near position defined by the
coordinates passed as parameters.
*/
	Object* GetAtomAt (double x, double y, double z = 0.);
/*!
@return the type of the bond.
*/
	BondType GetType() const {return m_type;}
/*!
@param type the new bond type.

Sets the bond type.
*/
	void SetType (BondType type);
/*!
@param pAtom one of the bonded atoms

@return the angle (0 to 306°) that the bond makes from the horizontal when
starting from \a pAtom.
*/
	double GetAngle2D (Atom* pAtom);
/*!
@param pCycle a cycle containing the bond.

Notifies the bond it is in the cycle.
*/
	void AddCycle (gcu::Cycle* pCycle);
/*!
@param pCycle a cycle.

Notifies the bond that it is not anymore in the cycle.
*/
	void RemoveCycle (gcu::Cycle* pCycle);
/*!
Clears the list of the cycles containing the bond.
*/
	void RemoveAllCycles ();
/*!
@param Num the index of the line representing a, possibly, multiple bond.
@param x1 where to store the first x coordinate.
@param y1 where to store the first y coordinate.
@param x2 where to store the second x coordinate.
@param y2 where to store the second y coordinate.

Retrievers the coordinates of one of the lines representing the bond. Num must
be lower than the bond order to succeed.
@return true on success, false otherwise.
*/
	bool GetLine2DCoords (unsigned Num, double* x1, double* y1, double* x2, double* y2);
/*!
@param xml the xmlDoc used to save the document.
@param node a pointer to the xmlNode to which this Bond is serialized.

Saves the GChemPaint Bond class specific properties, such as the bond type
(see gcp::BondType for more information).
*/
	virtual bool SaveNode (xmlDocPtr xml, xmlNodePtr node) const;
/*!
@param node a pointer to the xmlNode containing the serialized Bond.

Loads properties specific to GChemPaint bonds.
*/
	bool LoadNode (xmlNodePtr node);
/*!
@param x the x component of the transation vector.
@param y the y component of the transation vector.
@param z the z component of the transation vector.

Used to move a bond. Just tells the bond it has been moved and that it's
coordinates need to be reevaluated from the new atomic positions.
*/
	void Move (double x, double y, double z = 0);
/*!
@param m the Matrix2D of the transformation.
@param x the x component of the center of the transformation.
@param y the y component of the center of the transformation.

Used to move and/or transform a bond. Just tells the bond it has been moved
and /or rotated and that it's coordinates need to be reevaluated from the new
atomic positions.
*/
	void Transform2D (gcu::Matrix2D& m, double x, double y);
/*!
@param x the x coordinate.
@param y the x coordinate.

Evaluates the distance from the point with coordinates \a x and \a y to the bond.
The line segment joining the two atoms is used whatever the type and the order
of the bond.
@return the calculated distance in pm.
*/
	double GetDist (double x, double y);
/*!
Tells that the bond has changed, and that the items representing it should be
updated accordingly.
*/
	void SetDirty ();
/*!
Exchanges the start and end atoms.
*/
	void Revert ();
/*!
@param n the bond order increment. If not given, the default is 1.

Tries to increment the bond order by n units. If something goes wrong, the
bond order is set to 1.
*/
	void IncOrder (int n = 1);
/*!
Used to add a representation of the bond in the view.
*/
	void AddItem ();
/*!
Used to update the representation of the bond in the view.
*/
	void UpdateItem ();
/*!
@param state the selection state of the bond.

Used to set the selection state of the bond inside the widget.
The values of state might be gcp::SelStateUnselected, gcp::SelStateSelected,
gcp::SelStateUpdating, or gcp::SelStateErasing.
*/
	void SetSelected (int state);
/*!
Used to retrieve the y coordinate for alignment.
@return y coordinate of the bond center.
*/
	double GetYAlign () const;
/*!
@param pBond a bond which might cross.

The lines representing two bonds might cross. This method detects this
condition.
@return true if bonds cross, false otherwise.
*/
	bool IsCrossing (Bond *pBond);
/*!
@param UIManager: the gcu::UIManager to populate.
@param object the atom on which occured the mouse click.
@param x x coordinate of the mouse click.
@param y y coordinate of the mouse click.

This method is called to build a contextual menu for the bond.
*/
	bool BuildContextualMenu (gcu::UIManager *UIManager, Object *object, double x, double y);
/*!
Move the bond to the lowest position. It has an effect only for crossing bonds.
*/
	void MoveToBack ();
/*!
Brings the bond to front. It has an effect only for crossing bonds.
*/
	void BringToFront ();
/*!
@param property the identity of the property as defined in objprops.h.

Used by the gcu::Loader mechanism to retrieve properties of bonds.
@return the value of the property as a string.
*/
	std::string GetProperty (unsigned property) const;
/*!
@param property the property id as defined in objprops.h
@param value the property value as a string

Used when loading to set properties for the bond. This method supports
GCU_PROP_BOND_TYPE and calls gcu::Bond::SetProperty() for other properties.
@return true if the property could be set, or if the property is not relevant, false otherwise.
*/
	bool SetProperty (unsigned property, char const *value);

/*!
@param x the x coordinate to adjust
@param y the y coordinate to adjust

On entering coordinates are considered relative to the bond axis. This method
adjust them so that they are put farer from the bond taking line width or
multiple bonds line into account.
*/
	void AdjustPosition (double &x, double &y);

/*!
This method should be called when a bond has been fully loaded.
*/
	void OnLoaded ();

private:
//	GnomeCanvasPathDef* BuildPathDef (WidgetData* pData);
//	GnomeCanvasPathDef* BuildCrossingPathDef (WidgetData* pData);
	BondType m_type;
	double m_coords[16];//coordinates of the lines used to represent the bond in the canvas
	bool m_CoordsCalc; //true if m_coords have been calculated, false else
	std::map<Bond*, BondCrossing> m_Crossing;
	int m_level; // to know which bond should be considered front

GCU_PROP (DoubleBondPosition, DoublePosition)
};

}	//	namespace gcp

#endif // GCHEMPAINT_BOND_H
