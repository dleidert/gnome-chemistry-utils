// -*- C++ -*-

/*
 * GChemPaint library
 * reaction-separator.cc
 *
 * Copyright (C) 2013 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "document.h"
#include "reaction-substractor.h"
#include "settings.h"
#include "theme.h"
#include "view.h"
#include "widgetdata.h"
#include <gccv/canvas.h>
#include <gccv/text.h>
#include <glib/gi18n.h>
#include <cmath>
#include <cstring>

using namespace gcu;

namespace gcp {

TypeId ReactionSubstractorType = NoType;

ReactionSubstractor::ReactionSubstractor ():
	ReactionSeparator ("−", ReactionSubstractorType)
{
}

ReactionSubstractor::~ReactionSubstractor ()
{
}

std::string ReactionSubstractor::Name ()
{
	return _("Reaction substrator");
}

}	//	namespace gcp
