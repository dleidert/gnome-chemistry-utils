// -*- C++ -*-

/* 
 * GChemPaint library
 * atom.h 
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

#ifndef GCHEMPAINT_ATOM_H
#define GCHEMPAINT_ATOM_H

#include <map>
#include <glib.h>
#include <gcu/atom.h>
#include <gcu/dialog-owner.h>
#include <gcu/element.h>
#include <gcu/macros.h>
#include <libgnomecanvas/gnome-canvas.h>
#include "widgetdata.h"

namespace OpenBabel
{
	class OBAtom;
}

/*!\file*/
namespace gcp {

class Bond;
class Molecule;

/*!
Top left position for charges and electrons around an atom.
*/
#define POSITION_NE 1
/*!
Top right position for charges and electrons around an atom.
*/
#define POSITION_NW 2
/*!
Top center position for charges and electrons around an atom.
*/
#define POSITION_N 4
/*!
Bottom left position for charges and electrons around an atom.
*/
#define POSITION_SE 8
/*!
Bottom right position for charges and electrons around an atom.
*/
#define POSITION_SW 16
/*!
Bottom center position for charges and electrons around an atom.
*/
#define POSITION_S 32
/*!
Left position for charges and electrons around an atom.
*/
#define POSITION_E 64
/*!
Right position for charges and electrons around an atom.
*/
#define POSITION_W 128

/*!\enum HPos
Represents the various possiblepositions for implicit hydrogen atoms bonded
to non metals.
*/
typedef enum {
/*!
Hydrogen atoms at left.
*/
	LEFT_HPOS,
/*!
Hydrogen atoms at right.
*/
	RIGHT_HPOS,
/*!
Hydrogen atoms at top.
*/
	TOP_HPOS,
/*!
Hydrogen atoms at bottom.
*/
	BOTTOM_HPOS,
/*!
Automatic position.
*/
	AUTO_HPOS,
} HPos;

class Electron;

/*!\class Atom
Represents atoms in GChemPaint.
*/
class Atom: public gcu::Atom, public gcu::DialogOwner
{
public:
/*!
Default construtor.
*/
	Atom ();
/*!
@param Z the atomic number.
@param x the x coordinate.
@param y the y coordinate.
@param z the z coordinate.


*/
	Atom (int Z, double x, double y, double z);
/*!
@param


*/
	Atom (OpenBabel::OBAtom* atom);
/*!
The destructor.
*/
	virtual ~Atom ();

public :
/*!
@param Z the new atomic number.


*/
	virtual void SetZ (int Z);
/*!

*/
	void AddBond (gcu::Bond* pBond);
	void RemoveBond (gcu::Bond* pBond);
	virtual void Update ();
	virtual void Add (GtkWidget* w) const;
	virtual void Update (GtkWidget* w) const;
	int GetTotalBondsNumber () const; //take bond order into account
	int GetAttachedHydrogens () const {return m_nH;}
	HPos GetBestSide ();
	virtual int GetChargePosition (unsigned char& Pos, double Angle, double& x, double& y);
	virtual int GetAvailablePosition (double& x, double& y);
	virtual bool GetPosition (double angle, double& x, double& y);
	virtual xmlNodePtr Save (xmlDocPtr xml) const;
	virtual bool Load (xmlNodePtr);
	virtual bool LoadNode (xmlNodePtr);
	virtual void SetSelected (GtkWidget* w, int state);
	virtual bool AcceptNewBonds (int nb = 1);
	virtual bool AcceptCharge (int charge);
	virtual double GetYAlign ();
	virtual void Transform2D (gcu::Matrix2D& m, double x, double y);
	bool BuildContextualMenu (GtkUIManager *UIManager, Object *object, double x, double y);
	/*!
	@param Mol: a pointer to a molecule

	Adds the atom to the molecule calling gcpMolecule::AddAtom()
	*/
	virtual void AddToMolecule (Molecule* Mol);
	bool HasImplicitElectronPairs ();
	bool MayHaveImplicitUnpairedElectrons ();
	/*!
	@param electron: a pointer to an Electron instance.

	Adds the Electron (representing either a single electron or a pair) to the Atom.
	*/
	void AddElectron (Electron* electron);
	/*!
	@param electron: a pointer to an Electron instance.

	Removes the Electron (representing either a single electron or a pair) from the Atom.
	*/
	void RemoveElectron (Electron* electron);
	void NotifyPositionOccupation (unsigned char pos, bool occupied);
	void SetChargePosition (unsigned char Pos, bool def, double angle = 0., double distance = 0.);
	char GetChargePosition (double *Angle, double *Dist) const;
	void SetCharge (int charge);
	int GetCharge () const {return m_Charge;}
	void ForceChanged () {m_Changed = true;}
	bool Match (gcu::Atom *atom, gcu::AtomMatchState &state);

/*!
@param width where to store the width.
@param height where to store the height.
@param angle where to store the limit angle.
@param up whether considering the top half or the bottom half

Used to retrieve the size of the ink rectangle of the atom symbol (if displayed).
\a angle is absolute value of the angle between an horizontal line and the line joining
the center and the top left or the bottom left vertex.
The returned width value is actually half the full width. Height is the height.
This method is used to avoid bonds lines extyending over their atoms symbols.
*/
	void GetSymbolGeometry (double &width, double &height, double &angle, bool up) const;

protected:
	virtual void BuildSymbolGeometry (double width, double height, double ascent);

private:
	void BuildItems (WidgetData* pData);
	void UpdateAvailablePositions ();

private:
	gcu::Element *m_Element;
	int m_nH;
	int m_Valence; //valence
	int m_ValenceOrbitals;
	int m_nlp; //lone electron pairs number
	int m_nlu; //single electrons number
	double m_width, m_height; //size of the atomic symbol in the canvas
	double m_length, m_text_height; // size of the text buffer
	HPos m_HPos; //0 = left, 1 = right, 2 = top, 3 = bottom, 4 = auto
	bool m_ChargeAuto;
	int m_Changed; //update needs regenerate the buffer
	int m_ascent;
	double m_lbearing;
	unsigned char m_AvailPos; //available standard positions for charge and electrons representations
	unsigned char m_OccupiedPos;
	bool m_AvailPosCached;
	unsigned char m_ChargePos;
	bool m_ChargeAutoPos;
	double m_ChargeAngle;
	double m_ChargeDist;
	double m_ChargeWidth, m_ChargeTWidth, m_ChargeXOffset, m_ChargeYOffset;
	std::list<double> m_AngleList;
	std::map<double, double> m_InterBonds; /* positions betwen bonds. First  value is the
	angle between the two bonds and second value is the direction */
	PangoLayout *m_Layout, *m_ChargeLayout, *m_HLayout;
	double m_xHOffs, m_yHOffs;
	bool m_DrawCircle;
	std::string m_FontName;
	double m_SWidth, m_SHeightH, m_SHeightL, m_SAngleH, m_SAngleL;
	// special offset for underlying rectangle; will be removed in next version
	double m_xROffs, m_yROffs;

protected:
	double m_CHeight;

GCU_PROP (bool, ShowSymbol)
GCU_PROP (HPos, HPosStyle) //0=force left, 1=force right, 2=force top, 3=force bottom, 4=auto.
};

}	//	namespace gcp

#endif // GCHEMPAINT_ATOM_H
