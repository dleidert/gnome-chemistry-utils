// -*- C++ -*-

/* 
 * GChemPaint library
 * fragment.h 
 *
 * Copyright (C) 2002-2008 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_FRAGMENT_H
#define GCHEMPAINT_FRAGMENT_H

#include "text-object.h"
#include "libgnomecanvas/gnome-canvas.h"

/*!\file*/
namespace gcp {

class FragmentAtom;
class Atom;

/*!\class Fragment gcp/fragment.h
\brief Atoms groups.

Represents atoms groups displayed as a string. Currntly, the string is not
fully parsed, so that some non sense strings might be accepted. Anyway, this
will not always be the case in the future.
*/
class Fragment: public TextObject
{
public:
/*!
The default constructor.
*/
	Fragment ();
/*!
@param x the x position of the new fragment.
@param y the y position of the new fragment.

Constructs a new fragment and position it. x and y are the position of the
main atom or residue in the fragment.
*/
	Fragment (double x, double y);
/*!
The destructor.
*/
	virtual ~Fragment ();

/*!
@param w the GtkWidget inside which the fragment is displayed.
@param state the selection state of the fragment.

Used to set the selection state of the fragment inside the widget.
The values of state might be gcp::SelStateUnselected, gcp::SelStateSelected,
gcp::SelStateUpdating, or gcp::SelStateErasing.
*/
	void SetSelected (GtkWidget *w, int state);
/*!
@param w a GtkWidget.

Adds the representation of the fragment to the canvas widget.
*/
	void Add (GtkWidget *w) const;
/*!
@param w a GtkWidget.

Updates the representation of the fragment in the canvas widget.
*/
	void Update (GtkWidget *w) const;
/*!
@param xml the xmlDoc used to save the document.

Used to save the fragment to the xmlDoc.
@return the xmlNode containing the serialized fragment.
*/
	xmlNodePtr Save (xmlDocPtr xml) const;
/*!
@param xml the xmlDoc used for clipboard operations.

Saves the currently selected text inside the fragment. This method is used by
the framework when editing the fragment.
@return the xmlNode containing the serialized selection.
*/
	xmlNodePtr SaveSelection (xmlDocPtr xml) const;
/*!

*/
	bool Load (xmlNodePtr);
/*!

*/
	bool OnChanged (bool save);
/*!

*/
	void AnalContent ();
/*!

*/
	void AnalContent (unsigned start, unsigned &end);
/*!

*/
	Object* GetAtomAt (double x, double y, double z = 0.);
/*!

*/
	void Move (double x, double y, double z = 0);
/*!

*/
	void Transform2D (gcu::Matrix2D& m, double x, double y);
/*!

*/
	void OnChangeAtom ();
/*!

*/
	Atom* GetAtom () {return (Atom*) m_Atom;}
/*!

*/
	int GetElementAtPos (unsigned start, unsigned &end);
/*!

*/
	int GetChargePosition (FragmentAtom *pAtom, unsigned char &Pos, double Angle, double &x, double &y);
/*!

*/
	int GetAvailablePosition (double &x, double &y);
/*!

*/
	bool GetPosition (double angle, double &x, double &y);
/*!

*/
	bool Validate ();
/*!

*/
	double GetYAlign ();

/*!
*/
	bool SetProperty (unsigned property, char const *value);

/*!
*/
	bool Analyze ();

/*!
*/
	void Update ();

private:
	bool SavePortion (xmlDocPtr xml, xmlNodePtr node, unsigned start, unsigned end) const;

private:
	FragmentAtom *m_Atom;
	unsigned m_BeginAtom, m_EndAtom;
	int m_lbearing;
	double m_CHeight;
	bool m_Inversable;
};

}	//	namespace gcp

#endif	//GCHEMPAINT_FRAGMENT_H
