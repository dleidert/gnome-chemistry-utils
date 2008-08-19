// -*- C++ -*-

/* 
 * GChemPaint library
 * electron.h
 *
 * Copyright (C) 2004-2008 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_ELECTRON_H
#define GCHEMPAINT_ELECTRON_H

#include <gcu/object.h>

/*!\file*/
namespace gcp {

class Atom;

/*!
The dynamic TypeId for electrons.
*/
extern gcu::TypeId ElectronType;

/*!\class Electron gcp/electron.h
Represents either single elecgtrons or electrons pairs.
*/
class Electron: public gcu::Object
{
public:
/*!
@param pAtom the Atom to which the electrons belong.
@param IsPair true for an electron pair and false for a single electron.

Constructs a new electron representation.
*/
	Electron (Atom *pAtom, bool IsPair);
/*!
The destructor
*/
	~Electron ();

/*!
@return true for an electron pair and false for a single electron.
*/
	bool IsPair () {return m_IsPair;}
/*!
@param angle where to store the angle from east direction in the trigonometric convention.
@param distance where to store the distance from the center of the atom.

@return the electron position relative to its parent atom as one of
POSITION_E, POSITION_N,...
*/
	char GetPosition (double *angle, double *distance);
/*!
@param Pos one of POSITION_E, POSITION_N,...
@param angle the angle from the east direction in the trigonometric convention.
@param distance the distance from the center of the atom, or 0. if automatic.

Sets the position of an electronn relative to its parent atom.
*/
	void SetPosition (unsigned char Pos, double angle = 0., double distance = 0.);
/*!
@param w the GtkWidget inside which the Electron will be displayed.

Used to add a representation of the Electron in the widget.
*/
	void Add(GtkWidget* w) const;
/*!
@param w the GtkWidget inside which the Electron is displayed.

Used to update the representation of the Electron in the widget.
*/
	void Update(GtkWidget* w) const;
/*!
@param w the GtkWidget inside which the Electron is displayed.
@param state the selection state of the Electron.

Used to set the selection state of the Electron inside the widget.
The values of state might be gcp::SelStateUnselected, gcp::SelStateSelected,
gcp::SelStateUpdating, or gcp::SelStateErasing.
*/
	void SetSelected(GtkWidget* w, int state);
/*!
@param xml the xmlDoc used to save the document.

Used to save the Electron to the xmlDoc.
@return the xmlNode containing the serialized Electron.
*/
	xmlNodePtr Save (xmlDocPtr xml) const;
/*!
@param node a pointer to the xmlNode containing the serialized object.

Used to load an Electron in memory. The Electron must already exist.
@return true on succes, false otherwise.
*/
	bool Load (xmlNodePtr node);
/*!
@param Signal the appropriate SignalId
@param Child the child which emitted the signal or NULL.

This function is called by the framework when a signal has been emitted for
the object. Elecgtron don't have children, so that \a Child will be ignored.
Only the gcp::OnDeleteSignal is significant for this class.
It should not be called by a program; call Object::EmitSignal instead.

@return true if the signal should be propagated to the parent, false otherwise.
*/
	bool OnSignal (gcu::SignalId Signal, gcu::Object *Child);
/*!
@param m the Matrix2D of the transformation.
@param x the x component of the center of the transformation.
@param y the y component of the center of the transformation.

Used to move and/or transform an Electron.
*/
	void Transform2D (gcu::Matrix2D& m, double x, double y);

private:
	Atom* m_pAtom;
	bool m_IsPair;
	unsigned char m_Pos;
	double m_Angle, m_Dist;
};

}	//	namespace gcp

#endif	// GCHEMPAINT_ELECTRON_H
