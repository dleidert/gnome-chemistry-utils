// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * crystalviewer/crystalatom.cc 
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

#include "crystalatom.h"
#include "chemistry/element.h"
#include "chemistry/xml-utils.h"
#include <math.h>
#include <GL/glu.h>
#include <string>

using namespace gcu;

CrystalAtom::CrystalAtom(): Atom()
{
	m_dr = 0.0 ;
	m_bCustomColor = false;
	m_fRed = m_fBlue = m_fGreen = 0 ;
	m_fAlpha = 1 ;
	m_nCleave = 0 ;
}

CrystalAtom::~CrystalAtom()
{
}

CrystalAtom::CrystalAtom(int Z, double x, double y, double z): Atom(Z, x, y, z)
{
	SetDefaultColor();
	m_nCleave = 0;
}

CrystalAtom::CrystalAtom(CrystalAtom& caAtom): Atom(caAtom)
{
	m_dr = caAtom.m_dr ;
	m_bCustomColor = caAtom.m_bCustomColor;
	m_fRed = caAtom.m_fRed ;
	m_fGreen = caAtom.m_fGreen ;
	m_fBlue = caAtom.m_fBlue ;
	m_fAlpha = caAtom.m_fAlpha ;
	m_nCleave = 0 ;
}

CrystalAtom& CrystalAtom::operator=(CrystalAtom& caAtom)
{
	SetZ(caAtom.GetZ());
	double x, y, z;
	caAtom.GetCoords(&x, &y, &z);
	SetCoords(x, y, z);
	m_dr = caAtom.m_dr ;
	m_bCustomColor = caAtom.m_bCustomColor;
	m_fRed = caAtom.m_fRed ;
	m_fGreen = caAtom.m_fGreen ;
	m_fBlue = caAtom.m_fBlue ;
	m_fAlpha = caAtom.m_fAlpha ;
	m_nCleave = 0 ;
	return *this ;
}

void CrystalAtom::Draw()
{
	if (m_nCleave) return ;
	GLUquadricObj *quadObj ;
	glPushMatrix() ;
	glTranslated(y(), z(), x()) ;
	glColor4f(m_fRed, m_fGreen, m_fBlue, m_fAlpha) ;
	quadObj = gluNewQuadric() ;
    gluQuadricDrawStyle(quadObj, GL_FILL);
	gluQuadricNormals(quadObj, GL_SMOOTH) ;
	gluSphere(quadObj, m_dr, 20, 10) ;
	gluDeleteQuadric(quadObj) ;
	glPopMatrix() ;
}

void CrystalAtom::SetColor(float red, float green, float blue, float alpha)
{
	m_bCustomColor = true;
	m_fRed = red ;
	m_fGreen = green ;
	m_fBlue = blue ;
	m_fAlpha = alpha ;
}

void CrystalAtom::SetDefaultColor()
{
	m_bCustomColor = false;
	double *Colors = Element::GetElement(GetZ())->GetDefaultColor();
	m_fRed = (float) Colors[0];
	m_fGreen = (float) Colors[1];
	m_fBlue = (float) Colors[2];
}

void CrystalAtom::GetColor(double *red, double *green, double *blue, double *alpha)
{
	*red = m_fRed ;
	*green = m_fGreen ;
	*blue = m_fBlue ;
	*alpha = m_fAlpha ;
}

void CrystalAtom::SetSize(double r)
{
	m_dr = r ;
}


double CrystalAtom::GetSize()
{
	return m_dr ;
}

bool CrystalAtom::operator==(CrystalAtom& caAtom)
{
	return (x() == caAtom.x()) &&
			(y() == y()) &&
			(z() == z()) ;
}

double CrystalAtom::ScalProd(int h, int k, int l)
{
	return x() * h + y() * k + z() * l ;
}

double CrystalAtom::Distance(double dx, double dy, double dz, bool bFixed)
{
	if ((m_nCleave > 0) && ! bFixed) return 0 ;
	dx -= x() ;
	dy -= y() ;
	dz -= z() ;
	return sqrt(dx * dx + dy * dy + dz * dz) + m_dr ;
}

void CrystalAtom::NetToCartesian(double a, double b, double c, double alpha, double beta, double gamma)
{
	double dx = x() * a ;
	double dy = y() * b ;
	double dz = z() * c ;
	SetCoords(dx * sqrt(1-square(cos(beta)) - square((cos(gamma) - cos(beta)*cos(alpha))/sin(alpha))),
		dx * (cos(gamma) - cos(beta)*cos(alpha))/sin(alpha) + dy * sin(alpha),
		(dx * cos(beta) + dy * cos(alpha) + dz));
}

bool CrystalAtom::SaveNode(xmlDocPtr xml, xmlNodePtr node)
{
	xmlNodePtr parent, child;
	gchar buf[256];
	
	g_snprintf(buf, sizeof(buf) - 1, "%g", m_dr);
	child = xmlNewDocNode(xml, NULL, (xmlChar*)"radius", (xmlChar*)buf);
	if (child) xmlAddChild(node, child);
	else return false;
	
	if (m_bCustomColor && !WriteColor(xml, node, NULL, m_fRed, m_fGreen, m_fBlue, m_fAlpha)) return false;
	
	return true;
}

bool CrystalAtom::LoadNode(xmlNodePtr node)
{
	char* txt;
	xmlNodePtr child = FindNodeByNameAndId(node, "color");
	if (!child) SetDefaultColor();
	else
	{
		if (!ReadColor(node, NULL, &m_fRed, &m_fGreen, &m_fBlue, &m_fAlpha)) return false;
		m_bCustomColor = true;
	}
	child = FindNodeByNameAndId(node, "radius");
	if (!child || !(txt = (char*)xmlNodeGetContent(child)) || (sscanf(txt, "%lg", &m_dr),m_dr == 0)) return false;
	return true;
}

