// -*- C++ -*-

/* 
 * GChemPaint library
 * mesomery-arrow.h 
 *
 * Copyright (C) 2004-2007 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_MESOMERY_ARROW_H
#define GCHEMPAINT_MESOMERY_ARROW_H

#include "arrow.h"

namespace gcp {

class Mesomery;
class Mesomer;

class MesomeryArrow: public Arrow
{
public:
	MesomeryArrow (Mesomery* mesomery);
	virtual ~MesomeryArrow ();
	
	virtual xmlNodePtr Save (xmlDocPtr xml) const;
	virtual bool Load (xmlNodePtr);
	virtual void Add (GtkWidget* w) const;
	virtual void Update (GtkWidget* w) const;
	void SetStartMesomer (Mesomer *Mesomer) {m_Start = Mesomer;}
	Mesomer* GetStartMesomer () {return m_Start;}
	void SetEndMesomer (Mesomer *Mesomer) {m_End = Mesomer;}
	Mesomer* GetEndMesomer () {return m_End;}
	void RemoveMesomer (Mesomer *Mesomer);
	void Reverse ();

private:
	Mesomer *m_Start, *m_End;
};

}	//	namespace gcp

#endif	//	GCHEMPAINT_MESOMERY_ARROW_H
