// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * crystalline.h 
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

#ifndef CRYSTAL_LINE_H
#define CRYSTAL_LINE_H

#include <libxml/parser.h>
#include <list>

using namespace std;

namespace gcu
{
	
enum CrystalLineType
{
	edges = 0,
	diagonals,
	medians,
	normal,
	unique
};

class CrystalLine
{
public:
	CrystalLine();
	virtual ~CrystalLine();

public :
	CrystalLine(CrystalLineType Type, double X1, double Y1, double Z1, double X2, double Y2, double Z2, double r, float red, float green, float blue, float alpha);
	CrystalLine(CrystalLine& cbBond);
	CrystalLine& operator=(CrystalLine&);

	void Draw();
	double X1(void) {return m_dx;}
	double Y1(void) {return m_dy;}
	double Z1(void) {return m_dz;}
	double X2(void) {return m_dx2;}
	double Y2(void) {return m_dy2;}
	double Z2(void) {return m_dz2;}
	double Xmax();
	double Ymax();
	double Zmax();
	double Xmin();
	double Ymin();
	double Zmin();
	double Long() {return m_dl;}
	CrystalLineType Type() {return m_nType;}
	void SetPosition(double x, double y, double z, double x1, double y1, double z1);
	void SetColor(float red, float green, float blue, float alpha);
	void GetColor(double *red, double *green, double *blue, double *alpha);
	void SetRadius(double r);
	double GetRadius() {return m_dr;};
	bool operator==(CrystalLine&);
	virtual void Move(double x, double y, double z);
	double ScalProd(int h, int k, int l);
	void Cleave() {m_nCleave++;}
	void NetToCartesian(double a, double b, double c, double alpha, double beta, double gamma);
	double Distance(double x, double y, double z, bool bFixed);
	bool IsCleaved() {return m_nCleave != 0;}
	void GetRotation(double& x, double& y, double& z, double& th);
	virtual xmlNodePtr Save(xmlDocPtr xml);
	virtual bool Load(xmlNodePtr node);
	
protected :
	float m_fBlue, m_fRed, m_fGreen, m_fAlpha;
	double m_dx, m_dy, m_dz, m_dx2, m_dy2, m_dz2, m_dr, m_dl;
	double m_dxrot, m_dyrot, m_darot;//rotation axis coordinates (z = 0) and angle
	int m_nCleave; //0 if not cleaved
	CrystalLineType m_nType;
};

typedef list<CrystalLine*> CrystalLineList;

}// namespace gcu

#endif // CRYSTAL_BOND_H
