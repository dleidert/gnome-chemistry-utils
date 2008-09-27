// -*- C++ -*-

/* 
 * GChemPaint library
 * text.h 
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
	Text ();
	Text (double x, double y);
	virtual ~Text ();
	
	void GetCoords (double *x, double *y);
	void SetCoords (double x, double y);
	xmlNodePtr Save (xmlDocPtr xml) const;
/*!
@param xml the xmlDoc used for clipboard operations.

Saves the currently selected text. This method is used by
the framework when editing the textual object.
@return the xmlNode containing the serialized selection.
*/
	xmlNodePtr SaveSelection (xmlDocPtr xml) const;
	bool Load (xmlNodePtr);
	bool LoadSelection (xmlNodePtr node, unsigned pos);
	bool LoadNode (xmlNodePtr, unsigned &pos, int level = 0, int cur_size = 0);
	void Add (GtkWidget* w) const;
	void Update (GtkWidget* w) const;
	void SetSelected (GtkWidget* w, int state);
/*!
@param save whether the text should be saved for undo/redo operations.

Called after any change in the text.
*/
	bool OnChanged (bool save);
	void Transform2D (gcu::Matrix2D& m, double x, double y);
	bool OnEvent (GdkEvent *event);
	void GetSize (double& x, double& y) {x = m_length; y = m_height;}
/*!
@return the y coordinate at half height of a carbon atom symbol if any was
present at default size so that all texts and chemicalk symbols will have the
same base line when aligned.
*/
	double GetYAlign ();
	void SetText (char const *text) {m_buf = text;}
/*!
@param property the property id as defined in objprops.h
@param value the property value as a string

Used when loading to set properties for the fragment. This method supports
GCU_PROP_POS2D, GCU_PROP_TEXT_MARKUP, GCU_PROP_TEXT_TEXT, GCU_PROP_TEXT_ALIGNMENT
and GCU_PROP_TEXT_JUSTIFICATION.
@return true if the property could be set, or if the property is not relevant, false otherwise.
*/
	bool SetProperty (unsigned property, char const *value);

GCU_PROP (PangoAlignment, Align)
GCU_PROP (bool, Justified)
GCU_PROP (GtkAnchorType, Anchor)
};

}	//	namespace gcp

#endif	//GCHEMPAINT_TEXT_H
