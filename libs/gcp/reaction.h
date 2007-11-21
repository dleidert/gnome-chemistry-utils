// -*- C++ -*-

/* 
 * GChemPaint library
 * reaction.h 
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

#ifndef GCHEMPAINT_REACTION_H
#define GCHEMPAINT_REACTION_H

#include <gcu/object.h>
#include <stdexcept>

namespace gcp {

class Reaction: public gcu::Object
{
public:
	Reaction ();
	virtual ~Reaction ();
	
	virtual bool Build (std::list<gcu::Object*>& Children) throw (std::invalid_argument);
	virtual void Transform2D (gcu::Matrix2D& m, double x, double y);
	virtual bool BuildContextualMenu (GtkUIManager *UIManager, gcu::Object *object, double x, double y);
	virtual bool OnSignal (gcu::SignalId Signal, gcu::Object *Child);
	virtual bool Load (xmlNodePtr);
	virtual double GetYAlign ();
};

}	//	namespace gcp

#endif	//GCHEMPAINT_REACTION_H
