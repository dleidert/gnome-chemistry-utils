// -*- C++ -*-

/*
 * GChemPaint library
 * text.h
 *
 * Copyright (C) 2002-2012 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_TEXT_H
#define GCHEMPAINT_TEXT_H

#include <gcu/macros.h>
#include <gcu/object.h>
#include <libxml/tree.h>
#include <gtk/gtk.h>
#include "text-object.h"

/*!\file*/
namespace gcp {

/*!\class Text
The text objects in GChemPaint, excluding atomic symbols. Using this class to
represent chemical objects looses the chemical significance.
*/
class Text: public TextObject
{
public:
/*!
The default constructor for an empty and unpositioned text.
*/
	Text ();
/*!
@param x the x coordinate.
@param y the y coordinate.

Constructs a new empty text positioned according to the given coordiantes
*/
	Text (double x, double y);
/*!
@param tag a gccv::Tag to apply to the whole text.
@param x the x coordinate.
@param y the y coordinate.

Constructs a new empty text positioned according to the given coordiantes
*/
	Text (gccv::Tag tag, double x, double y);
/*!
The destructor.
*/
	virtual ~Text ();

/*!
@param x a pointer to the double value which will receive the x coordinate of the text.
@param y a pointer to the double value which will receive the y coordinate of the text.

Retrieves the coordinates of this text.
*/
	void GetCoords (double *x, double *y);
/*!
@param x the new x coordinate of the text.
@param y the new y coordinate of the text.

Changes the position of this text.
*/
	void SetCoords (double x, double y);
/*!
@param xml the xmlDoc used to save the document.
@return a pointer to the xmlNode representing this text or NULL if an error occured.
*/
	xmlNodePtr Save (xmlDocPtr xml) const;
/*!
@param xml the xmlDoc used for clipboard operations.

Saves the currently selected text. This method is used by
the framework when editing the textual object.
@return the xmlNode containing the serialized selection.
*/
	xmlNodePtr SaveSelection (xmlDocPtr xml) const;
/*!
@param node the xml node representing the text.

Loads the text in memory.
*/
	bool Load (xmlNodePtr node);
/*!
@param node the xml node representing the text to insert.
@param pos where to insert the new text.

Inserts a text inside an existing instance during a paste operation.
*/
	bool LoadSelection (xmlNodePtr node, unsigned pos);
/*!
@param node the xml node representing a portion of the text.
@param pos where to insert the new text.
@param cur_size the current size in bytes.

Loads a portion of a text with a unique set of attributes. \a pos and
\a cur_size are updated.
*/
	bool LoadNode (xmlNodePtr node, unsigned &pos, int cur_size = 0);
/*!
Used to add a representation of the text in the view.
*/
	void AddItem ();
/*!
Used to update the representation of the text in the view.
*/
	void UpdateItem ();
/*!
@param state the selection state of the text.

Used to set the selection state of text inside.
The values of state might be gcp::SelStateUnselected, gcp::SelStateSelected,
gcp::SelStateUpdating, or gcp::SelStateErasing.
*/
	void SetSelected (int state);
/*!
@param save whether the text should be saved for undo/redo operations.

Called after any change in the text.
*/
	bool OnChanged (bool save);
/*!
@param m the Matrix2D of the transformation.
@param x the x component of the center of the transformation.
@param y the y component of the center of the transformation.

Used to move and/or transform a text object.
Text rotation is not currently supported. The text which just be moved after
transformation of its coordinates.
*/
	void Transform2D (gcu::Matrix2D& m, double x, double y);
/*!
@param x where to store the width of the text.
@param y where to store the height of the text.

The sizes are given in canvas coordinates.
*/
	void GetSize (double& x, double& y) {x = m_length; y = m_height;}
/*!
@return the y coordinate at half height of a carbon atom symbol if any was
present at default size so that all texts and chemicalk symbols will have the
same base line when aligned.
*/
	double GetYAlign () const;
/*!
@param text the text to display.

Sets the text for this instance. The text will be displayed using the default
font settings.
*/
	void SetText (char const *text) {m_buf = text;}
/*!
@param property the property id as defined in objprops.h

Used when saving to get properties from various objects. All classes
supporting the mechanism must overload this method.
*/
	std::string GetProperty (unsigned property) const;
/*!
@param property the property id as defined in objprops.h
@param value the property value as a string

Used when loading to set properties for the fragment. This method supports
GCU_PROP_POS2D, GCU_PROP_TEXT_MARKUP, GCU_PROP_TEXT_TEXT, GCU_PROP_TEXT_ALIGNMENT
and GCU_PROP_TEXT_JUSTIFICATION.
@return true if the property could be set, or if the property is not relevant, false otherwise.
*/
	bool SetProperty (unsigned property, char const *value);
/*!
@param interline the spacing.

Sets the Text interline spacing expressed as points.
*/
	void InterlineChanged (double interline);
/*!
@param justification a GtkJustification.

Sets the justification for the Text. Only meaningful for multiline text.
*/
	void JustificationChanged (GtkJustification justification);

/*!
@return the localized object generic name.
*/
	std::string Name ();

	/*!
@param x where to store the x coordinate of the center of the text.
@param y where to store the y coordinate of the center of the text.
@param z where to store the z coordinate of the center of the text or NULL.

Retrieves the current coordinates of the operator. Aways set \a z to 0.
@return true if successful and false if an error occurs (if x or y is NULL).
*/
	bool GetCoords (double* x, double* y, double *z = NULL) const;

/*!\fn SetAnchor(gccv::Anchor Anchor)
@param Anchor the new gccv::Anchor for the text.

Sets the gccv::Anchor used for this text. Default is GTK_ANCHOR_W. The vertical
alignment being based on the base line with an offset equal to the half height of
a carbon atom symbol using the current document theme.
*/
/*!\fn GetAnchor()
@return the gccv::Anchor used for the text.
*/
	/*!fn GetRefAnchor()
@return the gccv::Anchor used for the text as a reference.
*/
GCU_PROP (gccv::Anchor, Anchor)

/*!\fn GetGlobalTag()
@return the global gccv::TextTag applied to this text or gccv::Invalid if none
has been explicitly set in the constructor.
*/
GCU_RO_PROP (gccv::Tag, GlobalTag)

private:
	double m_Interline;
	GtkJustification m_Justification;
};

}	//	namespace gcp

#endif	//GCHEMPAINT_TEXT_H
