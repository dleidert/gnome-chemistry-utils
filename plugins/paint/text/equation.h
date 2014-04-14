// -*- C++ -*-

/*
 * GChemPaint text plugin: math equation support
 * equation.h
 *
 * Copyright (C) 2014 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_EQUATION_H
#define GCHEMPAINT_EQUATION_H

#include <gcu/dialog-owner.h>
#include <gcu/object.h>
#include <gccv/item-client.h>
#include <lsmdom.h>

extern gcu::TypeId EquationType;

class gcpEquation: public gcu::Object, public gcu::DialogOwner, public gccv::ItemClient
{
public:
	gcpEquation (double x, double y);
	virtual ~gcpEquation ();

	void AddItem ();
	void UpdateItem ();
	xmlNodePtr Save (xmlDocPtr xml) const;
	bool Load (xmlNodePtr node);
	void SetSelected (int state);
	std::string Name ();
	char const *HasPropertiesDialog () const;
/*!
@param x the horizontal translation.
@param y the vertical translation.
@param z the depth translation.

The z variable is not useful.
*/
	void Move (double x, double y, double z = 0);

	void SetFontDesc (PangoFontDescription const *desc);
	void ItexChanged (char const *itex, bool compact);
	bool IsEmpty () { return m_Itex.length () == 0; }
	std::string const &GetITeX () { return m_Itex; }

protected:
	gcu::Dialog *BuildPropertiesDialog ();

private:
	double m_x, m_y;
	std::string m_Itex;
	LsmDomDocument *m_Math;
	LsmDomNode *m_ItexString;
	LsmDomNode *m_StyleElement;

GCU_PROP (PangoFontDescription const *, Font)
GCU_PROP (GOColor, Color)
GCU_PROP (bool, Inline)
};

#endif  // GCHEMPAINT_EQUATION_H
