// -*- C++ -*-

/* 
 * GChemPaint library
 * atom.h 
 *
 * Copyright (C) 2001-2007 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_ATOM_H
#define GCHEMPAINT_ATOM_H

#include <map>
#include <glib.h>
#include <gcu/atom.h>
#include <gcu/element.h>
#include <gcu/macros.h>
#include <libgnomecanvas/gnome-canvas.h>
#include "widgetdata.h"

namespace OpenBabel
{
	class OBAtom;
}

using namespace OpenBabel;
using namespace std;
using namespace gcu;

namespace gcp {

class Bond;
class Cycle;
class Molecule;

#define CHARGE_NE 1
#define CHARGE_NW 2
#define CHARGE_N 4
#define CHARGE_SE 8
#define CHARGE_SW 16
#define CHARGE_S 32
#define CHARGE_E 64
#define CHARGE_W 128

#define POSITION_NE 1
#define POSITION_NW 2
#define POSITION_N 4
#define POSITION_SE 8
#define POSITION_SW 16
#define POSITION_S 32
#define POSITION_E 64
#define POSITION_W 128

class Electron;

class Atom: public gcu::Atom
{
public:
	Atom ();
	Atom (int Z, double x, double y, double z);
	Atom (OBAtom* atom);
	virtual ~Atom ();

public :
	virtual void SetZ (int Z);
	void AddBond (Bond* pBond);
	void RemoveBond (Bond* pBond);
	virtual void Update ();
	virtual void Add (GtkWidget* w);
	virtual void Update (GtkWidget* w);
	int GetTotalBondsNumber (); //take bond order into account
	int GetAttachedHydrogens () {return m_nH;}
	bool GetBestSide ();
	bool IsInCycle (Cycle* pCycle);
	virtual int GetChargePosition (unsigned char& Pos, double Angle, double& x, double& y);
	virtual int GetAvailablePosition (double& x, double& y);
	virtual bool GetPosition (double angle, double& x, double& y);
	virtual xmlNodePtr Save (xmlDocPtr xml);
	virtual bool Load (xmlNodePtr);
	virtual bool LoadNode (xmlNodePtr);
	virtual void SetSelected (GtkWidget* w, int state);
	virtual bool AcceptNewBonds (int nb = 1);
	virtual bool AcceptCharge (int charge);
	virtual double GetYAlign ();
	virtual void Transform2D (Matrix2D& m, double x, double y);
	bool BuildContextualMenu (GtkUIManager *UIManager, Object *object);
	/*!
	@param Mol: a pointer to a molecule

	Adds the atom to the molecule calling gcpMolecule::AddAtom()
	*/
	virtual void AddToMolecule (Molecule* Mol);
	bool HasImplicitElectronPairs ();
	bool MayHaveImplicitUnpairedElectrons ();
	void AddElectron (Electron* electron);
	void NotifyPositionOccupation (unsigned char pos, bool occupied);
	void SetChargePosition (unsigned char Pos, bool def, double angle = 0., double distance = 0.);
	char GetChargePosition (double *Angle, double *Dist);
	void SetCharge (int charge);
	int GetCharge () {return m_Charge;}
	void ForceChanged () {m_Changed = true;}

private:
	void BuildItems (WidgetData* pData);
	void UpdateAvailablePositions ();

private:
	Element *m_Element;
	int m_nH;
	int m_Valence; //valence
	int m_ValenceOrbitals;
	int m_nlp; //lone electron pairs number
	int m_nlu; //single electrons number
	double m_width, m_height; //size of the atomic symbol in the canvas
	double m_length, m_text_height; // size of the text buffer
	bool m_HPos; //false = left, true = right
	bool m_ChargeAuto;
	int m_Changed; //update needs regenerate the buffer
	int m_ascent;
	double m_lbearing;
	double m_CHeight;
	unsigned char m_AvailPos; //available standard positions for charge and electrons representations
	unsigned char m_OccupiedPos;
	bool m_AvailPosCached;
	unsigned char m_ChargePos;
	bool m_ChargeAutoPos;
	double m_ChargeAngle;
	double m_ChargeDist;
	double m_ChargeWidth, m_ChargeTWidth, m_ChargeXOffset, m_ChargeYOffset;
	list<double> m_AngleList;
	map<double, double> m_InterBonds; /* positions betwen bonds. First  value is the
	angle between the two bonds and second value is the direction */
	PangoLayout *m_Layout, *m_ChargeLayout;
	bool m_DrawCircle;
	string m_FontName;

GCU_PROP (bool, ShowSymbol)
};

}	//	namespace gcp

#endif // GCHEMPAINT_ATOM_H
