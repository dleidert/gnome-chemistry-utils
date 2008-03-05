// -*- C++ -*-

/* 
 * GChemPaint arrows plugin
 * retrosynthesisstep.h 
 *
 * Copyright (C) 2005-2007 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_RETROSYNTHESIS_STEP_H
#define GCHEMPAINT_RETROSYNTHESIS_STEP_H

#include <gcu/object.h>

using namespace std;
using namespace gcu;

extern TypeId RetrosynthesisStepType;

class gcpRetrosynthesis;
class gcpRetrosynthesisArrow;

namespace gcp {
	class Molecule;
}

class gcpRetrosynthesisStep: public Object
{
public:
	gcpRetrosynthesisStep ();
	virtual ~gcpRetrosynthesisStep ();
	
	gcpRetrosynthesisStep (gcpRetrosynthesis *synthesis, gcp::Molecule *molecule) throw (std::invalid_argument);
	virtual double GetYAlign ();
	virtual bool Load(xmlNodePtr);
	virtual bool OnSignal (SignalId Signal, Object *Child);
	void AddArrow (gcpRetrosynthesisArrow *arrow, gcpRetrosynthesisStep *step, bool start) throw (std::invalid_argument);
	bool Validate () {return Arrow != NULL || Arrows.size () > 0;}
	map<gcpRetrosynthesisStep *, gcpRetrosynthesisArrow *> *GetArrows () {return &Arrows;}
	gcpRetrosynthesisArrow const *GetArrow () {return Arrow;}
	void RemoveArrow (gcpRetrosynthesisArrow *arrow, gcpRetrosynthesisStep *step);

private:
	gcp::Molecule *Molecule;
	gcpRetrosynthesisArrow *Arrow;
	gcpRetrosynthesisStep *Precursor;
	map<gcpRetrosynthesisStep *, gcpRetrosynthesisArrow *> Arrows;
};

#endif	// GCHEMPAINT_RETROSYNTHESIS_STEP_H