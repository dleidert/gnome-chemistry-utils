// -*- C++ -*-

/* 
 * Gnome Crystal
 * element.cc 
 *
 * Copyright (C) 2000-2002
 *
 * Developed by Jean Bréfort <jean.brefort@ac-dijon.fr>
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

#include "element.h"
#include <string.h>

std::vector<gcElement*> Elt(119);
std::map<std::string, int> EltsMap;

gcElement::gcElement(unsigned char Z, const char* Symbol)
{
	m_Z = Z;
	strncpy(m_Symbol, Symbol, 3);
	m_Symbol[3] = 0;
}

gcElement::~gcElement()
{
}
