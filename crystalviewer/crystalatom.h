// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * crystalviewer/crystalatom.h 
 *
 * Copyright (C) 2002-2003
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

#ifndef CRYSTAL_ATOM_H
#define CRYSTAL_ATOM_H

#include <list>
#include <libxml/parser.h>
#include <libxml/parserInternals.h>
#include <libxml/xmlmemory.h>
#include <chemistry/atom.h>

using namespace std;

namespace gcu
{

class CrystalAtom : public Atom
{
public:
	CrystalAtom();
	virtual ~CrystalAtom();

public :
	CrystalAtom(int Z, double x, double y, double z);
	CrystalAtom(CrystalAtom& caAtom);
	CrystalAtom& operator=(CrystalAtom&);

	void Draw();
	void SetColor(float red, float green, float blue, float alpha);
	void SetDefaultColor();
	bool HasCustomColor() {return m_bCustomColor;}
	void GetColor(double *red, double *green, double *blue, double *alpha);
	void SetSize(double r);
	double GetSize();
	bool operator==(CrystalAtom&);
	void Cleave() {m_nCleave++;}
	double ScalProd(int h, int k, int l);
	void NetToCartesian(double a, double b, double c, double alpha, double beta, double gamma);
	double Distance(double x, double y, double z, bool bFixed);
	double r() {return m_dr;}
	bool IsCleaved() {return m_nCleave != 0;}
	virtual bool SaveNode(xmlDocPtr, xmlNodePtr);
	virtual bool LoadNode(xmlNodePtr node);
	
protected:
	float m_fBlue, m_fRed, m_fGreen, m_fAlpha;
	bool m_bCustomColor;
	double m_dr;
	int m_nCleave; //0 if not cleaved
};

typedef list<CrystalAtom*> CrystalAtomList;

}// namespace gcu

#endif // CRYSTAL_ATOM_H
