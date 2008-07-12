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
	Bond ();
	Bond (Atom* first, Atom* last, unsigned char order);
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
	virtual bool LoadNode (xmlNodePtr);
	virtual void Update (GtkWidget* w) const;
	virtual void Move (double x, double y, double z = 0);
	virtual void Transform2D (gcu::Matrix2D& m, double x, double y);
	double GetDist (double x, double y);
	void SetDirty ();
	void Revert ();
	void IncOrder (int n = 1);
	virtual void SetSelected (GtkWidget* w, int state);
	void Add (GtkWidget* w) const;
	virtual double GetYAlign ();
	bool IsCrossing (Bond *pBond);
	bool BuildContextualMenu (GtkUIManager *UIManager, Object *object, double x, double y);
	void MoveToBack ();
	void BringToFront ();
	bool SetProperty (unsigned property, char const *value);

private:
	GnomeCanvasPathDef* BuildPathDef (WidgetData* pData);
	GnomeCanvasPathDef* BuildCrossingPathDef (WidgetData* pData);

protected:
	BondType m_type;
	double m_coords[16];//coordinates of the lines used to represent the bond in the canvas
	bool m_CoordsCalc; //true if m_coords have been calculated, false else
	std::map<Bond*, BondCrossing> m_Crossing;
	int m_level; // to know which bond should be considered front
};

}	//	namespace gcp

#endif // GCHEMPAINT_BOND_H
