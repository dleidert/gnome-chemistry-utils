// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * crystalbond.cc 
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

#include "crystalline.h"
#include "chemistry/xml-utils.h"
#include <math.h>
#include <glib.h>
#include <GL/gl.h>
#include <GL/glu.h>

#define __max(x,y)  ((x) > (y)) ? (x) : (y)
#define __min(x,y)  ((x) < (y)) ? (x) : (y)
#define square(x)	((x)*(x))

using namespace gcu;

static char *TypeName[] = {"edges",
		"diagonals",
		"medians",
		"normal",
		"unique"};

CrystalLine::CrystalLine()
{
	m_dx = m_dy = m_dz = m_dx2 = m_dy2 = m_dz2 = m_dl = m_dr = 0.0;
	m_fRed = m_fBlue = m_fGreen = 0.0;
	m_fAlpha = 1.0;
	m_dxrot = m_dyrot = m_darot = 0;
	m_nType = edges;
	m_nCleave = 0;
}

CrystalLine::~CrystalLine()
{
}


CrystalLine::CrystalLine(CrystalLineType Type, double X1, double Y1, double Z1, double X2, double Y2, double Z2, double r, float red, float green, float blue, float alpha)
{
	m_nCleave = 0 ;
	m_nType = Type;
	SetPosition(X1, Y1, Z1, X2, Y2, Z2) ;
	m_dr = r ;
	SetColor(red, green, blue, alpha) ;
}

CrystalLine::CrystalLine(CrystalLine& cbBond)
{
	m_nCleave = 0 ;
	m_dx = cbBond.m_dx ;
	m_dy = cbBond.m_dy ;
	m_dz = cbBond.m_dz ;
	m_dx2 = cbBond.m_dx2 ;
	m_dy2 = cbBond.m_dy2 ;
	m_dz2 = cbBond.m_dz2 ;
	m_dxrot = cbBond.m_dxrot ;
	m_dyrot = cbBond.m_dyrot ;
	m_darot = cbBond.m_darot ;
	m_dr = cbBond.m_dr ;
	m_dl = cbBond.m_dl ;
	m_fRed = cbBond.m_fRed ;
	m_fGreen = cbBond.m_fGreen ;
	m_fBlue = cbBond.m_fBlue ;
	m_fAlpha = cbBond.m_fAlpha ;
	m_nType = cbBond.m_nType ;
}

CrystalLine& CrystalLine::operator=(CrystalLine& cbBond)
{
	m_dx = cbBond.m_dx ;
	m_dy = cbBond.m_dy ;
	m_dz = cbBond.m_dz ;
	m_dx2 = cbBond.m_dx2 ;
	m_dy2 = cbBond.m_dy2 ;
	m_dz2 = cbBond.m_dz2 ;
	m_dxrot = cbBond.m_dxrot ;
	m_dyrot = cbBond.m_dyrot ;
	m_darot = cbBond.m_darot ;
	m_dr = cbBond.m_dr ;
	m_dl = cbBond.m_dl ;
	m_fRed = cbBond.m_fRed ;
	m_fGreen = cbBond.m_fGreen ;
	m_fBlue = cbBond.m_fBlue ;
	m_fAlpha = cbBond.m_fAlpha ;
	m_nType = cbBond.m_nType ;
	return *this ;
}

void CrystalLine::Draw()
{
	if (m_nCleave) return ;
	GLUquadricObj *quadObj ;
	glPushMatrix() ;
	glTranslated(m_dy, m_dz, m_dx) ;
	glRotated(m_darot, m_dxrot, m_dyrot, 0.0f);
	glColor4f(m_fRed, m_fGreen, m_fBlue, m_fAlpha) ;
	quadObj = gluNewQuadric() ;
    gluQuadricDrawStyle(quadObj, GL_FILL);
	gluQuadricNormals(quadObj, GL_SMOOTH) ;
	gluCylinder(quadObj,m_dr, m_dr, m_dl, 20, 10);
	gluDeleteQuadric(quadObj) ;
	glPopMatrix() ;
}

void CrystalLine::SetPosition(double x, double y, double z, double x1, double y1, double z1)
{
	m_dx = x ;
	m_dy = y ;
	m_dz = z ;
	m_dx2 = x1 ;
	m_dy2 = y1 ;
	m_dz2 = z1 ;
	//vector coordinates
	x1 -= x ;
	y1 -= y ;
	z1 -= z ;
	m_dl = sqrt(square(x1) + square(y1) + square(z1)) ;
	//normalization
	x = sqrt(square(y1) + square(z1)) ;
	//vectorial product
	if (x > 0)
	{
		m_dxrot = - z1 / x ;
		m_dyrot = y1 / x ;
		m_darot = atan2(x, x1) * 90 / 1.570796326794897 ;
	}
	else m_dxrot = m_dyrot = m_darot = 0 ;
}

void CrystalLine::SetColor(float red, float green, float blue, float alpha)
{
	m_fRed = red ;
	m_fGreen = green ;
	m_fBlue = blue ;
	m_fAlpha = alpha ;
}

void CrystalLine::GetColor(double *red, double *green, double *blue, double *alpha)
{
	*red = m_fRed ;
	*green = m_fGreen ;
	*blue = m_fBlue ;
	*alpha = m_fAlpha ;
}

void CrystalLine::SetRadius(double r)
{
	m_dr = r ;
}

