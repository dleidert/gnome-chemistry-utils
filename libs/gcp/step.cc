// -*- C++ -*-

/*
 * GChemPaint library
 * step.cc
 *
 * Copyright (C) 2011 Jean Bréfort <jean.brefort@normalesup.org>
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

#include "config.h"
#include "arrow.h"
#include "step.h"
#include <glib/gi18n-lib.h>

namespace gcp {

Step::Step (gcu::TypeId type): gcu::Object (type)
{
}

Step::~Step ()
{
}

void Step::AddArrow (Arrow *arrow, Step *step) throw (std::invalid_argument)
{
	if (m_Arrows.find (step) != m_Arrows.end ())
		throw std::invalid_argument (_("Only one arrow can link two given steps."));
	m_Arrows[step] = arrow;
}

void Step::RemoveArrow (Arrow *, Step *step)
{
	m_Arrows.erase (step);
}

}	//	namespace gcp
