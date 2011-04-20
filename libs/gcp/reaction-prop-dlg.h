// -*- C++ -*-

/* 
 * GChemPaint library
 * reaction-prop-dlg.h
 *
 * Copyright (C) 2007-2011 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_REACTION_PROP_DLG_H
#define GCHEMPAINT_REACTION_PROP_DLG_H

#include <gcugtk/dialog.h>
#include <gcu/object.h>
#include <gcu/macros.h>

/*!\file*/
namespace gcp {

class ReactionArrow;

class ReactionProp;

/*!\class  ReactionPropDlg gcp/reaction-prop-dlg.h
\brief reaction properties dialog box class.

The class for the dialog used to set the properties of objects attached to
reaction arrows.
*/
class ReactionPropDlg: public gcugtk::Dialog
{
public:
/*!
@param arrow the reaction arrow to which the object is attached.
@param prop the associated reaction property.

Builds and shows the reaction properties dialog box for \a arrow and \a prop.
*/
	ReactionPropDlg (ReactionArrow *arrow, ReactionProp *prop);
/*!
The destructor.
*/
	virtual ~ReactionPropDlg ();

private:
	ReactionArrow *m_Arrow;
	ReactionProp *m_Prop;
};

}	//	namespace gcp

#endif	//	GCHEMPAINT_REACTION_PROP_H

