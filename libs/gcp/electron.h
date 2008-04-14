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

namespace gcp {

class Atom;

extern gcu::TypeId ElectronType;

class Electron: public gcu::Object
{
public:
	Electron (Atom *pAtom, bool IsPair);
	~Electron ();

	bool IsPair () {return m_IsPair;}
	char GetPosition (double *angle, double *distance);
	void SetPosition (unsigned char Pos, double angle = 0., double distance = 0.);
	virtual void Add(GtkWidget* w) const;
	virtual void Update(GtkWidget* w) const;
	virtual void SetSelected(GtkWidget* w, int state);
	virtual xmlNodePtr Save (xmlDocPtr xml) const;
	virtual bool Load (xmlNodePtr);
	virtual bool OnSignal (gcu::SignalId Signal, gcu::Object *Child);
	virtual void Transform2D (gcu::Matrix2D& m, double x, double y);

private:
	Atom* m_pAtom;
	bool m_IsPair;
	unsigned char m_Pos;
	double m_Angle, m_Dist;
};

}	//	namespace gcp

#endif	// GCHEMPAINT_ELECTRON_H