bool CrystalLine::operator==(CrystalLine& cbBond)
{
	if (m_nType <= 2) return m_nType == cbBond.m_nType ; 
	return (m_dx == cbBond.m_dx) &&
			(m_dy == cbBond.m_dy) &&
			(m_dz == cbBond.m_dz) &&
			(m_dx2 == cbBond.m_dx2) &&
			(m_dy2 == cbBond.m_dy2) &&
			(m_dz2 == cbBond.m_dz2) &&
			(m_nType == cbBond.m_nType) ; 
}

void CrystalLine::Move(double x, double y, double z)
{
	m_dx += x ;
	m_dy += y ;
	m_dz += z ;
	m_dx2 += x ;
	m_dy2 += y ;
	m_dz2 += z ;
}

double CrystalLine::ScalProd(int h, int k, int l)
{
	return __max(m_dx * h + m_dy * k + m_dz * l, X2() * h + Y2() * k + Z2() * l) ;
}


double CrystalLine::Distance(double x, double y, double z, bool bFixe)
{
	if ((m_nCleave > 0) && !bFixe) return 0 ;
	return __max(	sqrt(square(m_dx - x) + square(m_dy - y) + square(m_dz - z)),
					sqrt(square(X2() - x) + square(Y2() - y) + square(Z2() - z))) ;
}

void CrystalLine::NetToCartesian(double a, double b, double c, double alpha, double beta, double gamma)
{
	double x = m_dx * a ;
	double y = m_dy * b ;
	double z = m_dz * c ;
	double x2 = X2() * a ;
	double y2 = Y2() * b ;
	double z2 = Z2() * c ;
	SetPosition(x * sqrt(1-square(cos(beta)) - square((cos(gamma) - cos(beta)*cos(alpha))/sin(alpha))),
				x * (cos(gamma) - cos(beta)*cos(alpha))/sin(alpha) + y * sin(alpha),
				(x * cos(beta) + y * cos(alpha) + z),
				x2 * sqrt(1-square(cos(beta)) - square((cos(gamma) - cos(beta)*cos(alpha))/sin(alpha))),
				x2 * (cos(gamma) - cos(beta)*cos(alpha))/sin(alpha) + y2 * sin(alpha),
				(x2 * cos(beta) + y2 * cos(alpha) + z2)) ;
}

double CrystalLine::Xmax()
{
	return __max(m_dx, m_dx2) ;
}

double CrystalLine::Ymax()
{
	return __max(m_dy, m_dy2) ;
}

double CrystalLine::Zmax()
{
	return __max(m_dz, m_dz2) ;
}

double CrystalLine::Xmin()
{
	return __min(m_dx, m_dx2) ;
}

double CrystalLine::Ymin()
{
	return __min(m_dy, m_dy2) ;
}

double CrystalLine::Zmin()
{
	return __min(m_dz, m_dz2) ;
}

void CrystalLine::GetRotation(double & x, double & y, double & z, double & theta)
{
	x = m_dy - m_dy2;
	y = m_dx2 - m_dx;
	double d = sqrt(square(x) + square(y));
	if (d > 1e-3)
	{
		theta = atan2(d, m_dz2 - m_dz);
//		theta = acos((m_dz2 - m_dz) / Long());
		x /= d;
		y /= d;
		z = 0;
	}
	else
	{
		z = 1.0;
		theta = 0;
	}
}

xmlNodePtr CrystalLine::Save(xmlDocPtr xml)
{
	xmlNodePtr parent, child;
	gchar buf[256];
	parent = xmlNewDocNode(xml, NULL, (xmlChar*)"line", NULL);
	if (!parent) return NULL;

	xmlSetProp(parent, (xmlChar*)"type", (xmlChar*)TypeName[m_nType]);
	
	g_snprintf(buf, sizeof(buf) - 1, "%g", m_dr);
	child = xmlNewDocNode(xml, NULL, (xmlChar*)"radius", (xmlChar*)buf);
	if (child) xmlAddChild(parent, child);
	else {xmlFreeNode(parent); return NULL;}
	
	if (((m_nType > 2) && ((!WritePosition(xml, parent, "start", m_dx, m_dy, m_dz))) ||
		(!WritePosition(xml, parent, "end", m_dx2, m_dy2, m_dz2))) || 
		(!WriteColor(xml, parent, NULL, m_fRed, m_fGreen, m_fBlue, m_fAlpha)))
			{xmlFreeNode(parent); return NULL;}
	
	return parent;
}

bool CrystalLine::Load(xmlNodePtr node)
{
	char *txt;
	txt = (char*)xmlGetProp(node, (xmlChar*)"type");
	int i = 0;
	while (strcmp(txt, TypeName[i]) && (i < 5)) i++;
	if (i < 5) m_nType = (CrystalLineType)i;
	else return false;
	if (((m_nType > 2) && ((!ReadPosition(node, "start", &m_dx, &m_dy, &m_dz)) ||
		(!ReadPosition(node, "end", &m_dx2, &m_dy2, &m_dz2)))) ||
		(!ReadColor(node, NULL, &m_fRed, &m_fGreen, &m_fBlue, &m_fAlpha)))
			return false;
	xmlNodePtr child = node->children;
	while(child)
	{
		if (!strcmp((gchar*)child->name, "radius"))
		{
			txt = (char*)xmlNodeGetContent(child);
			sscanf(txt, "%lg", &m_dr);
			break;
		}
		child = child->next;
	}
	if (m_dr == 0) return false;
	return true;
}
