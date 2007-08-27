// -*- C++ -*-

/* 
 * GChemPaint library
 * mesomery.h 
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

#ifndef GCHEMPAINT_MESOMERY_H
#define GCHEMPAINT_MESOMERY_H

#include <gcu/object.h>

using namespace gcu;

namespace gcp {

class Mesomer;

class Mesomery: public Object
{
public:
	Mesomery ();
	virtual ~Mesomery ();
	
	virtual bool Load (xmlNodePtr);
	virtual bool Build (list<Object*>& Children) throw (invalid_argument);
	virtual void Transform2D (Matrix2D& m, double x, double y);
	virtual bool BuildContextualMenu (GtkUIManager *UIManager, Object *object, double x, double y);
	virtual bool OnSignal (SignalId Signal, Object *Child);
	bool Validate (bool split);
	void Align ();
	virtual double GetYAlign ();

private:
	Mesomery (Object* parent, Mesomer *mesomer);
};

}	//	namespace gcp

#endif	//GCHEMPAINT_MESOMERY_H