// -*- C++ -*-

/* 
 * GChemPaint library
 * bond.h 
 *
 * Copyright (C) 2001-2008 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_BOND_H
#define GCHEMPAINT_BOND_H

#include <gcu/bond.h>
#include <list>
#include <libgnomecanvas/gnome-canvas-path-def.h>

namespace gcp {

/*!\enum BondType gcp/bond.h
The bond types recognized in GChemPaint. Possible values are:
 - NormalBondType: normal bonds,
 - UpBondType: wedge bond,
 - DownBondType: hash bond,
 - ForeBondType: large bond,
 - UndeterminedBondType: squiggled bond.
*/
typedef enum
{
	NormalBondType,
	UpBondType,
	DownBondType,
	ForeBondType,
	UndeterminedBondType
} BondType;

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

class Bond: public gcu::Bond
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

	virtual Object* GetAtomAt (double x, double y, double z = 0.);
	BondType GetType() const {return m_type;}
	void SetType (BondType type);
	double GetAngle2D (Atom* pAtom);
	void AddCycle (gcu::Cycle* pCycle);
	void RemoveCycle (gcu::Cycle* pCycle);
	void RemoveAllCycles ();
	bool GetLine2DCoords (unsigned Num, double* x1, double* y1, double* x2, double* y2);
	virtual bool SaveNode (xmlDocPtr xml, xmlNodePtr) const;
/*!
@param node a pointer to the xmlNode containing the serialized Bond.

Loads properties specific to GChemPaint bonds.
*/
	virtual bool LoadNode (xmlNodePtr node);
	virtual void Update (GtkWidget* w) const;
	virtual void Move (double x, double y, double z = 0);
	virtual void Transform2D (gcu::Matrix2D& m, double x, double y);
	double GetDist (double x, double y);
/*!
*/
	void SetDirty ();
/*!
*/
	void Revert ();
/*!
@param n the bond order increment. If not given, the default is 1.

Tries to increment the bond order by n units. If something goes wrong, the
bond order is set to 1.
*/
	void IncOrder (int n = 1);
/*!
@param w the GtkWidget inside which the bond is displayed.
@param state the selection state of the bond.

Used to set the selection state of the bond inside the widget.
The values of state might be gcp::SelStateUnselected, gcp::SelStateSelected,
gcp::SelStateUpdating, or gcp::SelStateErasing.
*/
	void SetSelected (GtkWidget* w, int state);
	void Add (GtkWidget* w) const;
	double GetYAlign ();
	bool IsCrossing (Bond *pBond);
/*!
@param UIManager: the GtkUIManager to populate.
@param object the atom on which occured the mouse click.
@param x x coordinate of the mouse click.
@param y y coordinate of the mouse click.

This method is called to build a contextual menu for the atom.
*/
	bool BuildContextualMenu (GtkUIManager *UIManager, Object *object, double x, double y);
	void MoveToBack ();
	void BringToFront ();
	bool SetProperty (unsigned property, char const *value);

private:
	GnomeCanvasPathDef* BuildPathDef (WidgetData* pData);
	GnomeCanvasPathDef* BuildCrossingPathDef (WidgetData* pData);
	BondType m_type;
	double m_coords[16];//coordinates of the lines used to represent the bond in the canvas
	bool m_CoordsCalc; //true if m_coords have been calculated, false else
	std::map<Bond*, BondCrossing> m_Crossing;
	int m_level; // to know which bond should be considered front
};

}	//	namespace gcp

#endif // GCHEMPAINT_BOND_H
