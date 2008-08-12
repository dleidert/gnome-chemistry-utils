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
@param atom an OpenBabel Atom instance.

Builds an atom importing as many properties from an existing OpenBabel
Atom instance.
*/
	Atom (OpenBabel::OBAtom* atom);
/*!
The destructor.
*/
	virtual ~Atom ();

public :
/*!
@param Z the new atomic number.

Changes the atomic number of the atom.
*/
	virtual void SetZ (int Z);
/*!
@param pBond a bond.

Adds a bond to the atom.
*/
	void AddBond (gcu::Bond* pBond);
/*!
@param pBond a bond.

Removes a bond from the atom.
*/
	void RemoveBond (gcu::Bond* pBond);
/*!
Updates the atom after changing its bonds, charge or explicit electrons.
*/
	virtual void Update ();
/*!
@param w a GtkWidget.

Adds the representation of the atom to the canvas widget.
*/
	virtual void Add (GtkWidget* w) const;
/*!
@param w a GtkWidget.

Updates the representation of the atom in the canvas widget.
*/
	virtual void Update (GtkWidget* w) const;
/*!
@return the bonds number for this atom taking bond order into account
*/
	int GetTotalBondsNumber () const; //take bond order into account
/*!
@return the number of implicit hydrogens lnked to the atom.
*/
	int GetAttachedHydrogens () const {return m_nH;}
/*!
@return the position of the attached hydrogen atoms symbol when automatically
arranged.
*/
	HPos GetBestSide ();
/*!
@param Pos the approximate position of the charge.
@param Angle the angle from horizontal left.
@param x the x position of the charge symbol.
@param y the y position of the charge symbol.

On input \a Pos can be one of POSITION_E, POSITION_N,... or 0xff, in which case,
it will be given a default value. \a x and \a y are set to the position where the charge
sign should be displayed usding the alignment code returned by this method.
@return a number to set how the charge symbol should be aligned relative to its
position. Possible values are:
- −2: center top.
- −1: right.
-  0: center.
-  1: left.
-  2: center bottom.
*/
	virtual int GetChargePosition (unsigned char& Pos, double Angle, double& x, double& y);
/*!
@param x the x position.
@param y the y position.

This method finds an available position for drawing a charge sign or electrons and returns
it as a symbolic value (see POSITION_E, POSITION_N,...). The \a x and \a y are updated so
that they give the absolute position.
@return an available position.
*/
	virtual int GetAvailablePosition (double& x, double& y);
/*!
@param angle the angle at which a charge sign or an eletcron should be displayed.
@param x the x position.
@param y the y position.

Updates \a x and \a y so that they become the absolute position corresponding to the angle
when the position is available.
@return true on success, false otherwise.
*/
	virtual bool GetPosition (double angle, double& x, double& y);
/*!
@param xml the xmlDoc used to save the document.

Used to save the Atom to the xmlDoc.
@return the xmlNode containing the serialized atom.
*/
	virtual xmlNodePtr Save (xmlDocPtr xml) const;
/*!
@param node a pointer to the xmlNode containing the serialized object.

Used to load an atom in memory. The Atom must already exist.
@return true on succes, false otherwise.
*/
	virtual bool Load (xmlNodePtr node);
/*!
@param node a pointer to the xmlNode containing the serialized Atom.

Used in this class to correctly set the atomic number.
*/
	virtual bool LoadNode (xmlNodePtr node);
/*!
@param w the GtkWidget inside which the atom is displayed.
@param state the selection state of the atom.

Used to set the selection state of the atom inside the widget.
The values of state might be gcp::SelStateUnselected, gcp::SelStateSelected,
gcp::SelStateUpdating, or gcp::SelStateErasing.
*/
	virtual void SetSelected (GtkWidget* w, int state);
/*!
@param nb the number of bonds to add, taking orders into account.
@return true if the operation is allowed, false if the new bonds would exceed
the maximum valence for the element.
*/
	virtual bool AcceptNewBonds (int nb = 1);
/*!
@param charge the charge that might be set.

@return true if the charge is acceptable.
*/
	virtual bool AcceptCharge (int charge);
/*!
Used to retrieve the y coordinate for alignment. The default implementation returns 0.0 and
every derived class for which alignment has a meaning should implement this method.
@return y coordinate used for objects alignment.
*/
	virtual double GetYAlign ();
