// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * canvas/fill-item.cc
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
#include "line-item.h"

namespace gccv {

LineItem::LineItem (Canvas *canvas):
	Item (canvas),
	m_LineWidth (1.),
	m_LineColor (RGBA_WHITE)
{
}

LineItem::LineItem (Group *parent, ItemClient *client):
	Item (parent, client),
	m_LineWidth (1.),
	m_LineColor (RGBA_WHITE)
{
}

LineItem::~LineItem ()
{
}

}