// -*- C++ -*-

/* 
 * Gnome Crystal
 * bond.h 
 *
 * Copyright (C) 2000-2004 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCRYSTAL_LINE_H
#define GCRYSTAL_LINE_H

#include <libxml/parser.h>
#ifdef GCU_OLD_VER
#	include <crystalviewer/crystalline.h>
#else
#	include <gcu/crystalline.h>
#endif

using namespace gcu;

class gcLine: public CrystalLine  
{
public:
	gcLine();
	virtual ~gcLine();

public :
	bool LoadOld(xmlNodePtr node, unsigned version);
};


#endif // GCRYSTAL_LINE_H