/*!
@param m the Matrix2D of the transformation.
@param x the x component of the center of the transformation.
@param y the y component of the center of the transformation.

Used to move and/or transform an object.
*/
	virtual void Transform2D (gcu::Matrix2D& m, double x, double y);
/*!
@param UIManager: the GtkUI%anager to populate.
@param object the atom on which occured the mouse click.
@param x x coordinate of the mouse click.
@param y y coordinate of the mouse click.

This method is called to build a contextual menu for the atom.
*/
	bool BuildContextualMenu (GtkUIManager *UIManager, Object *object, double x, double y);
/*!
@param Mol: a pointer to a molecule

Adds the atom to the molecule calling gcpMolecule::AddAtom()
*/
	virtual void AddToMolecule (Molecule* Mol);
/*!
@return true if the atom has implicit electron pairs, false otherwise.
*/
	bool HasImplicitElectronPairs ();
/*!
@return true if the atom has implcit electrons that might be unpaired.
*/
	bool MayHaveImplicitUnpairedElectrons ();
/*!
@param electron a pointer to an Electron instance.

Adds the Electron (representing either a single electron or a pair) to the Atom.
*/
	void AddElectron (Electron* electron);
/*!
@param electron a pointer to an Electron instance.

Removes the Electron (representing either a single electron or a pair) from the Atom.
*/
	void RemoveElectron (Electron* electron);
/*!
@param pos one of POSITION_E, POSITION_N,...
@param occupied true if occupied, false otherwise.

Notifies if a position is occupied or not.
*/
	void NotifyPositionOccupation (unsigned char pos, bool occupied);
/*!
@param Pos one of POSITION_E, POSITION_N,...
@param def true if the position is automatic.
@param angle the angle from the east direction in the trigonometric convention.
@param distance the distance from the center of the atom, or 0. if automatic.

Sets the relative position of a charge sign.
*/
	void SetChargePosition (unsigned char Pos, bool def, double angle = 0., double distance = 0.);
/*!
@param Angle where to store the angle from east direction in the trigonometric convention.
@param Dist where to store the distance from the center of the atom.

@return the charge position as one of POSITION_E, POSITION_N,...
*/
	char GetChargePosition (double *Angle, double *Dist) const;
/*!
@param charge the charge to set.

Sets the formal local charge of an atom.
*/
	void SetCharge (int charge);
/*!
@return the current formal local charge.
*/
	int GetCharge () const {return m_Charge;}
/*!
Forces an update.
*/
	void ForceChanged () {m_Changed = true;}
/*!
@param atom the atom to which the this instance is to be compared.
@param state the AtomMatchState representing the current comparison state.

Try to match atoms from two molecules which are compared. This function calls
itself recursively until all atoms from the two molecules have been matched or
until an difference is found. Overriden methods should call this base function
and return its result.
@return true if the atoms match, false otherwise.
*/
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
/*!
@param width the witdh of the atomic symbol.
@param height the height of the atomic symbol.
@param ascent the ascent of the atomic symbol.

Evaluates where lines representing bonds should end to not overload the symbol.
*/
	void BuildSymbolGeometry (double width, double height, double ascent);

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
/*!
Half the height of the "C" character.
*/
	double m_CHeight;

/*!\fn SetShowSymbol(bool ShowSymbol)
@param ShowSymbol whether the symbol of a carbon atom is to be displayed or not.

Sets the visibility of a carbon atom symbol in a chain.
*/
/*!\fn GetShowSymbol()
@return whether the symbol of a carbon atom is displayed or not.
*/
/*!\fn GetRefShowSymbol()
@return whether the symbol of a carbon atom is displayed or not as a reference.
*/
GCU_PROP (bool, ShowSymbol)

/*!\fn SetHPosStyle(HPos val)
@param val the new position.

Sets the position of attached hydrogen atoms symbol.
*/
/*!\fn GetHPosStyle()
@return the position of attached hydrogen atoms symbol.
*/
/*!\fn GetRefHPosStyle()
@return the position of attached hydrogen atoms symbol as a reference.
*/
GCU_PROP (HPos, HPosStyle) //0=force left, 1=force right, 2=force top, 3=force bottom, 4=auto.
};

}	//	namespace gcp

#endif // GCHEMPAINT_ATOM_H
