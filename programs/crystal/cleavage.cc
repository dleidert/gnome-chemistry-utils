// -*- C++ -*-

/* 
 * Gnome Crystal
 * cleavage.cc 
 *
 * Copyright (C) 2001-2004 Jean Bréfort <jean.brefort@normalesup.org>
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include "config.h"
#include "gcrystal.h"
#include "cleavage.h"
#include <glib.h>

gcCleavage::gcCleavage(): CrystalCleavage()
{
}

gcCleavage::~gcCleavage()
{
}

bool gcCleavage::LoadOld(xmlNodePtr node)
{
	char *txt;
	txt = (char*)xmlGetProp(node, (xmlChar*)"indices");
	if (!txt) return false;
	if (sscanf(txt, "%d %d %d", &m_nh, &m_nk, &m_nl) != 3)
	{
		xmlFree(txt);
		return false;
	}
	xmlFree(txt);
	txt = (char*)xmlGetProp(node, (xmlChar*)"planes");
	if (!txt) return false;
	if (sscanf(txt, "%d", &m_nPlanes)!= 1)
	{
		xmlFree(txt);
		return false;
	}
	xmlFree(txt);
	return true;
}
