// -*- C++ -*-

/* 
 * Gnome Chemisty Utils
 * crystalviewer/crystalcleavage.h 
 *
 * Copyright (C) 2002
 *
 * Developed by Jean Bréfort <jean.brefort@ac-dijon.fr>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA  02111-1307, USA.
 */

#ifndef CRYSTAL_CLEAVAGE_H
#define CRYSTAL_CLEAVAGE_H

#include <libxml/parser.h>
#include <list>

namespace gcu
{

class CrystalCleavage
{
public:
	CrystalCleavage();
	CrystalCleavage(CrystalCleavage& ccClivage) ;
	virtual ~CrystalCleavage();

	int& Planes() {return m_nPlanes ;}
	int &h() {return m_nh ;}
	int &k() {return m_nk ;}
	int &l() {return m_nl ;}
	CrystalCleavage& operator=(CrystalCleavage&) ;
	bool operator==(CrystalCleavage&) ;
	xmlNodePtr Save(xmlDocPtr xml);
	bool Load(xmlNodePtr node);
	
protected:
	int m_nh, m_nk, m_nl, m_nPlanes ;
};

typedef std::list<CrystalCleavage*> CrystalCleavageList;
	
} //namespace gcu

#endif //CRYSTAL_CLEAVAGE_H
