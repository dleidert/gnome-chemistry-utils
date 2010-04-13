// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gccv/client.cc 
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
#include "client.h"

namespace gccv {

Client::Client ():
	m_Canvas (NULL)
{
}

Client::~Client ()
{
}

bool Client::OnButtonPressed (G_GNUC_UNUSED ItemClient *client, G_GNUC_UNUSED unsigned button, G_GNUC_UNUSED double x, G_GNUC_UNUSED double y, G_GNUC_UNUSED unsigned state)
{
	return true;
}

bool Client::OnButtonReleased (G_GNUC_UNUSED ItemClient *client, G_GNUC_UNUSED unsigned button, G_GNUC_UNUSED double x, G_GNUC_UNUSED double y, G_GNUC_UNUSED unsigned state)
{
	return true;
}

bool Client::OnDrag (G_GNUC_UNUSED ItemClient *client, G_GNUC_UNUSED double x, G_GNUC_UNUSED double y, G_GNUC_UNUSED unsigned state)
{
	return true;
}

bool Client::OnMotion (G_GNUC_UNUSED ItemClient *client, G_GNUC_UNUSED double x, G_GNUC_UNUSED double y, G_GNUC_UNUSED unsigned state)
{
	return true;
}

bool Client::OnLeaveNotify (G_GNUC_UNUSED unsigned state)
{
	return true;
}

}
