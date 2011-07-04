// -*- C++ -*-

/*
 * GChemPaint atoms plugin
 * orbital.h
 *
 * Copyright (C) 2009-2011 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_ORBITAL_H
#define GCHEMPAINT_ORBITAL_H

#include <gcu/dialog-owner.h>
#include <gcu/object.h>
#include <gccv/item-client.h>

namespace gcp {
	class Atom;
};

namespace gcu {
	class Dialog;
	class UIManager;
};

extern gcu::TypeId OrbitalType;

typedef enum {
	GCP_ORBITAL_TYPE_S,
	GCP_ORBITAL_TYPE_P,
	GCP_ORBITAL_TYPE_DXY,
	GCP_ORBITAL_TYPE_DZ2,
	GCP_ORBITAL_INVALID
} gcpOrbitalType;

#define GCP_ORBITAL_P_WIDTH 0.8
#define GCP_ORBITAL_D_WIDTH 0.6
#define GCP_ORBITAL_DZ2_FACTOR .3

class gcpOrbital: public gcu::Object, public gcu::DialogOwner, public gccv::ItemClient
{
public:
	gcpOrbital (gcp::Atom *parent, gcpOrbitalType type);
	virtual ~gcpOrbital ();

	void AddItem ();
	xmlNodePtr Save (xmlDocPtr xml) const;
	bool Load (xmlNodePtr node);
	void SetSelected (int state);
	std::string Name ();
	bool BuildContextualMenu (gcu::UIManager *UIManager, gcu::Object *object, double x, double y);
	char const *HasPropertiesDialog () const;
	gcu::Dialog *BuildPropertiesDialog ();

private:
	gcp::Atom *m_Atom;

GCU_PROP (gcpOrbitalType, Type);
GCU_PROP (double, Coef);
GCU_PROP (double, Rotation);
};

#endif	//	GCHEMPAINT_ORBITAL_H
