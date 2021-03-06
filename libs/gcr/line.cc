// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gcr/line.cc
 *
 * Copyright (C) 2002-2011 Jean Bréfort <jean.brefort@normalesup.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the
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

#include "config.h"
#include "line.h"
#include <gcu/xml-utils.h>
#include <glib.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <cmath>
#include <cstring>

#define __max(x,y)  ((x) > (y)) ? (x) : (y)
#define __min(x,y)  ((x) < (y)) ? (x) : (y)
#define square(x)	((x)*(x))

namespace gcr {

char const *LineTypeName[] = {
	"edges",
	"diagonals",
	"medians",
	"normal",
	"unique"
};

Line::Line ()
{
	m_dx = m_dy = m_dz = m_dx2 = m_dy2 = m_dz2 = m_dl = m_dr = 0.0;
	m_fRed = m_fBlue = m_fGreen = 0.0;
	m_fAlpha = 1.0;
	m_dxrot = m_dyrot = m_darot = 0;
	m_nType = edges;
	m_nCleave = 0;
}

Line::~Line ()
{
}


Line::Line (LineType Type, double X1, double Y1, double Z1, double X2, double Y2, double Z2, double r, float red, float green, float blue, float alpha)
{
	m_nCleave = 0 ;
	m_nType = Type;
	SetPosition (X1, Y1, Z1, X2, Y2, Z2) ;
	m_dr = r ;
	SetColor (red, green, blue, alpha) ;
}

Line::Line (Line& clLine)
{
	m_nCleave = 0 ;
	m_dx = clLine.m_dx ;
	m_dy = clLine.m_dy ;
	m_dz = clLine.m_dz ;
	m_dx2 = clLine.m_dx2 ;
	m_dy2 = clLine.m_dy2 ;
	m_dz2 = clLine.m_dz2 ;
	m_dxrot = clLine.m_dxrot ;
	m_dyrot = clLine.m_dyrot ;
	m_darot = clLine.m_darot ;
	m_dr = clLine.m_dr ;
	m_dl = clLine.m_dl ;
	m_fRed = clLine.m_fRed ;
	m_fGreen = clLine.m_fGreen ;
	m_fBlue = clLine.m_fBlue ;
	m_fAlpha = clLine.m_fAlpha ;
	m_nType = clLine.m_nType ;
}

Line& Line::operator= (Line& clLine)
{
	m_dx = clLine.m_dx ;
	m_dy = clLine.m_dy ;
	m_dz = clLine.m_dz ;
	m_dx2 = clLine.m_dx2 ;
	m_dy2 = clLine.m_dy2 ;
	m_dz2 = clLine.m_dz2 ;
	m_dxrot = clLine.m_dxrot ;
	m_dyrot = clLine.m_dyrot ;
	m_darot = clLine.m_darot ;
	m_dr = clLine.m_dr ;
	m_dl = clLine.m_dl ;
	m_fRed = clLine.m_fRed ;
	m_fGreen = clLine.m_fGreen ;
	m_fBlue = clLine.m_fBlue ;
	m_fAlpha = clLine.m_fAlpha ;
	m_nType = clLine.m_nType ;
	return *this ;
}

void Line::SetPosition (double x, double y, double z, double x1, double y1, double z1)
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
	m_dl = sqrt (square(x1) + square(y1) + square(z1)) ;
	//normalization
	x = sqrt (square(y1) + square(z1)) ;
	//vectorial product
	if (x > 0) {
		m_dxrot = - z1 / x ;
		m_dyrot = y1 / x ;
		m_darot = atan2 (x, x1) * 180. / M_PI ;
	} else {
		m_dxrot = 0;
		if (x1 > 0)
			m_dyrot = m_darot = 0.0;
		else {
			m_dyrot = 1.0;
			m_darot = 180.0;
		}
	}
}

void Line::SetColor (float red, float green, float blue, float alpha)
{
	m_fRed = red;
	m_fGreen = green;
	m_fBlue = blue;
	m_fAlpha = alpha;
}

void Line::SetColor (GdkRGBA rgba)
{
	m_fRed = rgba.red;
	m_fGreen = rgba.green;
	m_fBlue = rgba.blue;
	m_fAlpha = rgba.alpha;
}

void Line::GetColor (double *red, double *green, double *blue, double *alpha)
{
	*red = m_fRed;
	*green = m_fGreen;
	*blue = m_fBlue;
	*alpha = m_fAlpha;
}

void Line::GetColor (GdkRGBA &rgba)
{
	rgba.red = m_fRed;
	rgba.green = m_fGreen;
	rgba.blue = m_fBlue;
	rgba.alpha = m_fAlpha;
}

void Line::SetRadius (double r)
{
	m_dr = r ;
}

bool Line::operator== (Line& clLine)
{
	if (m_nType <= 2)
		return m_nType == clLine.m_nType ;
	return (m_dx == clLine.m_dx) &&
			(m_dy == clLine.m_dy) &&
			(m_dz == clLine.m_dz) &&
			(m_dx2 == clLine.m_dx2) &&
			(m_dy2 == clLine.m_dy2) &&
			(m_dz2 == clLine.m_dz2) &&
			(m_nType == clLine.m_nType) ;
}

