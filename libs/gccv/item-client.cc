// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gccv/item-client.cc
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
#include "item.h"
#include "item-client.h"

namespace gccv {

ItemClient::ItemClient ():
	m_Item (NULL)
{
}

ItemClient::~ItemClient ()
{
	if (m_Item)
		delete m_Item;
}

void ItemClient::AddItem ()
{
}

void ItemClient::UpdateItem ()
{
	if (m_Item) {
		delete m_Item;
		m_Item = NULL;
	}
	AddItem ();
}

void ItemClient::SetSelected (G_GNUC_UNUSED int state)
{
}

}
