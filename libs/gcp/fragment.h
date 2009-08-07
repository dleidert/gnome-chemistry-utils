// -*- C++ -*-

/* 
 * GChemPaint library
 * fragment.h 
 *
 * Copyright (C) 2002-2009 Jean Bréfort <jean.brefort@normalesup.org>
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
#include <gccv/item-client.h>
#include <gccv/text-tag.h>

namespace gccv {
	class Text;
}

/*!\file*/
namespace gcp {

extern gccv::Tag ChargeTag, StoichiometryTag;

class ChargeTextTag: public gccv::PositionTextTag
{
public:
	ChargeTextTag (double size);
	virtual ~ChargeTextTag ();

	TextTag *Restrict (TextTag *tag);
};

class StoichiometryTextTag: public gccv::PositionTextTag
{
public:
	StoichiometryTextTag (double size);
	virtual ~StoichiometryTextTag ();

	TextTag *Restrict (TextTag *tag);
};

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
Used to add a representation of the fragment in the view.
*/
	void AddItem ();
/*!
Used to update a representation of the fragment in the view.
*/
	void UpdateItem ();
/*!
@param state the selection state of the fragment.

Used to set the selection state of the fragment inside the widget.
The values of state might be gcp::SelStateUnselected, gcp::SelStateSelected,
gcp::SelStateUpdating, or gcp::SelStateErasing.
*/
	void SetSelected (int state);
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
@param node a pointer to the xmlNode containing the serialized fragment.

Used to load a frafment in memory. The Fragment must already exist.
@return true on succes, false otherwise.
*/
	bool Load (xmlNodePtr node);
/*!
@param save whether the text should be saved for undo/redo operations.

Called after any change in the text.
*/
	bool OnChanged (bool save);
/*!
Analyses the whole contents.
*/
	void AnalContent ();
/*!
@param start the start fo the substring to parse, in bytes.
@param end the end fo the substring to parse, in bytes.

Analyses a substring.
*/
	void AnalContent (unsigned start, unsigned &end);
/*!
@param x the x coordinate
@param y the y coordinate
@param z the z coordinate (unused)

@return a pointer to the Atom correpoding to the symbol at or near the position
defined by the coordinates passed as parameters, if any.
*/
	Object* GetAtomAt (double x, double y, double z = 0.);
/*!
@param x the x component of the transation vector.
@param y the y component of the transation vector.
@param z the z component of the transation vector.

Moves the fragment.
*/
	void Move (double x, double y, double z = 0);
/*!
@param m the Matrix2D of the transformation.
@param x the x component of the center of the transformation.
@param y the y component of the center of the transformation.

Moves and/or transform an object.
*/
	void Transform2D (gcu::Matrix2D& m, double x, double y);
/*!
Called by the framework when the user changes the nature of an atom using the
element tool.
*/
	void OnChangeAtom ();
/*!
@return the main atom, which might have a bond.
*/
	Atom* GetAtom () {return (Atom*) m_Atom;}
/*!
@param start the start position of the atomic symbol.
@param end the end position of the atomic symbol.

@return the atomic number corresponding to the symbol starting at \a start, if any, or 0.
\a end is updated accordingly.
*/
	int GetElementAtPos (unsigned start, unsigned &end);
/*!
@param pAtom the main atom which will get the local charge.
@param Pos the approximate position of the charge.
@param Angle the angle from horizontal left.
@param x the x position of the charge symbol.
@param y the y position of the charge symbol.

If \a pAtom is not the main atom of the fragment, 0 is returned and \a Pos
is not updated.
On input \a Pos can be one of POSITION_E, POSITION_N,... or 0xff, in which case,
it will be given a default value. \a x and \a y are set to the position where the charge
sign should be displayed usding the alignment code returned by this method.
@return the anchor for the charge symbol. On error, gccv::AnchorCenter is used as
the returned value.
*/
	gccv::Anchor GetChargePosition (FragmentAtom *pAtom, unsigned char &Pos, double Angle, double &x, double &y);
/*!
@param x the x position.
@param y the y position.

This method finds an available position for drawing a charge sign or electrons and returns
it as a symbolic value (see POSITION_E, POSITION_N,...). The \a x and \a y are updated so
that they give the absolute position.
@return an available position.
*/
	int GetAvailablePosition (double &x, double &y);
/*!
@param angle the angle at which a charge sign should be displayed.
@param x the x position.
@param y the y position.

@return false and do not update the coordinates.
*/
	bool GetPosition (double angle, double &x, double &y);
/*!
Validates the contents of the fragment text, and display error messages when necessary.
*/
	bool Validate ();
/*!
@return the y coordinate at half height of a carbon atom symbol if any was present.
*/
	double GetYAlign ();

/*!
@param property the property id as defined in objprops.h
@param value the property value as a string

Used when loading to set properties for the fragment. This method supports
GCU_PROP_POS2D, GCU_PROP_TEXT_TEXT, GCU_PROP_FRAGMENT_ATOM_START,
and GCU_PROP_FRAGMENT_ATOM_ID.
@return true if the property could be set, or if the property is not relevant, false otherwise.
*/
	bool SetProperty (unsigned property, char const *value);

/*!
Analyses the text in the fragment. This calls gcp::Fragment::AnalContent()
and updates the attribute list.
*/
	bool Analyze ();

/*!
Changes the order of the symbols if necessary when a bond is at an extremity
of the fragment.
*/
	void Update ();
	gccv::Item *GetChargeItem ();

	typedef enum {
		Invalid,
		Valid,
		Valid2D,
		Valid3D
	} Validity;

	typedef enum {
		AutoMode,
		NormalMode,
		SubscriptMode,
		SuperscriptMode,
		ChargeMode,
		StoichiometryMode
	} FragmentMode;

private:
	bool SavePortion (xmlDocPtr xml, xmlNodePtr node, unsigned start, unsigned end) const;

private:
	FragmentAtom *m_Atom;
	unsigned m_BeginAtom, m_EndAtom;
	double m_lbearing;
	double m_CHeight;
	bool m_Inversable;

GCU_RO_PROP (gccv::Text *, TextItem)
GCU_RO_PROP (Validity, Valid)
GCU_PROP (FragmentMode, Mode)
};

}	//	namespace gcp

#endif	//GCHEMPAINT_FRAGMENT_H
