// -*- C++ -*-

/*
 * GChemPaint library
 * step.cc
 *
 * Copyright (C) 2011 Jean Bréfort <jean.brefort@normalesup.org>
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

#include "config.h"
#include "arrow.h"
#include "step.h"

namespace gcp {

Step::Step (gcu::TypeId type): gcu::Object (type)
{
}

Step::~Step ()
{
}


void Step::RemoveArrow (Arrow *arrow) {
	m_Arrows.erase (arrow);
	if (m_Arrows.empty ()) {
		// if there is no more arrows this is no more a step
		delete this;
	}
}

}	//	namespace gcp
