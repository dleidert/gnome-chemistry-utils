// -*- C++ -*-

/* 
 * GChemPaint libray
 * text-object.h 
 *
 * Copyright (C) 2002-2007 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_TEXT_OBJECT_H
#define GCHEMPAINT_TEXT_OBJECT_H

#include <gcu/object.h>
#include <gcu/macros.h>
#include <gccv/structs.h>
#include <pango/pango-layout.h>
#include <string>

/*!\file*/
namespace gcp {

/*!\class TextObject gcp/text-object.h

The base class for texts. This class is virtual with one pure virtual method
(gcp::TextObject::OnSave).
*/
class TextObject: public gcu::Object
{
public:
/*!
@param Type the TypeId of the derived class.

The default constructor.
*/
	TextObject (gcu::TypeId Type);
/*!
@param Type the TypeId of the derived class.
@param x the horizontal poisition.
@param y the vertical poisition.

Constructs and sets the position of the text object.
*/
	TextObject (double x, double y, gcu::TypeId Type);
/*!
The destructor.
*/
	virtual ~TextObject ();

/*!
@param x the variable to be set to the width value.
@param y the variable to be set to the height value.

Retrieves the size of the text in canvas coordinates.
*/
	void GetSize (double& x, double& y) {x = m_length; y = m_height;}
/*!
Saves the selection.
@return the xml node representing the selection.
*/
	xmlNodePtr SaveSelected ();
/*!
@param node the xml node representing the text to import.

Replaces the contents of the text object by what is represented by the node.
*/
	void LoadSelected (xmlNodePtr node);
/*!
@param save whether the text should be saved for undo/redo operations.

Must be called after any change in the text. This method is pure virtual and
must be overloaded in derived classes.
*/
	virtual bool OnChanged (bool save) = 0;
/*!
@param bounds the bounds of the selection.

Called during edition when the selection has changed, that is after any change.
The signature of this method will change with the deprecation of GnomeCanvas.
*/
	void OnSelChanged (gccv::TextSelBounds *bounds);
/*!
@param xml the xml document.
@param node the node representing the text.

Saves the position and Id. Called by derived classes when saving if meaningful.
*/
	bool SaveNode (xmlDocPtr xml, xmlNodePtr node) const;
/*!
@param node the xml node representing the text.

Loads the position and Id.
*/
	bool Load (xmlNodePtr node);
/*!
@param x the horizontal translation.
@param y the vertical translation.
@param z the depth translation.

The z variable is not useful.
*/
	void Move (double x, double y, double z = 0);
/*!
This is used because updates are not needed while loading util the whole
text has been loaded.

@return true while loading, false otherwise.
*/
	bool IsLocked () {return m_bLoading;}
/*!
@param start where to store the start of the selection.
@param end where to store the end of the selection.

The values set as bounds are in bytes.
*/
	void GetSelectionBounds (unsigned &start, unsigned &end) {start = m_StartSel; end = m_EndSel;}
/*!
@return the raw text contained in the object.
*/
	std::string GetBuffer () {return m_buf;}
/*!
@param property the property id as defined in gcu/objprops.h

Used when saving to get properties from a text object. Only on eproperty is
supported: GCU_PROP_TEXT_TEXT
*/
	virtual std::string GetProperty (unsigned property) const;

protected:
/*!
The horizontal of the object in canvas units.
*/
	double m_x;
/*!
The vertical of the object in canvas units.
*/
	double m_y;
/*!
The width of the object in canvas units.
*/
	double m_length;
/*!
The height of the object in canvas units.
*/
	double m_height;
/*!
The ascent of the text.
*/
	int m_ascent;
/*!
The current insertionposition.
*/
	int m_InsertOffset;
/*!
The text owned byt the object.
*/
	std::string m_buf;
/*!
treu on loading, false otherwise.
*/
	bool m_bLoading;
/*!
The index in bytes of the start of the selection.
*/
	unsigned m_StartSel;
/*!
The index in bytes of the end of the selection.
*/
	unsigned m_EndSel;
/*!
Tells whether the Save method is called for the whole text or just
the selection.
*/
	bool m_RealSave;

/*!\var m_Layout
The PangoLayout associated with the object.
*/
/*!\fn GetLayout
@return the PangoLayout for this object.
*/
GCU_PROT_PROP (PangoLayout*, Layout);
/*!\var m_AttrList
The PangoAttrList associated to the text.
*/
/*!\fn GetAttrList
@return the current attributes list.
*/
GCU_PROT_PROP (PangoAttrList*, AttrList);
/*!\var m_Anchor
The GtkAnchor used for the text object. Default is GTK_ANCHOR_W. When
GTK_ANCHOR_W, GTK_ANCHOR_CENTER, or GTK_ANCHOR_E is used, the base of the
first line is used for vertical alignment.
*/
/*!\fn GetAnchor()
@return the current GtkAnchor.
*/
GCU_PROT_PROP (GtkAnchorType, Anchor);
};

}	// namespace gcp

#endif	//GCHEMPAINT_TEXT_OBJECT_H
