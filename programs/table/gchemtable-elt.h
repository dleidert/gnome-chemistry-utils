// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * programs/gchemtable-elt.cc 
 *
 * Copyright (C) 2005
 *
 * Developed by Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMTABLE_ELT_H
#define GCHEMTABLE_ELT_H

#include <gcu/dialog.h>

using namespace gcu;

class GChemTableApp;
	
class GChemTableElt: public Dialog
{
public:
	GChemTableElt (GChemTableApp *App, int Z);
	virtual ~GChemTableElt ();

	void OnFocusIn ();

private:
	int m_Z;
};

#endif	// GCHEMTABLE_ELT_H