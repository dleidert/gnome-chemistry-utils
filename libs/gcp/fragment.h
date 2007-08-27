// -*- C++ -*-

/* 
 * GChemPaint library
 * fragment.h 
 *
 * Copyright (C) 2002-2007 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_FRAGMENT_H
#define GCHEMPAINT_FRAGMENT_H

#include "text-object.h"
#include "libgnomecanvas/gnome-canvas.h"

namespace gcp {

class FragmentAtom;
class Atom;

class Fragment: public TextObject
{
friend class FragmentTool;
public:
	Fragment ();
	Fragment (double x, double y);
	virtual ~Fragment ();

	void SetSelected (GtkWidget *w, int state);
	void Add (GtkWidget *w);
	void Update (GtkWidget *w);
	xmlNodePtr Save (xmlDocPtr xml);
	xmlNodePtr SaveSelection (xmlDocPtr xml);
	bool Load (xmlNodePtr);
	bool OnChanged (bool save);
	void AnalContent ();
	void AnalContent (unsigned start, unsigned &end);
	Object* GetAtomAt (double x, double y, double z = 0.);
	void Move (double x, double y, double z = 0);
	void Transform2D (Matrix2D& m, double x, double y);
	void OnChangeAtom ();
	Atom* GetAtom () {return (Atom*) m_Atom;}
	int GetElementAtPos (unsigned start, unsigned &end);
	int GetChargePosition (FragmentAtom *pAtom, unsigned char &Pos, double Angle, double &x, double &y);
	int GetAvailablePosition (double &x, double &y);
	bool GetPosition (double angle, double &x, double &y);
	bool Validate ();
	double GetYAlign ();

private:
	bool SavePortion (xmlDocPtr xml, xmlNodePtr node, unsigned start, unsigned end);

private:
	FragmentAtom *m_Atom;
	unsigned m_BeginAtom, m_EndAtom;
	int m_lbearing;
	double m_CHeight;
};

}	//	namespace gcp

#endif	//GCHEMPAINT_FRAGMENT_H