void Line::Move (double x, double y, double z)
{
	m_dx += x ;
	m_dy += y ;
	m_dz += z ;
	m_dx2 += x ;
	m_dy2 += y ;
	m_dz2 += z ;
}

double Line::ScalProd (int h, int k, int l)
{
	return __max(m_dx * h + m_dy * k + m_dz * l, X2() * h + Y2() * k + Z2() * l) ;
}


double Line::Distance (double x, double y, double z, bool bFixe)
{
	if ((m_nCleave > 0) && !bFixe)
		return 0 ;
	return __max (	sqrt (square (m_dx - x) + square (m_dy - y) + square (m_dz - z)),
					sqrt (square (X2() - x) + square (Y2() - y) + square (Z2() - z))) ;
}

void Line::NetToCartesian (double a, double b, double c, double alpha, double beta, double gamma)
{
	double x = m_dx * a ;
	double y = m_dy * b ;
	double z = m_dz * c ;
	double x2 = X2 () * a ;
	double y2 = Y2 () * b ;
	double z2 = Z2 () * c ;
	SetPosition (x * sqrt (1 - square (cos (beta)) - square ((cos (gamma) - cos (beta) * cos (alpha)) / sin(alpha))),
				x * (cos (gamma) - cos (beta) * cos (alpha)) / sin (alpha) + y * sin (alpha),
				(x * cos (beta) + y * cos (alpha) + z),
				x2 * sqrt (1-square (cos (beta)) - square ((cos (gamma) - cos (beta) * cos (alpha)) / sin (alpha))),
				x2 * (cos (gamma) - cos (beta) * cos (alpha)) / sin (alpha) + y2 * sin (alpha),
				(x2 * cos (beta) + y2 * cos (alpha) + z2)) ;
}

double Line::Xmax ()
{
	return __max (m_dx, m_dx2) ;
}

double Line::Ymax ()
{
	return __max (m_dy, m_dy2) ;
}

double Line::Zmax ()
{
	return __max (m_dz, m_dz2) ;
}

double Line::Xmin ()
{
	return __min (m_dx, m_dx2) ;
}

double Line::Ymin ()
{
	return __min (m_dy, m_dy2) ;
}

double Line::Zmin ()
{
	return __min(m_dz, m_dz2) ;
}

void Line::GetRotation (double & x, double & y, double & z, double & theta)
{
	x = m_dy - m_dy2;
	y = m_dx2 - m_dx;
	double d = sqrt (square (x) + square (y));
	if (d > 1e-3) {
		theta = atan2(d, m_dz2 - m_dz);
		x /= d;
		y /= d;
		z = 0;
	} else {
		z = 1.0;
		theta = 0;
	}
}

xmlNodePtr Line::Save (xmlDocPtr xml) const
{
	xmlNodePtr parent, child;
	char buf[256];
	parent = xmlNewDocNode (xml, NULL, (xmlChar*) "line", NULL);
	if (!parent)
		return NULL;

	xmlSetProp (parent, (xmlChar*) "type", (xmlChar*) LineTypeName[m_nType]);

	g_snprintf (buf, sizeof (buf) - 1, "%g", m_dr);
	child = xmlNewDocNode (xml, NULL, (xmlChar*) "radius", (xmlChar*) buf);
	if (child)
		xmlAddChild (parent, child);
	else {
		xmlFreeNode (parent);
		return NULL;
	}

	if (((m_nType > 2) && (((!gcu::WritePosition (xml, parent, "start", m_dx, m_dy, m_dz))) ||
		(!gcu::WritePosition (xml, parent, "end", m_dx2, m_dy2, m_dz2)))) ||
		(!gcu::WriteColor (xml, parent, NULL, m_fRed, m_fGreen, m_fBlue, m_fAlpha))) {
			xmlFreeNode (parent);
			return NULL;
		}

	return parent;
}

bool Line::Load (xmlNodePtr node)
{
	char *txt;
	txt = (char*) xmlGetProp (node, (xmlChar*) "type");
	if (!txt)
		return false;
	int i = 0;
	while ((i < 5) && strcmp (txt, LineTypeName[i]))
		i++;
	xmlFree (txt);
	if (i < 5)
		m_nType = (LineType) i;
	else
		return false;
	if (((m_nType > 2) && ((!gcu::ReadPosition (node, "start", &m_dx, &m_dy, &m_dz)) ||
		(!gcu::ReadPosition (node, "end", &m_dx2, &m_dy2, &m_dz2)))) ||
		(!gcu::ReadColor (node, NULL, &m_fRed, &m_fGreen, &m_fBlue, &m_fAlpha)))
			return false;
	xmlNodePtr child = node->children;
	while (child) {
		if (!strcmp ((char *) child->name, "radius")) {
			txt = (char*) xmlNodeGetContent (child);
			sscanf (txt, "%lg", &m_dr);
			xmlFree (txt);
			break;
		}
		child = child->next;
	}
	if (m_dr == 0)
		return false;
	return true;
}

}	//	namespace gcr
