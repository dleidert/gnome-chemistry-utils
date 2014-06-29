// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gcp/tolman-step.h
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

#ifndef GCP_TOLMAN_STEP_H
#define GCP_TOLMAN_STEP_H

#include <gcp/mechanism-step.h>

namespace gcp {

extern gcu::TypeId TolmanStepType;

/*!\class TolmanStep gcp/tolman-step.h
\brief the reactant between two arrows inside a Tolman loop.

The TolmanStep class embeds a molecule (peferably) or some text representing a
molecule (although no check is performed) inside a Tolman loop.
*/
class TolmanStep: public MechanismStep
{
public:
/*!
The default constructor.
*/
	TolmanStep ();
/*!
The destructor.
*/
	virtual ~TolmanStep ();
};

}   //  namespace gcc

#endif  //  GCP_TOLMAN_STEP_H
