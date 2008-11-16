// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gccv/arrow.cc
 *
 * Copyright (C) 2008 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "canvas.h"
#include "group.h"
#include "arrow.h"
#include <cmath>

namespace gccv {

Arrow::Arrow (Canvas *canvas, double xstart, double ystart, double xend, double yend):
	Line (canvas, xstart, ystart, xend, yend)
{
}

Arrow::Arrow (Group *parent, double xstart, double ystart, double xend, double yend, ItemClient *client):
	Line (parent, xstart, ystart, xend, yend, client)
{
}

Arrow::~Arrow ()
{
}

double Arrow::Distance (double x, double y, Item **item) const
{
	return G_MAXDOUBLE; //FIXME
}

void Arrow::Draw (cairo_t *cr, bool is_vector) const
{
}

void Arrow::UpdateBounds ()
{
}

void Arrow::Move (double x, double y)
{
}

}
