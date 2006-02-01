// -*- C++ -*-

/* 
 * Gnome Crystal
 * element.h 
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

#ifndef GCRYSTAL_ELEMENT_H
#define GCRYSTAL_ELEMENT_H

#include <map>
#include <string>
#include <vector>

class gcElement
{
public:
	gcElement(unsigned char Z, const char* Symbol);
	~gcElement();
	
	unsigned char Z() {return m_Z;}
	const char* Symbol() {return m_Symbol;}
	
private:
	unsigned char m_Z;
	char m_Symbol[4];
};

extern std::vector<gcElement*> Elt;
extern std::map<std::string, int> EltsMap;

#endif //GCRYSTAL_ELEMENT_H
