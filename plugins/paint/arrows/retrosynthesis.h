// -*- C++ -*-

/*
 * GChemPaint arrows plugin
 * retrosynthesis.h
 *
 * Copyright (C) 2004-2011 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_RETROSYNTHESIS_H
#define GCHEMPAINT_RETROSYNTHESIS_H

#include <gcu/object.h>

using namespace gcu;

extern TypeId RetrosynthesisType;

class gcpRetrosynthesisStep;

class gcpRetrosynthesis: public Object
{
public:
	gcpRetrosynthesis ();
	virtual ~gcpRetrosynthesis ();

	virtual xmlNodePtr Save (xmlDocPtr xml) const;
	virtual bool Load (xmlNodePtr);
	virtual bool Build (std::set < Object * > const &Children) throw (std::invalid_argument);
	virtual double GetYAlign ();
	virtual bool BuildContextualMenu (gcu::UIManager *UIManager, Object *object, double x, double y);
	virtual bool OnSignal (SignalId Signal, Object *Child);
	virtual void Transform2D (Matrix2D& m, double x, double y);
	int Validate (bool split);
	void Align ();
	std::string Name();

private:
	gcpRetrosynthesis (Object* parent, gcpRetrosynthesisStep *step);

private:
	gcpRetrosynthesisStep *Target;
};

#endif	// GCHEMPAINT_RETROSYNTHESIS_H
