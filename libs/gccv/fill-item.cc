// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gccv/fill-item.cc
 *
 * Copyright (C) 2008 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "canvas.h"
#include "fill-item.h"

namespace gccv {

FillItem::FillItem (Canvas *canvas):
	LineItem (canvas),
	m_FillColor (GO_COLOR_WHITE),
	m_AutoFillColor (false)
{
}

FillItem::FillItem (Group *parent, ItemClient *client):
	LineItem (parent, client),
	m_FillColor (GO_COLOR_WHITE),
	m_AutoFillColor (false)
{
}

FillItem::~FillItem ()
{
}

GOColor FillItem::GetEffectiveFillColor () const
{
	return m_AutoFillColor? GetCanvas ()->GetColor () ^0xffffff00: m_FillColor;
}

}
